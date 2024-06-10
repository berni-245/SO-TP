#include <lib.h>
#include <memoryManager.h>

// 4KB stack size
static const int stackSize = (1 << 10) * 4;
static const uint64_t addressByteSize = sizeof(void*);

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
  *stackEnd = globalMalloc(stackSize);
  *stackStart = *stackEnd + stackSize - 1;
  *stackStart = (void*)(((uint64_t)*stackStart - addressByteSize) & ~(addressByteSize - 1));
}

void* shittyRealloc(void* ptr, unsigned long oldSize, unsigned long newSize) {
  void* mem = globalMalloc(newSize);
  if (mem == NULL) return NULL;
  memcpy(mem, ptr, oldSize);
  globalFree(ptr);
  return mem;
}
