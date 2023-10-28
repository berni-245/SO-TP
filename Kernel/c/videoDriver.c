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

typedef struct RGBColor {
  uint8_t blue;
  uint8_t green;
  uint8_t red;
} RGBColor;

static RGBColor color = {0};
static RGBColor bgColor = {0};
static RGBColor strokeColor = {0};
static RGBColor fillColor = {0};
static RGBColor fontColor = {0};
static RGBColor savedColor = {0};

typedef struct vbe_mode_info_structure *VBEInfoPtr;

VBEInfoPtr VBE_mode_info = (VBEInfoPtr)0x0000000000005C00;

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
  saveColor();
  color = strokeColor;
  int vx = endX - startX;
  int vy = endY - startY;
  for (double a = 0; a <= 1; a += 0.0001) {
    int x = a * vx + startX;
    int y = a * vy + startY;
    printRectangle(x, y, strokeWidth, strokeWidth);
  }
  restoreColor();
}

void strokeRectangle(int x, int y, int width, int height) {
  strokeLine(x, y, x + width, y);
  strokeLine(x + width, y, x + width, y + height);
  strokeLine(x + width, y + height, x, y + height);
  strokeLine(x, y + height, x, y);
}

void fillRectangle(int x, int y, int width, int height) {
  saveColor();
  color = fillColor;
  printRectangle(x, y, width, height);
  strokeRectangle(x, y, width, height);
  restoreColor();
}

void clearScreen() {
  saveColor();
  color = bgColor;
  printRectangle(0, 0, VBE_mode_info->width, VBE_mode_info->height);
  restoreColor();
}

#define MAX_FONT_SIZE 4
static int fontSize = 1;
void increaseFont() {
  if (fontSize < MAX_FONT_SIZE)
    ++fontSize;
}
void decreaseFont() {
  if (fontSize > 1)
    --fontSize;
}

void printChar(int x, int y, char c) {
  if (c < ASCII_BF_MIN || c > ASCII_BF_MAX) return;
  saveColor();
  color = fontColor;
  c -= ASCII_BF_MIN;
  x *= ASCII_BF_WIDTH * fontSize;
  y *= ASCII_BF_HEIGHT * fontSize;
  for (int i = 0; i < ASCII_BF_HEIGHT; ++i) {
    for (int j = 0; j < ASCII_BF_WIDTH; ++j) {
      if (asciiBitFields[c][i * ASCII_BF_WIDTH + j] != 0) {
        printRectangle(x + j*fontSize, y + i*fontSize, fontSize, fontSize);
      }
    }
  }
  restoreColor();
}

void printBuffer(int x, int y, char buf[], int size) {
  for (int i = 0; i < size; ++i) {
    printChar(x + i, y, buf[i]);
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

void saveColor() {
  savedColor = color;
}

void restoreColor() {
  color = savedColor;
}
