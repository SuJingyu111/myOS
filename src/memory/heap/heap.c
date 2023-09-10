#include "heap.h"
#include "kernel.h"
#include "status.h"
#include "memory/memory.h"
#include <stdbool.h>

static bool heap_validate_table(void* ptr, void* end, struct heap_table* table)
{
    int res = 0;

    size_t heap_size = (size_t)(end - ptr);
    size_t total_blocks = heap_size / MYOS_HEAP_BLOCK_SIZE;
    if (table->total != total_blocks)
    {
        res = -EINVARG;
        goto bail;
    }

bail:
    return res;
}

static bool heap_validate_alignment(void* ptr)
{
    return ((unsigned int)ptr % MYOS_HEAP_BLOCK_SIZE) == 0;
}

int heap_create(struct heap* heap, void* ptr, void* end, struct heap_table* table)
{
    int res = 0;

    if (!heap_validate_alignment(ptr) || !heap_validate_alignment(end))
    {
        res = -EINVARG;
        goto bail;
    }

    memset(heap, 0, sizeof(struct heap));
    heap->saddr = ptr;
    heap->table = table;

    res = heap_validate_table(ptr, end, table);
    if (res < 0)
    {
        goto bail;
    }

    size_t table_size = sizeof(HEAP_BLOCK_TABLE_ENTRY) * table->total;
    memset(table->entries, HEAP_BLOCK_TABLE_ENTRY_FREE, table_size);

bail:
    return res;
}

// Align requested size to block size
static uint32_t heap_align_size(uint32_t size)
{
    return size - (size % MYOS_HEAP_BLOCK_SIZE) + ((size % MYOS_HEAP_BLOCK_SIZE) == 0 ? 0 : MYOS_HEAP_BLOCK_SIZE);
}

static int heap_get_entry_type(HEAP_BLOCK_TABLE_ENTRY entry)
{
    return entry & 0x0f;
}

int heap_get_start_block(struct heap* heap, uint32_t alloc_blocks)
{
    struct heap_table* table = heap->table;
    int block_cnt = 0, bs = -1;
    for (size_t i = 0; i < table->total; i++)
    {
        if (heap_get_entry_type(table->entries[i]) == HEAP_BLOCK_TABLE_ENTRY_FREE) {
            if (bs == -1)
            {
                bs = i;
            }
            block_cnt++;
            if (block_cnt == alloc_blocks)
            {
                break;
            }
        } else {
            block_cnt = 0, bs = -1;
        }
    }

    return bs == -1 ? -ENOMEM : bs;
}

void* heap_block_idx_to_addr(struct heap* heap, int block_idx)
{
    return (void *)(heap->saddr + block_idx * MYOS_HEAP_BLOCK_SIZE);
}

void heap_mark_blocks_taken(struct heap* heap, int start_block, int alloc_blocks)
{
    int end_block = start_block + alloc_blocks - 1;

    heap->table->entries[start_block] = HEAP_BLOCK_TABLE_ENTRY_TAKEN | HEAP_BLOCK_IS_FIRST;

    if (alloc_blocks > 1)
    {
        heap->table->entries[start_block] |= HEAP_BLOCK_HAS_NEXT;
        for (size_t i = start_block + 1; i < end_block; i++)
        {
            heap->table->entries[i] = HEAP_BLOCK_TABLE_ENTRY_TAKEN | HEAP_BLOCK_HAS_NEXT;
        }
        heap->table->entries[end_block] = HEAP_BLOCK_TABLE_ENTRY_TAKEN;
    }
}

void* heap_malloc_blocks(struct heap* heap, uint32_t alloc_blocks)
{
    void* address = NULL;
    
    int start_block = heap_get_start_block(heap, alloc_blocks);
    if (start_block < 0)
    {
        goto bail;
    }

    address = heap_block_idx_to_addr(heap, start_block);

    // Mark blocks as taken
    heap_mark_blocks_taken(heap, start_block, alloc_blocks);

bail:
    return address;
}

void* heap_malloc(struct heap* heap, size_t size)
{
    size_t aligned_size = heap_align_size(size);
    uint32_t alloc_blocks = aligned_size / MYOS_HEAP_BLOCK_SIZE;
    return heap_malloc_blocks(heap, alloc_blocks);
}

int heap_addr_to_block(struct heap* heap, void* ptr)
{
    return (int)(ptr - heap->saddr) / MYOS_HEAP_BLOCK_SIZE;
}

void heap_mark_blocks_free(struct heap* heap, int block_idx)
{
    struct heap_table* table = heap->table;
    while (block_idx < table->total && (table->entries[block_idx] & HEAP_BLOCK_HAS_NEXT))
    {
        table->entries[block_idx] = HEAP_BLOCK_TABLE_ENTRY_FREE;
        block_idx++;
    }
    if (block_idx < table->total)
    {
        table->entries[block_idx] = HEAP_BLOCK_TABLE_ENTRY_FREE;
    }
}

void heap_free(struct heap* heap, void* ptr)
{
    heap_mark_blocks_free(heap, heap_addr_to_block(heap, ptr));
}