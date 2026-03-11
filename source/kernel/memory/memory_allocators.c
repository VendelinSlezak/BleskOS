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
#include <libc/string.h>
#include <kernel/cpu/commands.h>
#include <kernel/memory/physical_memory.h>
#include <kernel/memory/virtual_memory.h>
#include <kernel/hardware/groups/logging/logging.h>
#include <kernel/cpu/mutex.h>
#include <kernel/cpu/interrupt.h>

/* local variables */
uint32_t temporary_kernel_allocator_base;
uint32_t temporary_kernel_allocator_free_size;
uint32_t user_space_allocation_start;
uint32_t user_space_allocation_size;
uint32_t user_space_allocation_end;
uint32_t number_of_shared_page_tables;

mutex_t temp_alloc_mutex;
mutex_t perm_alloc_mutex;
mutex_t kheap_mutex;

/* global variables */
uint32_t permanent_kernel_allocator_base;

/* initialize */
void initialize_allocators(void) {
    // temporary allocator
    temporary_kernel_allocator_base = TEMPORARY_KERNEL_ALLOCATOR_BASE;
    temporary_kernel_allocator_free_size = MEM_KERNEL_HEAP_START - TEMPORARY_KERNEL_ALLOCATOR_BASE;

    // permanent allocator
    permanent_kernel_allocator_base = MEM_KERNEL_HEAP_START;

    // kernel heap
    vm_allocate_pages(MEM_KERNEL_HEAP_START, sizeof(kernel_heap_metadata_t), VM_KERNEL);
    kernel_heap_metadata_t *kernel_heap_metadata = (kernel_heap_metadata_t *) MEM_KERNEL_HEAP_START;
    kernel_heap_metadata->memory_start = MEM_KERNEL_HEAP_START + sizeof(kernel_heap_metadata_t);
    if(PAGE_OFFSET_MASK(sizeof(kernel_heap_metadata_t)) != 0) {
        kernel_heap_metadata->memory_start = PAGE_MASK(kernel_heap_metadata->memory_start) + PAGE_SIZE;
    }
    kernel_heap_metadata->memory_size = PT_MEM_KERNEL - kernel_heap_metadata->memory_start;
    kernel_heap_metadata->memory_end = PT_MEM_KERNEL;
    kernel_heap_metadata->free_memory_start = kernel_heap_metadata->memory_start;
    kernel_heap_metadata->free_memory_size = kernel_heap_metadata->memory_size;
    kernel_heap_metadata->insert_entry = (kheap_entry_t *) &kernel_heap_metadata->first_entry;
    kernel_heap_metadata->first_entry.end = kernel_heap_metadata->memory_start;
    kernel_heap_metadata->first_entry.prev = NULL;
    kernel_heap_metadata->first_entry.next = NULL;

    // user space allocations
    user_space_allocation_start = NULL;
    user_space_allocation_end = NULL;
}

void initialize_user_space_allocation(void) {
    // first 4 MB block is for user stack, then there are blocks for user space and blocks with and above permanent allocator are for kernel
    user_space_allocation_start = MEM_USER_HEAP_BASE;
    user_space_allocation_size = ((permanent_kernel_allocator_base / 0x400000) * 0x400000) - user_space_allocation_start;
    user_space_allocation_end = user_space_allocation_start + user_space_allocation_size;
    number_of_shared_page_tables = (0 - user_space_allocation_end) / 0x400000;

    log("\nUser space allocation: start=0x%x, end=0x%x, size=0x%x, num=%d", user_space_allocation_start, user_space_allocation_end, user_space_allocation_size, number_of_shared_page_tables);

    // temporary allocator works in space for user space allocations, so we need to quit it
    quit_temp_alloc();

    log("\nUser space allocation initialized successfully");
}

