#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <memoryManager.h>
#include <stdbool.h>
#include <stdint.h>

#define MAX_NAME_LENGTH 60

#define KILL_EXIT_CODE 1

typedef enum { READY, RUNNING, BLOCKED, EXITED, WAITING_FOR_EXIT, BLOCKED_BY_USER } State;
extern const char* const StateStrings[];

typedef struct {
  int write;
  int read;
  int err;
} ProcessPipes;

#define PROCESS_HEAP_ORDER_COUNT 17
#define PROCESS_HEAP_SIZE (1 << (PROCESS_HEAP_ORDER_COUNT - 1))

typedef struct PCB {
  uint32_t pid;
  uint8_t priority;
  State state;
  void* rsp;
  void* rbp;
  char* name;
  int waitedProcessExitCode;
  struct PCB* waitingForMe[10]; // This should be of dynamic length
  struct PCB* parentProc;
  int wfmLen;
  void* stack;
  ProcessPipes pipes;
#ifdef BUDDY
  Block* freeList[PROCESS_HEAP_ORDER_COUNT];
  void* heap;
  bool heapFreed;
#endif
} PCB;

typedef struct {
  uint32_t pid;
  uint8_t priority;
  const char* state;
  void* rsp;
  void* rbp;
  char name[MAX_NAME_LENGTH + 1];
  char* location;
} PCBForUserland;

// void freePCBNode(PCBNode* node);
void initializePCBList();
void* schedule(void* rsp);
uint32_t createUserProcess(int argc, const char* argv[], void* processRip);
uint32_t createUserProcessWithPipeSwap(int argc, const char* argv[], void* processRip, ProcessPipes pipes);
extern void exit(int exitCode);
void startFirstProcess(void* processAddress);
void exitProcessByPCB(PCB* pcb, int exitCode);
void exitCurrentProcess(int exitCode);
PCB* getPCBByPid(uint32_t pid);
int waitPid(uint32_t pid);
PCBForUserland* getPCBList(int* len);
PCB* getCurrentPCB();
void blockCurrentProcess();
void readyProcess(const PCB* pcb);
uint32_t getpid();
bool kill(uint32_t pid);
void killCurrentProcessInForeground();
void changePriority(uint32_t pid, uint32_t newPriority);
void killCurrentProcess();
void changePipeRead(int p);
void changePipeWrite(int p);
bool block(uint32_t pid);
bool unBlock(uint32_t pid);
bool blockByUser(uint32_t pid);
ProcessPipes getPipes();
long read(int pipeId, char* buf, int len);
long write(int pipeId, const char* buf, int len);

#endif
