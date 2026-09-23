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
#include <kernel/libc/string.h>
#include <kernel/hardware/devices/cpu/commands.h>
#include <kernel/hardware/devices/memory/physical_memory.h>
#include <kernel/hardware/devices/memory/virtual_memory.h>
#include <kernel/hardware/groups/logging/logging.h>
#include <kernel/hardware/devices/cpu/mutex.h>
#include <kernel/hardware/devices/cpu/interrupt.h>
#include <kernel/hardware/devices/memory/block_alloc_cells_list.h>

/* local variables */
uint32_t temporary_kernel_allocator_base;
uint32_t temporary_kernel_allocator_free_size;
mutex_t temp_alloc_mutex;

uint32_t permanent_kernel_allocator_base;
uint32_t perm_allocator_num_of_allocations;
uint32_t perm_allocator_requested_size;
uint32_t perm_allocator_real_size;
uint32_t perm_allocator_phy_mem_mapped;
mutex_t perm_alloc_mutex;

mutex_t kheap_mutex;

block_cell_metadata_t **block_alloc_cells;
mutex_t block_alloc_mutex;

/* initialize */
void initialize_allocators(void) {
    // temporary allocator
    temporary_kernel_allocator_base = VM_TEMP_ALLOCATOR_START;
    temporary_kernel_allocator_free_size = PAGE_TABLE_SIZE * 511;
    temp_alloc_mutex.lock = MUTEX_UNLOCKED_VALUE;

    // permanent allocator
    permanent_kernel_allocator_base = VM_PERM_ALLOCATOR_END;
    perm_alloc_mutex.lock = MUTEX_UNLOCKED_VALUE;

    // kernel heap
    vm_allocate_pages(VM_KERNEL_HEAP_START, sizeof(kernel_heap_metadata_t), VM_KERNEL);
    kernel_heap_metadata_t *kernel_heap_metadata = (kernel_heap_metadata_t *) VM_KERNEL_HEAP_START;
    kernel_heap_metadata->memory_start = VM_KERNEL_HEAP_START + sizeof(kernel_heap_metadata_t);
    if(PAGE_OFFSET_MASK(sizeof(kernel_heap_metadata_t)) != 0) {
        kernel_heap_metadata->memory_start = PAGE_MASK(kernel_heap_metadata->memory_start) + PAGE_SIZE;
    }
    kernel_heap_metadata->memory_end = VM_KERNEL_HEAP_END;
    kernel_heap_metadata->memory_size = kernel_heap_metadata->memory_end - kernel_heap_metadata->memory_start;
    kernel_heap_metadata->free_memory_start = kernel_heap_metadata->memory_start;
    kernel_heap_metadata->free_memory_size = kernel_heap_metadata->memory_size;
    kernel_heap_metadata->insert_entry = (kheap_entry_t *) &kernel_heap_metadata->first_entry;
    kernel_heap_metadata->first_entry.end = kernel_heap_metadata->memory_start;
    kernel_heap_metadata->first_entry.prev = NULL;
    kernel_heap_metadata->first_entry.next = NULL;
    kheap_mutex.lock = MUTEX_UNLOCKED_VALUE;

    // block allocator
    block_alloc_cells = (block_cell_metadata_t **) kalloc((block_alloc_cells_list_biggest_entry + 1) * sizeof(block_cell_metadata_t *)); // list of cell metadata, each cell can be accessed by simple index of its size
    for(int i = 0; i < block_alloc_cells_list_count; i++) {
        uint32_t size = block_alloc_cells_list[i];
        if(block_alloc_cells[size] != NULL) {
            continue;
        }

        block_alloc_cells[size] = (block_cell_metadata_t *) kalloc(sizeof(block_cell_metadata_t));
        block_cell_metadata_t *cell = block_alloc_cells[size];
        for(int j = 0; j < NUMBER_OF_PREALLOCATED_BLOCKS_IN_CELL; j++) {
            cell->allocations[j] = kalloc(size);
        }
    }
}

