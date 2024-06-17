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
  int32_t write;
  int32_t read;
  int32_t err;
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
  int32_t waitedProcessExitCode;
  struct PCB* waitingForMe[10]; // This should be of dynamic length
  struct PCB* parentProc;
  int32_t wfmLen;
  void* stack;
  ProcessPipes pipes;
  void* heap;
  bool heapFreed;
#ifdef BUDDY
  Block* freeList[PROCESS_HEAP_ORDER_COUNT];
#else
  Block* freeListStart;
  Block* freeListEnd;
  uint64_t bytesAvailable;
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

extern void contextSwitch();
// void freePCBNode(PCBNode* node);
void initializePCBList();
void* schedule(void* rsp);
int32_t createUserProcess(int32_t argc, const char* argv[], void* processRip);
int32_t createUserProcessWithPipeSwap(int32_t argc, const char* argv[], void* processRip, ProcessPipes pipes);
void startFirstProcess(void* processAddress);
void exitProcessByPCB(PCB* pcb, int32_t exitCode);
void exitCurrentProcess(int32_t exitCode);
PCB* getPCBByPid(uint32_t pid);
int32_t waitPid(uint32_t pid);
PCBForUserland* getPCBList(int* len);
PCB* getCurrentPCB();
void blockCurrentProcess();
void readyProcess(const PCB* pcb);
uint32_t getpid();
bool kill(uint32_t pid);
void killCurrentProcess();
void killCurrentProcessInForeground();
void changePriority(uint32_t pid, uint32_t newPriority);
void changePipeRead(int32_t p);
void changePipeWrite(int32_t p);
bool blockByUser(uint32_t pid);
bool unblock(uint32_t pid);
ProcessPipes getPipes();
int64_t read(int32_t pipeId, char* buf, int32_t len);
int64_t write(int32_t pipeId, const char* buf, int32_t len);
void yield();


#endif
