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
#include <kernel/kernel.h>
#include <kernel/hardware/devices/memory/virtual_memory.h>
#include <kernel/hardware/groups/logging/logging.h>
#include <kernel/hardware/devices/cpu/mutex.h>
#include <kernel/hardware/devices/cpu/info.h>

/* global variables */
void *(*pm_alloc_page)(void);
void (*pm_free_page)(void *page);

/* local variables */
uint32_t *pm_stack_pointer;
uint32_t *pm_stack_last_entry;
uint32_t pm_stack_max_number_of_pages;
mutex_t pm_stack_mutex;
uint32_t number_of_all_pages;
uint32_t number_of_allocated_pages;

/* functions */
void initialize_physical_memory(void) {
    standardized_32_bit_free_memory_map_t *free_phy_mem_map = (standardized_32_bit_free_memory_map_t *) VM_STANDARDIZED_FREE_PHY_MEM_MAP;

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

    // allocate all page tables for kernel space
    uint32_t *page_directory_entry = (uint32_t *) (VM_PAGE_DIRECTORY + (((VM_KERNEL_SPACE_START + PAGE_TABLE_SIZE) >> 22) * sizeof(uint32_t)));
    for(int i = 769; i < 1023; i++, page_directory_entry++) {
        *page_directory_entry = ((uint32_t)pm_bump_alloc(PAGE_SIZE) | VM_KERNEL);
    }
    memset((void *) (VM_PAGE_TABLES + (((VM_KERNEL_SPACE_START + PAGE_TABLE_SIZE) >> 12) * sizeof(uint32_t))), 0, 254 * PAGE_SIZE); // clear content of page tables

    // count size of free memory
    uint32_t free_memory = 0;
    for(int i = 0; i < free_phy_mem_map->number_of_entries; i++) {
        free_memory += PAGE_MASK(free_phy_mem_map->entry[i].memory_size);
    }

    // allocate all pages of physical memory stack
    uint32_t pm_stack_number_of_entries = (free_memory >> 12);
    const uint32_t entries_in_page_table = (PAGE_SIZE / 4);
    uint32_t pm_stack_number_of_vm_pages = ((pm_stack_number_of_entries + entries_in_page_table - 1) / entries_in_page_table); // number of pages needed if all pages are written to stack
    uint32_t *page_table_entry = (uint32_t *) (VM_PAGE_TABLES + ((VM_PHYSICAL_PAGES_STACK >> 12) * sizeof(uint32_t)));
    for(int i = 0; i < pm_stack_number_of_vm_pages; i++, page_table_entry++) {
        *page_table_entry = ((uint32_t)pm_bump_alloc(PAGE_SIZE) | VM_KERNEL | VM_FLAG_GLOBAL);
    }

    // fill stack with clear physical memory pages
    pm_stack_pointer = (uint32_t *) VM_PHYSICAL_PAGES_STACK;
    page_table_entry = (uint32_t *) (VM_PAGE_TABLES + ((VM_FOR_CLEARING_MEMORY >> 12) * sizeof(uint32_t)));
    for(int i = 0; i < free_phy_mem_map->number_of_entries; i++) {
        while(free_phy_mem_map->entry[i].memory_size > 0) {
            uint32_t pm_page = free_phy_mem_map->entry[i].memory_start;
            uint32_t number_of_pages = free_phy_mem_map->entry[i].memory_size / PAGE_SIZE;
            if(number_of_pages > (1024 * 253)) {
                number_of_pages = (1024 * 253);
            }
            for(int j = 0; j < number_of_pages; j++, pm_page += PAGE_SIZE) {
                page_table_entry[j] = (pm_page | VM_KERNEL);
                *(pm_stack_pointer++) = pm_page;
                number_of_all_pages++;
            }
            write_cr3(read_cr3());

            memset((void *) VM_FOR_CLEARING_MEMORY, 0, number_of_pages * PAGE_SIZE);

            free_phy_mem_map->entry[i].memory_start += number_of_pages * PAGE_SIZE;
            free_phy_mem_map->entry[i].memory_size -= number_of_pages * PAGE_SIZE;
        }
    }
    memset((void *) (VM_PAGE_TABLES + (((VM_KERNEL_SPACE_START + PAGE_TABLE_SIZE) >> 12) * sizeof(uint32_t))), 0, 253 * PAGE_SIZE); // clear content of page tables
    write_cr3(read_cr3());

    // set pointer
    pm_stack_last_entry = pm_stack_pointer;

    // set functions for allocation
    pm_alloc_page = pm_direct_alloc_page;
    pm_free_page = pm_direct_free_page;
}

void *pm_bump_alloc(uint32_t size) {
    standardized_32_bit_free_memory_map_t *free_phy_mem_map = (standardized_32_bit_free_memory_map_t *) VM_STANDARDIZED_FREE_PHY_MEM_MAP;

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
    kernel_panic("pm_bump_alloc() can not allocate memory", NULL);

    return (void *) NULL;
}

