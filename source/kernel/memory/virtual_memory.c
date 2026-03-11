/* 
* BleskOS
*
* MIT License
* Copyright (c) 2023-2026 BleskOS developers
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

// TODO: add use of spinlocks everywhere

/* includes */
#include <libc/string.h>
#include <kernel/cpu/commands.h>
#include <kernel/memory/physical_memory.h>
#include <kernel/memory/memory_allocators.h>
#include <kernel/cpu/info.h>
#include <kernel/cpu/mutex.h>
#include <kernel/cpu/interrupt.h>
#include <kernel/kernel.h>
#include <kernel/hardware/groups/logging/logging.h>

/* global variables */
uint32_t pm_of_page_directory;
mutex_t creating_page_directory_mutex;

/* functions */
void initialize_virtual_memory(void) {
    pm_of_page_directory = PM_KERNEL_PAGE_DIRECTORY;
}

/* functions for reading page entries */
uint32_t get_page_entry_of_vm_page(uint32_t vm_page) {
    uint32_t *page_directory_entry = (uint32_t *) (P_MEM_PAGE_DIRECTORY + ((vm_page >> 22) * 4));
    if((*page_directory_entry & VM_FLAG_PRESENT) == 0) {
        return 0;
    }
    uint32_t *page_table_entry = (uint32_t *) (P_MEM_PAGE_TABLE + ((vm_page >> 12) * 4));
    return *page_table_entry;
}

void *get_pm_of_vm_page(uint32_t vm_page) {
    return (void *) PAGE_MASK(get_page_entry_of_vm_page(vm_page));
}

int is_vm_page_mapped(uint32_t vm_page) {
    if((get_page_entry_of_vm_page(vm_page) & VM_FLAG_PRESENT) == VM_FLAG_PRESENT) {
        return true;
    }
    else {
        return false;
    }
}

/* functions for (un)mapping pages */
void vm_map_page(uint32_t vm_page, uint32_t pm_page, uint32_t flags) {
    vm_allocate_page_tables(vm_page, PAGE_SIZE);
    uint32_t *page_table_entry = (uint32_t *) (P_MEM_PAGE_TABLE + ((vm_page >> 12) * 4));
    *page_table_entry = (pm_page | flags);
}

void vm_map_phy_pages(uint32_t vm_start_page, uint32_t pm_start_page, uint32_t size, uint32_t flags) {
    size = PAGE_MASK(size + PAGE_SIZE - 1); // align size to page size
    vm_allocate_page_tables(vm_start_page, size);
    uint32_t *page_table_entry = (uint32_t *) (P_MEM_PAGE_TABLE + ((vm_start_page >> 12) * 4));
    uint32_t number_of_pages = (size >> 12);
    for(uint32_t i = 0; i < number_of_pages; i++, pm_start_page += PAGE_SIZE) {
        *page_table_entry++ = (pm_start_page | flags);
    }
}

void vm_unmap_page(uint32_t vm_page) {
    uint32_t *page_directory_entry = (uint32_t *) (P_MEM_PAGE_DIRECTORY + ((vm_page >> 22) * 4));
    if((*page_directory_entry & VM_FLAG_PRESENT) == 0) {
        return;
    }
    uint32_t *page_table_entry = (uint32_t *) (P_MEM_PAGE_TABLE + ((vm_page >> 12) * 4));
    *page_table_entry = 0;
    invlpg(vm_page);
}

void vm_unmap_pages(uint32_t vm_start_page, uint32_t size) {
    size = PAGE_MASK(size + PAGE_SIZE - 1); // align size to page size
    uint32_t number_of_pages = (size >> 12);
    uint32_t *page_table_entry = (uint32_t *) (P_MEM_PAGE_TABLE + (vm_start_page >> 10));
    for(uint32_t i = 0; i < number_of_pages; i++) {
        *page_table_entry++ = 0;
    }
    if(number_of_pages < NUMBER_OF_PAGES_TO_FLUSH_BY_INVLPG) {
        for(uint32_t i = 0; i < number_of_pages; i++) {
            invlpg((vm_start_page + (i * PAGE_SIZE)));
        }
    }
    else {
        write_cr3(read_cr3());
    }
}

