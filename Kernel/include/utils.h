#ifndef UTILS_H
#define UTILS_H

#include <stdint.h>

int strncpy(char* dst, char* src, int max);
int strcpy(char* dst, char* src);
uint32_t uintToBase(uint64_t value, char* buffer, uint32_t base);

#endif