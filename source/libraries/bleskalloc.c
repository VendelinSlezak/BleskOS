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
#include <kernel/memory/virtual_memory.h>
#include <kernel/hardware/main.h>
#include <kernel/software/syscall.h>
#include <libraries/main.h>
#include <libraries/string.h>

/* local variables */
bleskalloc_metadata_t *bleskalloc_metadata;

/* functions */
void syslib_initialize_bleskalloc(void) {
    if(syslib->userspace_size < sizeof(bleskalloc_metadata_t)) {
        syslib->log("Not enough memory for bleskalloc");
        return;

        // TODO: panic
    }
    bleskalloc_metadata = (bleskalloc_metadata_t *) syslib->userspace_start;
    syslib->userspace_start += sizeof(bleskalloc_metadata_t);
    syslib->userspace_size -= sizeof(bleskalloc_metadata_t);
    uint32_t size_to_next_page_table_boundary = 0x00400000 - (syslib->userspace_start & 0x003FFFFF);
    if(size_to_next_page_table_boundary != 0x00400000) {
        syslib->userspace_start += size_to_next_page_table_boundary;
        syslib->userspace_size -= size_to_next_page_table_boundary;
    }

    bleskalloc_metadata->memory_start = syslib->userspace_start;
    bleskalloc_metadata->memory_size = syslib->userspace_size;
    bleskalloc_metadata->memory_end = bleskalloc_metadata->memory_start + bleskalloc_metadata->memory_size;
    bleskalloc_metadata->free_memory_start = bleskalloc_metadata->memory_start;
    bleskalloc_metadata->free_memory_size = bleskalloc_metadata->memory_size;
    bleskalloc_metadata->insert_entry = (bleskalloc_entry_t *) &bleskalloc_metadata->first_entry;
    bleskalloc_metadata->first_entry.start = bleskalloc_metadata->memory_start;
    bleskalloc_metadata->first_entry.end = bleskalloc_metadata->memory_start;
    bleskalloc_metadata->first_entry.prev = NULL;
    bleskalloc_metadata->first_entry.next = NULL;
    memset(bleskalloc_metadata->number_of_allocated_pages_in_page_table, 0, sizeof(bleskalloc_metadata->number_of_allocated_pages_in_page_table));
}

void userspace_allocate_pages(uint32_t virtual_address, uint32_t number_of_pages) {
    // map pages if it is faster than page faults
    if(number_of_pages > 8) {
        syscall_map_pages(virtual_address, number_of_pages * PAGE_SIZE);
    }

    uint32_t pt_first = virtual_address >> 22;
    uint32_t pt_last  = (virtual_address + (number_of_pages << 12) - 1) >> 22;

    if(pt_first == pt_last) {
        bleskalloc_metadata->number_of_allocated_pages_in_page_table[pt_first] += number_of_pages; // update only page table
    }
    else {
        uint32_t offset_in_pt = virtual_address & 0x3FFFFF;
        uint32_t pages_in_first = (0x400000 - offset_in_pt) >> 12;

        // update first page table
        bleskalloc_metadata->number_of_allocated_pages_in_page_table[pt_first] += pages_in_first;
        
        uint32_t remaining = number_of_pages - pages_in_first;

        // update middle page tables
        if(pt_last > pt_first + 1) {
            uint32_t mid_pts = pt_last - pt_first - 1;
            for(uint32_t pt = pt_first + 1; pt < pt_last; pt++) {
                bleskalloc_metadata->number_of_allocated_pages_in_page_table[pt] = 1024;
            }
            remaining -= (mid_pts << 10);
        }

        // update last page table
        bleskalloc_metadata->number_of_allocated_pages_in_page_table[pt_last] += remaining;
    }
}

