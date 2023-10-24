global waitForKey

section .text

waitForKey:
  call keyPressed
  cmp al, 1
  jne waitForKey
  call keyCode
  ret

keyCode:
  ; push rbp
  ; mov rbp, rsp
  xor rax, rax
  in al, 0x60
  ; mov rsp, rbp
  ; pop rbp
  ret

keyPressed:
  ; push rbp
  ; mov rbp, rsp
  xor rax, rax
  in al, 0x64
  and al, 0b00000001
  ; mov rsp, rbp
  ; pop rbp
  ret

