#include <interruptions.h>
#include <memoryManager.h>
#include <scheduler.h>
#include <timer.h>

static double freq = 65536 / 3600.0; // interruptions/second
static unsigned long ticks = 0;

typedef struct SleptProcess {
  unsigned long ticksRemaining;
  PCB* pcb;
  struct SleptProcess* next;
} SleptProcess;

SleptProcess* first = NULL;

SleptProcess* removeFromSleepList(SleptProcess* prev, SleptProcess* current) {
  if (prev == NULL) {
    first = current->next;
  } else {
    prev->next = current->next;
  }

  SleptProcess* toFree = current;
  current = current->next;
  globalFree(toFree);
  return current;
}

void incTicks() {
  ticks++;
  SleptProcess* current = first;
  SleptProcess* prev = NULL;

  while (current != NULL) {
    if (current->pcb->state == BLOCKED) {
      (current->ticksRemaining)--;

      if (current->ticksRemaining == 0) {
        readyProcess(current->pcb);
        current = removeFromSleepList(prev, current);
      } else {
        prev = current;
        current = current->next;
      }
    } else if (current->pcb->state == WAITING_FOR_EXIT) {
      exitProcessByPCB(current->pcb, KILL_EXIT_CODE);
      current = removeFromSleepList(prev, current);
    }
  }
}

unsigned long getTicks() {
  return ticks;
}

unsigned long getMs() {
  return ticks * 1000 / freq;
}

unsigned long calcTicks(unsigned long ms) {
  return (ms / 1000.0) * freq;
}

void sleep(unsigned long ms) {
  unsigned long initialTicks = calcTicks(ms);
  if (initialTicks > 0) {
    SleptProcess* aux = globalMalloc(sizeof(*aux));
    aux->ticksRemaining = initialTicks;
    aux->pcb = getCurrentPCB();
    aux->next = first;
    first = aux;
    blockCurrentProcess();
  }
}
