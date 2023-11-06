#include <colors.h>
#include <draw.h>
#include <syscalls.h>
#include <sysinfo.h>

void strokeHorizontalLine(int x, int y, int length, int width) {
  sysFillRectangle(x, y, length, width, strokeColor);
}
void strokeVerticalLine(int x, int y, int length, int width) {
  sysFillRectangle(x, y, width, length, strokeColor);
}

void strokeRectangle(int x, int y, int width, int height, int strokeWidth) {
  strokeHorizontalLine(x, y, width, strokeWidth);
  strokeHorizontalLine(x, y + height - strokeWidth, width, strokeWidth);
  strokeVerticalLine(x, y + strokeWidth, height - strokeWidth, strokeWidth);
  strokeVerticalLine(x + width - strokeWidth, y +strokeWidth, height - strokeWidth, strokeWidth);
}

void fillRectangle(int x, int y, int width, int height) {
  sysFillRectangle(x, y, width, height, fillColor);
}

void fillRectangleWithBorder(int x, int y, int width, int height, int strokeWidth) {
  sysFillRectangle(x, y, width, height, fillColor);
  strokeRectangle(x, y, width, height, strokeWidth);
}
