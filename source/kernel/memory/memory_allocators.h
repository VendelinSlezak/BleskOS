/* 
* BleskOS
*
* MIT License
* Copyright (c) 2023-2026 BleskOS developers
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

extern uint32_t permanent_kernel_allocator_base;
extern uint32_t user_space_allocation_start;
extern uint32_t user_space_allocation_size;
extern uint32_t user_space_allocation_end;
extern uint32_t number_of_shared_page_tables;

typedef struct kheap_entry {
    uint32_t end;

    struct kheap_entry *prev;
    struct kheap_entry *next;
} kheap_entry_t;
typedef struct {
    uint32_t memory_start;
    uint32_t memory_size;
    uint32_t memory_end;

    uint32_t free_memory_start;
    uint32_t free_memory_size;

    kheap_entry_t *insert_entry;

    kheap_entry_t first_entry;
} kernel_heap_metadata_t;