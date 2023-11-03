section .text

global sysHalt
global sysMs
global sysInfo
global sysSetLayout
global sysSetFontSize
global sysRead
global sysWrite
global sysWriteCharXY
global sysWriteCharNext
global sysMoveCursor

%macro syscall 1
  mov r8, %1
  int 0x80
  ret
%endmacro

sysHalt:
  syscall 0
sysMs:
  syscall 1
sysInfo:
  syscall 2
sysSetLayout:
  syscall 3
sysSetFontSize:
  syscall 4
sysRead:
  syscall 5
sysWriteCharXY:
  syscall 6
sysWriteCharNext:
  syscall 7
sysMoveCursor:
  syscall 8
