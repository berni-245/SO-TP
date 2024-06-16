#ifndef SYSINFO_H
#define SYSINFO_H

#include <stdint.h>

typedef enum KbLayout {
  QWERTY_LATAM = 0,
  QWERTY_US,
} KbLayout;
static const char* const LayoutStrings[] = {
    [QWERTY_LATAM] = "QWERTY LATAM",
    [QWERTY_US] = "QWERTY US",
};
#define TIME_STR_LEN 9
#define REGISTER_STR_LEN 4
#define REGISTER_QUANTITY 17

typedef struct Time {
  uint8_t hours;
  uint8_t minutes;
  uint8_t seconds;
  char string[TIME_STR_LEN];
} Time;

typedef struct Register {
  uint64_t value;
  char name[REGISTER_STR_LEN];
} Register;

typedef struct SystemInfo {
  int32_t screenWidth;
  int32_t screenHeight;
  int32_t charWidth;
  int32_t charHeight;
  int32_t layout;
  int32_t fontSize;
  int32_t charSeparation;
  int32_t fontCols;
  int32_t fontRows;
} SystemInfo;

extern SystemInfo systemInfo;

void getSysInfo();
void setFontSize(int32_t size);
void setLayout(KbLayout code);

#endif
