/* 
* BleskOS
*
* MIT License
* Copyright (c) 2023-2026 BleskOS developers
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

/* includes */
#include <kernel/hardware/devices/memory/memory_allocators.h>
#include <kernel/hardware/groups/logging/logging.h>
#include <kernel/libc/string.h>
#include <kernel/libc/ctype.h>

/* functions */
void dump_ramdisk_content(void) {
    uint32_t *ramdisk = (uint32_t *) P_MEM_RAMDISK;

    log("\nRAMDISK DUMP\nSize in bytes: %d", *ramdisk);

    ramdisk_file_entry_t *ramdisk_entry = (ramdisk_file_entry_t *) (P_MEM_RAMDISK + 4);
    while(ramdisk_entry->offset != 0) {
        log("\nFile: %s Start: 0x%x Size: %d", ramdisk_entry->name, ramdisk_entry->offset, ramdisk_entry->size);
        ramdisk_entry = (ramdisk_file_entry_t *) ((uint32_t)ramdisk_entry + 8 + strlen(ramdisk_entry->name) + 1);
    }
}

void *get_ramdisk_file_ptr(uint8_t *file) {
    ramdisk_file_entry_t *ramdisk_entry = (ramdisk_file_entry_t *) (P_MEM_RAMDISK + 4);

    while(ramdisk_entry->offset != 0) {
        if(strcmp(ramdisk_entry->name, file) == 0) {
            return (void *) (P_MEM_RAMDISK + ramdisk_entry->offset);
        }
        ramdisk_entry = (ramdisk_file_entry_t *) ((uint32_t)ramdisk_entry + 8 + strlen(ramdisk_entry->name) + 1);
    }

    return (void *) INVALID;
}

uint32_t get_ramdisk_file_size(uint8_t *file) {
    ramdisk_file_entry_t *ramdisk_entry = (ramdisk_file_entry_t *) (P_MEM_RAMDISK + 4);

    while(ramdisk_entry->offset != 0) {
        if(strcmp(ramdisk_entry->name, file) == 0) {
            return ramdisk_entry->size;
        }
        ramdisk_entry = (ramdisk_file_entry_t *) ((uint32_t)ramdisk_entry + 8 + strlen(ramdisk_entry->name) + 1);
    }

    return INVALID;
}

ramdisk_elf_program_list_t *get_ramdisk_elf_program_list(void) {
    ramdisk_elf_program_list_t *list = kalloc(sizeof(ramdisk_elf_program_list_t));
    ramdisk_file_entry_t *ramdisk_entry = (ramdisk_file_entry_t *) (P_MEM_RAMDISK + 4);

    while(ramdisk_entry->offset != 0) {
        uint8_t *extension = strchr(ramdisk_entry->name, '.');
        if(    extension != NULL
            && strcmp(ramdisk_entry->name, "userspace_library.elf") != 0
            && strcmp(extension, ".elf") == 0) {
            list = krealloc(list, sizeof(ramdisk_elf_program_list_t) + ((list->number_of_programs + 1) * sizeof(ramdisk_elf_program_t)));
            uint32_t index = list->number_of_programs;
            uint32_t size_of_name = extension - ramdisk_entry->name;
            list->programs[index].name = kalloc(size_of_name + 1);
            memcpy(list->programs[index].name, ramdisk_entry->name, size_of_name);
            for(int i = 0, in_word = false; i < size_of_name; i++) {
                if(list->programs[index].name[i] == '_' || list->programs[index].name[i] == '-' || list->programs[index].name[i] == ' ') {
                    list->programs[index].name[i] = ' ';
                    in_word = false;
                }
                else if(in_word == false) {
                    list->programs[index].name[i] = toupper(list->programs[index].name[i]);
                    in_word = true;
                }
            }
            list->programs[index].name[size_of_name] = '\0';
            uint8_t *icon_file = kalloc(size_of_name + 4 + 1);
            memcpy(icon_file, ramdisk_entry->name, size_of_name);
            memcpy(icon_file + size_of_name, ".png", 4);
            icon_file[size_of_name + 4] = '\0';
            list->programs[index].icon = load_image(get_ramdisk_file_ptr(icon_file), get_ramdisk_file_size(icon_file));
            kfree(icon_file);
            list->programs[index].elf_file_ptr = (void *) (P_MEM_RAMDISK + ramdisk_entry->offset);
            list->programs[index].elf_file_size = ramdisk_entry->size;
            list->number_of_programs++;
        }
        ramdisk_entry = (ramdisk_file_entry_t *) ((uint32_t)ramdisk_entry + 8 + strlen(ramdisk_entry->name) + 1);
    }

    return list;
}