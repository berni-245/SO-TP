GLOBAL _enter_region
GLOBAL _leave_region


_enter_region:
  mov rax,1
  xchg rax,[rdi]
  ret
    
_leave_region:
   mov qword [rdi], 0
   ret
