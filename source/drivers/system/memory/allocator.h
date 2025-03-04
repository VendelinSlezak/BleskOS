//BleskOS

/*
* MIT License
* Copyright (c) 2023-2025 BleskOS developers
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

struct memory_allocator_entry_t {
    dword_t start_of_memory;
    dword_t size_of_memory;
    dword_t end_of_memory;
    dword_t padding;
};

struct memory_allocator_t {
    dword_t first_used_entry;
    dword_t last_entry;

    dword_t biggest_free_entry;
    dword_t second_biggest_free_entry;

    dword_t first_available_free_entry;
    dword_t first_available_used_entry;

    struct memory_allocator_entry_t entries[];
};

#define get_size_of_memory_allocator(number_of_entries_of_one_type) (sizeof(struct memory_allocator_t) + (sizeof(struct memory_allocator_entry_t)*2*number_of_entries_of_one_type))

void initalize_memory_allocator(struct memory_allocator_t *memory_allocator, dword_t number_of_entries_of_one_type);
void allocator_update_biggest_entries_pointers(struct memory_allocator_t *allocator);
void *ma_fixed_calloc(struct memory_allocator_t *allocator, dword_t size);
void *ma_malloc(struct memory_allocator_t *allocator, dword_t size);
void *ma_calloc(struct memory_allocator_t *allocator, dword_t size);
void *ma_realloc(struct memory_allocator_t *allocator, void *memory, dword_t size);
void ma_free(struct memory_allocator_t *allocator, void *memory);