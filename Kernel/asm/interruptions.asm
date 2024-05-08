%include "asm/include/interruptions_macros.asm"

section .text

global disableInterruptions
global enableInterruptions
global haltTillNextInterruption
	
global picMask

global timerTickIrqHandler
global keyboardIrqHandler

global exception00Handler
global exception01Handler

extern irqDispatcher
extern readKeyCode
extern saveRegisters
extern exceptionDispatcher
extern getStackBase



timerTickIrqHandler:
  irqHandler 0

keyboardIrqHandler:
  push rax
  call readKeyCode
  cmp al, 0x3b ; f1 para sacar captura de los registros
  pop rax
  jne .skip
  pushState
  push qword normalRegistersCode
  call saveRegisters
  pop rax
  popState
.skip:
  irqHandler 1

exception00Handler:
  exceptionHandler 0
exception01Handler:
  exceptionHandler 1

disableInterruptions:
	cli
	ret

enableInterruptions:
	sti
	ret

haltTillNextInterruption:
  sti
  hlt
  ret

picMask:
	; push rbp
  ; mov rbp, rsp

  mov ax, di
  out	0x21, al
  shr ax, 8
  out	0xA1, al

  ; mov rsp, rbp
  ; pop rbp
  retn  ; return near: returns to address in same code segment.
        ; retf -> return far: can change code segment. Not used in
        ; modern systems as they use a single code segment.
        ; ret: compiler decides which of the above should be used. Basically same as retn


section .rodata
  normalRegistersCode equ 1
  exceptionRegistersCode equ 0
  userland equ 0x400000
