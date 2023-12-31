#include "kheap.h"
#include "heap.h"
#include "config.h"
#include "kernel.h"
#include "memory/memory.h"

struct heap kernel_heap;
struct heap_table kernel_heap_table;

void kheap_init()
{
    uint32_t total_heap_entries = MYOS_HEAP_SIZE_BYTES / MYOS_HEAP_BLOCK_SIZE;
    kernel_heap_table.entries = (HEAP_BLOCK_TABLE_ENTRY*) MYOS_HEAP_TABLE_ADDR;
    kernel_heap_table.total = total_heap_entries;

    void* end = (void* )(MYOS_HEAP_ADDR + MYOS_HEAP_SIZE_BYTES);
    int res = heap_create(&kernel_heap, (void*)MYOS_HEAP_ADDR, end, &kernel_heap_table);
    if (res < 0) {
        print("Failed to create kernel heap\n");
    }
}

void* kzalloc(size_t size)
{
    void* addr = kmalloc(size);
    if (!addr)
        return 0;

    memset(addr, 0x00, size);
    return addr;
}

void* kmalloc(size_t size)
{
    return heap_malloc(&kernel_heap, size);
}

void kfree(void* ptr)
{
    heap_free(&kernel_heap, ptr);
}