// this method guarantees that allocated page will be clear
void *pm_direct_alloc_page(void) {
    LOCK_MUTEX(&pm_stack_mutex);

    // check if stack is empty
    if(pm_stack_pointer == (uint32_t *) VM_PHYSICAL_PAGES_STACK) {
        kernel_panic("pm_alloc_page() out of physical memory pages", NULL);
    }

    // pop page from stack
    void *page = (void *) *(--pm_stack_pointer);
    number_of_allocated_pages++;

    UNLOCK_MUTEX(&pm_stack_mutex);
    // log("\n[PM] Allocated 0x%x page", page); // DEBUG
    return page;
}

void pm_direct_free_page(void *page) {
    LOCK_MUTEX(&pm_stack_mutex);

    // check if stack is full
    if(pm_stack_pointer == pm_stack_last_entry) {
        kernel_panic("pm_free_page() physical memory stack overflow", NULL);
    }

    // push page to stack
    *(pm_stack_pointer++) = (uint32_t) page;
    number_of_allocated_pages--;

    UNLOCK_MUTEX(&pm_stack_mutex);
    // log("\n[PM] Freed 0x%x page", page); // DEBUG
}

// this method guarantees that allocated page will be clear
void *pm_local_alloc_page(void) {
    logical_processor_t *lpdata = get_current_logical_processor_struct();
    uint32_t *stack = &lpdata->physical_pages_stack[0];
    uint32_t index = stack[0];

    // check if stack is empty
    if(index == 0) {
        // load new pages from global stack
        LOCK_MUTEX(&pm_stack_mutex);
        if(pm_stack_pointer == (uint32_t *) VM_PHYSICAL_PAGES_STACK) {
            // TODO: try to get pages from other cores, try to free something from kernel cache, if user program then kill it
            kernel_panic("pm_alloc_page() out of physical memory pages", NULL);
        }
        for(int i = 0; i < 64; i++) {
            if(pm_stack_pointer == (uint32_t *) VM_PHYSICAL_PAGES_STACK) {
                break;
            }
            stack[++index] = *(--pm_stack_pointer);
        }
        UNLOCK_MUTEX(&pm_stack_mutex);
        stack[0] = index;
    }

    // pop page from stack
    void *page = (void *) (stack[index]);
    stack[0]--;
    number_of_allocated_pages++;

    return page;
}

void pm_local_free_page(void *page) {
    logical_processor_t *lpdata = get_current_logical_processor_struct();
    uint32_t *stack = &lpdata->physical_pages_stack[0];
    uint32_t index = stack[0];

    // check if stack is full
    if(index == 255) {
        // move pages to global stack
        LOCK_MUTEX(&pm_stack_mutex);
        for(int i = 0; i < 128; i++) {
            if(pm_stack_pointer == pm_stack_last_entry) {
                kernel_panic("pm_free_page() physical memory stack overflow", NULL);
            }
            *(pm_stack_pointer++) = stack[index--];
        }
        UNLOCK_MUTEX(&pm_stack_mutex);
        stack[0] = index;
    }

    // push page to stack
    stack[++index] = (uint32_t) page;
    stack[0]++;
    number_of_allocated_pages--;
}

void *pm_alloc_continuous_memory(uint32_t size) {
    // align size to page size
    if(PAGE_OFFSET_MASK(size) != 0) {
        size = PAGE_MASK(size) + PAGE_SIZE;
    }
    uint32_t number_of_pages = (size >> 12);

    // check if enough pages are available
    if((pm_stack_pointer - (number_of_pages * 4)) < (uint32_t *) VM_PHYSICAL_PAGES_STACK) {
        return NULL;
    }

    // search for continuous pages
    uint32_t *start_pointer = pm_stack_pointer;
    start_pointer--;
    while(start_pointer > (uint32_t *) VM_PHYSICAL_PAGES_STACK) {
        // count number of continuous pages
        log("\n[PM] Checking for continuous pages starting at 0x%x", (uint32_t)(*start_pointer)); // DEBUG
        void *page = (void *) (*start_pointer);
        uint32_t block_in_stack_pointer = ((uint32_t)start_pointer + 4); // pointer to first page after block in stack
        uint32_t number_of_continuous_pages = 1;
        while(start_pointer > (uint32_t *) VM_PHYSICAL_PAGES_STACK && (start_pointer[0] + PAGE_SIZE) == start_pointer[-1]) {
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
        number_of_allocated_pages += number_of_pages;

        return page;
    }

    return NULL;
}

void log_physical_memory_map(void) {
    standardized_phy_memory_map_t *phy_mem_map = (standardized_phy_memory_map_t *) VM_STANDARDIZED_PHY_MEM_MAP;
    standardized_32_bit_free_memory_map_t *free_phy_mem_map = (standardized_32_bit_free_memory_map_t *) VM_STANDARDIZED_FREE_PHY_MEM_MAP;

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

void log_free_pm_pages(void) {
    log("\n[PM] Number of allocated pages: %d Number of free pages: %d", number_of_allocated_pages, number_of_all_pages - number_of_allocated_pages);
}