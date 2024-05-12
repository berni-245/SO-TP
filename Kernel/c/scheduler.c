#include <scheduler.h>

// The priority based scheduling will work like this:
// - Arranco desde el primer proceso de la pcbList.
// - Ejecuto ese proceso n cantidad de veces seguidas, donde n es la prioridad del proceso.
//   - Esto si se puede quizá convendría que sean 4 quantums seguidos sin context switching.
// - Luego de las n veces paso al siguiente proceso de la lista.

typedef struct PCBNode {
  PCB* pcb;
  PCB* waitingForMe[10]; // This should be of dynamic length
  int wfmLen;
  struct PCBNode* next;
} PCBNode;

typedef struct {
  PCBNode* head;
  PCBNode* tail;
  PCBNode* current;
  PCBNode* prev;
  // int len;
} PCBList;

extern void* createProcessStack(int argc, char* argv[], void* processRip);
extern void idleProc();

static uint32_t pid = 0;
PCBList pcbList;
PCB idleProcPCB;

PCB* createPCB(uint32_t pid, uint8_t priority, State state, void* rsp) {
  PCB* pcb = malloc(sizeof(PCB));
  pcb->pid = pid;
  pcb->priority = priority;
  pcb->state = state;
  pcb->rsp = rsp;

  return pcb;
}

PCBNode* createPCBNode(uint32_t pid, uint8_t priority, State state, void* rsp) {
  PCBNode* node = malloc(sizeof(PCBNode));
  node->next = NULL;
  node->pcb = createPCB(pid, priority, state, rsp);
  node->wfmLen = 0;

  return node;
}

void freePCBNode(PCBNode* node) {
  free(node->pcb);
  free(node);
}

void initializePCBList() {
  void* rsp = createProcessStack(0, NULL, idleProc);
  idleProcPCB.pid = pid;
  idleProcPCB.priority = 0;
  idleProcPCB.state = READY;
  idleProcPCB.rsp = rsp;

  pcbList.head = NULL;
  pcbList.tail = NULL;
  pcbList.current = NULL;
  pcbList.prev = NULL;

  // pcbList.len = 0;
}

uint32_t addPCB(void* rsp) {
  PCBNode* node = createPCBNode(pid, 1, READY, rsp);

  if (pcbList.tail == NULL) {
    pcbList.head = node;
    pcbList.tail = node;
  } else {
    pcbList.tail->next = node;
    pcbList.tail = node;
  }

  return pid++;
  // ++pcbList.len;
  // return node->pcb;
}

void* schedule(void* rsp) {
  static int n = 0;

  if (pcbList.head == NULL) return idleProcPCB.rsp;

  if (pcbList.current == NULL) {
    pcbList.current = pcbList.head;
  }

  pcbList.current->pcb->rsp = rsp;

  if (n > 0 && pcbList.current->pcb->state == READY) {
    --n;
    return rsp;
  }

  while (pcbList.current != NULL && pcbList.current->pcb->state != READY) {
    pcbList.prev = pcbList.current;
    pcbList.current = pcbList.current->next;
  }

  if (pcbList.current != NULL) {
    n = pcbList.current->pcb->priority - 1;
    return pcbList.current->pcb->rsp;
  } else {
    return idleProcPCB.rsp;
  }
}

uint32_t createProcess(int argc, char* argv[], void* processRip) {
  void* rsp = createProcessStack(argc, argv, processRip);
  return addPCB(rsp);
}

void exitProcess(int exitCode) {
  // Do something with exitCode?

  for (int i = 0; i < pcbList.current->wfmLen; ++i) {
    PCB* pcb = pcbList.current->waitingForMe[i];
    pcb->state = READY;
  }

  // pcbList.current->pcb->state = EXITED;
  if (pcbList.current == pcbList.head) {
    pcbList.head = pcbList.head->next;
    freePCBNode(pcbList.current);
    pcbList.current = pcbList.head;
  } else {
    // This should never happen and should be removed after testing...
    // I'm using it so I get an exception in case this isn't working as it should.
    if (pcbList.prev == NULL) 1 / 0;

    pcbList.prev->next = pcbList.current->next;
    freePCBNode(pcbList.current);
    pcbList.current = pcbList.prev->next;
  }
}

void waitPid(uint32_t pid) {
  PCBNode* node = pcbList.head;
  // Note pcbList is orded by pid because new nodes are always added at the end and
  // pid is always increasing..
  while (node != NULL && node->pcb->pid <= pid) {
    if (node->pcb->pid == pid) {
      node->waitingForMe[node->wfmLen++] = pcbList.current->pcb;
      pcbList.current->pcb->state = BLOCKED;
      return;
    }
    node = node->next;
  }
  // If no process with the specified pid is found then current process is not blocked.
}
