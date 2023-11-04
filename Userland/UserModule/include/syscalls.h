#ifndef SYSCALLS_H
#define SYSCALLS_H

#include <keyboard.h>
#include <sysinfo.h>

extern void sysHalt();
extern int sysMs();
extern void sysInfo(SystemInfo* si);
extern void sysSetLayout(int layoutIdx);
extern void sysSetFontSize(int fontsize);
extern void sysSetColor(ColorType c, uint32_t hexColor);
extern int sysRead(KeyStruct* buf, int len);
extern int sysWrite(int x, int y, char c);
extern int sysWriteCharXY(int x, int y, char c);
extern int sysWriteCharNext(char c);
extern int sysMoveCursor(int col, int row);


#endif
