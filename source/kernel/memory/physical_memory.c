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
#include <kernel/kernel.h>
#include <kernel/memory/virtual_memory.h>
#include <kernel/hardware/groups/logging/logging.h>
#include <kernel/cpu/mutex.h>

/* local variables */
uint32_t *pm_stack_pointer;
uint32_t *pm_stack_last_entry;
uint32_t pm_stack_max_number_of_pages;
mutex_t pm_stack_mutex;

/* functions */
void initialize_physical_memory(void) {
    standardized_32_bit_free_memory_map_t *free_phy_mem_map = (standardized_32_bit_free_memory_map_t *) P_MEM_STANDARDIZED_FREE_PHY_MEM_MAP;

    // align all entries to page boundaries
    for(int i = 0; i < free_phy_mem_map->number_of_entries; i++) {
        if(PAGE_OFFSET_MASK(free_phy_mem_map->entry[i].memory_start) != 0) {
            if((PAGE_SIZE - PAGE_OFFSET_MASK(free_phy_mem_map->entry[i].memory_start)) <= free_phy_mem_map->entry[i].memory_size) {
                free_phy_mem_map->entry[i].memory_size = 0;
                continue;
            }
            free_phy_mem_map->entry[i].memory_start = (PAGE_MASK(free_phy_mem_map->entry[i].memory_start) + PAGE_SIZE);
            free_phy_mem_map->entry[i].memory_size -= (PAGE_SIZE - PAGE_OFFSET_MASK(free_phy_mem_map->entry[i].memory_start));
        }
        if(PAGE_OFFSET_MASK(free_phy_mem_map->entry[i].memory_size) != 0) {
            free_phy_mem_map->entry[i].memory_size = (PAGE_MASK(free_phy_mem_map->entry[i].memory_size));
        }
    }

    // count size of free memory
    uint32_t free_memory = 0;
    for(int i = 0; i < free_phy_mem_map->number_of_entries; i++) {
        free_memory += PAGE_MASK(free_phy_mem_map->entry[i].memory_size);
    }

    // allocate all pages of physical memory stack
    pm_stack_max_number_of_pages = (free_memory >> 12);
    uint32_t pm_stack_number_of_vm_pages = ((pm_stack_max_number_of_pages + ((PAGE_SIZE - 1) / 4)) / (PAGE_SIZE / 4));
    // TODO: remove number of pages used for allocating this stack
    uint32_t *page_table_entry = (uint32_t *) (P_MEM_PAGE_TABLE + (MEM_PM_STACK >> 10));
    for(int i = 0; i < pm_stack_number_of_vm_pages; i++, page_table_entry++) {
        *page_table_entry = ((uint32_t)pm_bump_alloc(PAGE_SIZE) | VM_KERNEL);
    }

    // fill stack with clear physical memory pages
    pm_stack_pointer = (uint32_t *) MEM_PM_STACK;
    page_table_entry = (uint32_t *) (P_MEM_PAGE_TABLE + (P_MEM_FOR_CLEARING_MEMORY >> 10));
    for(int i = 0; i < free_phy_mem_map->number_of_entries; i++) {
        while(free_phy_mem_map->entry[i].memory_size > 0) {
            uint32_t pm_page = free_phy_mem_map->entry[i].memory_start;
            uint32_t number_of_pages = free_phy_mem_map->entry[i].memory_size / PAGE_SIZE;
            if(number_of_pages > 1024) {
                number_of_pages = 1024;
            }
            for(int i = 0; i < number_of_pages; i++, pm_page += PAGE_SIZE) {
                page_table_entry[i] = (pm_page | VM_KERNEL);
                *(pm_stack_pointer++) = pm_page;
            }
            write_cr3(PM_KERNEL_PAGE_DIRECTORY);

            memset((void *) P_MEM_FOR_CLEARING_MEMORY, 0, number_of_pages * PAGE_SIZE);

            free_phy_mem_map->entry[i].memory_start += number_of_pages * PAGE_SIZE;
            free_phy_mem_map->entry[i].memory_size -= number_of_pages * PAGE_SIZE;
        }
    }

    // set pointer
    pm_stack_last_entry = pm_stack_pointer;
}

void *pm_bump_alloc(uint32_t size) {
    standardized_32_bit_free_memory_map_t *free_phy_mem_map = (standardized_32_bit_free_memory_map_t *) P_MEM_STANDARDIZED_FREE_PHY_MEM_MAP;

    // align size to page size
    size = (size + PAGE_SIZE - 1) / PAGE_SIZE * PAGE_SIZE;

    // scan all memory entries
    for(int i = 0; i < free_phy_mem_map->number_of_entries; i++) {
        if(free_phy_mem_map->entry[i].memory_size >= size) {
            uint32_t pm_alloc_start = free_phy_mem_map->entry[i].memory_start;

            free_phy_mem_map->entry[i].memory_start += size;
            free_phy_mem_map->entry[i].memory_size -= size;

            return (void *) pm_alloc_start;
        }
    }

    // no memory was founded
    kernel_panic("pm_bump_alloc() can not allocate memory");

    return (void *) NULL;
}

