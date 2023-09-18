#include "paging.h"
#include "memory/heap/kheap.h"

static PAGING_MASTER_DIRECTORY current_directory = NULL;

void paging_load_directory(PAGING_MASTER_DIRECTORY directory);

struct paging_4gb_chunk* paging_new_4gb(uint8_t flags)
{
    PAGING_MASTER_DIRECTORY directory = kzalloc(sizeof(uint32_t) * PAGING_TOTAL_ENTRIES_PER_TABLE);
    int offset = 0;
    for (int i = 0; i < PAGING_TOTAL_ENTRIES_PER_TABLE; i++)
    {
        PAGING_PAGE_TABLE_ENTRY page_table = kzalloc(sizeof(uint32_t) * PAGING_TOTAL_ENTRIES_PER_TABLE);
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