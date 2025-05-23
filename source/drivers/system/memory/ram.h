//BleskOS

/*
* MIT License
* Copyright (c) 2023-2025 BleskOS developers
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#define BIOS_MEMORY_ENTRY_TYPE_FREE 1
#define BIOS_MEMORY_ENTRY_TYPE_USED 2

struct bios_memory_table_entry_t {
    qword_t memory;
    qword_t size;
    dword_t type;

    // ACPI 3.0 extended attributes
    dword_t usable : 1;
    dword_t non_volatile : 1;
    dword_t : 30;
}__attribute__((packed));

struct ram_info_t {
    qword_t full_memory_size;
    qword_t free_memory_size;
    dword_t available_free_memory_size;
};

void read_ram_info(void);
void log_ram_info(void);