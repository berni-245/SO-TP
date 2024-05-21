#include <memory.h>

// extern uint8_t endOfBinary;

// 64MB max heap size
static const int heapSize = (1 << 20) * 64;
// 4KB stack size
static const int stackSize = (1 << 10) * 4;

static void* heapStart;
static void* heapCurrent;

static const uint64_t addressByteSize = sizeof(void*);

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

void memoryInit(void* endOfModules) {
  heapStart = (void*)(((uint64_t)endOfModules + addressByteSize - 1) & ~(addressByteSize - 1));
  heapCurrent = heapStart;
}

void* malloc(uint64_t size) {
  if (heapCurrent + size - heapStart > heapSize) return NULL;
  void* heapRet = heapCurrent;
  heapCurrent += size;
  return heapRet;
}

/*
rsp = e                e                   
0x00000000000503d8  00 00 00 00 00 00 00 00
0x00000000000503e0  00 00 00 00 00 00 00 00
0x00000000000503e8  00 00 00 00 00 00 00 00
                                   s  >>   
0x00000000000503f0  00 00 00 00 00 00 00 00

rsp = e - 8            e                   
0x00000000000503d8  00 00 00 00 00 00 00 00
0x00000000000503e0  00 00 00 00 00 00 00 00
                                   v       
0x00000000000503e8  00 00 00 00 00 00 00 00
                                   s  >>   
0x00000000000503f0  00 00 00 00 00 00 00 00

rsp = (e - 8) & ~7
                       e                 
0x00000000000503d8  00 00 00 00 00 00 00 00
0x00000000000503e0  00 00 00 00 00 00 00 00
                    v                      
0x00000000000503e8  00 00 00 00 00 00 00 00
                                    s >>   
0x00000000000503f0  00 00 00 00 00 00 00 00
 */
void stackAlloc(void** stackStart, void** stackEnd) {
  *stackEnd = malloc(stackSize);
  *stackStart = *stackEnd + stackSize - 1;
  *stackStart = (void*)(((uint64_t)*stackStart - addressByteSize) & ~(addressByteSize - 1));
}

void free(void* ptr) {
  return;
}
