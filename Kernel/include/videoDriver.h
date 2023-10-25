#ifndef VIDEO_DRIVER_H
#define VIDEO_DRIVER_H

#include <stdint.h>


void setDefaultColor(uint32_t hexColor);
void printPixel(uint32_t hexColor, uint64_t x, uint64_t y);
void printPixelDefault(uint64_t x, uint64_t y);
void printBitFieldDefault(uint64_t x, uint64_t y, uint8_t * v, uint64_t rows, uint64_t cols);
void printBitField(uint32_t hexColor, uint64_t x, uint64_t y, uint8_t * v, uint64_t rows, uint64_t cols);

#endif