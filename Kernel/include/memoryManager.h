#ifndef MEMORY_MANAGER_H
#define MEMORY_MANAGER_H

#define BUDDY // comment this to change to the other mm implementation

#include <stdint.h>

#define NULL (void*)0

void* malloc(uint64_t size);
void free(void* ptr);
void memoryInit(void* heapStart);
// must be freed upon finishing it's use
char* getMemoryState();
void stackAlloc(void** stackStart, void** stackEnd);
void* shittyRealloc(void* ptr, unsigned long oldSize, unsigned long newSize);

#endif
