#include <utils.h>

int strncpy(char* dst, char* src, int max) {
  int i = 0;
  if (max > 0) for (; i < max && src[i] != 0; ++i) dst[i] = src[i];
  else for (; src[i] != 0; ++i) dst[i] = src[i];
  dst[i] = 0;
  return i;
}

int strcpy(char* dst, char* src) {
  return strncpy(dst, src, 0);
}

uint32_t uintToBase(uint64_t value, char* buffer, uint32_t base) {
  char* p = buffer;
  char* p1 = buffer;
  char* p2;
  uint32_t digits = 0;

  do {
    uint32_t remainder = value % base;
    *p++ = (remainder < 10) ? remainder + '0' : remainder + 'A' - 10;
    digits++;
  } while (value /= base);

  *p = 0;

  p2 = p - 1;
  while (p1 < p2) {
    char tmp = *p1;
    *p1 = *p2;
    *p2 = tmp;
    p1++;
    p2--;
  }

  return digits;
}