//BleskOS

/*
* MIT License
* Copyright (c) 2023-2025 BleskOS developers
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

void initalize_memory_allocator(struct memory_allocator_t *memory_allocator, dword_t number_of_entries_of_one_type) {
    memory_allocator->first_used_entry = number_of_entries_of_one_type;
    memory_allocator->last_entry = (number_of_entries_of_one_type*2);
    memory_allocator->biggest_free_entry = 0xFFFFFFFF;
    memory_allocator->second_biggest_free_entry = 0xFFFFFFFF;
    memory_allocator->first_available_free_entry = 0;
    memory_allocator->first_available_used_entry = number_of_entries_of_one_type;
}

void allocator_update_biggest_entries_pointers(struct memory_allocator_t *allocator) {
    dword_t new_biggest_entry = 0xFFFFFFFF;
    dword_t new_biggest_entry_size = 0;
    dword_t new_second_biggest_entry = 0xFFFFFFFF;
    dword_t new_second_biggest_entry_size = 0;
    for(dword_t i = 0; i < allocator->first_available_free_entry; i++) {
        if(allocator->entries[i].size_of_memory > new_biggest_entry_size) {
            new_second_biggest_entry = new_biggest_entry;
            new_second_biggest_entry_size = new_biggest_entry_size;
            new_biggest_entry = i;
            new_biggest_entry_size = allocator->entries[i].size_of_memory;
        }
        else if(allocator->entries[i].size_of_memory > new_second_biggest_entry_size) {
            new_second_biggest_entry = i;
            new_second_biggest_entry_size = allocator->entries[i].size_of_memory;
        }
    }
    allocator->biggest_free_entry = new_biggest_entry;
    allocator->second_biggest_free_entry = new_second_biggest_entry;
}

void *ma_fixed_calloc(struct memory_allocator_t *allocator, dword_t size) {
    // convert size to page size
    if((size & 0xFFF) != 0) {
        size = ((size & ~0xFFF) + 0x1000);
    }

    // check if this size can be allocated
    if(size == 0 || allocator->entries[allocator->biggest_free_entry].size_of_memory < size || allocator->first_available_used_entry >= allocator->last_entry) {
        return 0;
    }

    // allocate memory
    void *memory = (void *) allocator->entries[allocator->biggest_free_entry].start_of_memory;
    allocator->entries[allocator->biggest_free_entry].start_of_memory += size;
    allocator->entries[allocator->biggest_free_entry].size_of_memory -= size;
    dword_t original_biggest_entry = allocator->biggest_free_entry;
    dword_t new_size_of_original_biggest_entry = allocator->entries[allocator->biggest_free_entry].size_of_memory;

    // update pointers to biggest free entries
    if(allocator->second_biggest_free_entry != 0xFFFFFFFF && new_size_of_original_biggest_entry < allocator->entries[allocator->second_biggest_free_entry].size_of_memory) {
        allocator->biggest_free_entry = allocator->second_biggest_free_entry;

        dword_t second_biggest_entry_size = 0;
        allocator->second_biggest_free_entry = 0xFFFFFFFF;
        for(dword_t i = 0; i < allocator->first_available_free_entry; i++) {
            if(i != allocator->biggest_free_entry && allocator->entries[i].size_of_memory > second_biggest_entry_size) {
                second_biggest_entry_size = allocator->entries[i].size_of_memory;
                allocator->second_biggest_free_entry = i;
            }
        }
    }

    // if this entry was allocated as whole, remove it from list
    if(new_size_of_original_biggest_entry == 0) {
        remove_space_from_memory_area((dword_t)(&allocator->entries[0]), sizeof(struct memory_allocator_entry_t)*allocator->first_used_entry, (dword_t)(&allocator->entries[allocator->biggest_free_entry]), sizeof(struct memory_allocator_entry_t));
        allocator->first_available_free_entry--;
        if(allocator->biggest_free_entry > original_biggest_entry) {
            allocator->biggest_free_entry--;
        }
        if(allocator->second_biggest_free_entry != 0xFFFFFFFF && allocator->second_biggest_free_entry > original_biggest_entry) {
            allocator->second_biggest_free_entry--;
        }
    }

    //clear memory
    cmem(memory, size);

    return memory;
}

void *ma_malloc(struct memory_allocator_t *allocator, dword_t size) {
    // convert size to page size
    if((size & 0xFFF) != 0) {
        size = ((size & ~0xFFF) + 0x1000);
    }

    // check if this size can be allocated
    if(size == 0 || allocator->entries[allocator->biggest_free_entry].size_of_memory < size || allocator->first_available_used_entry >= allocator->last_entry) {
        return 0;
    }

    // allocate memory
    void *memory = (void *) allocator->entries[allocator->biggest_free_entry].start_of_memory;
    allocator->entries[allocator->biggest_free_entry].start_of_memory += size;
    allocator->entries[allocator->biggest_free_entry].size_of_memory -= size;
    dword_t original_biggest_entry = allocator->biggest_free_entry;
    dword_t new_size_of_original_biggest_entry = allocator->entries[allocator->biggest_free_entry].size_of_memory;

    // create entry that holds information about this allocated entry
    allocator->entries[allocator->first_available_used_entry].start_of_memory = (dword_t) memory;
    allocator->entries[allocator->first_available_used_entry].size_of_memory = size;
    allocator->entries[allocator->first_available_used_entry].end_of_memory = ((dword_t)memory + size);
    allocator->first_available_used_entry++;

    // update pointers to biggest free entries
    if(allocator->second_biggest_free_entry != 0xFFFFFFFF && new_size_of_original_biggest_entry < allocator->entries[allocator->second_biggest_free_entry].size_of_memory) {
        allocator->biggest_free_entry = allocator->second_biggest_free_entry;

        dword_t second_biggest_entry_size = 0;
        allocator->second_biggest_free_entry = 0xFFFFFFFF;
        for(dword_t i = 0; i < allocator->first_available_free_entry; i++) {
            if(i != allocator->biggest_free_entry && allocator->entries[i].size_of_memory > second_biggest_entry_size) {
                second_biggest_entry_size = allocator->entries[i].size_of_memory;
                allocator->second_biggest_free_entry = i;
            }
        }
    }

    // if this entry was allocated as whole, remove it from list
    if(new_size_of_original_biggest_entry == 0) {
        remove_space_from_memory_area((dword_t)(&allocator->entries[0]), sizeof(struct memory_allocator_entry_t)*allocator->first_used_entry, (dword_t)(&allocator->entries[allocator->biggest_free_entry]), sizeof(struct memory_allocator_entry_t));
        allocator->first_available_free_entry--;
        if(allocator->biggest_free_entry > original_biggest_entry) {
            allocator->biggest_free_entry--;
        }
        if(allocator->second_biggest_free_entry != 0xFFFFFFFF && allocator->second_biggest_free_entry > original_biggest_entry) {
            allocator->second_biggest_free_entry--;
        }
    }

    //clear last page of memory, so if user reallocate to few bytes more, memory will be zeroed
    cmem((void *)((dword_t)memory + size - 0x1000), 0x1000);
    
    return memory;
}

void *ma_calloc(struct memory_allocator_t *allocator, dword_t size) {
    void *memory = ma_malloc(allocator, size);
    cmem(memory, size);
    return memory;
}

void *ma_realloc(struct memory_allocator_t *allocator, void *memory, dword_t size) {
    // convert size to page size
    if((size & 0xFFF) != 0) {
        size = ((size & ~0xFFF) + 0x1000);
    }

    // find entry
    dword_t entry = 0;
    for(dword_t i = allocator->first_used_entry; i < allocator->first_available_used_entry; i++) {
        if(allocator->entries[i].start_of_memory == (dword_t)memory) {
            entry = i;
            break;
        }
    }

    // check if we can do reallocation
    if(size == 0 || entry == 0) {
        return 0;
    }

    // check if there is any reallocation needed
    if(allocator->entries[entry].size_of_memory == size) {
        return memory;
    }

    // reallocation to smaller memory
    if(allocator->entries[entry].size_of_memory > size) {
        // check if we can create new used entry
        if(allocator->first_available_used_entry >= allocator->last_entry) {
            return 0;
        }

        // create used entry for memory at end of allocated memory
        allocator->entries[allocator->first_available_used_entry].start_of_memory = (allocator->entries[entry].start_of_memory+size);
        allocator->entries[allocator->first_available_used_entry].size_of_memory = (allocator->entries[entry].size_of_memory-size);
        allocator->entries[allocator->first_available_used_entry].end_of_memory = (allocator->entries[allocator->first_available_used_entry].start_of_memory + allocator->entries[allocator->first_available_used_entry].size_of_memory);
        allocator->first_available_used_entry++;

        // reallocate actual memory entry
        allocator->entries[entry].size_of_memory = size;
        allocator->entries[entry].end_of_memory = (allocator->entries[entry].start_of_memory + size);

        // free end of memory
        ma_free(allocator, (void *)(allocator->entries[entry].start_of_memory+size));

        // return pointer to memory
        return (void *) (allocator->entries[entry].start_of_memory);
    }
    // reallocation to bigger memory
    else {
        //TODO: check if it is possible to extend memory block

        // allocate memory
        void *new_memory = ma_calloc(allocator, size);
        if(new_memory == 0) {
            return 0;
        }

        // copy content of previous memory block
        copy_memory((dword_t)memory, (dword_t)new_memory, allocator->entries[entry].size_of_memory);

        // free previous memory block
        ma_free(allocator, memory);

        // return pointer to new memory block
        return new_memory;
    }
}

void ma_free(struct memory_allocator_t *allocator, void *memory) {
    // find entry
    dword_t entry = 0;
    for(dword_t i = allocator->first_used_entry; i < allocator->first_available_used_entry; i++) {
        if(allocator->entries[i].start_of_memory == (dword_t)memory) {
            entry = i;
            break;
        }
    }
    // logf("\nFE: %d", entry);

    // return if entry was not founded
    if(entry == 0) {
        return;
    }

    // find free blocks of memory above and below this memory block
    dword_t entry_below = 0xFFFFFFFF;
    dword_t entry_above = 0xFFFFFFFF;
    for(dword_t i = 0; i < allocator->first_used_entry; i++) {
        if(allocator->entries[i].end_of_memory == allocator->entries[entry].start_of_memory) {
            entry_below = i;
        }
        else if(allocator->entries[i].start_of_memory == allocator->entries[entry].end_of_memory) {
            entry_above = i;
        }
    }
    // logf("\nB: %d", entry_below);
    // logf("\nA: %d", entry_above);

    // change founded memory entries
    if(entry_below != 0xFFFFFFFF && entry_above != 0xFFFFFFFF) {
        // this memory block will join two free memory blocks, so we need to update first memory block and delete other two
        allocator->entries[entry_below].size_of_memory += (allocator->entries[entry].size_of_memory + allocator->entries[entry_above].size_of_memory);
        allocator->entries[entry_below].end_of_memory = allocator->entries[entry_above].end_of_memory;

        remove_space_from_memory_area((dword_t)&allocator->entries[allocator->first_used_entry], sizeof(struct memory_allocator_entry_t)*allocator->first_used_entry, (dword_t)&allocator->entries[entry], sizeof(struct memory_allocator_entry_t));
        allocator->first_available_used_entry--;

        remove_space_from_memory_area((dword_t)&allocator->entries[0], sizeof(struct memory_allocator_entry_t)*allocator->first_used_entry, (dword_t)&allocator->entries[entry_above], sizeof(struct memory_allocator_entry_t));
        allocator->first_available_free_entry--;
        if(allocator->biggest_free_entry > entry_above) {
            allocator->biggest_free_entry--;
        }
        if(allocator->second_biggest_free_entry != 0xFFFFFFFF && allocator->second_biggest_free_entry > entry_above) {
            allocator->second_biggest_free_entry--;
        }

        // update pointers to biggest free entries
        allocator_update_biggest_entries_pointers(allocator);
    }
    else {
        dword_t edited_entry = 0xFFFFFFFF;

        if(entry_below == 0xFFFFFFFF && entry_above == 0xFFFFFFFF) {
            // check if we can create new free entry
            if(allocator->first_available_free_entry >= allocator->first_used_entry) {
                return;
            }

            // create new free entry
            allocator->entries[allocator->first_available_free_entry].start_of_memory = allocator->entries[entry].start_of_memory;
            allocator->entries[allocator->first_available_free_entry].size_of_memory = allocator->entries[entry].size_of_memory;
            allocator->entries[allocator->first_available_free_entry].end_of_memory = allocator->entries[entry].end_of_memory;
            edited_entry = allocator->first_available_free_entry;
            allocator->first_available_free_entry++;

            // update pointers to biggest free entries
            if(allocator->entries[edited_entry].size_of_memory > allocator->entries[allocator->biggest_free_entry].size_of_memory) {
                // we created biggest free entry
                allocator->second_biggest_free_entry = allocator->biggest_free_entry;
                allocator->biggest_free_entry = edited_entry;
            }
            else if(allocator->second_biggest_free_entry == 0xFFFFFFFF) {
                // there was only one free entry, and we created second biggest free entry
                allocator->second_biggest_free_entry = edited_entry;
            }
            else if(allocator->entries[edited_entry].size_of_memory > allocator->entries[allocator->second_biggest_free_entry].size_of_memory) {
                // we created entry that is biggest than previous second biggest entry, but not bigger than biggest entry, so it is second biggest
                allocator->second_biggest_free_entry = edited_entry;
            }

            // delete used entry
            remove_space_from_memory_area((dword_t)&allocator->entries[allocator->first_used_entry], sizeof(struct memory_allocator_entry_t)*allocator->first_used_entry, (dword_t)&allocator->entries[entry], sizeof(struct memory_allocator_entry_t));
            allocator->first_available_used_entry--;

            return;
        }
        else if(entry_below != 0xFFFFFFFF && entry_above == 0xFFFFFFFF) {
            // extend entry below
            allocator->entries[entry_below].size_of_memory += allocator->entries[entry].size_of_memory;
            allocator->entries[entry_below].end_of_memory = allocator->entries[entry].end_of_memory;
            edited_entry = entry_below;
        }
        else {
            // extend entry above
            allocator->entries[entry_above].start_of_memory = allocator->entries[entry].start_of_memory;
            allocator->entries[entry_above].size_of_memory += allocator->entries[entry].size_of_memory;
            edited_entry = entry_above;
        }

        // delete used entry
        remove_space_from_memory_area((dword_t)&allocator->entries[allocator->first_used_entry], sizeof(struct memory_allocator_entry_t)*allocator->first_used_entry, (dword_t)&allocator->entries[entry], sizeof(struct memory_allocator_entry_t));
        allocator->first_available_used_entry--;

        // update pointers to biggest free entries
        if(edited_entry == allocator->biggest_free_entry) {
            // this only extended already biggest entry, so pointers did not moved
            return;
        }
        else if(edited_entry == allocator->second_biggest_free_entry) {
            // if second biggest entry became bigger than biggest entry, swap them
            if(allocator->entries[edited_entry].size_of_memory > allocator->entries[allocator->biggest_free_entry].size_of_memory) {
                allocator->second_biggest_free_entry = allocator->biggest_free_entry;
                allocator->biggest_free_entry = edited_entry;
            }

            return;
        }
        else if(allocator->entries[edited_entry].size_of_memory > allocator->entries[allocator->second_biggest_free_entry].size_of_memory) {
            if(allocator->entries[edited_entry].size_of_memory > allocator->entries[allocator->biggest_free_entry].size_of_memory) {
                // edited entry became bigger than biggest entry, so move biggest entry to second biggest and make this entry biggest
                allocator->second_biggest_free_entry = allocator->biggest_free_entry;
                allocator->biggest_free_entry = edited_entry;
            }
            else {
                // edited entry became bigger than second biggest entry, but not bigger than biggest entry, so it is now second biggest entry
                allocator->second_biggest_free_entry = edited_entry;
            }
        }
    }
}

/* code for testing new memory allocator */
// struct memory_allocator_t *ma = (struct memory_allocator_t *) malloc(get_size_of_memory_allocator(10));
// initalize_memory_allocator(ma, 10);
// physical_memory_allocator->entries[0].start_of_memory = 0x1000;
// physical_memory_allocator->entries[0].size_of_memory = 1024*1024;
// physical_memory_allocator->entries[0].end_of_memory = 0x1000 + 1024*1024;
// physical_memory_allocator->first_available_free_entry++;
// physical_memory_allocator->biggest_free_entry = 0;

