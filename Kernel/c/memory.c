#include <memory.h>
#include <stdint.h>

// extern uint8_t endOfBinary;

/*
heapStart alignment example:

If endOfModules = 0x503e3 => heapStart = 0x503e8
0x00000000000503d8  00 00 00 00 00 00 00 00
0x00000000000503e0  00 00 00 xx 00 00 00 00
0x00000000000503e8  xx 00 00 00 00 00 00 00
0x00000000000503f0  00 00 00 00 00 00 00 00

endOfModules = 0x503e3        v                 
0x00000000000503e0  00 00 00 00 00 00 00 00

heapStart = 0x503e3 + 7
heapStart = 0x503ea        v                        
0x00000000000503e8  00 00 00 00 00 00 00 00

mask = ~7 = 0b1111...11111000 (with a total of 64 bits)
heapStart = heapStart & mask = 0b...1010 & 0b...1000 = 0b...1000
heapStart = 0x503e8  v                              
0x00000000000503e8  00 00 00 00 00 00 00 00
*/

typedef struct blockLink_t{
  struct blockLink_t * nextFreeBlock;
  uint64_t blockSize;
} blockLink_t;

// 64MB max heap size
static const int heapSize = (1 << 20) * 64;

static void* heapStart;
static void* heapCurrent;
static blockLink_t listStart;
static blockLink_t * listEnd = NULL;

static const uint64_t addressByteSize = sizeof(void*);
static const uint64_t heapStructSize = sizeof(blockLink_t);

void memoryInit(void* endOfModules) {
  void * alinedHeapStart = (void*)(((uint64_t)endOfModules + addressByteSize - 1) & ~(addressByteSize - 1));
  listStart.nextFreeBlock = alinedHeapStart;
  listStart.blockSize = 0;

  void * alinedHeapEnd = (void*)((uint64_t) (alinedHeapStart + heapSize - heapStructSize) & ~(addressByteSize - 1));
  listEnd = (blockLink_t *) alinedHeapEnd;
  listEnd->blockSize = 0;
  listEnd->nextFreeBlock = NULL;

  blockLink_t * firstFreeBlock = (blockLink_t *) alinedHeapStart;
  firstFreeBlock->blockSize = (uint64_t) (alinedHeapEnd - alinedHeapStart);
  firstFreeBlock->nextFreeBlock = listEnd;
}

void* malloc(uint64_t size) {
  if (heapCurrent + size - heapStart > heapSize) return NULL;
  void* heapRet = heapCurrent;
  heapCurrent += size;
  return heapRet;
}

void free(void* ptr) {
  return;
}
