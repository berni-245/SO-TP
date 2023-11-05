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

int getFontSize();
int getScreenWidth();
int getScreenHeight();

typedef struct RGBColor {
  uint8_t blue;
  uint8_t green;
  uint8_t red;
} RGBColor;
void setBgColor(uint32_t hexColor);
void setStrokeColor(uint32_t hexColor);
void setFillColor(uint32_t hexColor);
void setFontColor(uint32_t hexColor);
void saveColorAndSet(RGBColor newColor);
void restoreColor();

void setFontGridValues();
void setFontSize(int fs);
void printCharXY(int x, int y, char c, int fontSize);
void printChar(int col, int row, char c);
// void printBuffer(const char buf[], int size); nota de berni: no está definido en ningún lado
int printNextChar(char c);
void printNextString(const char * str);
void printNextBase(uint64_t value, uint32_t base);
void printNextDec(uint64_t value);
void printNextHex(uint64_t value);
void printNextHexWithPadding(uint64_t value);
void moveCursor(int col, int row);
int cursorHasNext();
int cursorNext();
int cursorPrev();
void printCursor();
void eraseCursor();

#endif
