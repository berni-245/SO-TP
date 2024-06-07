GLOBAL _enter_region
GLOBAL _leave_region


_enter_region:
  mov eax,1
  xchg eax,[rdi]
  cmp eax, 0
  je .exit
  int 0x22
  jmp _enter_region
.exit:
  ret
    
_leave_region:
   mov dword [rdi], 0
   ret
