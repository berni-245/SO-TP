#include <interruptions.h>
#include <memoryManager.h>
#include <scheduler.h>
#include <timer.h>

static double freq = 65536 / 3600.0; // interruptions/second
static uint64_t ticks = 0;

typedef struct SleptProcess {
  uint64_t ticksRemaining;
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

uint64_t getTicks() {
  return ticks;
}

uint64_t getMs() {
  return ticks * 1000 / freq;
}

uint64_t calcTicks(uint64_t ms) {
  return (ms / 1000.0) * freq;
}

void sleep(uint64_t ms) {
  uint64_t initialTicks = calcTicks(ms);
  if (initialTicks > 0) {
    SleptProcess* aux = globalMalloc(sizeof(*aux));
    aux->ticksRemaining = initialTicks;
    aux->pcb = getCurrentPCB();
    aux->next = first;
    first = aux;
    blockCurrentProcess();
  }
}
