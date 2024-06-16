#ifndef SYSCALLS_H
#define SYSCALLS_H

#include <colors.h>
#include <keyboard.h>
#include <pipes.h>
#include <processes.h>
#include <sysinfo.h>

extern void sysHalt();
extern int32_t sysGetTicks();
extern void sysInfo(SystemInfo* si);
extern void sysSetLayout(int32_t layoutIdx);
extern int32_t sysSetFontSize(int32_t fontSize);
extern void sysSetColor(FontColors c, uint32_t hexColor);
extern int32_t sysGetModKeys(ModifierKeys* dest);
extern int32_t sysWriteCharXY(int32_t x, int32_t y, char c, int32_t fontSize);
extern void sysWriteCharNext(char c);
extern int32_t sysMoveCursor(int32_t col, int32_t row);
extern void sysPrintPixel(int32_t x, int32_t y, RGBColor color);
extern void sysFillRectangle(int32_t x, int32_t y, int32_t width, int32_t height, RGBColor color);
extern void sysPlaySound(uint32_t nFrequence, int32_t ms);
extern void sysGetCurrentTime(Time* currentTime);
extern void sysGetRegisters(Register* registers);
extern void* sysMalloc(uint64_t size);
extern void sysFree(void* ptr);
extern char* sysGetGlobalMemoryState();
extern char* sysGetProcessMemoryState(uint32_t pid);
extern int32_t sysCreateProcess(int32_t argc, const char* argv[], void* processRip);
extern int32_t sysCreateProcessWithPipeSwap(int32_t argc, const char* argv[], void* processRip, ProcessPipes pipes);
extern void sysExit(int32_t exitCode);
extern int32_t sysWaitPid(uint32_t pid);
extern PCB* sysPCBList(int* len);
extern int32_t sysSemInit(uint32_t initialValue);
extern int32_t sysCreateSemaphore(char* name, int32_t value);
extern bool sysDestroySemaphore(int32_t semId);
extern bool sysDestroySemaphoreByName(char* name);
extern bool sysWaitSem(int32_t semId);
extern bool sysPostSem(int32_t semId);
extern int32_t sysOpenSem(char* name, int32_t value);
extern void* sysMemcpy(void* destination, const void* source, uint64_t length);
extern uint32_t sysGetPid();
extern bool sysKill(uint32_t pid);
extern void sysSleep(uint64_t ms);
extern void sysChangePriority(uint32_t pid, uint32_t newPriority);
extern uint64_t sysPipeInit();
extern bool sysDestroyPipe(int32_t pipeId);
extern void sysChangePipeRead(int32_t p);
extern void sysChangePipeWrite(int32_t p);
extern int32_t sysBlockByUser(uint32_t pid);
extern bool sysUnblock(uint32_t pid);
extern ProcessPipes sysGetPipes();
extern int64_t sysRead(int32_t pipeId, char* buf, int32_t len);
extern int64_t sysWrite(int32_t pipeId, const char* buf, int32_t len);
extern void sysYield();

#endif
