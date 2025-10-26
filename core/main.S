[section .boot]
[bits 16]
global boot

boot:
    mov ah, 0x0e
    mov si, msg1
    call ploop

    xor ax, ax
    mov ds, ax
    mov ss, ax
    mov sp, 0x7c00

    cld
    mov ax, 0x2401
    int 0x15

    mov ax, 0x3
    int 0x10
    mov [disk],dl

    mov ax, 0x0800
    mov es, ax

    mov ah, 0x2
    mov al, 64
    mov ch, 0
    mov dh, 0
    mov cl, 2
    mov dl, [disk]
    mov bx, 0
    int 0x13

    mov ax, 0x1000
    mov es, ax

    mov ah, 0x2
    mov al, 64
    mov ch, 1
    mov dh, 1
    mov cl, 12
    mov dl, [disk]
    mov bx, 0x0000
    int 0x13

    cli
    lgdt [gdt_pointer]
    mov eax, cr0
    or eax, 0x1
    mov cr0, eax
    jmp CODE_SEG:b32

print:
    call ploop
    mov si, newl
    call ploop
    mov si, carr
    call ploop
    ret

ploop:
    lodsb
    cmp al, 0
    je return
    int 0x10
    jmp ploop

return:
    ret

gdt_start:
    dq 0x0

gdt_code:
    dw 0xFFFF
    dw 0x0
    db 0x0
    db 10011010b
    db 11001111b
    db 0x0

gdt_data:
    dw 0xFFFF
    dw 0x0
    db 0x0
    db 10010010b
    db 11001111b
    db 0x0

gdt_end:
gdt_pointer:
    dw gdt_end - gdt_start
    dd gdt_start

disk:
    db 0x0

CODE_SEG equ gdt_code - gdt_start
DATA_SEG equ gdt_data - gdt_start

newl db 0x0A, 0
carr db 0X0D, 0

msg1 db "Entering protected mode...", 0
msg2 db "Setting up basic stack...  ", 0
msg3 db "Welcome to AriaOS!", 0

times 510-($-$$) db 0
dw 0xAA55

[bits 32]
[section .text.start]

b32:
    mov ax, DATA_SEG
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    mov ebx, 0xb8000
    mov eax, 0x0F00
    mov esi, msg2
    call pl32
    mov eax, 0x0200
    mov esi, msg3
    call pl32
    jmp halt

pl32:
    lodsb
    or al, al
    jz ret32
    mov word [ebx], ax
    add ebx, 2
    jmp pl32

halt:
    mov esp, kernel_stack_top
    extern _start
    extern __bss_start
    extern __bss_sizel
    mov ecx, __bss_sizel
    mov edi, __bss_start
    xor eax, eax
    rep stosd
    call _start
    cli
    hlt

ret32:
    ret

[section .bss]
[sectalign 4]

kernel_stack_bottom:
    resb 16384
kernel_stack_top: