#ifndef UTILS_H
#define UTILS_H

#include <keyboard.h>
#include <stdbool.h>

bool justCtrlMod(KeyStruct* key);
void* shittyRealloc(void* ptr, unsigned long oldSize, unsigned long newSize);

#endif
