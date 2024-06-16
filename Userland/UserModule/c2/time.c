#include <syscalls.h>
#include <time.h>

static double freq = 65536 / 3600.0; // interruptions/second
uint64_t getMs() {
  return sysGetTicks() * 1000 / freq;
}

void sleep(uint64_t ms) {
  uint64_t end = ms + getMs();
  while (getMs() < end) {
    sysHalt();
  }
}
