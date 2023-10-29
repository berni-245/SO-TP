section .text

global test

test:
  mov r8, 0
  int 0x80
  ret
