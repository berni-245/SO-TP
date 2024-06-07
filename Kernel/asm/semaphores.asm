GLOBAL _enter_region
GLOBAL _leave_region


_enter_region:
  mov eax,1
  xchg eax,[rdi]
  ret
    
_leave_region:
   mov dword [rdi], 0
   ret
