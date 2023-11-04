#include <stdlib.h>
#include <syscalls.h>
#include <sysinfo.h>

SystemInfo systemInfo;

void incFont() {
  sysSetFontSize(++systemInfo.fontSize);
}
void decFont() {
  sysSetFontSize(--systemInfo.fontSize);
}

void setLayout(KbLayout layoutCode) {
  systemInfo.layout = layoutCode;
  sysSetLayout(layoutCode);
}

char* getLayoutName(KbLayout code) {
  switch (code) {
    case QWERTY_LATAM: 
      return "QWERTY Latam";
    case QWERTY_US:
      return "QWERTY Us";
    default:
      return NULL;
  }
}