// this method guarantees that allocated page will be clear
void *pm_alloc_page(void) {
    LOCK_MUTEX(&pm_stack_mutex);

    // check if stack is empty
    if(pm_stack_pointer == (uint32_t *) MEM_PM_STACK) {
        kernel_panic("pm_alloc_page() out of physical memory pages");
    }

    // pop page from stack
    void *page = (void *) *(--pm_stack_pointer);

    UNLOCK_MUTEX(&pm_stack_mutex);
    // log("\n[PM] Allocated 0x%x page", page); // DEBUG
    return page;
}

void pm_free_page(void *page) {
    LOCK_MUTEX(&pm_stack_mutex);

    // check if stack is full
    if(pm_stack_pointer == pm_stack_last_entry) {
        kernel_panic("pm_free_page() physical memory stack overflow");
    }

    // push page to stack
    *(pm_stack_pointer++) = (uint32_t) page;

    UNLOCK_MUTEX(&pm_stack_mutex);
    // log("\n[PM] Freed 0x%x page", page); // DEBUG
}

uint32_t *pm_alloc_pages(uint32_t number_of_pages) {
    LOCK_MUTEX(&pm_stack_mutex);

    // check if enough pages are available
    if((pm_stack_pointer - (number_of_pages * 4)) < (uint32_t *) MEM_PM_STACK) {
        kernel_panic("pm_alloc_pages() not enough physical memory pages available");
    }

    // return pointer to first page
    pm_stack_pointer -= number_of_pages;
    return pm_stack_pointer;
}

void pm_finish_allocating_pages(void) {
    UNLOCK_MUTEX(&pm_stack_mutex);
}

void *pm_alloc_continuous_memory(uint32_t size) {
    // align size to page size
    if(PAGE_OFFSET_MASK(size) != 0) {
        size = PAGE_MASK(size) + PAGE_SIZE;
    }
    uint32_t number_of_pages = (size >> 12);

    // check if enough pages are available
    if((pm_stack_pointer - (number_of_pages * 4)) < (uint32_t *) MEM_PM_STACK) {
        return NULL;
    }

    // search for continuous pages
    uint32_t *start_pointer = pm_stack_pointer;
    start_pointer--;
    while(start_pointer > (uint32_t *) MEM_PM_STACK) {
        // count number of continuous pages
        log("\n[PM] Checking for continuous pages starting at 0x%x", (uint32_t)(*start_pointer)); // DEBUG
        void *page = (void *) (*start_pointer);
        uint32_t block_in_stack_pointer = ((uint32_t)start_pointer + 4); // pointer to first page after block in stack
        uint32_t number_of_continuous_pages = 1;
        while(start_pointer > (uint32_t *) MEM_PM_STACK && (start_pointer[0] + PAGE_SIZE) == start_pointer[-1]) {
            number_of_continuous_pages++;
            start_pointer--;
        }
        log("\n[PM] Found %d continuous pages", number_of_continuous_pages); // DEBUG

        // check if it is enough
        if(number_of_continuous_pages < number_of_pages) {
            start_pointer--;
            continue;
        }

        // move content of stack
        memcpy((void *) (block_in_stack_pointer - (number_of_pages * 4)), (void *) block_in_stack_pointer, ((uint32_t)pm_stack_pointer - block_in_stack_pointer));

        // move stack pointer
        pm_stack_pointer -= number_of_pages;

        return page;
    }

    return NULL;
}

void log_physical_memory_map(void) {
    standardized_phy_memory_map_t *phy_mem_map = (standardized_phy_memory_map_t *) P_MEM_STANDARDIZED_PHY_MEM_MAP;
    standardized_32_bit_free_memory_map_t *free_phy_mem_map = (standardized_32_bit_free_memory_map_t *) P_MEM_STANDARDIZED_FREE_PHY_MEM_MAP;

    for(int i = 0; i < phy_mem_map->number_of_entries; i++) {
        log("\n[PHYMEM] Type: ");
        if(phy_mem_map->entry[i].type == 1) {
            log("FREE");
        }
        else if(phy_mem_map->entry[i].type == 2) {
            log("USED");
        }
        log(" Start: 0x%x%x Size: 0x%x%x", (uint32_t)(phy_mem_map->entry[i].memory_start >> 32), (uint32_t)phy_mem_map->entry[i].memory_start, (uint32_t)(phy_mem_map->entry[i].memory_size >> 32), (uint32_t)phy_mem_map->entry[i].memory_size);
    }
}