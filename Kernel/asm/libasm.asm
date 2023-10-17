GLOBAL cpuVendor
GLOBAL outb
GLOBAL inb

section .text
	
cpuVendor:
	push rbp
	mov rbp, rsp

	push rbx

	mov rax, 0
	cpuid


	mov [rdi], ebx
	mov [rdi + 4], edx
	mov [rdi + 8], ecx

	mov byte [rdi+13], 0

	mov rax, rdi

	pop rbx

	mov rsp, rbp
	pop rbp
	ret

outb:
	push rbp
	mov rbp, rsp
	push rdx
	push rax

	mov rdx, rdi
	mov rax, rsi
	out dx, al

	pop rax
	pop rdx
	mov rsp, rbp
	pop rbp

	ret

inb:
	push rbp
	mov rbp, rsp
	push rdx
	mov rax, 0
	mov rdx, rdi

	in al, dx

	pop rdx 
	mov rsp, rbp
	pop rbp

	ret