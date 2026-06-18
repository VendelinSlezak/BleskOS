/* 
* BleskOS
*
* MIT License
* Copyright (c) 2023-2026 BleskOS developers
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

typedef struct bleskalloc_entry {
    uint32_t start;
    uint32_t end;

    struct bleskalloc_entry *prev;
    struct bleskalloc_entry *next;
} bleskalloc_entry_t;
typedef struct {
    uint32_t memory_start;
    uint32_t memory_size;
    uint32_t memory_end;

    uint32_t free_memory_start;
    uint32_t free_memory_size;

    bleskalloc_entry_t *insert_entry;

    bleskalloc_entry_t first_entry;

    uint16_t number_of_allocated_pages_in_page_table[1024];
} bleskalloc_metadata_t;