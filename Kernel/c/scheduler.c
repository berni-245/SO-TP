#include <memoryManager.h>
#include <pipes.h>
#include <scheduler.h>
#include <stdbool.h>
#include <stdint.h>
#include <utils.h>
#include <videoDriver.h>

// El priority based scheduling funciona así:
// - Arranco desde el primer proceso de la pcbList.
// - Ejecuto ese proceso n cantidad de veces seguidas, donde n es la prioridad del proceso.
//   - Esto si se puede quizá convendría que sean 4 quantums seguidos sin context switching.
// - Luego de las n veces paso al siguiente proceso de la lista.

const char* const StateStrings[] = {"READY", "RUNNING", "BLOCKED", "EXITED", "W-EXIT", "BLK_USER"};

#define IDLE_PID (-1)

typedef struct PCBNode {
  PCB* pcb;
  struct PCBNode* next;
} PCBNode;

typedef struct {
  PCBNode* head;
  PCBNode* tail;
  PCBNode* current;
  PCBNode* prev;
  int32_t len;
} PCBList;

extern void* initializeProcessStack(int32_t argc, char* argv[], void* processRip, void* stackStart);
extern void idleProc();
extern void* userModule;
extern void asdfInterruption();

PCB* getPCBByPid(uint32_t pid);
void exitProcessByPCB(PCB* pcb, int32_t exitCode);

PCB* processInForeground;
PCBList pcbList;
PCBNode* idleProcPCBNode;

PCBNode* createPCBNode(
    uint32_t pid, uint8_t priority, State state, void* stack, void* rsp, void* rbp, char* name, ProcessPipes pipes
) {
  PCBNode* node = globalMalloc(sizeof(PCBNode));
  if (node == NULL) return NULL;
  node->next = NULL;

  PCB* pcb = globalMalloc(sizeof(PCB));
  if (pcb == NULL) {
    globalFree(node);
    return NULL;
  }
  pcb->pid = pid;
  pcb->priority = priority;
  pcb->state = state;
  pcb->stack = stack;
  pcb->rsp = rsp;
  pcb->rbp = rbp;
  pcb->name = name;
  pcb->parentProc = pcbList.current->pcb;
  pcb->wfmLen = 0;
  pcb->pipes = pipes;
  if ((int32_t)pid != IDLE_PID) {
    pcb->heap = globalMalloc(PROCESS_HEAP_SIZE);
    if (pcb->heap == NULL) {
      globalFree(node);
      globalFree(pcb);
      return NULL;
    }
#ifdef BUDDY
    freeListInit(pcb->heap, pcb->freeList);
#else
    pcb->freeListStart = globalMalloc(sizeof(Block));
    if (pcb->freeListStart == NULL) {
      globalFree(node);
      globalFree(pcb);
      globalFree(pcb->heap);
      return NULL;
    }
    freeListInit(pcb->heap, pcb->freeListStart, &(pcb->freeListEnd), &(pcb->bytesAvailable));
#endif
  } else pcb->heap = NULL;
  pcb->heapFreed = false;
  node->pcb = pcb;

  return node;
}

bool addPCB(uint32_t pid, void* stack, void* rsp, void* rbp, char* name, ProcessPipes pipes) {
  PCBNode* node = createPCBNode(pid, 1, READY, stack, rsp, rbp, name, pipes);
  if (node == NULL) return false;

  if (pcbList.head == NULL) {
    pcbList.head = node;
    pcbList.head->next = node;
    pcbList.tail = node;
  } else {
    if (pcbList.prev == pcbList.tail) pcbList.prev = node;
    node->next = pcbList.head;
    pcbList.tail->next = node;
    pcbList.tail = node;
  }

  ++pcbList.len;
  return true;
}

