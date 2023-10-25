#ifndef VIDEO_DRIVER_H
#define VIDEO_DRIVER_H

#include <stdint.h>

void printPixel(uint32_t hexColor, uint64_t x, uint64_t y);
void printPixelDefault(uint64_t x, uint64_t y);
void setDefaultColor(uint32_t hexColor);

#endif