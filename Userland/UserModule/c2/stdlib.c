#include <draw.h>
#include <shell.h>
#include <stdarg.h>
#include <stdlib.h>
#include <syscalls.h>
#include <sysinfo.h>

bool getKey(KeyStruct* key) {
  ProcessPipes pipes = sysGetPipes();
  sysRead(pipes.read, &key->character, 1);
  sysGetModKeys(&key->md);
  return (int)key->character != EOF;
}

char getChar() {
  ProcessPipes pipes = sysGetPipes();
  char c;
  if (sysRead(pipes.read, &c, 1) < 0) return EOF;
  return c;
}

void clearScreen() {
  sysMoveCursor(0, 0);
  clearRectangle(0, 0, systemInfo.screenWidth, systemInfo.screenHeight);
}

int32_t printChar(char c) {
  ProcessPipes pipes = sysGetPipes();
  return sysWrite(pipes.write, &c, 1);
}

int32_t printString(const char* s) {
  int32_t i;
  for (i = 0; s[i] != 0; ++i) printChar(s[i]);
  return i;
}

void puts(const char* s) {
  printString(s);
  printChar('\n');
}

int32_t strcmp(const char* s1, const char* s2) {
  int32_t i = 0;
  for (; s1[i] != 0 && s2[i] != 0; ++i) {
    if (s1[i] < s2[i]) return -1;
    else if (s1[i] > s2[i]) return 1;
  }
  if (s1[i] != 0) return 1;
  else if (s2[i] != 0) return -1;
  else return 0;
}

int32_t strFindChar(const char* s, char c) {
  for (int32_t i = 0; s[i] != 0; ++i) {
    if (s[i] == c) return i;
  }
  return -1;
}

