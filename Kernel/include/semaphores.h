#ifndef SEMPAPHORES_SEMAPHORE_H
#define SEMPAPHORES_SEMAPHORE_H

#include <memory.h>
#include <scheduler.h>
#include <utils.h>

#define MAX_SEMAPHORES 40

// arreglo de semáforos (N=20 aprox) donde cada semáforo tiene una lista de procesos (doble encadenada por las dudas)
//  (N=5 como máx (idea de eficiencia, no me interesa acceder a la memoria en una lock, va a ser fifo))
// la idea es que los distintos procesos se manejen con los índices nada más.

extern int _enter_region(int32_t* lock);
extern int _leave_region(int32_t* lock);

typedef struct process_by_PCB {
  const PCB* process_pcb;
  struct process_by_PCB* next;
} process_by_PCB;

typedef struct semaphore {
  // nombre para identifiacarlo
  char* name;
  uint32_t value;
  int32_t lock;
  process_by_PCB* process_first;
  process_by_PCB* process_last;
} semaphore;
typedef struct semaphores_pos {
  semaphore* sem;
  uint32_t is_used;
} semaphores_pos;

int32_t semFinder(char* sem_name);
int32_t positionToInitSem();
void mySemBirth();
int32_t createSemaphore(char* name, uint32_t value);
int32_t destroySemaphore(char *sem_name);
int32_t postSemaphore(uint32_t sem_id);
int32_t waitSemaphore(uint32_t sem_id);
int32_t openSemaphore(char* name, uint32_t value);
#endif // SEMPAPHORES_SEMAPHORE_H
