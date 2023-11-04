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
