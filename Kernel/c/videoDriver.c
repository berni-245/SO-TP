#include <videoDriver.h>

struct vbe_mode_info_structure {
  uint16_t attributes; // deprecated, only bit 7 should be of interest
                       // to you, and it indicates the mode
                       // supports a linear frame buffer.

  uint8_t window_a;     // deprecated
  uint8_t window_b;     // deprecated
  uint16_t granularity; // deprecated; used while calculating bank numbers
  uint16_t window_size;
  uint16_t segment_a;
  uint16_t segment_b;
  uint32_t win_func_ptr; // deprecated; used to switch banks from protected mode
                         // without returning to real mode

  uint16_t pitch;  // number of bytes per horizontal line
  uint16_t width;  // width in pixels
  uint16_t height; // height in pixels
  uint8_t w_char;  // unused...
  uint8_t y_char;  // ...
  uint8_t planes;
  uint8_t bpp;   // bits per pixel in this mode
  uint8_t banks; // deprecated; total number of banks in this mode
  uint8_t memory_model;
  uint8_t bank_size; // deprecated; size of a bank, almost always 64 KB but may
                     // be 16 KB...
  uint8_t image_pages;
  uint8_t reserved0;

  uint8_t red_mask;
  uint8_t red_position;
  uint8_t green_mask;
  uint8_t green_position;
  uint8_t blue_mask;
  uint8_t blue_position;
  uint8_t reserved_mask;
  uint8_t reserved_position;
  uint8_t direct_color_attributes;

  uint32_t framebuffer; // physical address of the linear frame buffer; write
                        // here to draw to the screen
  uint32_t off_screen_mem_off;
  uint16_t off_screen_mem_size; // size of memory in the framebuffer but not
                                // being displayed on the screen
  uint8_t reserved1[206];
} __attribute__((packed));

static RGBColor color = {0};
static RGBColor bgColor = {0};
static RGBColor strokeColor = {0};
static RGBColor fillColor = {0};
static RGBColor fontColor = {0};
static RGBColor savedColor = {0};

typedef struct vbe_mode_info_structure *VBEInfoPtr;

VBEInfoPtr VBE_mode_info = (VBEInfoPtr)0x0000000000005C00;

#define MAX_FONT_SIZE 4
static int fontSize = 1;
static int fontCols, fontRows;
void setFontGridValues() {
  fontCols = VBE_mode_info->width / (ASCII_BF_WIDTH * fontSize);
  fontRows = VBE_mode_info->height / (ASCII_BF_HEIGHT * fontSize);
}
int getFontSize() {
  return fontSize;
}

int getScreenWidth() {
  return VBE_mode_info->width;
}
int getScreenHeight() {
  return VBE_mode_info->height;
}

void printPixel(int x, int y) {
  RGBColor *framebuffer = (RGBColor *)VBE_mode_info->framebuffer;
  uint64_t offset = x + (y * VBE_mode_info->pitch / (VBE_mode_info->bpp / 8));
  framebuffer[offset] = color;
}

void printRectangle(int x, int y, int width, int height) {
  for (int i = 0; i < height; ++i) {
    for (int j = 0; j < width; ++j) {
      printPixel(x + j, y + i);
    }
  }
}

static int strokeWidth = 1;

void setStrokeWidth(int width) {
  if (width <= 0) return;
  strokeWidth = width;
}

void strokeLine(int startX, int startY, int endX, int endY) {
  saveColorAndSet(strokeColor);
  int vx = endX - startX;
  int vy = endY - startY;
  for (double a = 0; a <= 1; a += 0.001*strokeWidth/2) {
    int x = a * vx + startX;
    int y = a * vy + startY;
    printRectangle(x, y, strokeWidth, strokeWidth);
  }
  restoreColor();
}

void strokeHorizontalLine(int x, int y, int length) {
  saveColorAndSet(strokeColor);
  int i = 0;
  for (; i < length - strokeWidth; i += strokeWidth) {
    printRectangle(x + i, y, strokeWidth, strokeWidth);
  }
  printRectangle(x + i, y, length - i, strokeWidth);
  restoreColor();
}
void strokeVerticalLine(int x, int y, int length) {
  saveColorAndSet(strokeColor);
  int i = 0;
  for (; i <= length - strokeWidth; i += strokeWidth) {
    printRectangle(x, y + i, strokeWidth, strokeWidth);
  }
  printRectangle(x, y + i, strokeWidth, length - i);
  restoreColor();
}

void strokeRectangle(int x, int y, int width, int height) {
  strokeHorizontalLine(x, y, width);
  strokeHorizontalLine(x, y + height - strokeWidth, width);
  strokeVerticalLine(x, y + strokeWidth, height - strokeWidth);
  strokeVerticalLine(x + width - strokeWidth, y +strokeWidth, height - strokeWidth);
}

