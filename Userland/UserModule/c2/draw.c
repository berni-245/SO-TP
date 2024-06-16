#include <colors.h>
#include <draw.h>
#include <syscalls.h>
#include <sysinfo.h>

void strokeHorizontalLine(int32_t x, int32_t y, int32_t length, int32_t width) {
  sysFillRectangle(x, y, length, width, strokeColor);
}
void strokeVerticalLine(int32_t x, int32_t y, int32_t length, int32_t width) {
  sysFillRectangle(x, y, width, length, strokeColor);
}

void strokeRectangle(int32_t x, int32_t y, int32_t width, int32_t height, int32_t strokeWidth) {
  strokeHorizontalLine(x, y, width, strokeWidth);
  strokeHorizontalLine(x, y + height - strokeWidth, width, strokeWidth);
  strokeVerticalLine(x, y + strokeWidth, height - strokeWidth, strokeWidth);
  strokeVerticalLine(x + width - strokeWidth, y + strokeWidth, height - strokeWidth, strokeWidth);
}

void strokeRectangleOutward(int32_t x, int32_t y, int32_t width, int32_t height, int32_t strokeWidth) {
  strokeHorizontalLine(x - strokeWidth, y - strokeWidth, width + 2 * strokeWidth, strokeWidth);
  strokeHorizontalLine(x - strokeWidth, y + height, width + 2 * strokeWidth, strokeWidth);
  strokeVerticalLine(x - strokeWidth, y, height, strokeWidth);
  strokeVerticalLine(x + width, y, height, strokeWidth);
}

void fillRectangle(int32_t x, int32_t y, int32_t width, int32_t height) {
  sysFillRectangle(x, y, width, height, fillColor);
}

void clearRectangle(int32_t x, int32_t y, int32_t width, int32_t height) {
  sysFillRectangle(x, y, width, height, bgColor);
}

void fillRectangleWithBorder(int32_t x, int32_t y, int32_t width, int32_t height, int32_t strokeWidth) {
  sysFillRectangle(x, y, width, height, fillColor);
  strokeRectangle(x, y, width, height, strokeWidth);
}
