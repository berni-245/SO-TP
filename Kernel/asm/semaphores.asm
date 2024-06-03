GLOBAL _enter_region
GLOBAL _leave_region


_enter_region:
  mov rax,1
  xchg rax,[rsp+8]
  cmp rax,0
  jne _enter_region
  ret
    
_leave_region:
   mov qword [rsp+8], 0
   ret
