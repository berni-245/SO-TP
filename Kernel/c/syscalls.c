#include <syscalls.h>

void test(char c) {
  char c2 = c;
};

static SyscallFunction syscalls[] = {
  (SyscallFunction)test
};

void syscallDispatcher(uint64_t a, uint64_t b, uint64_t c, uint64_t d, int idx) {
  syscalls[idx](a, b, c, d);
}
