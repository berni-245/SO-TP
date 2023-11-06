#ifndef SYSCALLS_H
#define SYSCALLS_H

#include <colors.h>
#include <keyboard.h>
#include <sysinfo.h>

extern void sysHalt();
extern int sysMs();
extern void sysInfo(SystemInfo* si);
extern void sysSetLayout(int layoutIdx);
extern int sysSetFontSize(int fontSize);
extern void sysSetColor(FontColors c, uint32_t hexColor);
extern int sysRead(KeyStruct* buf, int len);
extern int sysWrite(int x, int y, char c);
extern int sysWriteCharXY(int x, int y, char c);
extern int sysWriteCharNext(char c);
extern int sysMoveCursor(int col, int row);
void sysPrintPixel(int x, int y, RGBColor color);
void sysFillRectangle(int x, int y, int width, int height, RGBColor color);

#endif
