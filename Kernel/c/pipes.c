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

static Pipe stdinPipe;
static Pipe stderrPipe;

void initializePipes() {
  pipeArray = Array_initialize(sizeof(Pipe*), INITIAL_CAPACITY, (FreeEleFn)freePipe);
  freedPositions = Array_initialize(sizeof(int32_t), INITIAL_CAPACITY, NULL);
  // stdout --> Not used but I need to occupy this index anyways.
  pipeInit();
  stdinPipe = **(Pipe**)Array_get(pipeArray, pipeInit());
  stderrPipe = **(Pipe**)Array_get(pipeArray, pipeInit());
}

int64_t pipeInit() {
  Pipe* p = globalMalloc(sizeof(Pipe));
  p->mutex = semInit(1);
  p->writtenCountSem = semInit(0);
  p->emptyCountSem = semInit(BUFFER_SIZE);
  p->writeIdx = 0;
  p->readIdx = 0;
  p->destroyed = false;
  // p->readerPcb = NULL;
  // p->writerPcb = NULL;
  int32_t freeToUse;
  if (Array_popGetEle(freedPositions, &freeToUse)) {
    // I could use get and save having to allocate a new pipe.
    Array_set(pipeArray, freeToUse, &p);
    return freeToUse;
  } else {
    return Array_push(pipeArray, &p);
  }
}

Pipe* getPipe(int32_t pipeId) {
  if (pipeId < 0) return NULL;
  Pipe** pPtr = Array_get(pipeArray, pipeId);
  if (pPtr == NULL) return NULL;
  Pipe* p = *pPtr;
  if (p->destroyed) return NULL;
  return p;
}

int64_t readPipe(int32_t pipeId, char* buf, int32_t len) {
  if (len <= 0) return 0;
  Pipe* p = getPipe(pipeId);
  if (p == NULL) return -1;
  int64_t pos = 0;
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

int64_t writePipe(int32_t pipeId, const char* buf, int32_t len) {
  if (pipeId == stdin) return -1;
  if (len <= 0) return 0;
  Pipe* p = getPipe(pipeId);
  if (p == NULL) return -1;
  int64_t pos = 0;
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
  stdinPipe.buffer[stdinPipe.writeIdx] = c;
  stdinPipe.writeIdx = (stdinPipe.writeIdx + 1) % BUFFER_SIZE;
  postSemaphore(stdinPipe.writtenCountSem);
}

int64_t readStdin(char* buf, int32_t len) {
  if (len <= 0) return 0;
  int64_t pos = 0;
  bool reachedEnd = false;
  do {
    waitSemaphore(stdinPipe.writtenCountSem);
    waitSemaphore(stdinPipe.mutex);
    buf[pos++] = stdinPipe.buffer[stdinPipe.readIdx];
    stdinPipe.readIdx = (stdinPipe.readIdx + 1) % BUFFER_SIZE;
    if (stdinPipe.readIdx == stdinPipe.writeIdx) reachedEnd = true;
    postSemaphore(stdinPipe.mutex);
  } while (pos < len && !reachedEnd);

  return pos;
}

bool destroyPipe(int32_t pipeId) {
  if (0 <= pipeId && pipeId <= 2) return false;
  Pipe* p = getPipe(pipeId);
  if (p == NULL) return false;
  p->destroyed = true;
  Array_push(freedPositions, &pipeId);
  destroySemaphore(p->mutex);
  destroySemaphore(p->writtenCountSem);
  destroySemaphore(p->emptyCountSem);
  // Array_set will do the free of the pipe itself when it overrides this position.
  return true;
}
