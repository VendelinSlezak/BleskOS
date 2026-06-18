/* 
* BleskOS
*
* MIT License
* Copyright (c) 2023-2026 BleskOS developers
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

// TODO: add synchronization of virtual spaces across multiple cores

/* includes */
#include <kernel/libc/string.h>
#include <kernel/hardware/devices/cpu/commands.h>
#include <kernel/hardware/devices/memory/physical_memory.h>
#include <kernel/hardware/devices/memory/memory_allocators.h>
#include <kernel/hardware/devices/cpu/info.h>
#include <kernel/hardware/devices/cpu/mutex.h>
#include <kernel/hardware/devices/cpu/interrupt.h>
#include <kernel/kernel.h>
#include <kernel/hardware/groups/logging/logging.h>

/* global variables */
mutex_t creating_page_directory_mutex;

/* functions */
void initialize_virtual_memory(void) {
    //
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

/* functions for spinlocking page entry */
void lock_page_entry(uint32_t *page_entry) {
    while (__atomic_fetch_or(page_entry, VM_FLAG_SPINLOCK, __ATOMIC_ACQUIRE) & VM_FLAG_SPINLOCK) {
        asm volatile ("pause");
    }
}

void unlock_page_entry(uint32_t *page_entry) {
    __atomic_fetch_and(page_entry, ~VM_FLAG_SPINLOCK, __ATOMIC_RELEASE);
}

/* functions for (un)mapping pages */
void vm_map_page(uint32_t vm_page, uint32_t pm_page, uint32_t flags) {
    vm_allocate_page_tables(vm_page, PAGE_SIZE);
    uint32_t *page_table_entry = (uint32_t *) (P_MEM_PAGE_TABLE + ((vm_page >> 12) * 4));
    lock_page_entry(page_table_entry);
    *page_table_entry = (pm_page | flags); // this will also clear the spinlock
}

void vm_map_phy_pages(uint32_t vm_start_page, uint32_t pm_start_page, uint32_t size, uint32_t flags) {
    size = PAGE_MASK(size + PAGE_SIZE - 1); // align size to page size
    vm_allocate_page_tables(vm_start_page, size);
    uint32_t *page_table_entry = (uint32_t *) (P_MEM_PAGE_TABLE + ((vm_start_page >> 12) * 4));
    uint32_t number_of_pages = (size >> 12);
    for(uint32_t i = 0; i < number_of_pages; i++, pm_start_page += PAGE_SIZE) {
        lock_page_entry(page_table_entry);
        if((*page_table_entry & VM_FLAG_PRESENT) == VM_FLAG_PRESENT) {
            unlock_page_entry(page_table_entry);
            continue;
        }
        *page_table_entry++ = (pm_start_page | flags); // this will also clear the spinlock
    }
}

void vm_unmap_page(uint32_t vm_page) {
    uint32_t *page_directory_entry = (uint32_t *) (P_MEM_PAGE_DIRECTORY + ((vm_page >> 22) * 4));
    lock_page_entry(page_directory_entry);
    if((*page_directory_entry & VM_FLAG_PRESENT) == 0) {
        unlock_page_entry(page_directory_entry);
        return;
    }
    uint32_t *page_table_entry = (uint32_t *) (P_MEM_PAGE_TABLE + ((vm_page >> 12) * 4));
    lock_page_entry(page_table_entry);
    *page_table_entry = 0; // this will also clear the spinlock
    invlpg(vm_page);
}

void vm_unmap_pages(uint32_t vm_start_page, uint32_t size) {
    size = PAGE_MASK(size + PAGE_SIZE - 1); // align size to page size
    uint32_t number_of_pages = (size >> 12);
    uint32_t *page_table_entry = (uint32_t *) (P_MEM_PAGE_TABLE + (vm_start_page >> 10));
    for(uint32_t i = 0; i < number_of_pages; i++) {
        lock_page_entry(page_table_entry);
        *page_table_entry++ = 0; // this will also clear the spinlock
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
        lock_page_entry(page_directory_entry);
        if((*page_directory_entry & VM_FLAG_PRESENT) == VM_FLAG_PRESENT) {
            unlock_page_entry(page_directory_entry);
            continue;
        }
        *page_directory_entry = ((uint32_t)pm_alloc_page() | VM_PAGE_TABLE); // this will also clear the spinlock
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
        lock_page_entry(page_table_entry);
        if((*page_table_entry & VM_FLAG_PRESENT) == VM_FLAG_PRESENT) {
            unlock_page_entry(page_table_entry);
            continue;
        }
        *page_table_entry = ((uint32_t)pm_alloc_page() | flags); // this will also clear the spinlock
    }
}

void vm_deallocate_pages(uint32_t vm_start_page, uint32_t size) {
    // align size to page size
    size = PAGE_MASK(size + PAGE_SIZE - 1);

    // claim all pages and set them as kernel pages
    uint32_t number_of_pages = (size >> 12);
    uint32_t *page_table_entry = (uint32_t *) (P_MEM_PAGE_TABLE + (vm_start_page >> 10));
    for(uint32_t i = 0; i < number_of_pages; i++, page_table_entry++) {
        lock_page_entry(page_table_entry);
        if((*page_table_entry & VM_FLAG_PRESENT) == 0) {
            unlock_page_entry(page_table_entry);
            continue;
        }
        *page_table_entry = ((*page_table_entry & ~VM_FLAG_USER) | VM_FLAG_SPINLOCK);
    }

    // clear pages
    memset((void *) vm_start_page, 0, size);

    // deallocate pages
    page_table_entry = (uint32_t *) (P_MEM_PAGE_TABLE + (vm_start_page >> 10));
    for(uint32_t i = 0; i < number_of_pages; i++, page_table_entry++) {
        pm_free_page((void *) PAGE_MASK(*page_table_entry));
        *page_table_entry = 0; // this will also clear the spinlock
    }

    // flush TLB
    if(number_of_pages < NUMBER_OF_PAGES_TO_FLUSH_BY_INVLPG) {
        for(uint32_t i = 0; i < number_of_pages; i++) {
            invlpg(vm_start_page + (i * PAGE_SIZE));
        }
    }
    else {
        write_cr3(read_cr3());
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
            lock_page_entry(old_page_table_entry);
            lock_page_entry(new_page_table_entry);
            *new_page_table_entry = *old_page_table_entry;
            *old_page_table_entry = 0;
            unlock_page_entry(new_page_table_entry);
            unlock_page_entry(old_page_table_entry);
            old_page_table_entry--;
            new_page_table_entry--;
        }
    }
    else {
        uint32_t *old_page_table_entry = (uint32_t *) (P_MEM_PAGE_TABLE + (vm_old_start >> 10));
        uint32_t *new_page_table_entry = (uint32_t *) (P_MEM_PAGE_TABLE + (vm_new_start >> 10));
        for(uint32_t i = 0; i < size; i += PAGE_SIZE) {
            lock_page_entry(old_page_table_entry);
            lock_page_entry(new_page_table_entry);
            *new_page_table_entry = *old_page_table_entry;
            *old_page_table_entry = 0;
            unlock_page_entry(new_page_table_entry);
            unlock_page_entry(old_page_table_entry);
            old_page_table_entry++;
            new_page_table_entry++;
        }
    }

    // flush TLB
    if(flush_tlb == true) {
        write_cr3(read_cr3());
    }
}

