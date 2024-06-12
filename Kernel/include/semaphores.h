#ifndef SEMPAPHORES_H
#define SEMPAPHORES_H

#include <memoryManager.h>
#include <stdbool.h>
#include <utils.h>

#define MAX_SEM_NAME 50

// arreglo de semáforos (N=20 aprox) donde cada semáforo tiene una lista de procesos (doble encadenada por las dudas)
//  (N=5 como máx (idea de eficiencia, no me interesa acceder a la memoria en una lock, va a ser fifo))
// la idea es que los distintos procesos se manejen con los índices nada más.

struct PCB;
typedef int32_t sem_t;

extern int _enter_region(int32_t* lock);
extern int _leave_region(int32_t* lock);

typedef struct PCBNodeSem {
  struct PCB* pcb;
  struct PCBNodeSem* next;
} PCBNodeSem;

typedef struct {
  uint32_t value;
  int32_t lock;
  bool destroyed;
  PCBNodeSem* pcbNodeHead;
  PCBNodeSem* pcbNodeTail;
  char name[MAX_SEM_NAME + 1];
} Semaphore;

void initializeSemaphores();
sem_t semInit(uint32_t initialValue);
sem_t createSemaphore(char* name, uint32_t initialValue);
bool destroySemaphore(sem_t semId);
bool destroySemaphoreByName(char* name);
bool waitSemaphore(sem_t semId);
bool postSemaphore(sem_t semId);
sem_t openSemaphore(char* name, uint32_t value);
bool decSemOnlyForKernel(int semId);

#endif
