#ifndef DRAW_H
#define DRAW_H

#include <stdint.h>

void strokeHorizontalLine(int32_t x, int32_t y, int32_t length, int32_t width);
void strokeVerticalLine(int32_t x, int32_t y, int32_t length, int32_t width);
void strokeRectangle(int32_t x, int32_t y, int32_t width, int32_t height, int32_t strokeWidth);
void strokeRectangleOutward(int32_t x, int32_t y, int32_t width, int32_t height, int32_t strokeWidth);
void fillRectangle(int32_t x, int32_t y, int32_t width, int32_t height);
void clearRectangle(int32_t x, int32_t y, int32_t width, int32_t height);
void fillRectangleWithBorder(int32_t x, int32_t y, int32_t width, int32_t height, int32_t strokeWidth);

#endif