void initialize_allocators_for_user_space(void) {
    log("\n[USER SPACE] %d MB available", (VM_USER_SPACE_END - VM_USER_SPACE_START + (1024 * 1024) - 1) / (1024 * 1024));

    // temporary allocator works in space for user space allocations, so we need to quit it
    quit_temp_alloc();

    // log info about permanent allocator
    log("\n[PERM ALLOCATOR] Booting allocated %d allocations on permanent allocator\n requested size: %d B\n real size: %d KB\n physical memory mapped: %d KB\n Permanent allocator starts at: 0x%x", 
        perm_allocator_num_of_allocations, perm_allocator_requested_size, perm_allocator_real_size / 1024, perm_allocator_phy_mem_mapped / 1024, permanent_kernel_allocator_base);

    // find end of kernel heap
    kernel_heap_metadata_t *kernel_heap_metadata = (kernel_heap_metadata_t *) VM_KERNEL_HEAP_START;
    kheap_entry_t *current_entry = (kheap_entry_t *) &kernel_heap_metadata->first_entry;
    kheap_entry_t *last_entry = current_entry;
    uint32_t end_of_kernel_heap = last_entry->end;
    uint32_t kernel_number_of_allocations = 0;
    uint32_t kernel_allocated_space = 0;
    while(current_entry != NULL) {
        if(current_entry->end > end_of_kernel_heap) {
            last_entry = current_entry;
            end_of_kernel_heap = last_entry->end;
        }
        kernel_number_of_allocations++;
        kernel_allocated_space += current_entry->end - (uint32_t)current_entry + sizeof(kheap_entry_t);

        current_entry = current_entry->next;
    }
    log("\n[KERNEL HEAP] Booting allocated %d allocations on heap\n metadata size: %d B\n allocated space: %d B\n total space: %d B / %d KB\n last entry ends at: 0x%x\n now %d MB available",
        kernel_number_of_allocations,
        sizeof(kernel_heap_metadata_t) * kernel_number_of_allocations,
        kernel_allocated_space,
        kernel_allocated_space + (sizeof(kernel_heap_metadata_t) * kernel_number_of_allocations),
        (kernel_allocated_space + (sizeof(kernel_heap_metadata_t) * kernel_number_of_allocations) + 1023) / 1024,
        last_entry->end,
        (permanent_kernel_allocator_base - VM_KERNEL_HEAP_START + (1024 * 1024) - 1) / (1024 * 1024));
    kernel_heap_metadata->memory_end = permanent_kernel_allocator_base;

    log("\nUser space allocation initialized successfully");
}

/* TEMPORARY ALLOCATOR */
void *temp_phy_alloc(uint32_t phy_start, uint32_t size, uint32_t flags) {
    // align size to page size
    size += PAGE_OFFSET_MASK(phy_start);
    size = (((size + PAGE_SIZE - 1) / PAGE_SIZE) * PAGE_SIZE);

    // check if temporary allocator is working
    LOCK_MUTEX(&temp_alloc_mutex);
    if(temporary_kernel_allocator_base == NULL || temporary_kernel_allocator_free_size == 0 || size > temporary_kernel_allocator_free_size) {
        UNLOCK_MUTEX(&temp_alloc_mutex);
        return NULL;
    }

    // TODO: check if there is not overflow of 32-bit address space

    // allocate pages
    vm_map_phy_pages(temporary_kernel_allocator_base, PAGE_MASK(phy_start), size, flags);

    // update metadata
    void *allocated_address = (void *) (temporary_kernel_allocator_base + PAGE_OFFSET_MASK(phy_start));
    temporary_kernel_allocator_base += size;
    temporary_kernel_allocator_free_size -= size;

    UNLOCK_MUTEX(&temp_alloc_mutex);
    return allocated_address;
}

void quit_temp_alloc(void) {
    vm_unmap_pages(VM_TEMP_ALLOCATOR_START, temporary_kernel_allocator_base - VM_TEMP_ALLOCATOR_START);
    temporary_kernel_allocator_base = NULL;
    temporary_kernel_allocator_free_size = 0;
}