uint32_t strlen(char* s) {
  int32_t len = 0;
  while (s[len++] != 0);
  return len - 1;
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
uint32_t intToBase(int64_t value, char* buffer, uint32_t base) {
  char* p = buffer;
  if (value < 0) {
    *p++ = '-';
    value = -value;
  }
  return uintToBase(value, p, base);
}
int32_t printAsBase(int64_t n, int32_t base) {
  char buf[255];
  intToBase(n, buf, base);
  return printString(buf);
}
int32_t printUintAsBase(uint64_t n, int32_t base) {
  char buf[255];
  uintToBase(n, buf, base);
  return printString(buf);
}

static char paddingChar;
static int32_t paddingLen = 0;
static int32_t paddingSign = 1;
int32_t printPadding() {
  int32_t len = paddingLen;
  for (int32_t i = 0; i < len; ++i) {
    printChar(paddingChar);
  }
  paddingLen = 0;
  return len;
}
int32_t printStringWithAlignedPadding(const char* s) {
  int32_t len = 0;
  if (paddingSign > 0) {
    len += printPadding();
    len += printString(s);
  } else {
    len += printString(s);
    len += printPadding();
  }
  return len;
}
int32_t printAsBaseWithPadding(int64_t n, int32_t base) {
  char buf[255];
  int32_t digits = intToBase(n, buf, base);
  paddingLen = paddingLen - digits;
  char* s = buf;
  int32_t len = 0;
  if (n < 0) {
    printChar('-');
    ++s;
    --paddingLen;
    ++len;
  }
  len += printStringWithAlignedPadding(s);
  return len;
}
int32_t printUintAsBaseWithPadding(int64_t n, int32_t base) {
  char buf[255];
  int32_t digits = uintToBase(n, buf, base);
  paddingLen = paddingLen - digits;
  char* s = buf;
  return printStringWithAlignedPadding(s);
}
int32_t printStringWithPadding(const char* s) {
  for (int32_t i = 0; s[i] != 0 && paddingLen > 0; ++i) {
    --paddingLen;
  }
  return printStringWithAlignedPadding(s);
}

int32_t printf(const char* fmt, ...) {
  va_list p;
  va_start(p, fmt);
  paddingLen = 0;

  int32_t len = 0;

  int32_t i = 0;
  while (fmt[i] != 0) {
    if (fmt[i] != '%' && paddingLen == 0) {
      printChar(fmt[i]);
      ++len;
    } else {
      if (paddingLen == 0) ++i;
      else if (!strContains("udlxs", fmt[i])) {
        printf("...\nError: Only '%%s', '%%d', '%%l' and '%%x' identifier accept padding.\n");
        return -1;
      }
      switch (fmt[i]) {
      case '%':
        printChar('%');
        ++len;
        break;
      case 'd':
        len += printAsBaseWithPadding(va_arg(p, int), 10);
        paddingLen = 0;
        break;
      case 'u':
        len += printUintAsBaseWithPadding(va_arg(p, int), 10);
        paddingLen = 0;
        break;
      case 'l':
        switch (fmt[++i]) {
        case 'x':
          len += printString("0x");
          len += printUintAsBaseWithPadding(va_arg(p, long), 16);
          break;
        case 'i':
          len += printAsBaseWithPadding(va_arg(p, int), 10);
          break;
        case 'u':
          len += printUintAsBaseWithPadding(va_arg(p, long), 10);
          break;
        default:
          printf("...\nUnkown identifier `l`. Did you mean `li`, `lu` or `lx`?.\n");
          return -1;
        }
        break;
      case 'f':
        len += printString("'TODO print float'");
        break;
      case 'x':
        len += printString("0x");
        len += printUintAsBaseWithPadding(va_arg(p, int), 16);
        break;
      case 'p':
        paddingLen = 8;
        paddingSign = 1;
        paddingChar = '0';
        len += printString("0x");
        len += printUintAsBaseWithPadding(va_arg(p, long), 16);
        break;
      case 'b':
        len += printString("0b");
        len += printAsBase(va_arg(p, int), 2);
        break;
      case 's':
        len += printStringWithPadding(va_arg(p, char*));
        break;
      case 'c':
        printChar(va_arg(p, int));
        ++len;
        break;
      default:
        paddingSign = 1;
        if (fmt[i] == '-') {
          paddingSign = -1;
          ++i;
        }
        if (IS_DIGIT(fmt[i])) {
          paddingChar = ' ';
          if (fmt[i] == '0') {
            paddingChar = '0';
            ++i;
          }
          char nbr[MAX_PADDING_DIGITS + 1];
          int32_t j = 0;
          while (IS_DIGIT(fmt[i]) && j < MAX_PADDING_DIGITS) nbr[j++] = fmt[i++];
          if (IS_DIGIT(fmt[i])) {
            printf("...\nFormat error: \"%s\"\n", fmt);
            printf("Maximum padding of %li exceeded\n", pow(10, MAX_PADDING_DIGITS) - 1);
            return -1;
          }
          nbr[j] = 0;
          paddingLen = strToInt(nbr);
        } else {
          printf("\nUnkown option: %%%c\n", fmt[i]);
          return -1;
        }
      }
    }
    if (paddingLen == 0) ++i;
  }
  return len;
}

int32_t hexCharToDec(char c) {
  c = TO_LOWER(c);
  if (IS_DIGIT(c)) return c - '0';
  else if (IS_HEX_LETTER(c)) return c - 'a' + 10;
  else return -1;
}
int32_t strToInt(char* s) {
  int32_t base = 10;
  int32_t multiplier = 1;
  if (s[0] == '-') {
    multiplier = -1;
    ++s;
  }
  if (s[0] == '0' && s[1] == 'x') {
    base = 16;
    s += 2;
  }
  int32_t j = strlen(s) - 1;
  int32_t n = 0, k = 1;
  while (j >= 0 && (IS_DIGIT(s[j]) || (base == 16 && IS_HEX_LETTER(s[j])))) {
    n += hexCharToDec(s[j]) * k;
    --j;
    k *= base;
  }
  return n * multiplier;
}

void printKey(KeyStruct* key) {
  printf(
      "('%c')%s%s%s%s%s\n", key->character, key->md.ctrlPressed ? " + ctrl" : "",
      key->md.leftShiftPressed ? " + l-shift" : "", key->md.rightShiftPressed ? " + r-shift" : "",
      key->md.capsLockActive ? " + capsLock" : "", key->md.altPressed ? " + alt" : ""
  );
}

static uint32_t srand = 0;
void setSrand(uint32_t seed) {
  srand = seed;
}
// Return number between 0 and 1073741823 (0x3FFFFFFF).
uint32_t rand() {
  // Using Borland parameters from https://en.wikipedia.org/wiki/Linear_congruential_generator
  // Seems to always alternate between even and odd numbers which kinda sucks but oh well...
  return srand = ((22695477l * srand + 1) % (2l << 31)) & 0x3FFFFFFF;
}
// Apparently returning floating point values is not allowed, I get a compilation error.
// double normalizedRand() {
//   return (double)rand() / 0x3FFFFFFF;
// }
// unsigned int randBetween(int min, int max) {
//   double nrand = normalizedRand();
//   if (nrand >= 1.0) return max;
//   else return (max - min + 1) * nrand + min;
// }
uint32_t randBetween(int32_t min, int32_t max) {
  return rand() % (max - min + 1) + min;
}

void printStringXY(int32_t x, int32_t y, char* s, int32_t fontSize, int32_t charsPerRow) {
  int32_t col = 0;
  for (int32_t i = 0; s[i] != 0; ++i) {
    sysWriteCharXY(x + i * systemInfo.charWidth * fontSize + systemInfo.charSeparation, y, s[i], fontSize);
    if (charsPerRow && col > charsPerRow) {
      y += systemInfo.charHeight * fontSize + systemInfo.charSeparation;
      col = 0;
      x = 0;
    }
    ++col;
  }
}

int32_t strTrimStartOffset(const char* s) {
  int32_t i = 0;
  while (*(s++) == ' ') ++i;
  return i;
}

bool strContains(const char* s, const char c) {
  if (c <= 0) return false;
  for (int32_t i = 0; s[i] != 0; ++i) {
    if (s[i] == c) return true;
  }
  return false;
}

char toLower(char c) {
  return TO_LOWER(c);
}

long pow(int32_t x, int32_t n) {
  // Habría que settear un errno o algo en realidad.
  if (n < 0) return x;
  long res = 1;
  for (int32_t i = 0; i < n; ++i) {
    res *= x;
  }
  return res;
}
