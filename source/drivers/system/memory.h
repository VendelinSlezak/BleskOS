//BleskOS

/*
* MIT License
* Copyright (c) 2023 Vendelín Slezák
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#define MEM_ENTRY_TYPE 0
#define MEM_ENTRY_LENGTH 1
#define MEM_ENTRY_START 2
#define MEM_ENTRY_END 3

#define MEM_FREE_ENTRY 0
#define MEM_FREE_MEMORY 1
#define MEM_ALLOCATED_MEMORY 2

#define MEMORY_TYPE_FREE 1
#define MEMORY_TYPE_USED 2

dword_t all_memory_in_bytes = 0;
dword_t all_free_memory_in_bytes = 0;
dword_t all_actual_free_memory_in_bytes = 0;
dword_t mem_memory_entries = 0;

void initalize_memory(void);
void log_starting_memory(void);
dword_t malloc(dword_t mem_length);
dword_t aligned_malloc(dword_t mem_length, dword_t mem_alignment);
dword_t calloc(dword_t mem_length);
dword_t realloc(dword_t mem_pointer, dword_t mem_length);
void free(dword_t mem_pointer);
void memory_error_debug(dword_t color);
void clear_memory(dword_t memory, dword_t length);
void copy_memory(dword_t source_memory, dword_t destination_memory, dword_t size);
void copy_memory_back(dword_t source_memory, dword_t destination_memory, dword_t size);