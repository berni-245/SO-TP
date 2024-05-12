; Watch out!!! Modifiying this file isn't detected by the Makefile so a 
; sudo make clean is necessary for changes to take effect.

%macro pushGpr 0
  push rax
  push rbx
  push rcx
  push rdx
  push rsi
  push rdi
  push rbp
  push r8
  push r9
  push r10
  push r11
  push r12
  push r13
  push r14
  push r15
%endmacro

%macro popGpr 0
  pop r15
  pop r14
  pop r13
  pop r12
  pop r11
  pop r10
  pop r9
  pop r8
  pop rbp
  pop rdi
  pop rsi
  pop rdx
  pop rcx
  pop rbx
  pop rax
%endmacro

%macro initializeGpr 0
  push 0
  push 0
  push 0
  push 0
  push rsi  ; argv
  push rdi  ; argc
  push rbp
  push 0
  push 0
  push 0
  push 0
  push 0
  push 0
  push 0
  push 0
%endmacro