/* PERMANENT ALLOCATOR */
void *perm_alloc(uint32_t size) {
    // align size to page size
    perm_allocator_num_of_allocations++;
    perm_allocator_requested_size += size;
    size = (((size + PAGE_SIZE - 1) / PAGE_SIZE) * PAGE_SIZE);

    // check if permanent allocator is working
    LOCK_MUTEX(&perm_alloc_mutex);
    if((permanent_kernel_allocator_base - size) < VM_PERM_ALLOCATOR_START) {
        UNLOCK_MUTEX(&perm_alloc_mutex);
        return NULL;
    }

    // TODO: check if there is not overflow of 32-bit address space

    // allocate in kernel memory
    permanent_kernel_allocator_base -= size;
    vm_allocate_pages(permanent_kernel_allocator_base, size, VM_KERNEL | VM_FLAG_GLOBAL);
    void *allocated_address = (void *) permanent_kernel_allocator_base;
    perm_allocator_real_size += size;

    UNLOCK_MUTEX(&perm_alloc_mutex);
    return allocated_address;
}

void *perm_phy_alloc(uint32_t phy_start, uint32_t size, uint32_t flags) {
    // align size to page size
    perm_allocator_num_of_allocations++;
    size += PAGE_OFFSET_MASK(phy_start);
    size = (((size + PAGE_SIZE - 1) / PAGE_SIZE) * PAGE_SIZE);

    // check if permanent allocator is working
    LOCK_MUTEX(&perm_alloc_mutex);
    if((permanent_kernel_allocator_base - size) < VM_PERM_ALLOCATOR_START) {
        UNLOCK_MUTEX(&perm_alloc_mutex);
        return NULL;
    }

    // allocate in kernel memory
    permanent_kernel_allocator_base -= size;
    vm_map_phy_pages(permanent_kernel_allocator_base, PAGE_MASK(phy_start), size, flags | VM_FLAG_GLOBAL);
    void *allocated_address = (void *) (permanent_kernel_allocator_base + PAGE_OFFSET_MASK(phy_start));
    perm_allocator_phy_mem_mapped += size;

    UNLOCK_MUTEX(&perm_alloc_mutex);
    return allocated_address;
}

/* KERNEL HEAP */
void kheap_dump(void) {
    kernel_heap_metadata_t *kernel_heap_metadata = (kernel_heap_metadata_t *) VM_KERNEL_HEAP_START;
    log("\nACTUAL FREE POINTER: 0x%x - %d - 0x%x",
        kernel_heap_metadata->free_memory_start,
        kernel_heap_metadata->free_memory_size,
        kernel_heap_metadata->free_memory_start + kernel_heap_metadata->free_memory_size);
    kheap_entry_t *current_entry = (kheap_entry_t *) &kernel_heap_metadata->first_entry;
    while(current_entry != NULL) {
        log("\nENTRY: metadata=0x%x, start=0x%x, end=0x%x, prev=0x%x, next=0x%x, size=%d",
            current_entry,
            (uint32_t)current_entry + sizeof(kheap_entry_t),
            current_entry->end,
            current_entry->prev,
            current_entry->next,
            current_entry->end - (uint32_t)current_entry + sizeof(kheap_entry_t));
        if((uint32_t)current_entry->next != current_entry->end && current_entry->next != NULL) {
            log("\nFREE: start=0x%x, end=0x%x, size=%d",
                current_entry->end,
                current_entry->next,
                (uint32_t)current_entry->next - current_entry->end);
        }
        current_entry = (kheap_entry_t *) current_entry->next;
    }
}

