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
#include <kernel/x86/kernel.h>
#include <hardware/groups/logging/logging.h>
#include <kernel/x86/cpu/commands.h>
#include <kernel/x86/memory/vm_allocator.h>
#include <kernel/x86/libc/stdlib.h>
#include <kernel/x86/scheduler/lock.h>

/* global variables */
dword_t number_of_allocated_pages;

/* functions */
void log_physical_memory_map(void) {
    standardized_phy_memory_map_t *phy_mem_map = (standardized_phy_memory_map_t *) P_MEM_STANDARDIZED_PHY_MEM_MAP;
    standardized_32_bit_free_memory_map_t *free_phy_mem_map = (standardized_32_bit_free_memory_map_t *) P_MEM_STANDARDIZED_FREE_PHY_MEM_MAP;

    for(int i = 0; i < phy_mem_map->number_of_entries; i++) {
        lock_core();
        log("\n[PHYMEM] Type: ");
        if(phy_mem_map->entry[i].type == 1) {
            log("FREE");
        }
        else if(phy_mem_map->entry[i].type == 2) {
            log("USED");
        }
        log(" Start: 0x%x%x Size: 0x%x%x", (dword_t)(phy_mem_map->entry[i].memory_start >> 32), (dword_t)phy_mem_map->entry[i].memory_start, (dword_t)(phy_mem_map->entry[i].memory_size >> 32), (dword_t)phy_mem_map->entry[i].memory_size);
        unlock_core();
    }
}

void initialize_physical_memory(void) {
    standardized_32_bit_free_memory_map_t *free_phy_mem_map = (standardized_32_bit_free_memory_map_t *) P_MEM_STANDARDIZED_FREE_PHY_MEM_MAP;

    // count size of free memory
    dword_t free_memory = 0;
    for(int i = 0; i < free_phy_mem_map->number_of_entries; i++) {
        free_memory += (free_phy_mem_map->entry[i].memory_size & 0xFFFFF000);
    }
    dword_t number_of_free_pages = (free_memory >> 12);
    dword_t size_of_stack = (sizeof(pm_pages_stack_t) + (number_of_free_pages * 4));
    dword_t number_of_stack_pages = ((size_of_stack >> 12) + 1); // add one page for other structures
    if((size_of_stack & 0xFFF) != 0x000) {
        number_of_stack_pages++;
    }

    // allocate all pages of stack
    dword_t *page_table = (dword_t *) (P_MEM_PAGE_TABLE + (P_MEM_PM_STACK >> 10));
    for(int i = 0; i < number_of_stack_pages; i++, page_table++) {
        *page_table = ((dword_t)pm_bump_alloc(PAGE_SIZE) | VM_FLAG_PRESENT | VM_FLAG_READ_WRITE | VM_FLAG_SUPERVISOR);
    }
    write_cr3(PM_KERNEL_PAGE_DIRECTORY); // flush TLB
 
    // fill stack with pages
    pm_pages_stack_t *stack = (pm_pages_stack_t *) P_MEM_PM_STACK;
    stack->pointer_to_free_page = (number_of_free_pages - 1);
    stack->last_valid_index = (stack->pointer_to_free_page + 16); // create space for bootloader-used pages under 0x10000 that are never in free memory map, but are free and therefore can be theoretically freed and used again
    dword_t pointer = stack->pointer_to_free_page;
    for(int i = 0; i < free_phy_mem_map->number_of_entries; i++) {
        if(free_phy_mem_map->entry[i].memory_size != 0) {
            for(dword_t page = free_phy_mem_map->entry[i].memory_start; page < (free_phy_mem_map->entry[i].memory_start + free_phy_mem_map->entry[i].memory_size); page += PAGE_SIZE) {
                stack->pages[pointer] = page;
                pointer--;
            }
        }
    }
    stack->pointer_alloc_stack_page = (stack->pointer_to_free_page | 0x3FF); // when pointer equals to this value, new stack page above will be allocated
    stack->pte_stack_page_for_allocation = (dword_t *) (P_MEM_PAGE_TABLE + (P_MEM_PM_STACK >> 10) + 4 + (((stack->pointer_alloc_stack_page * 4) >> 12) * 4) + 4);
    stack->pointer_dealloc_stack_page = (stack->pointer_alloc_stack_page - 0x400); // when pointer equals to this value, stack page above will be deallocated
    stack->pte_stack_page_for_deallocation = (dword_t *) ((dword_t)stack->pte_stack_page_for_allocation - 4);

    // count number of already allocated pages
    number_of_allocated_pages = 0;
    dword_t *page_directory = (dword_t *) P_MEM_PAGE_DIRECTORY;
    for(int i = 0; i < 1024; i++) {
        if((page_directory[i] & VM_FLAG_PRESENT) == VM_FLAG_PRESENT) {
            page_table = (dword_t *) (P_MEM_PAGE_TABLE + (i * PAGE_SIZE));
            for(int j = 0; j < 1024; j++) {
                if((page_table[j] & VM_FLAG_PRESENT) == VM_FLAG_PRESENT) {
                    number_of_allocated_pages++;
                }
            }
        }
    }
}

