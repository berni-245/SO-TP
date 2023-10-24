GLOBAL cpuVendor
GLOBAL outb
GLOBAL inb
GLOBAL setBinaryClockFormat
GLOBAL getCurrentSeconds
GLOBAL getCurrentMinutes
GLOBAL getCurrentHours 

section .text
global cpuVendor
	
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

setBinaryClockFormat:
	push rbp
	mov rbp, rsp

	mov rax, 0x0B ; consigo el RTC Status Register B
	out 70h, al
	mov rax, 0b00000110 ; le cambio la configuración para devuelva la hora en formato binario en lugar de BCD
	out 71h, al

	mov rsp, rbp
	pop rbp
	ret

getCurrentSeconds:
	push rbp
	mov rbp, rsp

	mov rax, 0x00
	out 70h, al
	in al, 71h

	mov rsp, rbp
	pop rbp
	ret

getCurrentMinutes:
	push rbp
	mov rbp, rsp

	mov rax, 0x02
	out 70h, al
	in al, 71h

	mov rsp, rbp
	pop rbp
	ret

getCurrentHours:
	push rbp
	mov rbp, rsp
	push rdx
	mov rdx, 24

	mov rax, 0x04
	out 70h, al
	in al, 71h
	sub al, 3 ; para que de el horario de argentina
	cmp al, 0
	jge .fin
	add al, 24
.fin:
	pop rdx
	mov rsp, rbp
	pop rbp
	ret