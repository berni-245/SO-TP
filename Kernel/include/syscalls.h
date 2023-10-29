#ifndef SYSCALLS_H
#define SYSCALLS_H

#include <stdint.h>

extern void syscallHandler();

typedef void (*SyscallFunction)(uint64_t a, uint64_t b, uint64_t c, uint64_t d);

void syscallDispatcher(uint64_t a, uint64_t b, uint64_t c, uint64_t d, int idx);

#endif
