section .text

global sysHalt
global sysGetTicks
global sysInfo
global sysSetLayout
global sysSetFontSize
global sysSetColor
global sysRead
global sysWrite
global sysWriteCharXY
global sysWriteCharNext
global sysMoveCursor
global sysPrintPixel
global sysFillRectangle
global sysPlaySound
global sysGetCurrentTime
global sysGetRegisters
global sysMalloc
global sysFree
global sysGetMemoryState
global sysCreateProcess
global sysExit
global sysWaitPid
global sysPCBList
global sysCreateSemaphore
global sysDestroySemaphore
global sysWaitSem
global sysPostSem
global sysOpenSem
global sysMemcpy
global sysGetPid
global sysKill
global sysSleep

%macro syscall 1
  mov r9, %1
  int 0x80
  ret
%endmacro


sysHalt:
  syscall 0
sysGetTicks:
  syscall 1
sysInfo:
  syscall 2
sysSetLayout:
  syscall 3
sysSetFontSize:
  syscall 4
sysSetColor:
  syscall 5
sysRead:
  syscall 6
sysWriteCharXY:
  syscall 7
sysWriteCharNext:
  syscall 8
sysMoveCursor:
  syscall 9
sysPrintPixel:
  syscall 10
sysFillRectangle:
  syscall 11
sysPlaySound:
  syscall 12
sysGetCurrentTime:
  syscall 13
sysGetRegisters:
  syscall 14
sysMalloc:
  syscall 15
sysFree:
  syscall 16
sysGetMemoryState:
  syscall 17
sysCreateProcess:
  syscall 18
sysExit:
  syscall 19
sysWaitPid:
  syscall 20
sysPCBList:
  syscall 21
sysCreateSemaphore:
  syscall 22
sysDestroySemaphore:
  syscall 23
sysWaitSem:
  syscall 24
sysPostSem:
  syscall 25
sysOpenSem:
  syscall 26
sysMemcpy:
  syscall 27
sysGetPid:
  syscall 28
sysKill:
  syscall 29
sysSleep:
  syscall 30
