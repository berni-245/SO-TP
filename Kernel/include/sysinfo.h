#ifndef SYSINFO_H
#define SYSINFO_H

typedef struct SystemInfo {
  int32_t screenWidth;
  int32_t screenHeight;
  int32_t charWidth;
  int32_t charHeight;
  int32_t layout;
  int32_t fontSize;
  int32_t charSeparation;
} SystemInfo;

void getSysInfo(SystemInfo* si);

#endif
