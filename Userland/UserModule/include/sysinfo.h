#ifndef SYSINFO_H
#define SYSINFO_H

typedef enum KbLayout {
  QWERTY_LATAM = 0,
  QWERTY_US,
} KbLayout;
static const char* const LayoutStrings[] = {
  [QWERTY_LATAM] = "QWERTY LATAM",
  [QWERTY_US] = "QWERTY US",
};

typedef struct SystemInfo {
  int screenWidth;
  int screenHeight;
  int charWidth;
  int charHeight;
  int layout;
  int fontSize;
} SystemInfo;

typedef enum {
  BACKGROUND = 0,
  STROKE,
  FILL,
  FONT,
} ColorType;

extern SystemInfo systemInfo;

void incFont();
void decFont();
void setLayout(KbLayout code);

#endif
