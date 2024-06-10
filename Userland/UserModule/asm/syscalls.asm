section .text

global sysHalt
global sysGetTicks
global sysInfo
global sysSetLayout
global sysSetFontSize
global sysSetColor
global sysGetKey
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
global sysSemInit
global sysCreateSemaphore
global sysDestroySemaphore
global sysDestroySemaphoreByName
global sysWaitSem
global sysPostSem
global sysOpenSem
global sysMemcpy
global sysGetPid
global sysKill
global sysSleep
global sysChangePriority
global sysChangePipeRead
global sysChangePipeWrite
global sysRead
global sysWrite
global sysCreateProcessWithPipeSwap
global sysBlock
global sysUnBlock

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
sysGetKey:
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
sysCreateProcessWithPipeSwap:
  syscall 19
sysExit:
  syscall 20
sysWaitPid:
  syscall 21
sysPCBList:
  syscall 22
sysSemInit:
  syscall 23
sysCreateSemaphore:
  syscall 24
sysDestroySemaphore:
  syscall 25
sysDestroySemaphoreByName:
  syscall 26
sysWaitSem:
  syscall 27
sysPostSem:
  syscall 28
sysOpenSem:
  syscall 29
sysMemcpy:
  syscall 30
sysGetPid:
  syscall 31
sysKill:
  syscall 32
sysSleep:
  syscall 33
sysChangePriority:
  syscall 34
sysChangePipeRead:
  syscall 35
sysChangePipeWrite:
  syscall 36
sysRead:
  syscall 37
sysWrite:
  syscall 38
sysBlock:
  syscall 39
sysUnBlock:
  syscall 40
