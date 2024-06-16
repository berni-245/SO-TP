#ifndef UTILS_H
#define UTILS_H

#include <stdint.h>

int32_t strncpy(char* dst, const char* src, int32_t max);
int32_t strcpy(char* dst, char* src);
uint32_t strlen(char* s);
int32_t strcmp(const char* s1, const char* s2);
uint32_t uintToBase(uint64_t value, char* buffer, uint32_t base);
#endif