void *kalloc(uint32_t size) {
    if(size == 0) {
        return NULL;
    }

    // log("\nKALLOC request %d bytes", size);

    // extend size for allocation metadata
    size += sizeof(kheap_entry_t);

    // align size up to 4 bytes
    if((size & 0x3) != 0x0) {
        size = ((size & 0xFFFFFFFC) + 0x4);
    }

    // find suitable space
    LOCK_MUTEX(&kheap_mutex);
    kernel_heap_metadata_t *kernel_heap_metadata = (kernel_heap_metadata_t *) VM_KERNEL_HEAP_START;
    if(size > kernel_heap_metadata->free_memory_size) {
        // find biggest free space block
        uint32_t biggest_space = 0; kheap_entry_t *biggest_space_entry;
        kheap_entry_t *current_entry = (kheap_entry_t *) &kernel_heap_metadata->first_entry;
        while(current_entry != NULL) {
            uint32_t free_space_size = (current_entry->next == NULL) ? (kernel_heap_metadata->memory_end - current_entry->end) : ((uint32_t)current_entry->next - current_entry->end);
            if(free_space_size > biggest_space) {
                biggest_space = free_space_size;
                biggest_space_entry = current_entry;
            }
            current_entry = current_entry->next;
        }
        if(biggest_space == 0) {
            UNLOCK_MUTEX(&kheap_mutex);
            return NULL;
        }

        // update allocation block
        kernel_heap_metadata->free_memory_start = biggest_space_entry->end;
        kernel_heap_metadata->free_memory_size = biggest_space;
        kernel_heap_metadata->insert_entry = biggest_space_entry;

        // check if it is enough
        if(biggest_space < size) {
            UNLOCK_MUTEX(&kheap_mutex);
            return NULL;
        }
    }

    // create allocation metadata
    kheap_entry_t *new_entry = (kheap_entry_t *) kernel_heap_metadata->free_memory_start;
    uint32_t new_entry_end = (uint32_t)new_entry + size;
    kheap_entry_t *new_entry_prev = kernel_heap_metadata->insert_entry;
    kheap_entry_t *new_entry_next = kernel_heap_metadata->insert_entry->next;

    // update global metadata
    kernel_heap_metadata->free_memory_start += size;
    kernel_heap_metadata->free_memory_size -= size;
    if(new_entry_next != NULL) {
        new_entry_next->prev = new_entry;
    }
    kernel_heap_metadata->insert_entry->next = new_entry;
    kernel_heap_metadata->insert_entry = new_entry;

    // map pages
    uint32_t first_page = PAGE_MASK((uint32_t)new_entry);
    if(PAGE_OFFSET_MASK(new_entry_prev->end) != 0 && PAGE_MASK(new_entry_prev->end) == first_page) {
        first_page += PAGE_SIZE;
    }
    uint32_t last_page = PAGE_MASK(new_entry_end - 1);
    if(new_entry_next != NULL && PAGE_MASK((uint32_t)new_entry_next) == last_page) {
        last_page -= PAGE_SIZE;
    }
    if(last_page >= first_page) {
        vm_allocate_pages(first_page, last_page - first_page + PAGE_SIZE, VM_KERNEL);
    }

    // clear allocation
    memset((void *) new_entry, 0, size);

    // save allocation metadata
    new_entry->end = new_entry_end;
    new_entry->prev = new_entry_prev;
    new_entry->next = new_entry_next;

    void *allocated_address = (void *) ((uint32_t)new_entry + sizeof(kheap_entry_t));
    UNLOCK_MUTEX(&kheap_mutex);
    // log(" allocated at %x", allocated_address);
    return allocated_address;
}

