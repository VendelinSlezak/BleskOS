//BleskOS

/*
* MIT License
* Copyright (c) 2023-2025 Vendelín Slezák
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#define MTRR_REG_CAPABILITY 0xFE
#define MTRR_REG_DEFAULT_TYPE 0x2FF
#define MTRR_REG_BASE 0x200
#define MTRR_REG_MASK 0x201

#define MTRR_MEMORY_TYPE_UNCACHEABLE 0
#define MTRR_MEMORY_TYPE_WRITE_COMBINING 1
#define MTRR_MEMORY_TYPE_WRITE_THROUGH 4
#define MTRR_MEMORY_TYPE_WRITE_PROTECT 5
#define MTRR_MEMORY_TYPE_WRITE_BACK 6

byte_t mtrr_available, mtrr_write_combining_available, mtrr_number_of_entries, mtrr_default_memory_type;

void initalize_mtrr(void);
void log_mtrr_memory_type(byte_t type);
void mtrr_set_free_entry(dword_t memory, dword_t mask, byte_t type);
void mtrr_set_memory_type(void *memory, dword_t size, byte_t type);