/* functions for flushing TLB */
void vm_refresh_mappings(void) {
    write_cr3(read_cr3());
}

void load_page_directory(uint32_t pm_page_directory) {
    write_cr3(pm_page_directory);
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
    uint32_t start_vaddr = address & ~(PAGE_SIZE - 1);
    uint32_t end_vaddr = (address + size + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1);

    for(uint32_t vaddr = start_vaddr; vaddr < end_vaddr; vaddr += PAGE_SIZE) {
        uint32_t pd_index = vaddr >> 22;
        uint32_t pt_index = (vaddr >> 12) & 0x3FF;

        // map page table
        uint32_t *pd_entry = (uint32_t *)(P_MEM_PAGE_DIRECTORY + (pd_index * 4));
        lock_page_entry(pd_entry);
        if((*pd_entry & VM_FLAG_PRESENT) == 0) {
            *pd_entry = ((uint32_t)pm_alloc_page() | VM_PAGE_TABLE); // this will also clear the spinlock
        }
        else {
            unlock_page_entry(pd_entry);
        }

        // map page
        uint32_t *pt_entries = (uint32_t *)(P_MEM_PAGE_TABLE + (pd_index * PAGE_SIZE));
        lock_page_entry(&pt_entries[pt_index]);
        if((pt_entries[pt_index] & VM_FLAG_PRESENT) == 0) {
            pt_entries[pt_index] = ((uint32_t)pm_alloc_page() | VM_USER); // this will also clear the spinlock
        }
        else {
            unlock_page_entry(&pt_entries[pt_index]);
        }
    }
}

