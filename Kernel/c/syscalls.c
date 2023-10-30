#include <interruptions.h>
#include <keyboard.h>
#include <syscalls.h>
#include <timer.h>
#include <videoDriver.h>

static SyscallFunction syscalls[] = {
  (SyscallFunction)haltTillNextInterruption,
  (SyscallFunction)getMs,
  (SyscallFunction)setLayout,
  (SyscallFunction)getVideoInfo,
  (SyscallFunction)readKbBuffer,
  (SyscallFunction)printChar,
};

SyscallFunction* getSyscallsArray() {
  return syscalls;
}