void fillRectangle(int x, int y, int width, int height) {
  saveColorAndSet(fillColor);
  printRectangle(x, y, width, height);
  strokeRectangle(x, y, width, height);
  restoreColor();
}

void printCharXY(int x, int y, char c, int fontSize) {
  if (c < ASCII_BF_MIN || c > ASCII_BF_MAX) return;
  saveColorAndSet(fontColor);
  c -= ASCII_BF_MIN;
  for (int i = 0; i < ASCII_BF_HEIGHT; ++i) {
    for (int j = 0; j < ASCII_BF_WIDTH; ++j) {
      if (asciiBitFields[c][i * ASCII_BF_WIDTH + j] != 0) {
        printRectangle(x + j*fontSize, y + i*fontSize, fontSize, fontSize);
      }
    }
  }
  restoreColor();
}

void clearScreen() {
  saveColorAndSet(bgColor);
  printRectangle(0, 0, VBE_mode_info->width, VBE_mode_info->height);
  restoreColor();
}

void setFontSize(int fs) {
  if (fs < 1 && fs > MAX_FONT_SIZE) return;
  fontSize = fs;
  setFontGridValues();
}

void printChar(int col, int row, char c) {
  if (c < ASCII_BF_MIN || c > ASCII_BF_MAX) return;
  saveColorAndSet(fontColor);
  c -= ASCII_BF_MIN;
  col *= ASCII_BF_WIDTH * fontSize;
  row *= ASCII_BF_HEIGHT * fontSize;
  for (int i = 0; i < ASCII_BF_HEIGHT; ++i) {
    for (int j = 0; j < ASCII_BF_WIDTH; ++j) {
      if (asciiBitFields[c][i * ASCII_BF_WIDTH + j] != 0) {
        printRectangle(col + j*fontSize, row + i*fontSize, fontSize, fontSize);
      }
    }
  }
  restoreColor();
}

void printBuffer(int col, int row, const char buf[], int size) {
  for (int i = 0; i < size; ++i) {
    printChar(col, row, buf[i]);
    if (++col >= fontCols) {
      col = 0;
      if (++row >= fontRows) {
        // move everything one row up
        return;
      }
    }
  }
}


static int cursorCol = 0;
static int cursorRow = 0;
void eraseChar() {
  saveColorAndSet(bgColor);
  if (--cursorCol < 0) {
    cursorCol = 0;
    if (--cursorRow < 0) {
      cursorRow = 0;
      return;
    }
  }
  int col = cursorCol * ASCII_BF_WIDTH * fontSize;
  int row = cursorRow * ASCII_BF_HEIGHT * fontSize;
  for (int i = 0; i < ASCII_BF_HEIGHT; ++i) {
    for (int j = 0; j < ASCII_BF_WIDTH; ++j) {
      printRectangle(col + j*fontSize, row + i*fontSize, fontSize, fontSize);
    }
  }
  restoreColor();
  return;
}
// 0 if was able to print char, 1 othwerwise (if cursor reached end of screen).
int printNextChar(char c) {
  if (c == '\b') {
    eraseChar();
  } else {
    printChar(cursorCol, cursorRow, c);
    if (++cursorCol >= fontCols) {
      cursorCol = 0;
      if (++cursorRow >= fontRows) {
        // move everything one row up
        --cursorRow;
        return 1;
      }
    }
  }
  return 0;
}

void moveCursor(int col, int row) {
  cursorCol = col;
  cursorRow = row;
}

void cursorNext() {
  if (++cursorCol >= fontCols) {
    cursorCol = 0;
    if (++cursorRow >= fontRows) {
      // move everything one row up
      --cursorRow;
      // return 1;
    }
  }
}

void setRGBColor(RGBColor* color, uint32_t hexColor) {
  color->blue = hexColor & 0xFF;
  color->green = (hexColor >> 8) & 0xFF;
  color->red = (hexColor >> 16) & 0xFF;
}

void setBgColor(uint32_t hexColor) {
  setRGBColor(&bgColor, hexColor);
}

void setStrokeColor(uint32_t hexColor) {
  setRGBColor(&strokeColor, hexColor);
}

void setFillColor(uint32_t hexColor) {
  setRGBColor(&fillColor, hexColor);
}

void setFontColor(uint32_t hexColor) {
  setRGBColor(&fontColor, hexColor);
}

void saveColorAndSet(RGBColor newColor) {
  savedColor = color;
  color = newColor;
}

void restoreColor() {
  color = savedColor;
}