void userspace_deallocate_pages(uint32_t virtual_address, uint32_t number_of_pages) {
    uint32_t current_addr = virtual_address;
    uint32_t end_addr = virtual_address + (number_of_pages << 12);

    while(current_addr < end_addr) {
        uint32_t pt_index = current_addr >> 22;
        uint32_t next_pt_boundary = (pt_index + 1) << 22;
        uint32_t remaining_in_range = (end_addr - current_addr) >> 12;
        uint32_t space_to_boundary = (next_pt_boundary - current_addr) >> 12;
        uint32_t to_remove = (remaining_in_range < space_to_boundary) ? remaining_in_range : space_to_boundary;
        bleskalloc_metadata->number_of_allocated_pages_in_page_table[pt_index] -= to_remove;

        // unmap page table if it is empty
        if(bleskalloc_metadata->number_of_allocated_pages_in_page_table[pt_index] == 0) {
            syscall_unmap_pages(pt_index << 22, 1024 * PAGE_SIZE);
        }

        current_addr += (to_remove << 12);
    }
}

void *syslib_malloc(uint32_t size) {
    // align size
    if(size == 0) {
        return NULL;
    }
    size = (size + sizeof(bleskalloc_entry_t) + 3) & ~3U;

    // load metadata
    bleskalloc_metadata_t *meta = bleskalloc_metadata;
    bleskalloc_entry_t *new_entry = (bleskalloc_entry_t *)meta->free_memory_start;
    bleskalloc_entry_t *insert_node = meta->insert_entry;
    uint32_t new_start = (uint32_t)new_entry;
    bleskalloc_entry_t *next_node = insert_node->next;
    uint32_t new_end = new_start + size;

    // check if there is enough free space
    if(size > meta->free_memory_size) {
        // go through whole memory map to fing biggest block
        uint32_t biggest_space = 0;
        bleskalloc_entry_t *biggest_node = NULL;
        bleskalloc_entry_t *curr = (bleskalloc_entry_t *)&meta->first_entry;
        uint32_t mem_end = meta->memory_end;
        while(curr != NULL) {
            bleskalloc_entry_t *next = curr->next;
            uint32_t current_end = curr->end;
            uint32_t limit = (next == NULL) ? mem_end : next->start;
            uint32_t available = limit - current_end;
            if(available > biggest_space) {
                biggest_space = available;
                biggest_node = curr;
            }
            curr = next;
        }

        // update allocation block
        meta->free_memory_start = biggest_node->end;
        meta->free_memory_size = biggest_space;
        meta->insert_entry = biggest_node;

        // check if it is enough
        if(biggest_space < size) {
            // syslib->log(biggest_space == 0 ? "\nbleskalloc: no free space found" : "\nbleskalloc: not enough free space");
            return NULL;
        }

        // update metadata
        insert_node = meta->insert_entry;
        next_node = insert_node->next;
        new_entry = (bleskalloc_entry_t *)meta->free_memory_start;
        new_start = (uint32_t)new_entry;
        new_end = new_start + size;
    }

    // allocate memory from metadata
    meta->free_memory_start += size;
    meta->free_memory_size -= size;
    meta->insert_entry = new_entry;

    // map pages
    uint32_t first_page = PAGE_MASK(new_start);
    uint32_t prev_end_page = PAGE_MASK(insert_node->end);
    if(prev_end_page == first_page && PAGE_OFFSET_MASK(insert_node->end) != 0) {
        first_page += PAGE_SIZE;
    }
    uint32_t last_page = PAGE_MASK(new_end - 1);
    if(next_node) {
        next_node->prev = new_entry; // update linked list
        last_page -= (PAGE_MASK(next_node->start) == last_page) ? PAGE_SIZE : 0;
    }
    if(last_page >= first_page) {
        uint32_t num_pages = (last_page - first_page + PAGE_SIZE) >> 12;
        userspace_allocate_pages(first_page, num_pages);
    }

    // save allocation metadata
    new_entry->start = new_start;
    new_entry->end = new_end;
    new_entry->prev = insert_node;
    new_entry->next = next_node;

    // update linked list
    insert_node->next = new_entry;

    return (void *)(new_start + sizeof(bleskalloc_entry_t));
}

