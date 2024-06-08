#include <syscalls.h>
#include <utils.h>
#include <stdlib.h>

bool justCtrlMod(KeyStruct* key) {
  return key->md.ctrlPressed && !key->md.altPressed && !key->md.leftShiftPressed && !key->md.rightShiftPressed;
}

void sysFree(void* ptr) {
  return;
}

void* shittyRealloc(void* ptr, unsigned long oldSize, unsigned long newSize) {
  void* mem = sysMalloc(newSize);
  if (mem == NULL) return NULL;
  sysMemcpy(mem, ptr, oldSize);
  sysFree(ptr);
  return mem;
}
