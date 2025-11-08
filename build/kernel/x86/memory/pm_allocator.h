#ifndef BUILD_KERNEL_X86_MEMORY_PM_ALLOCATOR_H
#define BUILD_KERNEL_X86_MEMORY_PM_ALLOCATOR_H

/* 
* BleskOS
*
* MIT License
* Copyright (c) 2023-2025 BleskOS developers
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

typedef struct {
    dword_t type;
    qword_t memory_start;
    qword_t memory_size;
}__attribute__((packed)) standardized_phy_memory_map_entry_t;

typedef struct {
    dword_t number_of_entries;
    standardized_phy_memory_map_entry_t entry[];
}__attribute__((packed)) standardized_phy_memory_map_t;

typedef struct {
    dword_t memory_start;
    dword_t memory_size;
}__attribute__((packed)) standardized_32_bit_free_memory_map_entry_t;

typedef struct {
    dword_t number_of_entries;
    standardized_32_bit_free_memory_map_entry_t entry[];
}__attribute__((packed)) standardized_32_bit_free_memory_map_t;

typedef struct {
    dword_t pointer_to_free_page;
    dword_t last_valid_index;

    dword_t pointer_alloc_stack_page;
    dword_t *pte_stack_page_for_allocation;
    dword_t pointer_dealloc_stack_page;
    dword_t *pte_stack_page_for_deallocation;

    byte_t padding[4096 - 24];

    dword_t pages[];
}__attribute__((packed)) pm_pages_stack_t;

extern dword_t number_of_allocated_pages;
void log_physical_memory_map(void);
void initialize_physical_memory(void);
void *pm_bump_alloc(dword_t size);
void *pm_alloc_page(void);
void *pm_alloc_continuous_memory(dword_t size);
void pm_free_page(void *page);

#endif /* BUILD_KERNEL_X86_MEMORY_PM_ALLOCATOR_H */
