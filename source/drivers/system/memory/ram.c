//BleskOS

/*
* MIT License
* Copyright (c) 2023-2025 BleskOS developers
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

void read_ram_info(void) {
    // read memory size
    struct bios_memory_table_entry_t *memory_entry = (struct bios_memory_table_entry_t *) 0x1000; //bootloader loaded memory map here
    dword_t number_of_memory_table_entries = *((dword_t *) 0x1FF0);
    for(dword_t i = 0; i < number_of_memory_table_entries; i++) {
        components->ram.full_memory_size += memory_entry[i].size;

        if(memory_entry[i].type == BIOS_MEMORY_ENTRY_TYPE_FREE) {
            components->ram.free_memory_size += memory_entry[i].size;
        }
    }

    // set actual usable memory size
    if(components->ram.free_memory_size > 0xFFFFFFFF) {
        components->ram.available_free_memory_size = 0xFFFFFFFF;
    }
    else {
        components->ram.available_free_memory_size = (dword_t) components->ram.free_memory_size;
    }
}

void log_ram_info(void) {
    logf("\n\nRAM INFO\nFull memory: %d MB 0x%x%x\nFree memory: %d MB 0x%x%x\nUsable free memory: %d MB",
        (dword_t)(components->ram.full_memory_size/1024/1024),
        (dword_t)(components->ram.full_memory_size >> 32),
        (dword_t)components->ram.full_memory_size,
        (dword_t)(components->ram.free_memory_size/1024/1024),
        (dword_t)(components->ram.free_memory_size >> 32),
        (dword_t)components->ram.free_memory_size,
        (dword_t)(components->ram.available_free_memory_size/1024/1024));
    
    struct bios_memory_table_entry_t *memory_entry = (struct bios_memory_table_entry_t *) 0x1000; //bootloader loaded memory map here
    dword_t number_of_memory_table_entries = *((dword_t *) 0x1FF0);
    for(dword_t i = 0; i < number_of_memory_table_entries; i++) {
        logf("\n");
        if(memory_entry[i].type == BIOS_MEMORY_ENTRY_TYPE_FREE) {
            logf("Free");
        }
        else if(memory_entry[i].type == BIOS_MEMORY_ENTRY_TYPE_USED) {
            logf("Used");
        }
        else {
            logf("%d", memory_entry[i].type);
        }
        logf(" entry 0x%x%x size %d b %d MB",
            (dword_t)(memory_entry[i].memory >> 32),
            (dword_t)(memory_entry[i].memory),
            (dword_t)memory_entry[i].size,
            (dword_t)(memory_entry[i].size/1024/1024));
    }
}