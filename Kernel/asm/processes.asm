%include "asm/include/processes_macros.asm"

global createProcessStack
global idleProc
global exit

extern stackAlloc
extern exitProcess


section .text

; -------------------------     FUNCTION     ----------------------------
; Description: Creates new process
; Arguments
;  rdi: argc
;  rsi: argv
;  rdx: pointer to process function
; Return
;  rax: stack pointer for created process
; -----------------------------------------------------------------------
createProcessStack:
  ; This is just so gdb detects this function for the call stack.
  push rbp
  mov rbp, rsp

  ; I need to store the original rsp in a register because I'll loose access to the current
  ; stack when I make the swap for the new process' stack.
  push r11
  mov r11, rsp

  ; I need to preserve all this registers as they may be modified by stackAlloc (or need to
  ; be manually modified like rdi).
  push rdi
  push rsi
  push rdx
  mov rdi, [stackSizePtr]
  call stackAlloc
  pop rdx
  pop rsi
  pop rdi

  ; Move the stack pointer to the allocated memory (this will be the new process' stack)
  ; Note that stackAlloc already reaturns the address of the "end" of the block (as stack
  ; grows upwards) and address aligned.
  mov rsp, rax
  mov rbp, rax
  push 0      ; ss
  push rax    ; original rsp (before the pushes)
  push 0x202  ; rflags
  push 0x8    ; cs
  push rdx    ; rip

  ; Note that rdi and rsi still have the correct values of argc and argv
  initializeGpr

  mov rax, rsp ; Set final rsp value (after pushes) as return value

  ; Return to original stack
  mov rsp, r11
  pop r11
  pop rbp
  ret

; -------------------------     FUNCTION     ----------------------------
; Description: A process created at kernel initialization and which is always ready
; Arguments: None
; Return: None
; -----------------------------------------------------------------------
idleProc:
  hlt
  jmp idleProc

; -------------------------     FUNCTION     ----------------------------
; Description: Exit from process. Process stack and pcb will get cleared.
;              Then call timer tick interruption.
; Arguments
;  rdi: exit code (not currently used)
; Return: doesn't return
; -----------------------------------------------------------------------
exit:
  call exitProcess
  int 0x20


section .rodata
  stackSizePtr dq 0x1000 ; 4kb
