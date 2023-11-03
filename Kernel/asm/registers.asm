section .bss
registers resq 16

section .data
registersQuantity equ 16

global saveRegisters
global getRegistersValues

section .text

; paso los registros por stack
saveRegisters:
    push rbp
    mov rbp, rsp
    push rax
    push rbx
    push rcx
    push rdx

    lea rax, [rbp + 16]     ; guardo en rax la dirección del primer argumento, es decir el rax pusheado anteriormente
    mov rbx, registers      ; guardo la dirección donde almacenaré los registros
    xor rcx, rcx            ; seteo el contador en 0

.loop:
    cmp cl, registersQuantity
    jge .end                ; saltar si cl es >= que 16, que es la cantidad de registros a almacenar

    mov rdx, [rax]          ; desreferencio rax para conseguir el argumento
    mov qword [rbx], rdx    ; lo guardo en registers para usarlo después
    add rax, 8              ; voy al siguiente argumento
    add rbx, 8              ; voy a la siguiente dirección en registers para guardar el sig argumento
    
    inc rcx                 ; incremento el contador
    jmp .loop

.end:

    pop rdx
    pop rcx
    pop rbx
    pop rax
    mov rsp, rbp
    pop rbp
    ret

; uint64_t * getRegistersValues()
getRegistersValues:
    push rbp
    mov rbp, rsp

    mov rax, registers

    mov rsp, rbp
    pop rbp
    ret
    