// void *aaa = ma_fixed_calloc(ma, 0x1000);
// logf("\nAllocated memory: %x", (dword_t)aaa);

// void *bbb = ma_malloc(ma, 0x3500);
// logf("\nAllocated memory: %x", (dword_t)bbb);

// void *ccc = ma_malloc(ma, 1024*500+50);
// logf("\nAllocated memory: %x", (dword_t)ccc);

// bbb = ma_realloc(ma, bbb, 0x8000);
// logf("\nAllocated memory: %x", (dword_t)bbb);

// skip_logs();
// logf("\nAllocator position: %x", (dword_t)physical_memory_allocator);
// logf("\nBF entry: %d SBF entry: %d", physical_memory_allocator->biggest_free_entry, physical_memory_allocator->second_biggest_free_entry);
// logf("\nFA free entry: %d FA used entry: %d", physical_memory_allocator->first_available_free_entry, physical_memory_allocator->first_available_used_entry);
// l("\n");
// for(dword_t i = 0; i < 10; i++) {
//     logf("\nFree entry %d: %x %d %x", i, physical_memory_allocator->entries[i].start_of_memory, physical_memory_allocator->entries[i].size_of_memory, physical_memory_allocator->entries[i].end_of_memory);
// }
// l("\n");
// for(dword_t i = 10000; i < 10050; i++) {
//     logf("\nUsed entry %d: %x %d %x", i, physical_memory_allocator->entries[i].start_of_memory, physical_memory_allocator->entries[i].size_of_memory, physical_memory_allocator->entries[i].end_of_memory);
// }
// show_log();
// shutdown();