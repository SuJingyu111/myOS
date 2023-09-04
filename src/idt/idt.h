#ifndef IDT_H
#define IDT_H

#include <stdint.h>


struct idt_desc
{
    uint16_t offset_1;        // offset bits 0..15
    uint16_t selector;        // a code segment selector in GDT or LDT
    uint8_t  zero;            // unused, set to 0
    uint8_t  type_attributes; // gate type, dpl, and p fields
    uint16_t offset_2;        // offset bits 16..31
} __attribute__((packed));

struct idtr_desc
{
    uint16_t limit; 
    uint32_t base;
} __attribute__((packed));

// Init the iterrupt descriptor table
void idt_init();


#endif