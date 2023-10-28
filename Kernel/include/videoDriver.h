#ifndef VIDEO_DRIVER_H
#define VIDEO_DRIVER_H

#include <asciiBitFields.h>
#include <stdint.h>

void printPixel(int x, int y);
void fillRectangle(int x, int y, int width, int height);
void setStrokeWidth(int width);
void strokeLine(int startX, int startY, int endX, int endY);
void strokeRectangle(int x, int y, int width, int height);
void clearScreen();

void strokeHorizontalLine(int x, int y, int length);
void strokeVerticalLine(int x, int y, int length);


void setBgColor(uint32_t hexColor);
void setStrokeColor(uint32_t hexColor);
void setFillColor(uint32_t hexColor);
void setFontColor(uint32_t hexColor);
void saveColor();
void restoreColor();

void increaseFont();
void decreaseFont();
void printChar(int x, int y, char c);
void printBuffer(int x, int y, char buf[], int size);

#endif