void *syslib_calloc(uint32_t number, uint32_t size) {
    void *ptr = syslib_malloc(number * size);
    if(ptr != NULL) {
        memset(ptr, 0, number * size);
    }
    return ptr;
}

void *syslib_realloc(void *allocation, uint32_t new_size) {
    if(allocation == NULL) {
        return syslib_malloc(new_size);
    }

    bleskalloc_entry_t *entry = (bleskalloc_entry_t *) ((uint32_t)allocation - sizeof(bleskalloc_entry_t));
    uint32_t entry_start = entry->start;
    uint32_t entry_end = entry->end;
    uint32_t entry_size = entry_end - entry_start;

    // extend size for allocation metadata
    new_size += sizeof(bleskalloc_entry_t);

    // align size up to 4 bytes
    new_size = (new_size + 3) & ~0x3;

    // reallocate according to request
    if(new_size == sizeof(bleskalloc_entry_t)) { // free allocation
        syslib_free(allocation);
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
        uint32_t free_space_size = (entry->next == NULL) ? (bleskalloc_metadata->memory_end - entry_new_end) : (entry->next->start - entry_new_end);
        if(free_space_size > bleskalloc_metadata->free_memory_size) {
            bleskalloc_metadata->free_memory_start = entry_new_end;
            bleskalloc_metadata->free_memory_size = free_space_size;
            bleskalloc_metadata->insert_entry = entry;
        }

        // unmap pages
        uint32_t first_page = PAGE_MASK(entry_new_end);
        if(PAGE_OFFSET_MASK(entry_new_end) != 0) {
            first_page += PAGE_SIZE;
        }
        uint32_t last_page = PAGE_MASK(entry_end - 1);
        if(entry->next != NULL && PAGE_MASK(entry->next->start) == last_page) {
            last_page -= PAGE_SIZE;
        }
        if(last_page >= first_page) {
            userspace_deallocate_pages(first_page, (last_page - first_page + PAGE_SIZE) / PAGE_SIZE);
        }

        return allocation;
    }
    else { // expand allocation
        uint32_t size_diff = new_size - entry_size;

        // try to expand allocation in place
        uint32_t available_space = (entry->next == NULL) ? (bleskalloc_metadata->memory_end - entry_end) : (entry->next->start - entry_end);
        if(size_diff <= available_space) {
            uint32_t entry_new_end = entry_start + new_size;
            entry->end = entry_new_end;

            // update allocator if it was right after this entry
            if(bleskalloc_metadata->insert_entry == entry) {
                bleskalloc_metadata->free_memory_start += size_diff;
                bleskalloc_metadata->free_memory_size -= size_diff;
            }

            // map new pages
            uint32_t first_page = PAGE_MASK(entry_end);
            if(PAGE_OFFSET_MASK(entry_end) != 0) {
                first_page += PAGE_SIZE;
            }
            uint32_t last_page = PAGE_MASK(entry_new_end - 1);
            if(entry->next != NULL && PAGE_MASK(entry->next->start) == last_page) {
                last_page -= PAGE_SIZE;
            }
            if(last_page >= first_page) {
                userspace_allocate_pages(first_page, (last_page - first_page + PAGE_SIZE) / PAGE_SIZE);
            }

            return allocation;
        }

        // check if we would free this entry, whether new free space would be enough
        available_space = (entry->next == NULL) ? (bleskalloc_metadata->memory_end - entry->prev->end) : (entry->next->start - entry->prev->end);
        if(new_size <= available_space) {
            uint32_t entry_new_start = entry->prev->end;
            uint32_t entry_new_end = entry_new_start + new_size;

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
                userspace_allocate_pages(first_page, (last_page - first_page + PAGE_SIZE) / PAGE_SIZE);
            }

            // map new pages above current entry
            if(entry_new_end > entry_end) {
                first_page = PAGE_MASK(entry_end);
                if(PAGE_OFFSET_MASK(entry_end) != 0) {
                    first_page += PAGE_SIZE;
                }
                last_page = PAGE_MASK(entry_new_end - 1);
                if(entry->next != NULL && PAGE_MASK(entry->next->start) == last_page) {
                    last_page -= PAGE_SIZE;
                }
                if(last_page >= first_page) { // map all pages that are above current entry and should be mapped
                    userspace_allocate_pages(first_page, (last_page - first_page + PAGE_SIZE) / PAGE_SIZE);
                }
            }

            // copy data
            bleskalloc_entry_t *next_entry = entry->next;
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
                if(PAGE_MASK(entry_new_end) == last_page || (next_entry != NULL && PAGE_MASK(next_entry->start) == last_page)) {
                    last_page -= PAGE_SIZE;
                }
                if(last_page >= first_page) {
                    userspace_deallocate_pages(first_page, (last_page - first_page + PAGE_SIZE) / PAGE_SIZE);
                }
            }

            // update entry
            entry = (bleskalloc_entry_t *) entry_new_start;
            entry->start = entry_new_start;
            entry->end = entry_new_end;

            // update linked list
            entry->prev->next = entry;
            if(next_entry != NULL) {
                next_entry->prev = entry;
            }

            // check new free space left after this entry and update allocator if it is bigger
            uint32_t free_space_size = (available_space - new_size);
            if(free_space_size > bleskalloc_metadata->free_memory_size) {
                bleskalloc_metadata->free_memory_start = entry_new_end;
                bleskalloc_metadata->free_memory_size = free_space_size;
                bleskalloc_metadata->insert_entry = entry;
            }

            return (void *) (entry_new_start + sizeof(bleskalloc_entry_t));
        }

        // else try to allocate new block
        void *new_allocation = syslib_malloc(new_size - sizeof(bleskalloc_entry_t));
        if(new_allocation == NULL) {
            syslib->log("\nbleskalloc: realloc failed to allocate new block");
            return NULL;
        }

        // copy data
        memcpy(new_allocation, allocation, (entry_size - sizeof(bleskalloc_entry_t)));

        // free old block
        syslib_free(allocation);

        return new_allocation;
    }
}

