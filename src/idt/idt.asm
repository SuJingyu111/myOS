section .asm

extern int21h_handler
extern idt_zero
extern no_interrupt_handler

global idt_load
global int21h
global int0h
global no_interrupt
global enable_interrupts
global disable_interrupts

disable_interrupts:
    cli
    ret

enable_interrupts:
    sti
    ret

idt_load:
    push ebp
    mov ebp, esp

    mov ebx, [ebp+8]
    lidt [ebx]

    pop ebp
    ret

int0h: ; Divide by 0 interrupt
    cli
    pushad
    call idt_zero
    popad
    sti
    iret

int21h: ; keyboard interrupt
    cli
    pushad
    call int21h_handler
    popad
    sti
    iret

no_interrupt:
    cli
    pushad
    call no_interrupt_handler
    popad
    sti
    iret
