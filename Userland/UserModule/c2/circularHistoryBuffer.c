

#include <array.h>
#include <circularHistoryBuffer.h>
#include <stdlib.h>
#include <syscalls.h>
#include <utils.h>

typedef struct CircularHistoryBufferADT {
  uint64_t size;
  uint64_t readIdx;
  uint64_t writeIdx;
  uint64_t toReadBackwards;
  Array array;
} CircularHistoryBufferADT;

void copynEleAt(CircularHistoryBuffer cb, uint64_t idx, const void* eleArr, uint64_t n);
void increaseWriteIdx(CircularHistoryBuffer cb);
void increaseReadIdx(CircularHistoryBuffer cb);
void decreaseReadIdx(CircularHistoryBuffer cb);

CircularHistoryBuffer CHB_initialize(uint64_t elementSize, uint64_t size, FreeEleFn freeEleFn, PrintEleFn printEleFn) {
  CircularHistoryBuffer cb = sysMalloc(sizeof(CircularHistoryBufferADT));
  if (cb == NULL) exitWithError("@CHB_initialize malloc error");
  cb->array = Array_initialize(elementSize, size, freeEleFn, printEleFn);
  cb->size = size;
  cb->readIdx = 0;
  cb->writeIdx = 0;
  cb->toReadBackwards = 0;

  return cb;
}

void CHB_push(CircularHistoryBuffer cb, void* ele) {
  if (cb == NULL) exitWithError("@CHB_writeNext cb instance can't be NULL");
  if (arrayGetLen(cb->array) < cb->size) arrayPush(cb->array, ele);
  else arraySet(cb->array, cb->writeIdx, ele);
  increaseWriteIdx(cb);
  cb->readIdx = cb->writeIdx;
  cb->toReadBackwards = arrayGetLen(cb->array);
}

void* CHB_readNext(CircularHistoryBuffer cb) {
  if (cb == NULL) exitWithError("@CHB_readNext cb instance can't be NULL");
  if (cb->toReadBackwards + 1 >= arrayGetLen(cb->array)) return NULL;
  increaseReadIdx(cb);
  ++cb->toReadBackwards;
  return arrayGet(cb->array, cb->readIdx);
}

void* CHB_readPrev(CircularHistoryBuffer cb) {
  if (cb == NULL) exitWithError("@CHB_readPrev cb instance can't be NULL");
  if (cb->toReadBackwards == 0) return NULL;
  decreaseReadIdx(cb);
  --cb->toReadBackwards;
  return arrayGet(cb->array, cb->readIdx);
}

uint64_t CHB_getLen(CircularHistoryBuffer cb) {
  if (cb == NULL) exitWithError("@CHB_getLen cb instance can't be NULL");
  return cb->toReadBackwards;
}

uint64_t CHB_getSize(CircularHistoryBuffer cb) {
  if (cb == NULL) exitWithError("@CHB_getSize cb instance can't be NULL");
  return cb->size;
}

void CHB_free(CircularHistoryBuffer cb) {
  if (cb == NULL) exitWithError("@CHB_free cb instance can't be NULL");
  arrayFree(cb->array);
  sysFree(cb);
}

void CHB_printState(CircularHistoryBuffer cb) {
  Array_print(cb->array);
}

/////////////// Internal functions ///////////////

uint64_t getIncreasedIdxBy(CircularHistoryBuffer cb, uint64_t idx, uint64_t val) {
  return (idx + val) % cb->size;
}

uint64_t getDecreasedIdxBy(CircularHistoryBuffer cb, uint64_t idx, uint64_t val) {
  val %= arrayGetLen(cb->array) + 1;
  if (idx < val) idx += arrayGetLen(cb->array) - val;
  else idx -= val;
  return idx;
}

void increaseWriteIdx(CircularHistoryBuffer cb) {
  cb->writeIdx = getIncreasedIdxBy(cb, cb->writeIdx, 1);
}

void increaseReadIdx(CircularHistoryBuffer cb) {
  cb->readIdx = getIncreasedIdxBy(cb, cb->readIdx, 1);
}

void decreaseReadIdx(CircularHistoryBuffer cb) {
  cb->readIdx = getDecreasedIdxBy(cb, cb->readIdx, 1);
}
