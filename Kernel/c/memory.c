#include <memory.h>

// extern uint8_t endOfBinary;

// 5MB max heap size
const int heapSize = (1 << 20) * 5;
void* const heapStart = (void*)0x9000000; // &endOfBinary;
void* heapCurrent;

void memoryInit() {
  heapCurrent = heapStart;
}

void* malloc(uint32_t size) {
  if (heapCurrent + size - heapStart > heapSize) return NULL;
  void* heapRet = heapCurrent;
  heapCurrent += size;
  return heapRet;
}
