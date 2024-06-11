#ifndef SYSCALLS_H
#define SYSCALLS_H

#include <colors.h>
#include <keyboard.h>
#include <pipes.h>
#include <processes.h>
#include <sysinfo.h>

extern void sysHalt();
extern int sysGetTicks();
extern void sysInfo(SystemInfo* si);
extern void sysSetLayout(int layoutIdx);
extern int sysSetFontSize(int fontSize);
extern void sysSetColor(FontColors c, uint32_t hexColor);
extern int sysGetModKeys(ModifierKeys* dest);
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
extern char* sysGetGlobalMemoryState();
extern char* sysGetProcessMemoryState(uint32_t pid);
extern uint32_t sysCreateProcess(int argc, const char* argv[], void* processRip);
extern uint32_t sysCreateProcessWithPipeSwap(int argc, const char* argv[], void* processRip, ProcessPipes pipes);
extern void sysExit(int exitCode);
extern int sysWaitPid(uint32_t pid);
extern PCB* sysPCBList(int* len);
extern int sysSemInit(uint32_t initialValue);
extern int sysCreateSemaphore(char* name, int value);
extern bool sysDestroySemaphore(int semId);
extern bool sysDestroySemaphoreByName(char* name);
extern bool sysWaitSem(int semId);
extern bool sysPostSem(int semId);
extern int sysOpenSem(char* name, int value);
extern void* sysMemcpy(void* destination, const void* source, uint64_t length);
extern uint32_t sysGetPid();
extern bool sysKill(uint32_t pid);
extern void sysSleep(uint64_t ms);
extern void sysChangePriority(uint32_t pid, uint32_t newPriority);
extern uint64_t sysPipeInit();
extern bool sysDestroyPipe(int pipeId);
extern void sysChangePipeRead(int p);
extern void sysChangePipeWrite(int p);
extern bool sysBlock(uint32_t pid);
extern bool sysUnBlock(uint32_t pid);
extern ProcessPipes sysGetPipes();
extern long sysRead(int pipeId, char* buf, int len);
extern long sysWrite(int pipeId, const char* buf, int len);
extern int sysBlockedByUser(uint32_t pid);

#endif
