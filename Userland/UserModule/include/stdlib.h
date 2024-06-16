#ifndef STDLIB_H
#define STDLIB_H

#include <float.h>
#include <keyboard.h>
#include <stdint.h>

#define NULL (void*)0
#define EOF (-1)

#define stdout 0
#define stdin 1
#define stderr 2

#define MAX_PADDING_DIGITS 2

#define TO_LOWER(c) (((c) >= 'A' && (c) <= 'Z') ? ((c) + 'a' - 'A') : (c))
#define IS_HEX_LETTER(c) ('a' <= TO_LOWER(c) && TO_LOWER(c) <= 'f')
#define IS_DIGIT(c) ('0' <= (c) && (c) <= '9')

bool getKey(KeyStruct* key);
char getChar();
int32_t printChar(char c);
void clearScreen();
int32_t printString(const char* s);
void puts(const char* s);
uint32_t uintToBase(uint64_t value, char* buffer, uint32_t base);
uint32_t intToBase(long value, char* buffer, uint32_t base);
int32_t strcmp(const char*, const char*);
int32_t strFindChar(const char* s, char c);
uint32_t strlen(char* s);
int32_t printf(const char* fmt, ...);
int32_t strToInt(char* s);
void printKey(KeyStruct* key);
void setSrand(uint32_t seed);
uint32_t rand();
// double normalizedRand();
uint32_t randBetween(int32_t min, int32_t max);
void printStringXY(int32_t x, int32_t y, char* s, int32_t fontSize, int32_t charsPerRow);
int32_t strTrimStartOffset(const char* s);
bool strContains(const char* s, const char c);
char toLower(char c);
long pow(int32_t x, int32_t n);

#endif