void kfree(void *allocation) {
    if(allocation == NULL) {
        return;
    }

    // log("\nKFREE request %x", allocation);

    LOCK_MUTEX(&kheap_mutex);
    kernel_heap_metadata_t *kernel_heap_metadata = (kernel_heap_metadata_t *) VM_KERNEL_HEAP_START;
    kheap_entry_t *entry = (kheap_entry_t *) ((uint32_t)allocation - sizeof(kheap_entry_t));
    uint32_t entry_start = (uint32_t) entry;
    uint32_t entry_end = entry->end;
    uint32_t entry_size = entry_end - entry_start;
    kheap_entry_t *prev_entry = entry->prev;
    kheap_entry_t *next_entry = entry->next;

    // unmap pages
    uint32_t first_page = PAGE_MASK(entry_start);
    if(PAGE_MASK(prev_entry->end - 1) == first_page) {
        first_page += PAGE_SIZE;
    }
    uint32_t last_page = PAGE_MASK(entry_end - 1);
    if(next_entry != NULL && PAGE_MASK((uint32_t)next_entry) == last_page) {
        last_page -= PAGE_SIZE;
    }
    if(last_page >= first_page) {
        vm_deallocate_pages(first_page, last_page - first_page + PAGE_SIZE);
    }

    // remove entry from linked list
    prev_entry->next = next_entry;
    if(next_entry != NULL) {
        next_entry->prev = prev_entry;
    }

    // check new free space left after this entry and update allocator if it is bigger
    uint32_t free_space_size = (next_entry == NULL) ? (kernel_heap_metadata->memory_end - prev_entry->end) : ((uint32_t)next_entry - prev_entry->end);
    if(free_space_size > kernel_heap_metadata->free_memory_size) {
        kernel_heap_metadata->free_memory_start = prev_entry->end;
        kernel_heap_metadata->free_memory_size = free_space_size;
        kernel_heap_metadata->insert_entry = prev_entry;
    }

    UNLOCK_MUTEX(&kheap_mutex);
}

