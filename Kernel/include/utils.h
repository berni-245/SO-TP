#ifndef UTILS_H
#define UTILS_H

#include <stdint.h>

int strncpy(char* dst, const char* src, int max);
int strcpy(char* dst, char* src);
unsigned int s_strlen(char* s);
int s_strcmp(const char* s1, const char* s2);
uint32_t uintToBase(uint64_t value, char* buffer, uint32_t base);
#endif
