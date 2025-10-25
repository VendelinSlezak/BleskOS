/* 
* BleskOS
*
* MIT License
* Copyright (c) 2023-2025 BleskOS developers
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

/* includes */
#include <hardware/groups/logging/logging.h>
#include <kernel/x86/libc/string.h>

/* functions */
void dump_ramdisk_content(void) {
    dword_t *ramdisk = (dword_t *) P_MEM_RAMDISK;

    log("\nRAMDISK DUMP\nSize in bytes: %d", *ramdisk);

    ramdisk_file_entry_t *ramdisk_entry = (ramdisk_file_entry_t *) (P_MEM_RAMDISK + 4);
    while(ramdisk_entry->offset != 0) {
        log("\nFile: %s Start: 0x%x Size: %d", ramdisk_entry->name, ramdisk_entry->offset, ramdisk_entry->size);
        ramdisk_entry = (ramdisk_file_entry_t *) ((dword_t)ramdisk_entry + 8 + strlen(ramdisk_entry->name) + 1);
    }
}

void *get_ramdisk_file_ptr(byte_t *file) {
    ramdisk_file_entry_t *ramdisk_entry = (ramdisk_file_entry_t *) (P_MEM_RAMDISK + 4);

    while(ramdisk_entry->offset != 0) {
        if(strcmp(ramdisk_entry->name, file) == 0) {
            return (void *) (P_MEM_RAMDISK + ramdisk_entry->offset);
        }
        ramdisk_entry = (ramdisk_file_entry_t *) ((dword_t)ramdisk_entry + 8 + strlen(ramdisk_entry->name) + 1);
    }

    return (void *) INVALID;
}

dword_t get_ramdisk_file_size(byte_t *file) {
    ramdisk_file_entry_t *ramdisk_entry = (ramdisk_file_entry_t *) (P_MEM_RAMDISK + 4);

    while(ramdisk_entry->offset != 0) {
        if(strcmp(ramdisk_entry->name, file) == 0) {
            return ramdisk_entry->size;
        }
        ramdisk_entry = (ramdisk_file_entry_t *) ((dword_t)ramdisk_entry + 8 + strlen(ramdisk_entry->name) + 1);
    }

    return INVALID;
}