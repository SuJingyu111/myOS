#include "idt.h"
#include "config.h"
#include "kernel.h"
#include "memory/memory.h"

struct idt_desc idt_descriptors[MYOS_TOTAL_INTERRUPTS];
struct idtr_desc idtr_descriptor;

extern void idt_load(struct idtr_desc* ptr);

void idt_zero()
{
    print("ERROR: Divide by zero\n");
}

void idt_set(int interrupt_idx, void* address) // Note address is the address of an interrupt, not int_desc
{
    struct idt_desc* desc = &idt_descriptors[interrupt_idx];
    desc->offset_1 = (uint32_t) address & 0x0000ffff;
    desc->selector = KERNEL_CODE_SELECTOR;
    desc->zero = 0x00;
    desc->type_attributes  = 0xEE; // 11101110 -> used, ring3, storage segment set to 0 for interrupts, high 16 bits indicating gate type is interrupt gate
    desc->offset_2 = (uint32_t) address >> 16;
}

void idt_init()
{
    memset(idt_descriptors, 0, sizeof(idt_descriptors));
    idtr_descriptor.limit = sizeof(idt_descriptors) - 1;
    idtr_descriptor.base = (uint32_t) idt_descriptors;

    idt_set(0, idt_zero);

    // load the interrupt descriptor table
    idt_load(&idtr_descriptor);
}

