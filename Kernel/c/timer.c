#include <interruptions.h>
#include <timer.h>
#include <scheduler.h>
#include <scheduler.h>
#include <memoryManager.h>

static double freq = 65536 / 3600.0; // interruptions/second
static unsigned long ticks = 0;

SleptProcess* first = NULL;

typedef struct SleptProcess{
  unsigned long ticksRemaining;
  PCB* process;
  struct sleptProcess* next;
}SleptProcess;

void incTicks() {
    ticks++;
    SleptProcess* currentProcess = first;
    SleptProcess* previousProcess = NULL;

    while (currentProcess != NULL) {
        (currentProcess->ticksRemaining)--;

        if (currentProcess->ticksRemaining == 0) {
            readyProcess(currentProcess->process);

            if (previousProcess == NULL) {
                first = currentProcess->next;
            } else {
                previousProcess->next = currentProcess->next;
            }

            SleptProcess* toFree = currentProcess;
            currentProcess = currentProcess->next;
            free(toFree);
        } else {
            previousProcess = currentProcess;
            currentProcess = currentProcess->next;
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
  return (ms / 1000) * freq;
}

void sleep(unsigned long ms) {
  unsigned long initialTicks = calcTicks(ms);
  if(initialTicks > 0){
    SleptProcess * aux = malloc(sizeof(*aux));
    aux->ticksRemaining = initialTicks;
    aux->process = getCurrentPCB();
    aux->next = first;
    first = aux;
    blockCurrentProcess();
  }
}
