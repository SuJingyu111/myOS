#include "paging.h"
#include "memory/heap/kheap.h"
#include <stdbool.h>
#include "status.h"

static PAGING_MASTER_DIRECTORY current_directory = NULL;

void paging_load_directory(PAGING_MASTER_DIRECTORY directory);

struct paging_4gb_chunk* paging_new_4gb(uint8_t flags)
{
    PAGING_MASTER_DIRECTORY directory = kzalloc(sizeof(uint32_t) * PAGING_TOTAL_ENTRIES_PER_TABLE);
    int offset = 0;
    for (int i = 0; i < PAGING_TOTAL_ENTRIES_PER_TABLE; i++)
    {
        PAGING_PAGE_TABLE page_table = kzalloc(sizeof(PAGING_PAGE_ENTRY) * PAGING_TOTAL_ENTRIES_PER_TABLE);
        for (int j = 0; j < PAGING_TOTAL_ENTRIES_PER_TABLE; j++)
        {
            page_table[j] = (offset + (j * PAGING_PAGE_SIZE)) | flags;
        }

        offset += PAGING_TOTAL_ENTRIES_PER_TABLE * PAGING_PAGE_SIZE;
        directory[i] = (PAGING_PAGE_TABLE_ENTRY)((uint32_t)page_table | flags | PAGING_IS_WRITABLE);
    }

    struct paging_4gb_chunk* mem_chunk_4gb = kzalloc(sizeof(struct paging_4gb_chunk));
    mem_chunk_4gb->directory_entries = directory;
    return mem_chunk_4gb;
}

void paging_switch(PAGING_MASTER_DIRECTORY directory)
{
    paging_load_directory(directory);
    current_directory = directory;
}

PAGING_MASTER_DIRECTORY paging_4gb_chunk_get_directory(struct paging_4gb_chunk* mem_chunk)
{
    return mem_chunk->directory_entries;
}

bool paging_is_aligned(void* addr)
{
    return ((uint32_t)addr % PAGING_PAGE_SIZE) == 0;
}

int paging_get_indexes(void* vaddr, uint32_t* directory_index_out, uint32_t* table_index_out)
{
    int res = MYOS_HEALTHY;
    if (!paging_is_aligned(vaddr))
    {
        res = -EINVARG;
        goto out;
    }

    *directory_index_out = ((uint32_t)vaddr / (PAGING_TOTAL_ENTRIES_PER_TABLE * PAGING_PAGE_SIZE));
    *table_index_out = ((uint32_t)vaddr % (PAGING_TOTAL_ENTRIES_PER_TABLE * PAGING_PAGE_SIZE) / PAGING_PAGE_SIZE);
out:
    return res;
}

int paging_set(PAGING_MASTER_DIRECTORY directory, void* vaddr, PAGING_PAGE_ENTRY page_entry)
{
    if (!paging_is_aligned(vaddr))
    {
        return -EINVARG;
    }

    uint32_t directory_idx = 0;
    uint32_t table_idx = 0;
    int res = paging_get_indexes(vaddr, &directory_idx, &table_idx);
    if (res < 0)
    {
        return res;
    }

    PAGING_PAGE_TABLE_ENTRY entry = directory[directory_idx];
    PAGING_PAGE_TABLE table = (PAGING_PAGE_TABLE)(entry & 0xfffff000);
    table[table_idx] = page_entry;

    return res;
}