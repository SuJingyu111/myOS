ORG 0
BITS 16
_start: ; required by BPB of some bios
    jmp short start
    nop

times 33 db 0 ; create a fake BPB in case of BIOS rewrite

start:
    jmp 0x7c0:execute

execute:
    cli ; Clear Interrupts
    mov ax, 0x7c0
    mov ds, ax
    mov es, ax
    mov ax, 0x00
    mov ss, ax
    mov sp, 0x7c00
    sti ; Enable Interrupts

    mov ah, 2 ; Read sector cmd (CHS)
    mov al, 1 ; Read one sector
    mov ch, 0 ; Cylinder low eight bits
    mov cl, 2 ; Read sector 2 (1-indexed)
    mov dh, 0 ; Head number
    mov bx, buffer ; Set dest addr
    int 0x13
    jc error

    mov si, buffer
    call print
    
    jmp $

error:
    mov si, error_message
    call print
    jmp $

print:
    mov bx, 0
.loop:    
    lodsb
    cmp al, 0
    je .done
    call print_char
    jmp .loop
.done:    
    ret

print_char:
    mov ah, 0eh
    int 0x10
    ret


error_message: db 'Failed to load sector', 0

times 510-($ - $$) db 0
dw 0xAA55

buffer: 