void freeCurrentProcess() {
  if (pcbList.current == NULL || pcbList.head == NULL) return;
  if (pcbList.head == pcbList.tail) return;

  PCBNode* toRemove = pcbList.current;

  pcbList.current = toRemove->next;
  pcbList.prev->next = pcbList.current;
  if (pcbList.head == toRemove) pcbList.head = pcbList.current;
  else if (pcbList.tail == toRemove) pcbList.tail = pcbList.prev;

  if (!toRemove->pcb->heapFreed) globalFree(toRemove->pcb->heap);
#ifndef BUDDY
  globalFree(toRemove->pcb->freeListStart);
#endif
  globalFree(toRemove->pcb->stack);
  globalFree(toRemove->pcb);
  globalFree(toRemove);

  --pcbList.len;
}

void nextPCB() {
  if (pcbList.head == NULL) return;
  if (pcbList.current == NULL) {
    pcbList.current = pcbList.head;
    pcbList.prev = pcbList.tail;
  } else if (pcbList.current == idleProcPCBNode) {
    pcbList.current = pcbList.prev->next;
  } else {
    pcbList.prev = pcbList.current;
    pcbList.current = pcbList.current->next;
  }
}

void initializePCBList() {
  void* stackStart;
  void* stackEnd;
  stackAlloc(&stackStart, &stackEnd);
  void* rsp = initializeProcessStack(0, NULL, idleProc, stackStart);
  ProcessPipes pipes = {.write = stdout, .read = stdin, .err = stderr};
  idleProcPCBNode = createPCBNode(IDLE_PID, 1, READY, stackEnd, rsp, rsp, "idle", pipes);
  pcbList.head = NULL;
  pcbList.tail = NULL;
  pcbList.current = NULL;
  pcbList.prev = NULL;
  pcbList.len = 0;
}

// Should only be called after creating userModule process, so head and current won't be NULL.
void* schedule(void* rsp) {
  static int32_t quantumsLeft = 0;

  pcbList.current->pcb->rsp = rsp;
  pcbList.current->pcb->rbp = *(void**)(rsp + 8 * 8);

  if (quantumsLeft > 0 && pcbList.current->pcb->state == RUNNING) {
    --quantumsLeft;
    return rsp;
  } else if (pcbList.current->pcb->state == RUNNING) {
    pcbList.current->pcb->state = READY;
  }

  if (pcbList.current == idleProcPCBNode) nextPCB();
  PCBNode* ogCurrent = pcbList.current;
  // We need to always go to next process first so we don't free current process
  // while we are inside its own stack.
  nextPCB();
  while (true) {
    if (pcbList.current->pcb->state == READY) {
      pcbList.current->pcb->state = RUNNING;
      quantumsLeft = pcbList.current->pcb->priority - 1;
      return pcbList.current->pcb->rsp;
    } else if (pcbList.current == ogCurrent) {
      // Note: this check also prevents the case where a process exits and all other processes
      // are blocked, which would otherwise also lead to freeing the current stack.
      pcbList.current = idleProcPCBNode;
      return idleProcPCBNode->pcb->rsp;
    } else if (pcbList.current->pcb->state == EXITED) {
      // Note that freeCurrentProcess changes current pcb so we need to avoid running
      // nextPCB() in this branch.
      freeCurrentProcess();
    } else {
      nextPCB();
    }
  }
}

/*
Process arguments (argv) will be passed as a pointer to a userland local
array (aka will be on the stack of the parent function/process), so it
could be deleted before the child process finishes using it. That's why
we need to copy it into the current process' stack.
I decided to add argv at the end/top of the stack, and the strings themselves
just bellow it. This way the process can use the stack from the bottom without
ever knowing about it (unless it reaches the end of the stack which should
never happen).

argc = 3

         |- stackEnd, argvStack
         v
0x503d8  00 00 00 00 00 05 03 f0 <-- argvStack[0]: char* to "Arg 1"
0x503e0  00 00 00 00 00 05 03 f6 <-- argvStack[1]: char* to "Arg 2"
0x503e8  00 00 00 00 00 05 03 fc <-- argvStack[2]: char* to "Arg argc"

         |- stackEnd + argc * sizeof(char*)
         |- argvStack[0]
         |                 |- 0x503f6
         |                 |- argvStack[1]
         v                 v
0x503f0  41 72 67 20 31 00 41 72 <-- "Arg 1", "Ar"

                     |- 0x503fc
                     |- argvStack[2]
                     v
0x503f8  67 20 32 00 41 72 67 20 <-- "g 2", "Arg "
0x50400  33 00 00 00 00 00 00 00 <-- "3"
0x50408  00 00 00 00 00 00 00 00
0x50410  00 00 00 00 00 00 00 00
0x50418  00 00 00 00 00 00 00 00
   .
   . stackStart
   .     v
0xXXXXX  00 00 00 00 00 00 00 00
 */
static uint32_t pid = 0;
void* createProcess(int32_t argc, const char* argv[], void* processRip, ProcessPipes pipes) {
  void* stackStart;
  void* stackEnd;
  stackAlloc(&stackStart, &stackEnd);
  char** argvStack = stackEnd;
  char* arg = stackEnd + argc * sizeof(char*);
  for (int32_t i = 0; i < argc; ++i) {
    argvStack[i] = arg;
    int32_t j = strncpy(arg, argv[i], MAX_NAME_LENGTH);
    arg = arg + j + 1;
  }
  void* rsp = initializeProcessStack(argc, argvStack, processRip, stackStart);
  if (!addPCB(pid++, stackEnd, rsp, stackStart, argvStack[0], pipes)) return NULL;
  return rsp;
}

void* createUserModuleProcess() {
  const char* argv[1] = {"init"};
  ProcessPipes pipes = {.write = stdout, .read = stdin, .err = stderr};
  void* rsp = createProcess(1, argv, userModule, pipes);
  if (rsp == NULL) return NULL;
  nextPCB();
  processInForeground = pcbList.current->pcb;
  pcbList.current->pcb->parentProc = NULL;
  return rsp;
}

int32_t createUserProcessWithPipeSwap(int32_t argc, const char* argv[], void* processRip, ProcessPipes pipes) {
  if (createProcess(argc, argv, processRip, pipes) == NULL) return -1;
  return pid - 1;
}

int32_t createUserProcess(int32_t argc, const char* argv[], void* processRip) {
  ProcessPipes pipes = {.write = stdout, .read = stdin, .err = stderr};
  if (createProcess(argc, argv, processRip, pipes) == NULL) return -1;
  return pid - 1;
}

void exitProcessByPCB(PCB* pcb, int32_t exitCode) {
  if (pcb->state == EXITED) return;
  if (pcb->state == BLOCKED) {
    pcb->state = WAITING_FOR_EXIT;
    globalFree(pcb->heap);
    pcb->heapFreed = true;
    return;
  }

  pcb->state = EXITED;
  if (pcb->pid == processInForeground->pid) processInForeground = pcb->parentProc;
  for (int32_t i = 0; i < pcb->wfmLen; ++i) {
    PCB* pcb2 = pcb->waitingForMe[i];
    if (pcb2->state == BLOCKED) {
      pcb2->state = READY;
      pcb2->waitedProcessExitCode = exitCode;
    } else if (pcb2->state == WAITING_FOR_EXIT) {
      exitProcessByPCB(pcb2, KILL_EXIT_CODE);
    }
  }
}

void exitCurrentProcess(int32_t exitCode) {
  exitProcessByPCB(pcbList.current->pcb, exitCode);
  asdfInterruption();
}

void exitCurrentProcessInForeground(int32_t exitCode) {
  exitProcessByPCB(processInForeground, exitCode);
  asdfInterruption();
}

PCB* getPCBByPid(uint32_t pid) {
  PCBNode* node = pcbList.head;
  // Note pcbList is orded by pid because new nodes are always added at the end and
  // pid is always increasing..
  do {
    if (node->pcb->pid == pid) return node->pcb;
    node = node->next;
  } while (node->pcb->pid <= pid && node != pcbList.head);
  return NULL;
}

