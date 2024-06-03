#include "semaphores.h"
#include "scheduler.h"
#include "utils.h"


//Supongamos que un semáforo se comporta como una lista
//un arreglo con todos los elems inicializados y si estan
//siendo usados me importa que pasa con ellos, si no los
//dejo vacíos y los puedo ir ocupando y vaciando a medida
//que necesite


//Los semaforos no los puede gestionar cada proceso como tal
//porque tienen que ser accesibles por varios procesos entonces
//hay que usar nombre

//usando solamente indices puedo encapsular los semaforos de forma que solo se puedan acceder desde aca
//en cada llamada se debe usar el nombre del semaforo y a la vez el pid del proceso que lo llama





#define ERROR (-1)

semaphores_pos sem_array[MAX_SEMAPHORES];

/*-----------AUX. FUNCTIONS-------------------*/ //FUNCIIONES QUE NO ESTÁN EN LOS TEST
//Se fija si encuentra el valor del semaforo dentro de mi arreglo. Si falla -1

int my_sem_birth(){
    for (int i = 0; i < MAX_SEMAPHORES; ++i) {
        sem_array[i].is_used=0;
    }
    return 1;
}

int sem_finder(char *sem_name) {
    for (int i=0; i < MAX_SEMAPHORES; i++) {
        if (sem_array[i].is_used) {
            if (s_strcmp(sem_array[i].sem->name, sem_name) == 0)
                return i;
        }
    }
    return ERROR;
}

//Devuelve una posicion para iniciar un nuevo semaforo, si falla retorna -1
int position_to_init_sem() {
    for (int i = 0; i < MAX_SEMAPHORES; i++) {
        if (!sem_array[i].is_used) {
            return i;
        }
    }
    return ERROR;
}


int fifo_queue(int pos, const PCB* process_by_pcb) {
    process_by_PCB *process = malloc(sizeof(process_by_PCB));
    if (process==NULL){
        return ERROR;
    }
    process->process_pcb = process_by_pcb;
    if (sem_array[pos].sem->process_first == NULL) {
        sem_array[pos].sem->process_first = process;
    } else{
        process->next=NULL;
        process->before=sem_array[pos].sem->process_last;
        sem_array[pos].sem->process_last->next=process;
    }
    sem_array[pos].sem->process_last = process;
    return 0;
}


const PCB* fifo_unqueue(int pos) {
    if (sem_array[pos].sem->process_first == NULL)
        return NULL;
    const PCB* process = sem_array[pos].sem->process_first->process_pcb;
    process_by_PCB *temp = sem_array[pos].sem->process_first;
    if (sem_array[pos].sem->process_first->next == NULL) {
        sem_array[pos].sem->process_first = NULL;
        sem_array[pos].sem->process_last = NULL;
    } else {
        sem_array[pos].sem->process_first = sem_array[pos].sem->process_first->next;
        sem_array[pos].sem->process_first->before = NULL;
    }
    free(temp);
    return process;
}

int createSemaphore(char* name, int value ){
    return my_sem_init(name, value);
}
int destroySemaphore(char* name){
    return my_sem_close(sem_finder(name));
}
int postSemaphore(int sem_id){
    return my_sem_post(sem_id);
}
int waitSemaphore(int sem_id){
    return my_sem_wait(sem_id);
}
int my_sem_init(char *sem_name, unsigned int init_value) {
    if (sem_finder(sem_name)!=ERROR)
        return ERROR;
    int pos = position_to_init_sem();
    if (pos == ERROR) {
        return ERROR;
    }
    sem_array[pos].sem = malloc(sizeof(semaphore));
    if (sem_array[pos].sem==NULL){
        return ERROR;
    }
   sem_array[pos].sem->name = malloc(s_strlen(sem_name) + 1);
    if (sem_array[pos].sem->name == NULL) {
        free(sem_array[pos].sem);
        return ERROR;
    }
    strcpy(sem_array[pos].sem->name, sem_name);
    sem_array[pos].sem->value = init_value;
    sem_array[pos].sem->lock=0;
    sem_array[pos].is_used = 1;
    sem_array[pos].sem->process_first = NULL;
    sem_array[pos].sem->process_last = NULL;
    return pos;
}

/*--------------------------------------------*/




int my_sem_open(char *name, int value) {
    int sem_id = sem_finder(name);
    if (sem_id == ERROR) {
        sem_id = my_sem_init(name, value);
        if (sem_id == ERROR)
            return ERROR;
    }
    return sem_id;
}

int my_sem_close(int sem_id) {
    if (sem_id >= MAX_SEMAPHORES || sem_id < 0)
        return ERROR;
    _enter_region(&sem_array[sem_id].sem->lock);
    while (sem_array[sem_id].sem->process_first != NULL) {
        fifo_unqueue(sem_id);
    }
    _leave_region(&sem_array[sem_id].sem->lock);
    free(sem_array[sem_id].sem->name);
    free(sem_array[sem_id].sem);
    sem_array[sem_id].is_used = 0;
    return 0;
}
//Busca el semaforo, si no lo encuentra sale. Si lo encuentra le decrementa el valor (si es 0 lo manda a la cola del sem)
//y le dice al scheduler que lo bloquee
int my_sem_wait(int sem_id) {
    if (sem_id>=MAX_SEMAPHORES || sem_id <0)
        return ERROR;
    _enter_region(&sem_array[sem_id].sem->lock);
    if (sem_array[sem_id].sem->value > 0) {
        sem_array[sem_id].sem->value--;
        _leave_region(&sem_array[sem_id].sem->lock);
        return 0;
    }
    //lo tengo que encolar porque tiene que ponerse a esperar
    else {
        const PCB* process_pcb = getCurrentPCB();
        fifo_queue(sem_id, process_pcb);
        _leave_region(&sem_array[sem_id].sem->lock);
        blockCurrentProcess();
        return 0;
    }
}
//Busca el semaforo, si no lo encuentra sale. Si lo encuentra le aumenta el valor y se fija si hay algún elemento que se
//necesite desencolar. Si hay alguno le dice al scheduler que lo pase a ready
int my_sem_post(int sem_id) {
    if (sem_id>=MAX_SEMAPHORES || sem_id <0 || !sem_array[sem_id].is_used)
    _enter_region(&sem_array[sem_id].sem->lock);
    if (sem_array[sem_id].sem->process_first!=NULL) {
        PCB* to_ready=fifo_unqueue(sem_id);
        readyProcess(to_ready);
    }else sem_array[sem_id].sem->value++;
    _leave_region(&sem_array[sem_id].sem->lock);

    return 0;
}