void *krealloc(void *allocation, uint32_t new_size) {
    if(allocation == NULL) {
        return kalloc(new_size);
    }

    // log("\nKREALLOC request %x to %d bytes", allocation, new_size);

    LOCK_MUTEX(&kheap_mutex);
    kernel_heap_metadata_t *kernel_heap_metadata = (kernel_heap_metadata_t *) VM_KERNEL_HEAP_START;
    kheap_entry_t *entry = (kheap_entry_t *) ((uint32_t)allocation - sizeof(kheap_entry_t));
    uint32_t entry_start = (uint32_t) entry;
    uint32_t entry_end = entry->end;
    uint32_t entry_size = entry_end - entry_start;

    // extend size for allocation metadata
    new_size += sizeof(kheap_entry_t);

    // align size up to 4 bytes
    if((new_size & 0x3) != 0x0) {
        new_size = ((new_size & 0xFFFFFFFC) + 0x4);
    }

    // reallocate according to request
    if(new_size == sizeof(kheap_entry_t)) { // free allocation
        UNLOCK_MUTEX(&kheap_mutex);
        kfree(allocation);
        return NULL;
    }
    else if(new_size == entry_size) { // do nothing
        UNLOCK_MUTEX(&kheap_mutex);
        // log(" REALLOCATED IN SAME PLACE");
        return allocation;
    }
    else if(new_size < entry_size) { // shrink allocation
        uint32_t size_diff = entry_size - new_size;
        uint32_t entry_new_end = entry_start + new_size;
        entry->end = entry_new_end;

        // check new free space left after this entry and update allocator if it is bigger
        uint32_t free_space_size = (entry->next == NULL) ? (kernel_heap_metadata->memory_end - entry_new_end) : ((uint32_t)entry->next - entry_new_end);
        if(free_space_size > kernel_heap_metadata->free_memory_size) {
            kernel_heap_metadata->free_memory_start = entry_new_end;
            kernel_heap_metadata->free_memory_size = free_space_size;
            kernel_heap_metadata->insert_entry = entry;
        }

        // unmap pages
        uint32_t first_page = PAGE_MASK(entry_new_end);
        if(PAGE_OFFSET_MASK(entry_new_end) != 0) {
            first_page += PAGE_SIZE;
        }
        uint32_t last_page = PAGE_MASK(entry_end - 1);
        if(entry->next != NULL && PAGE_MASK((uint32_t)entry->next) == last_page) {
            last_page -= PAGE_SIZE;
        }
        if(last_page >= first_page) {
            vm_deallocate_pages(first_page, last_page - first_page + PAGE_SIZE);
        }

        UNLOCK_MUTEX(&kheap_mutex);
        // log(" REALLOCATED IN SAME PLACE");
        return allocation;
    }
    else { // expand allocation
        uint32_t size_diff = new_size - entry_size;

        // try to expand allocation in place
        uint32_t available_space = (entry->next == NULL) ? (kernel_heap_metadata->memory_end - entry_end) : ((uint32_t)entry->next - entry_end);
        if(size_diff <= available_space) {
            uint32_t entry_new_end = entry_start + new_size;
            entry->end = entry_new_end;

            // update allocator if it was right after this entry
            if(kernel_heap_metadata->insert_entry == entry) {
                kernel_heap_metadata->free_memory_start += size_diff;
                kernel_heap_metadata->free_memory_size -= size_diff;
            }

            // map new pages
            uint32_t first_page = PAGE_MASK(entry_end);
            if(PAGE_OFFSET_MASK(entry_end) != 0) {
                first_page += PAGE_SIZE;
            }
            uint32_t last_page = PAGE_MASK(entry_new_end - 1);
            if(entry->next != NULL && PAGE_MASK((uint32_t)entry->next) == last_page) {
                last_page -= PAGE_SIZE;
            }
            if(last_page >= first_page) {
                vm_allocate_pages(first_page, last_page - first_page + PAGE_SIZE, VM_KERNEL);
            }

            UNLOCK_MUTEX(&kheap_mutex);
            // log(" REALLOCATED IN SAME PLACE");
            return allocation;
        }

        // check if we would free this entry, whether new free space would be enough
        available_space = (entry->next == NULL) ? (kernel_heap_metadata->memory_end - entry->prev->end) : ((uint32_t)entry->next - entry->prev->end);
        if(new_size <= available_space) {
            uint32_t entry_new_start = entry->prev->end;
            uint32_t entry_new_end = entry_new_start + new_size;
            
            // check new free space left after this entry and update allocator if it is bigger
            uint32_t free_space_size = (available_space - new_size);
            if(free_space_size > kernel_heap_metadata->free_memory_size) {
                kernel_heap_metadata->free_memory_start = entry_new_end;
                kernel_heap_metadata->free_memory_size = free_space_size;
                kernel_heap_metadata->insert_entry = entry;
            }

            // map new pages below current entry
            uint32_t first_page = PAGE_MASK(entry_new_start);
            if(PAGE_OFFSET_MASK(entry_new_start) != 0) {
                first_page += PAGE_SIZE;
            }
            uint32_t last_page;
            if((entry_new_end - 1) < entry_start) { // last page is below current entry mapping
                last_page = PAGE_MASK(entry_new_end - 1);
                if(last_page == PAGE_MASK(entry_start)) {
                    last_page -= PAGE_SIZE;
                }
            }
            else { // last page overlaps current entry mapping
                last_page = (PAGE_MASK(entry_start) - PAGE_SIZE);
            }
            if(last_page >= first_page) { // map all pages that are below current entry and should be mapped
                vm_allocate_pages(first_page, last_page - first_page + PAGE_SIZE, VM_KERNEL);
            }

            // map new pages above current entry
            if(entry_new_end > entry_end) {
                first_page = PAGE_MASK(entry_end);
                if(PAGE_OFFSET_MASK(entry_end) != 0) {
                    first_page += PAGE_SIZE;
                }
                last_page = PAGE_MASK(entry_new_end - 1);
                if(entry->next != NULL && PAGE_MASK((uint32_t)entry->next) == last_page) {
                    last_page -= PAGE_SIZE;
                }
                if(last_page >= first_page) { // map all pages that are above current entry and should be mapped
                    vm_allocate_pages(first_page, last_page - first_page + PAGE_SIZE, VM_KERNEL);
                }
            }

            // copy data
            kheap_entry_t *next_entry = entry->next;
            memmove((void *)entry_new_start, (void *)entry_start, entry_size);

            // unmap pages that are unused after reallocation
            if(entry_new_end < entry_end) {
                first_page = PAGE_MASK(entry_new_end); // page right after reallocation
                if(PAGE_OFFSET_MASK(entry_new_end) != 0) {
                    first_page += PAGE_SIZE;
                }
                if(first_page < PAGE_MASK(entry_start)) { // whole reallocation is below current entry mapping
                    first_page = PAGE_MASK(entry_start);
                }
                last_page = PAGE_MASK(entry_end - 1);
                if(PAGE_MASK(entry_new_end) == last_page || (next_entry != NULL && PAGE_MASK((uint32_t)next_entry) == last_page)) {
                    last_page -= PAGE_SIZE;
                }
                if(last_page >= first_page) {
                    vm_deallocate_pages(first_page, last_page - first_page + PAGE_SIZE);
                }
            }

            // update entry
            entry = (kheap_entry_t *) entry_new_start;
            entry->end = entry_new_end;

            // update linked list
            entry->prev->next = entry;
            if(next_entry != NULL) {
                next_entry->prev = entry;
            }

            void *entry_new_start_ptr = (void *) (entry_new_start + sizeof(kheap_entry_t));
            UNLOCK_MUTEX(&kheap_mutex);
            // log(" REALLOCATED: 0x%x -> 0x%x", allocation, entry_new_start_ptr);
            return entry_new_start_ptr;
        }

        // else try to allocate new block
        UNLOCK_MUTEX(&kheap_mutex);
        void *new_allocation = kalloc(new_size - sizeof(kheap_entry_t));
        if(new_allocation == NULL) {
            return NULL;
        }

        // copy data
        memcpy(new_allocation, allocation, (entry_size - sizeof(kheap_entry_t)));

        // free old block
        kfree(allocation);

        // log(" REALLOCATED: 0x%x -> 0x%x", allocation, new_allocation);
        return new_allocation;
    }
}