void unmap_physical_pages_from_userspace(uint32_t address, uint32_t size) {
    // unmap pages
    uint32_t start_vaddr = address & ~(PAGE_SIZE - 1);
    uint32_t end_vaddr = (address + size + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1);

    for(uint32_t vaddr = start_vaddr; vaddr < end_vaddr; vaddr += PAGE_SIZE) {
        uint32_t pd_index = vaddr >> 22;
        uint32_t pt_index = (vaddr >> 12) & 0x3FF;

        // if table does not exist, skip it
        uint32_t *pd_entry = (uint32_t *)(P_MEM_PAGE_DIRECTORY + (pd_index * 4));
        lock_page_entry(pd_entry);
        if((*pd_entry & VM_FLAG_PRESENT) == 0) {
            unlock_page_entry(pd_entry);
            vaddr = (vaddr + 0x400000) & ~0x3FFFFF; // skip right to next page table
            vaddr -= PAGE_SIZE; // compensate for vaddr += PAGE_SIZE in the loop
            continue;
        }
        unlock_page_entry(pd_entry);

        uint32_t *pt_entries = (uint32_t *)(P_MEM_PAGE_TABLE + (pd_index * PAGE_SIZE));
        lock_page_entry(&pt_entries[pt_index]);
        uint32_t pte = pt_entries[pt_index];

        if((pte & VM_FLAG_PRESENT) != 0) {
            uint32_t type = (pte & VM_FLAGS_TYPE);
            
            // unmap only if it is normal page
            if(type != VM_LAZY_ALLOCATION && type != VM_COW_ALLOCATION && type != VM_SPAWN_TEMPLATE) {
                memset((void *)vaddr, 0, PAGE_SIZE);
                pm_free_page((void *)PAGE_MASK(pte));
            }

            // remove page from page table
            pt_entries[pt_index] = 0; // this will also clear the spinlock
        }
        else {
            unlock_page_entry(&pt_entries[pt_index]);
        }
    }

    // unmap page tables
    uint32_t first_pd = start_vaddr >> 22;
    uint32_t last_pd = (end_vaddr - 1) >> 22;

    for (uint32_t pd_idx = first_pd; pd_idx <= last_pd; pd_idx++) {
        uint32_t *pd_entry = (uint32_t *)(P_MEM_PAGE_DIRECTORY + (pd_idx * 4));
        lock_page_entry(pd_entry);
        if((*pd_entry & VM_FLAG_PRESENT) == 0) {
            unlock_page_entry(pd_entry);
            continue;
        }

        uint32_t *pt_entries = (uint32_t *)(P_MEM_PAGE_TABLE + (pd_idx * PAGE_SIZE));
        int is_empty = true;

        for(int i = 0; i < 1024; i++) {
            lock_page_entry(&pt_entries[i]);
            if(pt_entries[i] != 0) {
                unlock_page_entry(&pt_entries[i]);
                is_empty = false;
                break;
            }
            unlock_page_entry(&pt_entries[i]);
        }

        if(is_empty == true) {
            pm_free_page((void *)PAGE_MASK(*pd_entry));
            *pd_entry = 0; // remove page table from page directory, this will also clear the spinlock
        }
        else {
            unlock_page_entry(pd_entry);
        }
    }

    vm_refresh_mappings();
    // TODO: for all other processors
}

// this method will be always running only on one processor at same time
void free_virtual_space(uint32_t page_directory) {
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
    // TODO: clear and free page directory itself
}