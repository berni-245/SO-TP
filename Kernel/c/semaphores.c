#include <scheduler.h>
#include <semaphores.h>
#include <utils.h>

#define ERROR (-1)

semaphores_pos sem_array[MAX_SEMAPHORES];
uint32_t size;


sem_t semFinder(char* sem_name) {
  if (!size) return ERROR;
  for (int i = 0; i < MAX_SEMAPHORES; i++) {
    if (sem_array[i].is_used) {
      if (strcmp(sem_array[i].sem->name, sem_name) == 0) return i;
    }
  }
  return ERROR;
}

sem_t positionToInitSem() {
  for (int i = 0; i < MAX_SEMAPHORES; i++) {
    if (!sem_array[i].is_used) {
      return i;
    }
  }
  return ERROR;
}

int32_t fifoQueue(sem_t pos, const PCB* process_by_pcb) {
  process_by_PCB* process = malloc(sizeof(process_by_PCB));
  if (process == NULL) {
    return ERROR;
  }
  process->process_pcb = process_by_pcb;
  process->next = NULL;
  if (sem_array[pos].sem->process_first == NULL) {
    sem_array[pos].sem->process_first = process;
    sem_array[pos].sem->process_last = process;
  } else {
    sem_array[pos].sem->process_last->next = process;
    sem_array[pos].sem->process_last = process;
  }
  return 0;
}

const PCB* fifoUnqueue(sem_t pos) {
  if (sem_array[pos].sem->process_first == NULL) return NULL;
  const PCB* process = sem_array[pos].sem->process_first->process_pcb;
  process_by_PCB* temp = sem_array[pos].sem->process_first;
  if (sem_array[pos].sem->process_first->next == NULL) {
    sem_array[pos].sem->process_first = NULL;
  } else {
    sem_array[pos].sem->process_first = sem_array[pos].sem->process_first->next;
  }
  free(temp);
  return process;
}
//To ensure semaphores are initialized to 0
void mySemBirth() {
  for (int i = 0; i < MAX_SEMAPHORES; ++i) {
    sem_array[i].is_used = 0;
  }
  size = 0;
}
//If semaphore with that name exists it returns the semaphore else it creates it
sem_t openSemaphore(char* name, uint32_t value) {
  int32_t sem_id = semFinder(name);
  if (sem_id == ERROR) {
    sem_id = createSemaphore(name, value);
    if (sem_id == ERROR) return ERROR;
  }
  return sem_id;
}
sem_t createSemaphore(char* sem_name, uint32_t init_value) {
  if (semFinder(sem_name) != ERROR) return ERROR;
  int pos = positionToInitSem();
  if (pos == ERROR) {
    return ERROR;
  }
  sem_array[pos].sem = malloc(sizeof(semaphore));
  if (sem_array[pos].sem == NULL) {
    return ERROR;
  }
  sem_array[pos].sem->name = malloc(strlen(sem_name) + 1);
  if (sem_array[pos].sem->name == NULL) {
    free(sem_array[pos].sem);
    return ERROR;
  }
  strcpy(sem_array[pos].sem->name, sem_name);
  sem_array[pos].sem->value = init_value;
  sem_array[pos].sem->lock = 0;
  sem_array[pos].is_used = 1;
  sem_array[pos].sem->process_first = NULL;
  sem_array[pos].sem->process_last = NULL;
  size++;
  return pos;
}
//Semaphores with processes left to unqueue can't be destroyed
int32_t destroySemaphore(char *sem_name) {
  int32_t sem_id = semFinder(sem_name);
  if (sem_id == ERROR) return ERROR;
  _enter_region(&sem_array[sem_id].sem->lock);
  while (sem_array[sem_id].sem->process_first != NULL) {
    const PCB* to_ready = fifoUnqueue(sem_id);
    _leave_region(&sem_array[sem_id].sem->lock);

    if (to_ready->state == BLOCKED) {
      readyProcess(to_ready);
    } else if (to_ready->state == WAITING_FOR_EXIT) {
      exitProcessByPCB(to_ready, KILL_EXIT_CODE);
    }
  }
  _leave_region(&sem_array[sem_id].sem->lock);
  free(sem_array[sem_id].sem->name);
  free(sem_array[sem_id].sem);
  sem_array[sem_id].is_used = 0;
  --size;
  return 0;
}
//If process can't decrement the semaphore the process enqueues itself
int32_t waitSemaphore(sem_t sem_id) {
  if (sem_id >= MAX_SEMAPHORES || !sem_array[sem_id].is_used) return ERROR;
  _enter_region(&sem_array[sem_id].sem->lock);
  if (sem_array[sem_id].sem->value > 0) {
    sem_array[sem_id].sem->value--;
    _leave_region(&sem_array[sem_id].sem->lock);
  }
  else {
    const PCB* process_pcb = getCurrentPCB();
    fifoQueue(sem_id, process_pcb);
    _leave_region(&sem_array[sem_id].sem->lock);
    blockCurrentProcess();
  }
  return 0;
}
//If process can increment the semaphore the process checks if there is a process to unqueue
int32_t postSemaphore(sem_t sem_id) {
  if (sem_id >= MAX_SEMAPHORES || !sem_array[sem_id].is_used) return ERROR;
  _enter_region(&sem_array[sem_id].sem->lock);
  if (sem_array[sem_id].sem->process_first != NULL) {
    const PCB* to_ready = fifoUnqueue(sem_id);
    _leave_region(&sem_array[sem_id].sem->lock);
    readyProcess(to_ready);
  } else {
    sem_array[sem_id].sem->value++;
    _leave_region(&sem_array[sem_id].sem->lock);
  }
  return 0;
}
