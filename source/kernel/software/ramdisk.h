/* 
* BleskOS
*
* MIT License
* Copyright (c) 2023-2026 BleskOS developers
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include <kernel/hardware/subsystems/screen/stb_image_implementation.h>
#include <kernel/hardware/subsystems/screen/screen.h>

typedef struct {
    uint32_t offset;
    uint32_t size;
    uint8_t name[];
}__attribute__((packed)) ramdisk_file_entry_t;

typedef struct {
    uint8_t *name;
    image_t *icon;
    void *elf_file_ptr;
    uint32_t elf_file_size;
    uint32_t is_loaded_into_memory;
    void *program_info;
    screen_part_t *part_where_program_is_running;
} ramdisk_elf_program_t;

typedef struct {
    uint32_t number_of_programs;
    ramdisk_elf_program_t programs[];
} ramdisk_elf_program_list_t;