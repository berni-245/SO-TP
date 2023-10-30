section .text

extern getSyscallsArray

global syscallDispatcher

; -------------------------     FUNCTION     ----------------------------
; Arguments
;  r8: syscall index
;  rdi: first syscall arg   (optional)
;  rsi: second syscall arg  (optional)
;  rdx: third syscall arg   (optional)
;  rcx: fourth syscall arg   (optional)
; Return:
;  rax (optional)
; -----------------------------------------------------------------------
syscallDispatcher:
  call getSyscallsArray
  lea rax, [rax + r8*8]
  call [rax]
  iretq
