#ifndef UTILS_H
#define UTILS_H

#include <keyboard.h>
#include <stdbool.h>

bool justCtrlMod(KeyStruct* key);
void* shittyRealloc(void* ptr, unsigned long oldSize, unsigned long newSize);
void exitWithError(const char* msg);
int strncpy(char* dst, const char* src, int max);
int strcpy(char* dst, char* src);

#endif
