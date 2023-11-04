#ifndef STDLIB_H
#define STDLIB_H

#include <keyboard.h>
#include <stdint.h>

#define NULL (void*)0
#define EOF -1

KeyStruct getKey();
char getChar();
void printString(const char* s);
void printBuffer(const char buf[], int size);
int strcmp(const char*, const char*);
int printf(const char* fmt, ...);
void setBgColor(uint32_t hexColor);
void setStrokeColor(uint32_t hexColor);
void setFillColor(uint32_t hexColor);
void setFontColor(uint32_t hexColor);
void printKey(KeyStruct key);

#endif
