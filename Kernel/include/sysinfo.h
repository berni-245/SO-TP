#ifndef SYSINFO_H
#define SYSINFO_H

typedef struct SystemInfo {
  int screenWidth;
  int screenHeight;
  int charWidth;
  int charHeight;
  int layout;
  int fontSize;
} SystemInfo;

void getSysInfo(SystemInfo* si);


#endif