int32_t waitPid(uint32_t pid) {
  if (pid == pcbList.current->pcb->pid) return pcbList.current->pcb->waitedProcessExitCode;

  PCB* pcb = getPCBByPid(pid);
  if (pcb == NULL || pcb->state == EXITED) return pcbList.current->pcb->waitedProcessExitCode;
  pcb->waitingForMe[pcb->wfmLen++] = pcbList.current->pcb;
  pcbList.current->pcb->state = BLOCKED;
  if (pcbList.current->pcb->pid == processInForeground->pid) processInForeground = pcb;
  asdfInterruption(); // Replace for int 0x20 when schedule gets called there.
  return pcbList.current->pcb->waitedProcessExitCode;
}

void copyPCBToPCBForUserland(PCBForUserland* userlandPcb, PCB* kernelPcb) {
  strcpy(userlandPcb->name, kernelPcb->name);
  userlandPcb->pid = kernelPcb->pid;
  userlandPcb->rsp = kernelPcb->rsp;
  userlandPcb->rbp = kernelPcb->rbp;
  userlandPcb->state = StateStrings[kernelPcb->state];
  userlandPcb->priority = kernelPcb->priority;
  userlandPcb->location = (kernelPcb->pid == processInForeground->pid ? "Fg" : "Bg");
}
PCBForUserland* getPCBList(int* len) {
  *len = pcbList.len;
  if (pcbList.head == NULL) return NULL;
  PCBForUserland* pcbArray = malloc(sizeof(PCBForUserland) * pcbList.len);
  if (pcbArray == NULL) return NULL;
  PCBNode* node = pcbList.head;
  for (int32_t i = 0; i < pcbList.len; ++i) {
    copyPCBToPCBForUserland(pcbArray + i, node->pcb);
    node = node->next;
  }
  return pcbArray;
}

PCB* getCurrentPCB() {
  return pcbList.current->pcb;
}

void blockCurrentProcess() {
  pcbList.current->pcb->state = BLOCKED;
  asdfInterruption();
}

void readyProcess(const PCB* pcb) {
  ((PCB*)pcb)->state = READY;
}

uint32_t getpid() {
  return pcbList.current->pcb->pid;
}

bool kill(uint32_t pid) {
  if (pid == 0) return false;
  PCB* pcb = getPCBByPid(pid);
  if (pcb == NULL || pcb->state == EXITED || pcb->state == WAITING_FOR_EXIT) return false;
  exitProcessByPCB(pcb, KILL_EXIT_CODE);
  return true;
}

void killCurrentProcessInForeground() {
  if (processInForeground->pid == 0) return;
  exitProcessByPCB(processInForeground, KILL_EXIT_CODE);
  asdfInterruption();
}

void changePriority(uint32_t pid, uint32_t newPriority) {
  PCB* pcb = getPCBByPid(pid);
  if (pcb != NULL) {
    pcb->priority = newPriority;
  }
}

void changePipeRead(int32_t p) {
  pcbList.current->pcb->pipes.read = p;
}

void changePipeWrite(int32_t p) {
  pcbList.current->pcb->pipes.write = p;
}

ProcessPipes getPipes() {
  return pcbList.current->pcb->pipes;
}

long read(int32_t pipeId, char* buf, int32_t len) {
  if (pipeId == stdin) return readStdin(buf, len);
  return readPipe(pipeId, buf, len);
}

long write(int32_t pipeId, const char* buf, int32_t len) {
  if (pipeId == stdout) {
    printNextBuf(buf, len);
    return len;
  }
  return writePipe(pipeId, buf, len);
}

bool blockByUser(uint32_t pid) {
  PCB* pcb = getPCBByPid(pid);
  if (pcb != NULL && (pcb->state == READY || pcb->state == RUNNING)) {
    pcb->state = BLOCKED_BY_USER;
    if (pcb->pid == pcbList.current->pcb->pid) asdfInterruption();
    return true;
  }
  return false;
}

bool unblock(uint32_t pid) {
  PCB* pcb = getPCBByPid(pid);
  if (pcb != NULL && pcb->state == BLOCKED_BY_USER) {
    pcb->state = READY;
    return true;
  }
  return false;
}
