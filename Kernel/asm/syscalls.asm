section .text

extern getSyscallsArray

global syscallDispatcher

; -------------------------     FUNCTION     ----------------------------
; Arguments
;  r8: syscall index
;  rdi: first syscall arg   (optoinal)
;  rsi: second syscall arg  (optoinal)
;  rdx: third syscall arg   (optoinal)
;  rcx: fourth syscall arg   (optoinal)
; Return:
;  rax (optional)
; -----------------------------------------------------------------------
syscallDispatcher:
  call getSyscallsArray
  lea rax, [rax + r8*8]
  call [rax]
  iretq
