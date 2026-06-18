/* 
* BleskOS
*
* MIT License
* Copyright (c) 2023-2026 BleskOS developers
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

typedef struct {
    uint32_t type;
    uint64_t memory_start;
    uint64_t memory_size;
}__attribute__((packed)) standardized_phy_memory_map_entry_t;
typedef struct {
    uint32_t number_of_entries;
    standardized_phy_memory_map_entry_t entry[];
}__attribute__((packed)) standardized_phy_memory_map_t;

typedef struct {
    uint32_t memory_start;
    uint32_t memory_size;
}__attribute__((packed)) standardized_32_bit_free_memory_map_entry_t;
typedef struct {
    uint32_t number_of_entries;
    standardized_32_bit_free_memory_map_entry_t entry[];
}__attribute__((packed)) standardized_32_bit_free_memory_map_t;