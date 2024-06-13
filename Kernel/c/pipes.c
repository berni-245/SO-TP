#include <array.h>
#include <memoryManager.h>
#include <pipes.h>
#include <semaphores.h>
#include <stdbool.h>
#include <stdint.h>
#include <utils.h>

#define INITIAL_CAPACITY 20

static Array pipeArray;
static Array freedPositions;

void freePipe(Pipe** p) {
  globalFree(*p);
}

void initializePipes() {
  pipeArray = arrayInitialize(sizeof(Pipe*), INITIAL_CAPACITY, (FreeEleFn)freePipe);
  freedPositions = arrayInitialize(sizeof(int), INITIAL_CAPACITY, NULL);
  pipeInit(); // stdout
  pipeInit(); // stdin
  pipeInit(); // stderr
}

long pipeInit() {
  Pipe* p = globalMalloc(sizeof(Pipe));
  p->mutex = semInit(1);
  p->writtenCountSem = semInit(0);
  p->emptyCountSem = semInit(BUFFER_SIZE);
  p->writeIdx = 0;
  p->readIdx = 0;
  p->destroyed = false;
  // p->readerPcb = NULL;
  // p->writerPcb = NULL;
  int freeToUse;
  if (arrayPopGetEle(freedPositions, &freeToUse)) {
    arraySet(pipeArray, freeToUse, &p);
    return freeToUse;
  } else {
    return arrayPush(pipeArray, &p);
  }
}

Pipe* getPipe(int pipeId) {
  if (pipeId < 0) return NULL;
  Pipe** pPtr = arrayGet(pipeArray, pipeId);
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
    waitSemaphore(p->writtenCountSem);
    waitSemaphore(p->mutex);
    buf[pos++] = p->buffer[p->readIdx];
    p->readIdx = (p->readIdx + 1) % BUFFER_SIZE;
    if (p->readIdx == p->writeIdx) reachedEnd = true;
    postSemaphore(p->mutex);
    postSemaphore(p->emptyCountSem);
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
    waitSemaphore(p->emptyCountSem);
    waitSemaphore(p->mutex);
    p->buffer[p->writeIdx] = buf[pos++];
    p->writeIdx = (p->writeIdx + 1) % BUFFER_SIZE;
    postSemaphore(p->mutex);
    postSemaphore(p->writtenCountSem);
  }
  return pos;
}

void writeStdin(char c) {
  Pipe* p = getPipe(stdin);
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
  arrayPush(freedPositions, &pipeId);
  destroySemaphore(p->mutex);
  destroySemaphore(p->writtenCountSem);
  destroySemaphore(p->emptyCountSem);
  // arraySet will do the free of the pipe itself when it overrides this position.
  return true;
}
