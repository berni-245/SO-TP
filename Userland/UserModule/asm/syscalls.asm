section .text

global sysHalt
global sysMs
global sysInfo
global sysSetLayout
global sysSetFontSize
global sysSetColor
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
