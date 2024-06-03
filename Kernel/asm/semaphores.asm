GLOBAL _enter_region
GLOBAL _leave_region


_enter_region:
  mov rax,1
  xchg rax,[rdi]
  cmp rax,0
  je _exit
  int 22h
  jmp _enter_region
_exit:
  ret
    
_leave_region:
   mov qword [rdi], 0
   ret
