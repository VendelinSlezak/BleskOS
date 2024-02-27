//BleskOS

/*
* MIT License
* Copyright (c) 2023-2024 Vendelín Slezák
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#define BYTE_STREAM_100_KB_BLOCK 100*1024
#define BYTE_STREAM_1_MB_BLOCK 1024*1024

struct byte_stream_descriptor {
 dword_t start_of_allocated_memory;
 byte_t *memory_pointer;
 dword_t size_of_allocated_memory;
 dword_t end_of_allocated_memory;
 dword_t size_of_stream;
 dword_t size_of_one_block;
}__attribute__((packed));

struct byte_stream_descriptor *create_byte_stream(dword_t size_of_block);
void add_byte_to_byte_stream(struct byte_stream_descriptor *actual_byte_stream_descriptor, byte_t value);
void add_string_to_byte_stream(struct byte_stream_descriptor *actual_byte_stream_descriptor, byte_t *string);
void create_free_space_in_byte_stream(struct byte_stream_descriptor *actual_byte_stream_descriptor, dword_t space_size);
void close_byte_stream(struct byte_stream_descriptor *actual_byte_stream_descriptor);