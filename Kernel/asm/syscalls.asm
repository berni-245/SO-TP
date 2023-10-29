section .text

extern syscallDispatcher

global syscallHandler

; -------------------------     FUNCTION     ----------------------------
; Arguments
;  rdi: first syscall arg   (optoinal)
;  rsi: second syscall arg  (optoinal)
;  rdx: third syscall arg   (optoinal)
;  rcx: fourth syscall arg   (optoinal)
;  r8: syscall index
; -----------------------------------------------------------------------
syscallHandler:
  push rax
  call syscallDispatcher
  ; mov al, 0x20
  ; out 0x20, al
  pop rax
  iretq