/* BLOCK ALLOCATOR*/
void *block_alloc(uint32_t size) {
    LOCK_MUTEX(&block_alloc_mutex);
    block_cell_metadata_t *metadata = block_alloc_cells[size];

    // if cell is full, allocate new one
    if(metadata->lifo_index == NUMBER_OF_PREALLOCATED_BLOCKS_IN_CELL) {
        if(metadata->next != NULL) {
            metadata = metadata->next;
        }
        else {
            // create new cell
            metadata->next = kalloc(sizeof(block_cell_metadata_t));
            metadata->next->prev = metadata;
            metadata = metadata->next;
            for(int i = 0; i < NUMBER_OF_PREALLOCATED_BLOCKS_IN_CELL; i++) {
                metadata->allocations[i] = kalloc(size);
            }
        }
        block_alloc_cells[size] = metadata; // update pointer to cell with available blocks
    }

    // pop allocation
    void *allocation = metadata->allocations[metadata->lifo_index++];

    UNLOCK_MUTEX(&block_alloc_mutex);
    return allocation;
}

void block_free(uint32_t size, void *allocation) {
    LOCK_MUTEX(&block_alloc_mutex);
    block_cell_metadata_t *metadata = block_alloc_cells[size];

    // if cell is empty, move to previous cell
    if(metadata->lifo_index == 0) {
        if(metadata->prev == NULL) {
            log("\n[ERROR] Block allocator is empty");
            UNLOCK_MUTEX(&block_alloc_mutex);
            return;
        }
        metadata = metadata->prev;
    }

    // push allocation
    metadata->allocations[--metadata->lifo_index] = allocation;

    // delete next cell if there is enough space in actual cell
    if(metadata->lifo_index == (NUMBER_OF_PREALLOCATED_BLOCKS_IN_CELL / 2) && metadata->next != NULL) {
        block_cell_metadata_t *next_metadata = metadata->next;
        for(int i = 0; i < NUMBER_OF_PREALLOCATED_BLOCKS_IN_CELL; i++) {
            kfree(next_metadata->allocations[i]);
        }
        kfree(next_metadata);
        metadata->next = NULL;
        block_alloc_cells[size] = metadata;
    }

    UNLOCK_MUTEX(&block_alloc_mutex);
}