//BleskOS

/*
* MIT License
* Copyright (c) 2023-2025 BleskOS developers
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

void *memory_table_alloc(dword_t size) {
    // bootloader loaded these things here
    struct bios_memory_table_entry_t *memory_table = (struct bios_memory_table_entry_t *) 0x1000;
    dword_t number_of_memory_table_entries = *((dword_t *) 0x1FF0);

    // check for invalid request
    if(size == 0) {
        return 0;
    }

    // go through all entries to find free one with enough memory
    for(dword_t i = 0; i < number_of_memory_table_entries; i++) {
        if(memory_table[i].type == BIOS_MEMORY_ENTRY_TYPE_FREE && memory_table[i].memory >= 0x100000 && memory_table[i].size >= size) {
            void *memory = (void *) ((dword_t) memory_table[i].memory);
            memory_table[i].memory += size;
            memory_table[i].size -= size;
            return memory;
        }
    }

    // not enough free memory
    return 0;
}

void create_physical_memory_allocator(void) {
    // create empty allocator with size 10000 entries
    physical_memory_allocator = memory_table_alloc(get_size_of_memory_allocator(10000));
    initalize_memory_allocator(physical_memory_allocator, 10000);

    // fill allocator with free memory entries
    struct bios_memory_table_entry_t *memory_entry = (struct bios_memory_table_entry_t *) 0x1000; //bootloader loaded memory map here
    dword_t number_of_memory_table_entries = *((dword_t *) 0x1FF0);
    for(dword_t i = 0; i < number_of_memory_table_entries; i++) {
        if(memory_entry[i].type == BIOS_MEMORY_ENTRY_TYPE_FREE && memory_entry[i].memory >= 0x100000 && memory_entry[i].memory < 0xFFFFE000 && memory_entry[i].size >= 8192) {
            // align memory entry to page size with 32 bit address
            dword_t entry_start_of_memory = memory_entry[i].memory;
            if((entry_start_of_memory & 0xFFF) != 0) {
                entry_start_of_memory = ((entry_start_of_memory & ~0xFFF) + 0x1000);
            }
            dword_t entry_end_of_memory = (dword_t) (memory_entry[i].memory + memory_entry[i].size);
            if((memory_entry[i].memory >> 32) != 0x00000000) {
                entry_end_of_memory = 0xFFFFF000;
            }
            else if((entry_end_of_memory & 0xFFF) != 0) {
                entry_end_of_memory = (entry_end_of_memory & ~0xFFF);
            }

            // add memory entry to allocator
            physical_memory_allocator->entries[physical_memory_allocator->first_available_free_entry].start_of_memory = entry_start_of_memory;
            physical_memory_allocator->entries[physical_memory_allocator->first_available_free_entry].size_of_memory = (entry_end_of_memory - entry_start_of_memory);
            physical_memory_allocator->entries[physical_memory_allocator->first_available_free_entry].end_of_memory = entry_end_of_memory;
            physical_memory_allocator->first_available_free_entry++;

            // there will be enough free entries in all cases, because we will load max 150 entries from BIOS memory table
        }
    }

    // finalize allocator initalization
    allocator_update_biggest_entries_pointers(physical_memory_allocator);
}

static inline void *fixed_calloc(size_t mem_length) {
    return ma_fixed_calloc(physical_memory_allocator, mem_length);
}

static inline void *malloc(size_t mem_length) {
    return ma_malloc(physical_memory_allocator, mem_length);
}

static inline void *aligned_malloc(size_t mem_length, dword_t mem_alignment) {
    return ma_malloc(physical_memory_allocator, mem_length);
}

static inline void *calloc(size_t mem_length) {
    return ma_calloc(physical_memory_allocator, mem_length);
}

static inline void *aligned_calloc(size_t mem_length, dword_t mem_alignment) {
    return ma_calloc(physical_memory_allocator, mem_length);
}

static inline void *realloc(void *mem_pointer, size_t mem_length) {
    return ma_realloc(physical_memory_allocator, mem_pointer, mem_length);
}

static inline void *free(void *mem_pointer) {
    ma_free(physical_memory_allocator, mem_pointer);
}