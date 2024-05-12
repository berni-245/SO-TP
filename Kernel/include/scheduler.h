#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <stdint.h>
#include <memory.h>

typedef enum { READY, RUNNING, BLOCKED, EXITED } State;

typedef struct {
  uint32_t pid;
  uint8_t priority;
  State state;
  void* rsp;

} PCB;

// void freePCBNode(PCBNode* node);
void initializePCBList();
void* schedule(void* rsp);
uint32_t createProcess(int argc, char* argv[], void* processRip);
extern void exit(int exitCode);

#endif
