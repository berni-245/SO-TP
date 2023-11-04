#ifndef STDLIB_H
#define STDLIB_H

#include <float.h>
#include <keyboard.h>
#include <stdint.h>

#define NULL (void*)0
#define EOF -1

int getKey(KeyStruct* key);
char getChar();
void printChar(char c);
void printString(const char* s);
void puts(const char* s);
void printBuffer(const char buf[], int size);
int strcmp(const char*, const char*);
unsigned int strlen(char* s);
int printf(const char* fmt, ...);
int strToInt(char* s);
void setBgColor(uint32_t hexColor);
void setStrokeColor(uint32_t hexColor);
void setFillColor(uint32_t hexColor);
void setFontColor(uint32_t hexColor);
void printKey(KeyStruct* key);
void setSrand(unsigned int seed);
unsigned int rand();
// double normalizedRand();
unsigned int randBetween(int min, int max);

#endif
