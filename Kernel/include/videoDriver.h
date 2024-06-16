#ifndef VIDEO_DRIVER_H
#define VIDEO_DRIVER_H

#include <asciiBitFields.h>
#include <stdint.h>

typedef struct {
  uint8_t blue;
  uint8_t green;
  uint8_t red;
} RGBColor;

void initializeFrameBuffer();

void printPixel(int32_t x, int32_t y, RGBColor color);
void fillRectangle(int32_t x, int32_t y, int32_t width, int32_t height, RGBColor color);

int32_t getFontSize();
int32_t getScreenWidth();
int32_t getScreenHeight();
int32_t getCharSeparation();

typedef enum {
  BACKGROUND = 0,
  FONT,
  CURSOR,
} ColorType;
void setColor(ColorType c, uint32_t hexColor);

void setFontGridValues();
int32_t setFontSize(int32_t fs);
void printCharXY(int32_t x, int32_t y, char c, int32_t fontSize);
void printChar(int32_t col, int32_t row, char c);
void printNextChar(char c);
void printNextString(const char* str);
void printNextBuf(const char* buf, uint32_t len);
void printNextBase(uint64_t value, uint32_t base);
void printNextDec(uint64_t value);
void printNextHex(uint64_t value);
void printNextHexWithPadding(uint64_t value);
void moveCursor(int32_t col, int32_t row);
int32_t cursorHasNext();
int32_t cursorNext();
int32_t cursorPrev();
void printCursor();
void eraseCursor();
void clearScreen();

#endif
