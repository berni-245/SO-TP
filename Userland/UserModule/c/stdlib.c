#include <stdarg.h>
#include <stdlib.h>
#include <syscalls.h>

KeyStruct getKey() {
  KeyStruct key;
  int read = sysRead(&key, 1);
  return key;
}

char getChar() {
  KeyStruct key = getKey();
  if (key.key == 0) return EOF;
  else return key.key;
}

void printBuffer(const char buf[], int size) {
  for (int i = 0; i < size; ++i) {
    sysWriteCharNext(buf[i]);
  }
}

void printString(const char *s) {
  for (int i = 0; s[i] != 0; ++i) {
    sysWriteCharNext(s[i]);
  }
}

int strcmp(const char* s1, const char* s2) {
  int i = 0;
  for (; s1[i] != 0 && s2[i] != 0; ++i) {
    if (s1[i] < s2[i]) return -1;
    else if (s1[i] > s2[i]) return 1;
  }
  if (s1[i] != 0) return 1;
  else if (s2[i] != 0) return -1;
  else return 0;
}

static uint32_t intToBase(long value, char* buffer, uint32_t base) {
	char* p = buffer;
	char* p1 = buffer;
  char* p2;
  if (value < 0) {
    *p++ = '-';
    p1 = p;
    value = -value;
  }
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
void printAsBase(int n, int base) {
  char buf[255];
  intToBase(n, buf, base);
  printString(buf);
}
// Return 0 on successful print, non 0 on error.
int printf(const char* fmt, ...) {
  va_list p;
  va_start (p, fmt);

  for (int i = 0; fmt[i] != 0; ++i) {
    if (fmt[i] != '%') {
      sysWriteCharNext(fmt[i]);
    } else {
      switch (fmt[++i]) {
        case '%':
          sysWriteCharNext('%');
          break;
        case 'd': 
          printAsBase(va_arg(p, int), 10);
          break;
        case 'l': 
          printAsBase(va_arg(p, long), 10);
          break;
        case 'f':
          printString("'TODO print float'");
          break;
        case 'x': 
          printString("0x");
          printAsBase(va_arg(p, int), 16);
          break;
        case 'b': 
          printString("0b");
          printAsBase(va_arg(p, int), 2);
          break;
        case 's':
          printString(va_arg(p, char*));
          break;
        case 'c':
          sysWriteCharNext(va_arg(p, int));
          break;
        default:
          printf("\nUnkown option: %%%c\n", fmt[i]);
          return 1;
      }
    }
  }
  return 0;
}

void setBgColor(uint32_t hexColor) {
  sysSetColor(BACKGROUND, hexColor);
}
void setStrokeColor(uint32_t hexColor) {
  sysSetColor(STROKE, hexColor);
}
void setFillColor(uint32_t hexColor) {
  sysSetColor(FILL, hexColor);
}
void setFontColor(uint32_t hexColor) {
  sysSetColor(FONT, hexColor);
}

void printKey(KeyStruct key) {
  printf(
    "{ key: %c, code: %d, ctrl: %d, l-shift: %d, r-shift: %d, capsLock: %d, alt: %d }\n",
    key.key,
    key.code,
    key.md.ctrlPressed,
    key.md.leftShiftPressed,
    key.md.rightShiftPressed,
    key.md.capsLockActive,
    key.md.altPressed
  );
}
