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