/* functions for (de)allocating pages */
void vm_allocate_page_tables(uint32_t vm_start, uint32_t size) {
    uint32_t first_page_table = (vm_start >> 22);
    uint32_t last_page_table = ((vm_start + size - 1) >> 22);
    uint32_t *page_directory_entry = (uint32_t *) (P_MEM_PAGE_DIRECTORY + (first_page_table * 4));
    for(uint32_t i = first_page_table; i <= last_page_table; i++, page_directory_entry++) {
        if((*page_directory_entry & VM_FLAG_PRESENT) == 0) {
            *page_directory_entry = ((uint32_t)pm_alloc_page() | VM_PAGE_TABLE); // add page table to page directory
        }
    }
}

void vm_allocate_page(uint32_t vm_page, uint32_t flags) {
    vm_map_page(vm_page, (uint32_t) pm_alloc_page(), flags);
    invlpg(vm_page);
}

void vm_allocate_pages(uint32_t vm_start, uint32_t size, uint32_t flags) {
    if(size == 0) {
        return;
    }
    vm_allocate_page_tables(vm_start, size);
    uint32_t end = (vm_start + size - 1);
    uint32_t *page_table_entry = (uint32_t *) (P_MEM_PAGE_TABLE + ((vm_start >> 12) * 4));
    for(uint32_t mem = vm_start; mem <= end; mem += PAGE_SIZE, page_table_entry++) {
        if((*page_table_entry & VM_FLAG_PRESENT) == 0) {
            *page_table_entry = ((uint32_t)pm_alloc_page() | flags);
        }
    }
}

void vm_deallocate_pages(uint32_t vm_start_page, uint32_t size) {
    // align size to page size
    if(PAGE_OFFSET_MASK(size) != 0) {
        size = PAGE_MASK(size) + PAGE_SIZE;
    }

    // clear pages
    memset((void *) vm_start_page, 0, size);

    // deallocate pages
    uint32_t number_of_pages = (size >> 12);
    uint32_t *page_table_entry = (uint32_t *) (P_MEM_PAGE_TABLE + (vm_start_page >> 10));
    for(uint32_t i = 0; i < number_of_pages; i++, page_table_entry++) {
        pm_free_page((void *) PAGE_MASK(*page_table_entry));
        *page_table_entry = 0;
    }

    // flush TLB
    if(number_of_pages < NUMBER_OF_PAGES_TO_FLUSH_BY_INVLPG) {
        for(uint32_t i = 0; i < number_of_pages; i++) {
            invlpg(vm_start_page + (i * PAGE_SIZE));
        }
    }
    else {
        write_cr3(pm_of_page_directory);
    }
}

void vm_move_pages(int flush_tlb, uint32_t vm_old_start, uint32_t vm_new_start, uint32_t size) {
    if(vm_old_start == vm_new_start) {
        return;
    }

    // align size to page size
    size = PAGE_MASK(size + PAGE_SIZE - 1);

    // ensure that page tables are mapped
    vm_allocate_page_tables(vm_new_start, size);

    // move pages
    if(vm_old_start < vm_new_start) {
        uint32_t *old_page_table_entry = (uint32_t *) (P_MEM_PAGE_TABLE + ((vm_old_start + size - PAGE_SIZE) >> 10));
        uint32_t *new_page_table_entry = (uint32_t *) (P_MEM_PAGE_TABLE + ((vm_new_start + size - PAGE_SIZE) >> 10));
        for(uint32_t i = size; i > 0; i -= PAGE_SIZE) {
            *new_page_table_entry-- = *old_page_table_entry;
            *old_page_table_entry-- = 0;
        }
    }
    else {
        uint32_t *old_page_table_entry = (uint32_t *) (P_MEM_PAGE_TABLE + (vm_old_start >> 10));
        uint32_t *new_page_table_entry = (uint32_t *) (P_MEM_PAGE_TABLE + (vm_new_start >> 10));
        for(uint32_t i = 0; i < size; i += PAGE_SIZE) {
            *new_page_table_entry++ = *old_page_table_entry;
            *old_page_table_entry++ = 0;
        }
    }

    // flush TLB
    if(flush_tlb == true) {
        write_cr3(pm_of_page_directory);
    }
}

/* functions for flushing TLB */
void vm_refresh_mappings(void) {
    write_cr3(pm_of_page_directory);
}

void load_page_directory(uint32_t pm_page_directory) {
    lock_core();
    pm_of_page_directory = pm_page_directory;
    write_cr3(pm_of_page_directory);
    unlock_core();
}

