#include "kernel.h"
#include <stdint.h>
#include <stddef.h>
#include "idt/idt.h"
#include "io/io.h"
#include "memory/heap/kheap.h"
#include "memory/paging/paging.h"

uint16_t* video_mem = 0;
uint16_t terminal_row = 0;
uint16_t terminal_col = 0;

uint16_t terminal_make_char(char c, char color)
{
    return (color << 8) | c;
}

void terminal_putchar(int x, int y, char c, char color)
{
    video_mem[y * VGA_WIDTH + x] = terminal_make_char(c, color);
}

void terminal_print_char(char c, char color)
{
    if (c == '\n') {
        terminal_col = 0;
        terminal_row++;
    } else {
        terminal_putchar(terminal_col, terminal_row, c, color);
        terminal_col = (terminal_col + 1) % VGA_WIDTH;
        if (terminal_col == 0)
        {
            terminal_row++;
        }
    }
}

void terminal_initialize()
{
    video_mem = (uint16_t*)(0xB8000);
    for (int y = 0; y < VGA_HEIGHT; y++)
    {
        for (int x = 0; x < VGA_WIDTH; x++)
        {
            terminal_putchar(x, y, ' ', 0);
        }
    }
}

size_t strlen(const char* str)
{
    size_t len = 0;
    while (str[len] != 0)
    {
        len++;
    }
    return len;
}

void print(const char* str)
{
    size_t len = strlen(str);
    for (int i = 0; i < len; i++) {
        terminal_print_char(str[i], 15);
    }
}

static struct paging_4gb_chunk* kernel_chunk = NULL;

void kernel_main()
{
    terminal_initialize();
    char* hello_word = "Hello World!\nThis is my first kernel!\n";
    print(hello_word);

    // Initalize the heap
    kheap_init();

    // Initialize the iterrupt descriptor table
    idt_init();

    // Setup paging
    kernel_chunk = paging_new_4gb(PAGING_IS_WRITABLE | PAGING_IS_PRESENT | PAGING_ACCESS_FROM_ALL);

    // Switch to kernel paging chunk
    paging_switch(paging_4gb_chunk_get_directory(kernel_chunk));

    char* ptr = kzalloc(4096);
    paging_set(paging_4gb_chunk_get_directory(kernel_chunk), (void*)0x1000, (uint32_t)ptr | PAGING_ACCESS_FROM_ALL | PAGING_IS_PRESENT | PAGING_IS_WRITABLE);
    ptr[0] = 'a';

    // Enable paging
    enable_paging();

    char* ptr2 = (char *)0x1000;
    ptr2[0] = 'A';
    ptr2[1] = 'B';
    print(ptr2);

    print("Content of ptr:");
    print(ptr);

    // Enable interrupts
    enable_interrupts();
}