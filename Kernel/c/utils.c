#include <utils.h>

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

unsigned int s_strlen(char* s) {
  int len = 0;
  while (s[len++] != 0);
  return len - 1;
}

int s_strcmp(const char* s1, const char* s2) {
  int i = 0;
  for (; s1[i] != 0 && s2[i] != 0; ++i) {
    if (s1[i] < s2[i]) return -1;
    else if (s1[i] > s2[i]) return 1;
  }
  if (s1[i] != 0) return 1;
  else if (s2[i] != 0) return -1;
  else return 0;
}
