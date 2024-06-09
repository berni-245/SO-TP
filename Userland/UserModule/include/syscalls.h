#ifndef SYSCALLS_H
#define SYSCALLS_H

#include <colors.h>
#include <keyboard.h>
#include <processes.h>
#include <sysinfo.h>

extern void sysHalt();
extern int sysGetTicks();
extern void sysInfo(SystemInfo* si);
extern void sysSetLayout(int layoutIdx);
extern int sysSetFontSize(int fontSize);
extern void sysSetColor(FontColors c, uint32_t hexColor);
extern int sysRead(KeyStruct* buf, int len);
extern int sysWrite(int col, int row, char c);
extern int sysWriteCharXY(int x, int y, char c, int fontSize);
extern void sysWriteCharNext(char c);
extern int sysMoveCursor(int col, int row);
extern void sysPrintPixel(int x, int y, RGBColor color);
extern void sysFillRectangle(int x, int y, int width, int height, RGBColor color);
extern void sysPlaySound(uint32_t nFrequence, int ms);
extern void sysGetCurrentTime(Time* currentTime);
extern void sysGetRegisters(Register* registers);
extern void* sysMalloc(uint64_t size);
extern void sysFree(void* ptr);
extern char* sysGetMemoryState();
extern uint32_t sysCreateProcess(int argc, const char* argv[], void* processRip);
extern void sysExit(int exitCode);
extern int sysWaitPid(uint32_t pid);
extern PCB* sysPCBList(int* len);
extern int sysCreateSemaphore(char* name, int value);
extern int sysDestroySemaphore(char* name);
extern int sysWaitSem(int sem_id);
extern int sysPostSem(int sem_id);
extern int sysOpenSem(char* name, int value);
extern void* sysMemcpy(void* destination, const void* source, uint64_t length);
extern uint32_t sysGetPid();
extern bool sysKill(uint32_t pid);
extern void sysSleep(uint64_t ms);
extern void sysChangePriority(uint32_t pid, uint32_t newPriority);

#endif
