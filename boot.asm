ORG 0x7c00
BITS 16

CODE_SEG equ gdt_code - gdt_start
DATA_SEG equ gdt_data - gdt_start

_start: ; required by BPB of some bios
    jmp short start
    nop

times 33 db 0 ; create a fake BPB in case of BIOS rewrite

start:
    jmp 0:execute

execute:
    cli ; Clear Interrupts
    mov ax, 0x00
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x7c00
    sti ; Enable Interrupts

.load_protected: 
    cli            ; disable interrupts
    lgdt [gdt_descriptor]    ; load GDT register with start address of Global Descriptor Table
    mov eax, cr0 
    or al, 1       ; set PE (Protection Enable) bit in CR0 (Control Register 0)
    mov cr0, eax
    jmp CODE_SEG:load32

; GDT
gdt_start:
gdt_null: 
    dd 0x0
    dd 0x0

; offset 0x8
gdt_code:     ; CS SHOULD POINT to this
    dw 0xffff ; Segment limit first 0-15 bits (limit is maximum addressible unit, if choose paging and 4kb granularity, set limit=0xfffff(yes 20 bits), address spans entire 4GB addr space)
    dw 0      ; Base first 0-15 bits (Base addr of the segment)
    db 0      ; Base 16-23 bits
    db 0x9a   ; Access byte (Access bit mask)
    db 11001111b ; High and low 4 bit flags
    db 0      ; Base 24-31 bits

; Offset 0x10
gdt_data:      ; DS, SS, ES, FS, GS SHOULD POINT TO THIS
    dw 0xffff ; Segment limit first 0-15 bits (limit is maximum addressible unit, if choose paging and 4kb granularity, set limit=0xfffff(yes 20 bits), address spans entire 4GB addr space)
    dw 0      ; Base first 0-15 bits (Base addr of the segment)
    db 0      ; Base 16-23 bits
    db 0x92   ; Access byte (Access bit mask)
    db 11001111b ; High and low 4 bit flags
    db 0      ; Base 24-31 bits

gdt_end:

gdt_descriptor:
    dw gdt_end - gdt_start - 1
    dd gdt_start

[BITS 32]
load32:
    mov ax, DATA_SEG
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    mov ebp, 0x00200000
    mov esp, ebp
    jmp $

times 510-($ - $$) db 0
dw 0xAA55
