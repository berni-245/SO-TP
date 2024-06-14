#include <stdlib.h>
#include <syscalls.h>
#include <utils.h>

bool justCtrlMod(KeyStruct* key) {
  return key->md.ctrlPressed && !key->md.altPressed && !key->md.leftShiftPressed && !key->md.rightShiftPressed;
}

void* shittyRealloc(void* ptr, unsigned long oldSize, unsigned long newSize) {
  void* mem = sysMalloc(newSize);
  if (mem == NULL) return NULL;
  sysMemcpy(mem, ptr, oldSize);
  sysFree(ptr);
  return mem;
}

void exitWithError(const char* msg) {
  printf("%s\n", msg);
  sysExit(1);
}

int strncpy(char* dst, const char* src, int max) {
  int i = 0;
  if (max > 0)
    for (; i < max && src[i] != 0; ++i) dst[i] = src[i];
  else
    for (; src[i] != 0; ++i) dst[i] = src[i];
  dst[i] = 0;
  return i;
}

int strcpy(char* dst, char* src) {
  return strncpy(dst, src, 0);
}
