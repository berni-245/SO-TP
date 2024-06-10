#include <array.h>
#include <memoryManager.h>
#include <pipes.h>
#include <semaphores.h>
#include <stdbool.h>
#include <stdint.h>
#include <utils.h>

#define INITIAL_CAPACITY 20

int pipes_amount;
// Pipe pipeArray[MAX_PIPES];

static Array pipeArray;
static Array freedPositions;

// Hacer a pipeArray un Array dinámico (traer Array lib para kernel tmbn)
// y tener un lista de free pipes indexes. Si la lista está vacía createPipe
// me pushea un nuevo pipe al array. Si hay alguna vacía la reusa (o reusa
// el espacio en el array at least, xq podemos hacer que se haga free en closePipe
// o que queden para siempre creadas y listo).

void freePipe(Pipe** p) {
  globalFree(*p);
}

void initializePipes() {
  pipeArray = Array_initialize(sizeof(Pipe*), INITIAL_CAPACITY, (FreeEleFn)freePipe);
  freedPositions = Array_initialize(sizeof(int), INITIAL_CAPACITY, NULL);
  pipeInit(); // stdout
  pipeInit(); // stdin
  pipeInit(); // stderr
}

long pipeInit() {
  Pipe* p = globalMalloc(sizeof(Pipe));
  p->mutex = semInit(1);
  p->writtenCount = semInit(0);
  p->emptyCount = semInit(BUFFER_SIZE);
  p->writeIdx = 0;
  p->readIdx = 0;
  p->destroyed = false;
  // p->readerPcb = NULL;
  // p->writerPcb = NULL;
  int freeToUse;
  if (Array_popGetEle(freedPositions, &freeToUse)) {
    Array_set(pipeArray, freeToUse, &p);
    return freeToUse;
  } else {
    return Array_push(pipeArray, &p);
  }
}

Pipe* getPipe(int pipeId) {
  if (pipeId < 0) return NULL;
  Pipe** pPtr = Array_get(pipeArray, pipeId);
  if (pPtr == NULL) return NULL;
  Pipe* p = *pPtr;
  if (p->destroyed) return NULL;
  return p;
}

long readPipe(int pipeId, char* buf, int len) {
  if (len <= 0) return 0;
  Pipe* p = getPipe(pipeId);
  if (p == NULL) return -1;
  long pos = 0;
  bool reachedEnd = false;
  do {
    waitSemaphore(p->writtenCount);
    waitSemaphore(p->mutex);
    buf[pos++] = p->buffer[p->readIdx];
    p->readIdx = (p->readIdx + 1) % BUFFER_SIZE;
    if (p->readIdx == p->writeIdx) reachedEnd = true;
    postSemaphore(p->mutex);
    postSemaphore(p->emptyCount);
  } while (pos < len && !reachedEnd);

  return pos;
}

long writePipe(int pipeId, const char* buf, int len) {
  if (pipeId == stdin) return -1;
  if (len <= 0) return 0;
  Pipe* p = getPipe(pipeId);
  if (p == NULL) return -1;
  long pos = 0;
  while (pos < len) {
    waitSemaphore(p->emptyCount);
    waitSemaphore(p->mutex);
    p->buffer[p->writeIdx] = buf[pos++];
    p->writeIdx = (p->writeIdx + 1) % BUFFER_SIZE;
    postSemaphore(p->mutex);
    postSemaphore(p->writtenCount);
  }
  return pos;
}

void writeStdin(char c) {
  Pipe* p = getPipe(stdin);
  long pos = 0;
  decSemOnlyForKernel(p->emptyCount);
  p->buffer[p->writeIdx] = c;
  p->writeIdx = (p->writeIdx + 1) % BUFFER_SIZE;
  postSemaphore(p->writtenCount);
}

bool destroyPipe(int pipeId) {
  if (0 <= pipeId && pipeId <= 2) return false;
  Pipe* p = getPipe(pipeId);
  if (p == NULL) return false;
  p->destroyed = true;
  Array_push(freedPositions, &pipeId);
  destroySemaphore(p->mutex);
  destroySemaphore(p->writtenCount);
  destroySemaphore(p->emptyCount);
  // Array_set will do the free of the pipe itself when it overrides this pisition.
  return true;
}