void syslib_free(void *allocation) {
    // load entry data
    bleskalloc_entry_t *entry = (bleskalloc_entry_t *)((uint32_t)allocation - sizeof(bleskalloc_entry_t));
    bleskalloc_entry_t *prev = entry->prev;
    bleskalloc_entry_t *next = entry->next;

    uint32_t e_start = entry->start;
    uint32_t e_end = entry->end;
    uint32_t p_end = prev->end;

    // unmap pages and unlink from linked list
    uint32_t first_page = PAGE_MASK(e_start);
    first_page += (PAGE_MASK(p_end - 1) == first_page) ? PAGE_SIZE : 0; // if previous entry end is on the same page we can not deallocate
    uint32_t last_page = PAGE_MASK(e_end - 1);
    if(next != NULL) {
        uint32_t n_start = next->start; // update linked list
        last_page -= (PAGE_MASK(n_start) == last_page) ? PAGE_SIZE : 0; // if next entry start is on the same page we can not deallocate
        next->prev = prev; // update linked list
    }
    prev->next = next; // update linked list
    if(last_page >= first_page) {
        uint32_t num_pages = (last_page - first_page) / PAGE_SIZE + 1;
        userspace_deallocate_pages(first_page, num_pages);
    }

    // update metadata
    bleskalloc_metadata_t *meta = bleskalloc_metadata;
    uint32_t limit = (next == NULL) ? meta->memory_end : next->start;
    uint32_t free_space = limit - p_end;
    if(free_space > meta->free_memory_size) {
        meta->free_memory_start = p_end;
        meta->free_memory_size  = free_space;
        meta->insert_entry  = prev;
    }
}