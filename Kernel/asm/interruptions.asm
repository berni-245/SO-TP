section .text

global disableInterruptions
global enableInterruptions
global haltTillNextInterruption
	
global picMask

global irq00Handler
global irq01Handler
global irq02Handler
global irq03Handler
global irq04Handler
global irq05Handler
global irq06Handler
global irq07Handler

extern irqDispatcher
extern readKeyCode
extern saveRegisters

%macro pushState 0
	push r15
  lea r15, [rsp + 8] ; me guardo el stack pointer
	push r14
	push r13
	push r12
	push r11
	push r10
	push r9
	push r8
	push rsi
	push rdi
  push r15 ; que realmente es el rsp
	push rbp
	push rdx
	push rcx
	push rbx
	push rax
%endmacro

%macro popState 0
	pop rax
	pop rbx
	pop rcx
	pop rdx
	pop rbp
  pop r15 ; será sobrescribido
	pop rdi
	pop rsi
	pop r8
	pop r9
	pop r10
	pop r11
	pop r12
	pop r13
	pop r14
	pop r15
%endmacro



%macro irqHandler 1
  push rax
  mov rdi, %1
  call irqDispatcher
  mov al, 0x20
  out 0x20, al
  pop rax
  iretq
%endmacro

irq00Handler:
  irqHandler 0
irq01Handler:
  push rax
  call readKeyCode
  cmp al, 0x27
  pop rax
  jne .skip
  pushState
  call saveRegisters
  popState
.skip:
  irqHandler 1
irq02Handler:
  irqHandler 2
irq03Handler:
  irqHandler 3
irq04Handler:
  irqHandler 4
irq05Handler:
  irqHandler 5
irq06Handler:
  irqHandler 6
irq07Handler:
  irqHandler 7

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