/* functions for managing userspace */
uint32_t vm_create_new_userspace(void) {
    LOCK_MUTEX(&creating_page_directory_mutex);

    // create new page directory
    vm_allocate_page(P_MEM_NEW_PAGE_DIRECTORY, VM_KERNEL);
    memset((void *) P_MEM_NEW_PAGE_DIRECTORY, 0, PAGE_SIZE);

    // map kernel space to new page directory
    uint32_t *page_directory_entry = (uint32_t *) P_MEM_PAGE_DIRECTORY;
    uint32_t *new_page_directory_entry = (uint32_t *) P_MEM_NEW_PAGE_DIRECTORY;
    for(int i = 1022; i >= (1024 - number_of_shared_page_tables); i--) {
        if((page_directory_entry[i] & VM_FLAG_PRESENT) == VM_FLAG_PRESENT) {
            new_page_directory_entry[i] = page_directory_entry[i];
        }
    }
    new_page_directory_entry[1023] = ((uint32_t)get_pm_of_vm_page(P_MEM_NEW_PAGE_DIRECTORY) | VM_KERNEL); // map page directory to itself for easy access to page tables

    // lazy allocation for whole user space
    for(int i = 1; i < (1024 - number_of_shared_page_tables); i++) {
        new_page_directory_entry[i] = 0;
    }

    // create first page table for user space
    vm_allocate_page(P_MEM_NEW_PAGE_TABLE, VM_KERNEL);
    uint32_t *new_page_table_entry = (uint32_t *) P_MEM_NEW_PAGE_TABLE;
    new_page_table_entry[0] = VM_FLAG_NOT_PRESENT; // page for NULL pointer must not be present
    for(int i = 1; i < 1024; i++) {
        new_page_table_entry[i] = 0;
    }
    new_page_directory_entry[0] = ((uint32_t)get_pm_of_vm_page(P_MEM_NEW_PAGE_TABLE) | VM_PAGE_TABLE);

    // load new page directory
    uint32_t new_pm_page_directory = (uint32_t)get_pm_of_vm_page(P_MEM_NEW_PAGE_DIRECTORY);
    load_page_directory(new_pm_page_directory);

    UNLOCK_MUTEX(&creating_page_directory_mutex);
    return new_pm_page_directory;
}

void map_physical_pages_to_userspace(uint32_t address, uint32_t size) {
    LOCK_MUTEX(&get_current_logical_processor_struct()->current_process->virtual_memory_mutex);

    uint32_t start_vaddr = address & ~(PAGE_SIZE - 1);
    uint32_t end_vaddr = (address + size + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1);

    for(uint32_t vaddr = start_vaddr; vaddr < end_vaddr; vaddr += PAGE_SIZE) {
        uint32_t pd_index = vaddr >> 22;
        uint32_t pt_index = (vaddr >> 12) & 0x3FF;

        // map page table
        uint32_t *pd_entry = (uint32_t *)(P_MEM_PAGE_DIRECTORY + (pd_index * 4));
        if ((*pd_entry & VM_FLAG_PRESENT) == 0) {
            *pd_entry = ((uint32_t)pm_alloc_page() | VM_PAGE_TABLE);
            uint32_t *new_pt = (uint32_t *)(P_MEM_PAGE_TABLE + (pd_index * PAGE_SIZE));
            for (int i = 0; i < 1024; i++) new_pt[i] = 0;
        }

        // map page
        uint32_t *pt_entries = (uint32_t *)(P_MEM_PAGE_TABLE + (pd_index * PAGE_SIZE));
        if ((pt_entries[pt_index] & VM_FLAG_PRESENT) == 0) {
            pt_entries[pt_index] = ((uint32_t)pm_alloc_page() | VM_USER);
        }
    }

    UNLOCK_MUTEX(&get_current_logical_processor_struct()->current_process->virtual_memory_mutex);
}