/* TEMPORARY ALLOCATOR */
void *temp_phy_alloc(uint32_t phy_start, uint32_t size, uint32_t flags) {
    LOCK_MUTEX(&temp_alloc_mutex);

    // check if temporary allocator is working
    if(temporary_kernel_allocator_base == NULL || temporary_kernel_allocator_free_size == 0 || size > temporary_kernel_allocator_free_size) {
        UNLOCK_MUTEX(&temp_alloc_mutex);
        return NULL;
    }

    // align size to page size
    size += PAGE_OFFSET_MASK(phy_start);
    if(PAGE_OFFSET_MASK(size) != 0) {
        size = PAGE_MASK(size) + PAGE_SIZE;
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
    vm_unmap_pages(TEMPORARY_KERNEL_ALLOCATOR_BASE, temporary_kernel_allocator_base - TEMPORARY_KERNEL_ALLOCATOR_BASE);
    temporary_kernel_allocator_base = NULL;
    temporary_kernel_allocator_free_size = 0;
}

/* PERMANENT ALLOCATOR */
void *perm_alloc(uint32_t size) {
    // align size to page size
    if(PAGE_OFFSET_MASK(size) != 0) {
        size = PAGE_MASK(size) + PAGE_SIZE;
    }

    // TODO: check if there is not overflow of 32-bit address space

    // allocate in kernel memory
    LOCK_MUTEX(&perm_alloc_mutex);
    permanent_kernel_allocator_base -= size;
    vm_allocate_pages(permanent_kernel_allocator_base, size, VM_KERNEL | VM_FLAG_GLOBAL);

    // update temporary allocator
    temporary_kernel_allocator_free_size = permanent_kernel_allocator_base - temporary_kernel_allocator_base;
    void *allocated_address = (void *) permanent_kernel_allocator_base;

    UNLOCK_MUTEX(&perm_alloc_mutex);
    return allocated_address;
}

void *perm_phy_alloc(uint32_t phy_start, uint32_t size, uint32_t flags) {
    // align size to page size
    size += PAGE_OFFSET_MASK(phy_start);
    if(PAGE_OFFSET_MASK(size) != 0) {
        size = PAGE_MASK(size) + PAGE_SIZE;
    }

    // allocate in kernel memory
    LOCK_MUTEX(&perm_alloc_mutex);
    permanent_kernel_allocator_base -= size;
    vm_map_phy_pages(permanent_kernel_allocator_base, PAGE_MASK(phy_start), size, flags | VM_FLAG_GLOBAL);

    // update temporary allocator
    temporary_kernel_allocator_free_size = permanent_kernel_allocator_base - temporary_kernel_allocator_base;
    void *allocated_address = (void *) (permanent_kernel_allocator_base + PAGE_OFFSET_MASK(phy_start));

    UNLOCK_MUTEX(&perm_alloc_mutex);
    return allocated_address;
}

/* KERNEL HEAP */
void *kalloc(uint32_t size) {
    if(size == 0) {
        return NULL;
    }

    // extend size for allocation metadata
    size += sizeof(kheap_entry_t);

    // align size up to 4 bytes
    if((size & 0x3) != 0x0) {
        size = ((size & 0xFFFFFFFC) + 0x4);
    }

    // find suitable space
    LOCK_MUTEX(&kheap_mutex);
    kernel_heap_metadata_t *kernel_heap_metadata = (kernel_heap_metadata_t *) MEM_KERNEL_HEAP_START;
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
    return allocated_address;
}

void kfree(void *allocation) {
    if(allocation == NULL) {
        return;
    }

    LOCK_MUTEX(&kheap_mutex);
    kernel_heap_metadata_t *kernel_heap_metadata = (kernel_heap_metadata_t *) MEM_KERNEL_HEAP_START;
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

    LOCK_MUTEX(&kheap_mutex);
    kernel_heap_metadata_t *kernel_heap_metadata = (kernel_heap_metadata_t *) MEM_KERNEL_HEAP_START;
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

        UNLOCK_MUTEX(&kheap_mutex);
        return new_allocation;
    }
}