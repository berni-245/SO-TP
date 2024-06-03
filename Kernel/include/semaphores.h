#ifndef SEMPAPHORES_SEMAPHORE_H
#define SEMPAPHORES_SEMAPHORE_H

#include "stdlib.h"
#include "semaphores.h"
#include "scheduler.h"
#include "utils.h"

#define MAX_SEMAPHORES 50


//arreglo de semáforos (N=20 aprox) donde cada semáforo tiene una lista de procesos (doble encadenada por las dudas)
// (N=5 como máx (idea de eficiencia, no me interesa acceder a la memoria en una lock, va a ser fifo))
//la idea es que los distintos procesos se manejen con los índices nada más. 

extern int _enter_region(int *lock);
extern int _leave_region(int *lock);

typedef struct process_by_PCB{
    const PCB* process_pcb;
    struct process_by_PCB* next;
    struct process_by_PCB* before;
}process_by_PCB;

typedef struct semaphore{
    //nombre para identifiacarlo
    char* name;
    int value;
    int lock;
    process_by_PCB * process_first;
    process_by_PCB * process_last;
} semaphore;
typedef struct semaphores_pos {
    semaphore *sem;
    int is_used;
} semaphores_pos;



int sem_finder(char* sem_name);
int position_to_init_sem();

int my_sem_birth();
int my_sem_init(char* sem_name, unsigned int value);
int my_sem_wait(int sem_id);
int my_sem_post(int sem_id);
int my_sem_open(char* name, int value);
int my_sem_close(int sem_id);


int createSemaphore(char* name, int value);
int destroySemaphore(char* name);
int postSemaphore(int sem_id);
int waitSemaphore(int sem_id);
#endif //SEMPAPHORES_SEMAPHORE_H
