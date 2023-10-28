#ifndef VIDEO_DRIVER_H
#define VIDEO_DRIVER_H

#include <asciiBitFields.h>
#include <stdint.h>

void setColor(uint32_t hexColor);
void printPixel(int x, int y);
void fillRectangle(int x, int y, int width, int height);
void clearScreen();
// void printPixelDefault(uint64_t x, uint64_t y);
// void printBitFieldDefault(uint64_t x, uint64_t y, uint8_t * v, uint64_t rows, uint64_t cols);
// void printBitField(uint64_t x, uint64_t y, const uint8_t v[], int rows, int cols);
void printChar(int x, int y, char c);

void increaseFont();
void decreaseFont();

#endif