void unmap_physical_pages_from_userspace(uint32_t address, uint32_t size) {
    LOCK_MUTEX(&get_current_logical_processor_struct()->current_process->virtual_memory_mutex);

    // unmap pages
    uint32_t start_vaddr = address & ~(PAGE_SIZE - 1);
    uint32_t end_vaddr = (address + size + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1);

    for (uint32_t vaddr = start_vaddr; vaddr < end_vaddr; vaddr += PAGE_SIZE) {
        uint32_t pd_index = vaddr >> 22;
        uint32_t pt_index = (vaddr >> 12) & 0x3FF;

        // if table does not exist, skip it
        uint32_t *pd_entry = (uint32_t *)(P_MEM_PAGE_DIRECTORY + (pd_index * 4));
        if ((*pd_entry & VM_FLAG_PRESENT) == 0) {
            vaddr = (vaddr + 0x400000) & ~0x3FFFFF; // skip right to next page table
            vaddr -= PAGE_SIZE; // compensate for vaddr += PAGE_SIZE in the loop
            continue;
        }

        uint32_t *pt_entries = (uint32_t *)(P_MEM_PAGE_TABLE + (pd_index * PAGE_SIZE));
        uint32_t pte = pt_entries[pt_index];

        if ((pte & VM_FLAG_PRESENT) != 0) {
            uint32_t type = (pte & VM_FLAGS_TYPE);
            
            // unmap only if it is normal page
            if (type != VM_LAZY_ALLOCATION && type != VM_COW_ALLOCATION && type != VM_SPAWN_TEMPLATE) {
                memset((void *)vaddr, 0, PAGE_SIZE);
                pm_free_page((void *)PAGE_MASK(pte));
            }

            // remove page from page table
            pt_entries[pt_index] = 0;
        }
    }

    // unmap page tables
    uint32_t first_pd = start_vaddr >> 22;
    uint32_t last_pd = (end_vaddr - 1) >> 22;

    for (uint32_t pd_idx = first_pd; pd_idx <= last_pd; pd_idx++) {
        uint32_t *pd_entry = (uint32_t *)(P_MEM_PAGE_DIRECTORY + (pd_idx * 4));
        if((*pd_entry & VM_FLAG_PRESENT) == 0) {
            continue;
        }

        uint32_t *pt_entries = (uint32_t *)(P_MEM_PAGE_TABLE + (pd_idx * PAGE_SIZE));
        int is_empty = true;

        for(int i = 0; i < 1024; i++) {
            if(pt_entries[i] != 0) {
                is_empty = false;
                break;
            }
        }

        if(is_empty == true) {
            pm_free_page((void *)PAGE_MASK(*pd_entry));
            *pd_entry = 0; // remove page table from page directory
        }
    }

    vm_refresh_mappings();
    // TODO: for all other processors

    UNLOCK_MUTEX(&get_current_logical_processor_struct()->current_process->virtual_memory_mutex);
}

void free_virtual_space(uint32_t page_directory) {
    lock_core();
    load_page_directory(page_directory);
    uint32_t *page_directory_entry = (uint32_t *) (P_MEM_PAGE_DIRECTORY);
    for(int i = 0; i < (1024 - number_of_shared_page_tables); i++, page_directory_entry++) {
        if((*page_directory_entry & VM_FLAG_PRESENT) != VM_FLAG_PRESENT) {
            continue;
        }

        uint32_t *page_table_entry = (uint32_t *) (P_MEM_PAGE_TABLE + (i * PAGE_SIZE));
        for(int j = 0; j < 1024; j++, page_table_entry++) {
            if((*page_table_entry & VM_FLAG_PRESENT) != VM_FLAG_PRESENT) {
                continue;
            }
            if((*page_table_entry & VM_FLAG_READ_WRITE) == VM_FLAG_READ_ONLY) {
                continue;
            }

            // log("\nDeallocating page %x at 0x%x", PAGE_MASK(*page_table_entry), i * 0x400000 + j * 0x1000);
            memset((void *) (i * 0x00400000 + j * PAGE_SIZE), 0, PAGE_SIZE);
            pm_free_page((void *) PAGE_MASK(*page_table_entry));
        }

        if(PAGE_MASK(*page_directory_entry) != 0) {
            // log("\nDeallocating page table %x at 0x%x", PAGE_MASK(*page_directory_entry), i * 0x400000);
            memset((void *) (P_MEM_PAGE_TABLE + (i * PAGE_SIZE)), 0, PAGE_SIZE);
            pm_free_page((void *) PAGE_MASK(*page_directory_entry));
            *page_directory_entry = 0;
        }
    }
    load_page_directory(PM_KERNEL_PAGE_DIRECTORY);
    // TODO: clear and free page directory itself
    unlock_core();
}