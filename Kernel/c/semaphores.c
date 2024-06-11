#include <array.h>
#include <pipes.h>
#include <scheduler.h>
#include <semaphores.h>
#include <utils.h>

// #define ERROR (-1)
#define INITIAL_CAPACITY 50

// semaphores_pos sem_array[MAX_SEMAPHORES];
uint32_t size;

static Array semArray;
static Array freedPositions;

sem_t findName(char* name);

void initializeSemaphores() {
  semArray = Array_initialize(sizeof(Semaphore), INITIAL_CAPACITY, NULL);
  freedPositions = Array_initialize(sizeof(int), INITIAL_CAPACITY, NULL);
}

sem_t addSem(char* name, uint32_t initialValue) {
  Semaphore sem = {.value = initialValue, .lock = 0, .process_first = NULL, .process_last = NULL, .destroyed = false};
  int i;
  for (i = 0; name[i] != 0 && i < MAX_SEM_NAME; ++i) {
    sem.name[i] = name[i];
  }
  if (name[i] != 0) return -1;
  name[i] = 0;
  int freeToUseSem;
  if (Array_popGetEle(freedPositions, &freeToUseSem)) {
    Array_set(semArray, freeToUseSem, &sem);
    return freeToUseSem;
  } else {
    return Array_push(semArray, &sem);
  }
}

sem_t semInit(uint32_t initialValue) {
  return addSem("", initialValue);
}

sem_t createSemaphore(char* name, uint32_t initialValue) {
  if (findName(name) >= 0) return -1;
  return addSem(name, initialValue);
}

bool fifoQueue(sem_t semId, PCB* process_by_pcb) {
  PCBNodeSem* process = globalMalloc(sizeof(PCBNodeSem));
  if (process == NULL) return false;

  process->pcb = process_by_pcb;
  process->next = NULL;
  Semaphore* sem = Array_get(semArray, semId);
  if (sem == NULL) return false;
  if (sem->process_first == NULL) {
    sem->process_first = process;
    sem->process_last = process;
  } else {
    sem->process_last->next = process;
    sem->process_last = process;
  }

  return true;
}

PCB* fifoUnqueue(sem_t semId) {
  Semaphore* sem = Array_get(semArray, semId);
  if (sem == NULL || sem->process_first == NULL) return false;
  PCB* process = sem->process_first->pcb;
  PCBNodeSem* temp = sem->process_first;
  sem->process_first = sem->process_first->next;
  globalFree(temp);
  return process;
}

bool destroySemaphore(sem_t semId) {
  if (semId < 0) return false;
  Semaphore* sem = Array_get(semArray, semId);
  if (sem == NULL || sem->destroyed) return false;
  _enter_region(&sem->lock);
  while (sem->process_first != NULL) {
    PCB* toReady = fifoUnqueue(semId);
    _leave_region(&sem->lock);
    if (toReady->state == BLOCKED) {
      readyProcess(toReady);
    } else if (toReady->state == WAITING_FOR_EXIT) {
      exitProcessByPCB(toReady, KILL_EXIT_CODE);
    }
  }
  _leave_region(&sem->lock);
  sem->destroyed = true;
  Array_push(freedPositions, &semId);
  return true;
}

bool destroySemaphoreByName(char* name) {
  return destroySemaphore(findName(name));
}

bool waitSemaphore(sem_t semId) {
  if (semId < 0) return false;
  Semaphore* sem = Array_get(semArray, semId);
  if (sem == NULL || sem->destroyed) return false;
  _enter_region(&sem->lock);
  if (sem->value > 0) {
    sem->value--;
    _leave_region(&sem->lock);
  } else {
    PCB* pcb = getCurrentPCB();
    fifoQueue(semId, pcb);
    _leave_region(&sem->lock);
    blockCurrentProcess();
  }
  return true;
}

bool decSemOnlyForKernel(int semId) {
  if (stdin < 0) return false;
  Semaphore* sem = Array_get(semArray, semId);
  if (sem->value > 0) sem->value--;
  return true;
}

bool postSemaphore(sem_t semId) {
  if (semId < 0) return false;
  Semaphore* sem = Array_get(semArray, semId);
  if (sem == NULL || sem->destroyed) return false;
  _enter_region(&sem->lock);
  if (sem->process_first != NULL) {
    PCB* toReady = fifoUnqueue(semId);
    _leave_region(&sem->lock);
    if (toReady->state == BLOCKED) {
      readyProcess(toReady);
    } else if (toReady->state == WAITING_FOR_EXIT) {
      exitProcessByPCB(toReady, KILL_EXIT_CODE);
    }
  } else {
    sem->value++;
    _leave_region(&sem->lock);
  }
  return true;
}

sem_t openSemaphore(char* name, uint32_t value) {
  sem_t semId = findName(name);
  if (semId < 0) semId = addSem(name, value);
  return semId;
}

sem_t findName(char* name) {
  int len = Array_getLen(semArray);
  for (int i = 0; i < len; ++i) {
    Semaphore* sem = Array_get(semArray, i);
    if (!sem->destroyed) {
      if (strcmp(name, sem->name) == 0) return i;
    }
  }
  return -1;
}