void *pm_bump_alloc(dword_t size) {
    standardized_32_bit_free_memory_map_t *free_phy_mem_map = (standardized_32_bit_free_memory_map_t *) P_MEM_STANDARDIZED_FREE_PHY_MEM_MAP;

    // align size to page size
    if((size & 0xFFF) != 0x000) {
        size &= 0xFFFFF000;
        size += 0x1000;
    }

    // scan all memory entries
    for(int i = 0; i < free_phy_mem_map->number_of_entries; i++) {
        if(free_phy_mem_map->entry[i].memory_size >= size) {
            dword_t pm_alloc_start = free_phy_mem_map->entry[i].memory_start;

            free_phy_mem_map->entry[i].memory_start += size;
            free_phy_mem_map->entry[i].memory_size -= size;

            number_of_allocated_pages += (size >> 12);

            return (void *) pm_alloc_start;
        }
    }

    // no memory was founded
    kernel_panic("pm_bump_alloc() can not allocate memory");

    return FALSE;
}

void *pm_alloc_page(void) {
    // cli

    // set pointer to stack
    pm_pages_stack_t *stack = (pm_pages_stack_t *) P_MEM_PM_STACK;

    // check if there is something in stack
    if(stack->pointer_to_free_page == 0xFFFFFFFF) {
        kernel_panic("All physical memory pages are allocated");
        return (void *) INVALID;
    }

    // take page from stack
    void *page;
    if(stack->pointer_to_free_page == stack->pointer_dealloc_stack_page) {
        // stack page can be released, so we will return stack page instead of taking page from stack
        page = (void *) (*stack->pte_stack_page_for_deallocation & 0xFFFFF000);
        
        stack->pte_stack_page_for_deallocation--;
        stack->pointer_dealloc_stack_page -= 0x400;
        stack->pte_stack_page_for_allocation--;
        stack->pointer_alloc_stack_page -= 0x400;
    }
    else {
        page = (void *) stack->pages[stack->pointer_to_free_page];
        stack->pointer_to_free_page--;
        number_of_allocated_pages++;
    }

    // sti

    // log("\n[PM] Allocated 0x%x page", page); // DEBUG

    return page;
}

void *pm_alloc_continuous_memory(dword_t size) {
    pm_pages_stack_t *stack = (pm_pages_stack_t *) P_MEM_PM_STACK;

    // check if there is something in stack
    if(stack->pointer_to_free_page == 0xFFFFFFFF) {
        kernel_panic("All physical memory pages are allocated");
        return (void *) INVALID;
    }

    // convert size to number of pages
    dword_t number_of_needed_pages = (size >> 12);
    if((size & 0xFFF) != 0x000) {
        number_of_needed_pages++;
    }

    // find continuous block of memory for allocation
    for(dword_t i = stack->pointer_to_free_page, first_page_index = INVALID, last_page_mem = INVALID, continuous_pages = 0; i > 0; i--) {
        // this page is right after page before, they are block of continuous memory
        if(stack->pages[i] == (last_page_mem + PAGE_SIZE)) {
            continuous_pages++;
        }
        // this page starts new memory block
        else {
            continuous_pages = 1;
            first_page_index = i;
        }
        last_page_mem = stack->pages[i];

        // block that is big enough was founded
        if(number_of_needed_pages == continuous_pages) {
            // get start of memory block
            void *first_page = (void *) (stack->pages[first_page_index]);

            // move free pages in stack
            dword_t dest = i; // last page that will be allocated
            dword_t src = (i + number_of_needed_pages); // first page above continuous block of pages in stack
            for(int j = 0; j < (stack->pointer_to_free_page - first_page_index); j++, dest++, src++) { // copy all pages above continuous block
                stack->pages[dest] = stack->pages[src];
            }

            stack->pointer_to_free_page = (dest - 1); // dest points to where to place new page, so dest - 1 points to first page in stack

            // deallocate unused stack pages
            while(stack->pointer_dealloc_stack_page > stack->pointer_to_free_page) {
                stack->pages[stack->pointer_to_free_page] = (*stack->pte_stack_page_for_deallocation & 0xFFFFF000);
                stack->pointer_to_free_page++;

                stack->pte_stack_page_for_deallocation--;
                stack->pointer_dealloc_stack_page -= 0x400;
                stack->pte_stack_page_for_allocation--;
                stack->pointer_alloc_stack_page -= 0x400;
            }

            number_of_allocated_pages += number_of_needed_pages;

            return (void *) first_page;
        }
    }

    // no memory was founded
    kernel_panic("pm_alloc_continuous_memory() can not allocate memory");

    return (void *) INVALID;
}

void pm_free_page(void *page) {
    // cli

    pm_pages_stack_t *stack = (pm_pages_stack_t *) P_MEM_PM_STACK;
    if(stack->pointer_to_free_page == stack->last_valid_index) {
        kernel_panic("Freeing physical page to full stack");
        return;
    }

    if(stack->pointer_to_free_page == stack->pointer_alloc_stack_page) {
        // stack page needs to be allocated, so we will allocate stack page instead of pushing page to stack
        *stack->pte_stack_page_for_allocation = ((dword_t)page | VM_FLAG_PRESENT | VM_FLAG_READ_WRITE | VM_FLAG_SUPERVISOR);
        invlpg((dword_t)&stack->pages[(stack->pointer_to_free_page + 1)]);

        stack->pte_stack_page_for_deallocation++;
        stack->pointer_dealloc_stack_page += 0x400;
        stack->pte_stack_page_for_allocation++;
        stack->pointer_alloc_stack_page += 0x400;
    }
    else {
        stack->pointer_to_free_page++;
        stack->pages[stack->pointer_to_free_page] = (dword_t) page;
        number_of_allocated_pages--;
    }

    // log("\n[PM] Freed 0x%x page", page); // DEBUG

    // sti
}