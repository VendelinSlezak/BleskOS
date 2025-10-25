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
#include <kernel/x86/memory/pm_allocator.h>
#include <kernel/x86/entities/entities.h>
#include <kernel/x86/libc/stdlib.h>
#include <kernel/x86/libc/string.h>

/* local variables */
dword_t *pm_zero_page;
dword_t *pm_zero_page_table;

/* functions */
void initialize_virtual_memory(void) {
    // initialize zero memory
    pm_zero_page = pm_alloc_page();
    vm_map_page(0x00000000, (dword_t)pm_zero_page, VM_FLAGS_KERNEL_RW);
    memset(0x00000000, 0, PAGE_SIZE);

    pm_zero_page_table = pm_alloc_page();
    vm_map_page(0x00000000, (dword_t)pm_zero_page_table, VM_FLAGS_KERNEL_RW); // we will never need to write to zero page again, so we do not need to keep it in vm
    dword_t *pm_zero_page_table_ptr = (dword_t *) 0x00000000;
    for(int i = 0; i < 1024; i++) {
        pm_zero_page_table_ptr[i] = (((dword_t)pm_zero_page) | VM_FLAG_PRESENT | VM_FLAG_READ_ONLY | VM_FLAG_USER | VM_FLAG_WRITE_BACK);
    }
    vm_unmap_page(0x00000000);
}

void pd_map_page(dword_t vm_page, dword_t pm_page, dword_t flags) {
    flags |= VM_FLAG_PRESENT;

    // get virtual address of page directory entry
    dword_t pd_index = vm_page >> 22;
    dword_t *page_directory_entry = (dword_t *) (P_MEM_PAGE_DIRECTORY + (pd_index * 4));

    // check if page table exist
    if((*page_directory_entry & VM_FLAG_PRESENT) == 0) {
        void *new_pt_phys = pm_alloc_page(); // allocate new page table
        *page_directory_entry = ((dword_t)new_pt_phys | VM_FLAG_PRESENT | VM_FLAG_READ_WRITE | VM_FLAG_USER); // add page table to page directory
        memset((void *)(P_MEM_PAGE_TABLE + (pd_index << 12)), 0, PAGE_SIZE); // fill it with zeros
    }
    // be sure that all flags are present
    else {
        *page_directory_entry |= flags;
    }

    // set page table entry
    dword_t pt_index = ((vm_page >> 12) & 0x3FF);
    dword_t *page_table_entry = (dword_t *) (P_MEM_PAGE_TABLE + (pd_index << 12) + (pt_index * 4));
    *page_table_entry = (pm_page | flags);
}

void pd_unmap_page(dword_t vm_page) {
    // get virtual address of page directory entry
    dword_t pd_index = vm_page >> 22;
    dword_t *page_directory_entry = (dword_t *) (P_MEM_PAGE_DIRECTORY + (pd_index * 4));

    // check if page table exist
    if((*page_directory_entry & VM_FLAG_PRESENT) == 0) {
        return; // page is already unmapped
    }

    // unmap page table entry
    dword_t pt_index = ((vm_page >> 12) & 0x3FF);
    dword_t *page_table_entry = (dword_t *) (P_MEM_PAGE_TABLE + (pd_index << 12) + (pt_index * 4));
    if((*page_table_entry & VM_FLAG_PRESENT) == 0) {
        return; // page is already unmapped
    }
    *page_table_entry = 0x00000000;
}

void *get_pm_of_vm_page(dword_t vm_page) {
    // get virtual address of page directory entry
    dword_t pd_index = vm_page >> 22;
    dword_t *page_directory_entry = (dword_t *) (P_MEM_PAGE_DIRECTORY + (pd_index * 4));

    // check if page table exist
    if((*page_directory_entry & VM_FLAG_PRESENT) == 0) {
        return (void *) INVALID;
    }

    // get virtual address of page table entry
    dword_t pt_index = ((vm_page >> 12) & 0x3FF);
    dword_t *page_table_entry = (dword_t *) (P_MEM_PAGE_TABLE + (pd_index << 12) + (pt_index * 4));
    
    // return physical memory
    if((*page_table_entry & VM_FLAG_PRESENT) == 0) {
        return (void *) INVALID;
    }
    else {
        return (void *) (*page_table_entry & 0xFFFFF000);
    }
}

void vm_map_page(dword_t vm_page, dword_t pm_page, dword_t flags) {
    // map page to page directory
    pd_map_page(vm_page, pm_page, flags);

    // invalidate TLB
    invlpg(vm_page);
}

void vm_unmap_page(dword_t vm_page) {
    // map page to page directory
    pd_unmap_page(vm_page);

    // invalidate TLB
    invlpg(vm_page);
}

// TODO: check page table
void vm_map_block(dword_t vm_page, dword_t flags, dword_t size) {
    // align size to page size
    if((size & 0xFFF) != 0x000) {
        size &= 0xFFFFF000;
        size += PAGE_SIZE;
    }

    // map pages to page directory
    for(int i = 0; i < size; i += PAGE_SIZE) {
        pd_map_page(vm_page, (dword_t)pm_alloc_page(), flags);
        vm_page += PAGE_SIZE;
    }

    // flush TLB
    write_cr3(PM_KERNEL_PAGE_DIRECTORY);
}

// TODO: check page table
void vm_unmap_block(dword_t vm_page, dword_t size) {
    // align size to page size
    if((size & 0xFFF) != 0x000) {
        size &= 0xFFFFF000;
        size += PAGE_SIZE;
    }

    // unmap pages to page directory
    for(int i = 0; i < size; i += PAGE_SIZE) {
        pd_unmap_page(vm_page);
        vm_page += PAGE_SIZE;
    }

    // flush TLB
    write_cr3(PM_KERNEL_PAGE_DIRECTORY);
}

void vm_map_zeros(dword_t vm_page, dword_t size) {
    // align size to page size
    if((size & 0xFFF) != 0x000) {
        size &= 0xFFFFF000;
        size += PAGE_SIZE;
    }

    // calculate aligned sizes
    dword_t offset_in_4mb = vm_page & 0x003FFFFF;
    dword_t size_to_4_mb_align = (offset_in_4mb == 0) ? 0 : (0x400000 - offset_in_4mb);
    dword_t number_of_aligned_4_mb = 0;
    dword_t size_to_last_4_mb = 0;
    if(size > size_to_4_mb_align) {
        size -= size_to_4_mb_align;
        number_of_aligned_4_mb = (size / 0x400000);
        size_to_last_4_mb = (size % 0x400000);
    }

    // map pages to align first 4 MB
    for(int i = 0; i < (size_to_4_mb_align >> 12); i++, vm_page += PAGE_SIZE) {
        pd_map_page(vm_page, (dword_t)pm_zero_page, VM_FLAG_READ_ONLY | VM_FLAG_USER | VM_FLAG_WRITE_BACK);
    }

    // map page tables to 4 MB blocks
    dword_t *page_directory_entry = (dword_t *) (P_MEM_PAGE_DIRECTORY + ((vm_page >> 22) * 4));
    for(int i = 0; i < number_of_aligned_4_mb; i++, page_directory_entry++, vm_page += (PAGE_SIZE * PAGE_SIZE)) {
        *page_directory_entry = (((dword_t) pm_zero_page_table) | VM_FLAG_PRESENT | VM_FLAG_READ_ONLY | VM_FLAG_USER | VM_FLAG_WRITE_BACK);
    }

    // map pages to last 4 MB
    for(int i = 0; i < (size_to_last_4_mb >> 12); i++, vm_page += PAGE_SIZE) {
        pd_map_page(vm_page, (dword_t)pm_zero_page, VM_FLAG_READ_ONLY | VM_FLAG_USER | VM_FLAG_WRITE_BACK);
    }

    // flush TLB
    write_cr3(PM_KERNEL_PAGE_DIRECTORY);
}

void vm_alloc_page(dword_t page, dword_t flags) {
    pd_map_page(page, (dword_t)pm_alloc_page(), flags);
    invlpg(page);
    memset((void *)page, 0, PAGE_SIZE);
}

void vm_log_flags(dword_t flags) {
    flags &= 0xFFF;

    log("(");

    if(flags & VM_FLAG_PRESENT) {
        log("present");

        if(flags & VM_FLAG_READ_WRITE) {
            log(" read-write");
        }
        else {
            log(" read-only");
        }

        if(flags & VM_FLAG_USER) {
            log(" user");
        }
        else {
            log(" supervisor");
        }

        if((flags & VM_FLAG_WRITE_BACK) == VM_FLAG_WRITE_BACK) {
            log(" write-back");
        }
        else if((flags & VM_FLAG_WRITE_THROUGH) == VM_FLAG_WRITE_THROUGH) {
            log(" write-through");
        }
        else if((flags & VM_FLAG_WRITE_COMBINED) == VM_FLAG_WRITE_COMBINED) {
            log(" write-combined");
        }
        else if((flags & VM_FLAG_UNCACHEABLE) == VM_FLAG_UNCACHEABLE) {
            log(" uncacheable");
        }
        else {
            log(" unknown");
        }
    }
    else {
        log("not present");
    }

    if((flags & VM_FLAG_TYPE_LAZY_ALLOCATED) == VM_FLAG_TYPE_LAZY_ALLOCATED) {
        log(" LAZY ALLOCATION");
    }
    else if((flags & VM_FLAG_TYPE_ERROR_BY_ACCESS) == VM_FLAG_TYPE_ERROR_BY_ACCESS) {
        log(" ERROR BY ACCESS");
    }
    else if((flags & VM_FLAG_TYPE_UNUSABLE) == VM_FLAG_TYPE_UNUSABLE) {
        log(" UNUSABLE");
    }

    log(")");
}

/* functions for allocator */
void bitmap_search_for_free_block(virtual_memory_allocator_bitmap_t *vma_bitmap, dword_t block_size) {
    log("\n Search starts");

    dword_t free_block_start = INVALID;
    dword_t free_block_size = 0xFFFFFFFF;

    dword_t actual_free_block_start = INVALID;
    dword_t actual_free_block_size = 0;
    for(dword_t i = 0, block = 0x00000000; i < SIZE_OF_BITMAP; i++) {
        for(int j = 0; j < 8; j++, block += SIZE_OF_VM_BLOCK) {
            // we found free block
            if((vma_bitmap->bitmap[i] & (1 << j)) == 0) {
                if(actual_free_block_start == INVALID) {
                    actual_free_block_start = block;
                    actual_free_block_size = SIZE_OF_VM_BLOCK;
                }
                else {
                    actual_free_block_size += SIZE_OF_VM_BLOCK;
                }
            }
            // we found used block
            else if(actual_free_block_size != 0) {
                log("\n[BITMAP] Founded free block from 0x%x to 0x%x", actual_free_block_start, (actual_free_block_start + actual_free_block_size));
                if(actual_free_block_size >= block_size && actual_free_block_size < free_block_size) {
                    free_block_start = actual_free_block_start;
                    free_block_size = actual_free_block_size;
                }
                actual_free_block_size = 0;
            }
        }
    }
    if(actual_free_block_size != 0) {
        log("\n[BITMAP] Founded free block from 0x%x to 0x%x", actual_free_block_start, (actual_free_block_start + actual_free_block_size));
        if(actual_free_block_size >= block_size && actual_free_block_size < free_block_size) {
            free_block_start = actual_free_block_start;
            free_block_size = actual_free_block_size;
        }
    }

    log("\n Search ends");

    if(free_block_start != INVALID) {
        vma_bitmap->free_block_start = free_block_start;
        vma_bitmap->free_block_size = free_block_size;
    }
}

void *alloc_from_bitmap(dword_t *page_directory, virtual_memory_allocator_bitmap_t *vma_bitmap, dword_t size) {
    // align size to size of memory block
    dword_t aligned_size = size;
    if((aligned_size & (SIZE_OF_VM_BLOCK-1)) != 0) {
        aligned_size &= ~(SIZE_OF_VM_BLOCK-1);
        aligned_size += SIZE_OF_VM_BLOCK;
    }

    // check if virtual memory has free block big enough for allocation
    if(vma_bitmap->free_block_size < aligned_size) {
        bitmap_search_for_free_block(vma_bitmap, aligned_size);
        if(vma_bitmap->free_block_size < aligned_size) {
            kernel_panic("Can not allocate virtual memory");
            return (void *) INVALID;
        }
    }

    // allocate block
    void *allocation_start = (void *) (vma_bitmap->free_block_start);
    vma_bitmap->free_block_start += aligned_size;
    vma_bitmap->free_block_size -= aligned_size;

    // update bitmap
    dword_t first_block = ((dword_t)allocation_start >> 17);
    dword_t number_of_blocks = (aligned_size >> 17);
    for(dword_t i = first_block; i < (first_block + number_of_blocks); i++) {
        vma_bitmap->bitmap[(i >> 3)] |= (1 << (i & 0x7));
    }

    // allocate page tables
    dword_t first_page_table = ((dword_t)allocation_start >> 22);
    dword_t last_page_table = (((dword_t)allocation_start + aligned_size - 1) >> 22);
    for(int i = first_page_table; i <= last_page_table; i++) {
        if((page_directory[i] & VM_FLAG_PRESENT) == 0) {
            page_directory[i] = ((dword_t)pm_alloc_page() | VM_FLAGS_PAGE_TABLE);
            memset((void *)(P_MEM_PAGE_TABLE + i*PAGE_SIZE), 0, PAGE_SIZE);
        }
    }

    return allocation_start;
}

void *prepare_alloc(dword_t aligned_size) {
    // align size to size of memory block
    if((aligned_size & (SIZE_OF_VM_BLOCK-1)) != 0) {
        aligned_size &= ~(SIZE_OF_VM_BLOCK-1);
        aligned_size += SIZE_OF_VM_BLOCK;
    }

    // check if virtual memory has free block big enough for allocation
    command_info_t *command_info = (command_info_t *) P_MEM_COMMAND_INFO;
    virtual_memory_allocator_bitmap_t *vma_bitmap = (virtual_memory_allocator_bitmap_t *) command_info->vm_of_vma_bitmap;
    if(vma_bitmap->free_block_size < aligned_size) {
        bitmap_search_for_free_block(vma_bitmap, aligned_size);
        if(vma_bitmap->free_block_size < aligned_size) {
            kernel_panic("Can not allocate virtual memory");
            return (void *) INVALID;
        }
    }

    // allocate block
    void *allocation_start = (void *) (vma_bitmap->free_block_start);
    vma_bitmap->free_block_start += aligned_size;
    vma_bitmap->free_block_size -= aligned_size;

    // update bitmap
    dword_t first_block = ((dword_t)allocation_start >> 17);
    dword_t number_of_blocks = (aligned_size >> 17);
    for(dword_t i = first_block; i < (first_block + number_of_blocks); i++) {
        vma_bitmap->bitmap[(i >> 3)] |= (1 << (i & 0x7));
    }

    // update metadata
    virtual_memory_allocator_blocks_metadata_t *vma_metadata = (virtual_memory_allocator_blocks_metadata_t *) command_info->vm_of_vma_metadata;
    vma_metadata->allocated_blocks[first_block] = number_of_blocks;

    // allocate page tables
    dword_t first_page_table = ((dword_t)allocation_start >> 22);
    dword_t last_page_table = (((dword_t)allocation_start + aligned_size - 1) >> 22);
    dword_t *page_directory = (dword_t *) P_MEM_PAGE_DIRECTORY;
    for(int i = first_page_table; i <= last_page_table; i++) {
        if((page_directory[i] & VM_FLAG_PRESENT) == 0) {
            page_directory[i] = ((dword_t)pm_alloc_page() | VM_FLAGS_PAGE_TABLE);
            memset((void *)(P_MEM_PAGE_TABLE + i*PAGE_SIZE), 0, PAGE_SIZE);
        }
    }

    return allocation_start;
}

// TODO: clear rest of PAGE_SIZE that is not used - prevent leak of data
void *kmalloc(dword_t size) {
    // prepare allocation
    void *allocation_start = prepare_alloc(size);
    if((dword_t)allocation_start == INVALID) {
        return (void *) INVALID;
    }

    // allocate pages
    dword_t first_page = ((dword_t)allocation_start >> 12);
    dword_t last_page = (((dword_t)allocation_start + size - 1) >> 12);
    dword_t *page_table = (dword_t *) P_MEM_PAGE_TABLE;
    for(int i = first_page; i <= last_page; i++) {
        page_table[i] = ((dword_t)pm_alloc_page() | VM_FLAGS_USER_RW | VM_FLAG_PRESENT);
    }

    return allocation_start;
}

void *kcalloc(dword_t size) {
    // prepare allocation
    void *allocation_start = prepare_alloc(size);
    if((dword_t)allocation_start == INVALID) {
        return (void *) INVALID;
    }

    // allocate pages
    dword_t first_page = ((dword_t)allocation_start >> 12);
    dword_t last_page = (((dword_t)allocation_start + size - 1) >> 12);
    dword_t *page_table = (dword_t *) P_MEM_PAGE_TABLE;
    for(int i = first_page; i <= last_page; i++) {
        page_table[i] = ((dword_t)pm_alloc_page() | VM_FLAGS_USER_RW | VM_FLAG_PRESENT);
    }

    // clear
    memset(allocation_start, 0, ((size + (PAGE_SIZE - 1)) / PAGE_SIZE) * PAGE_SIZE);

    return allocation_start;
}

void *klalloc(dword_t size) {
    // prepare allocation
    void *allocation_start = prepare_alloc(size);
    if((dword_t)allocation_start == INVALID) {
        return (void *) INVALID;
    }

    // allocate pages
    dword_t first_page = ((dword_t)allocation_start >> 12);
    dword_t last_page = (((dword_t)allocation_start + size - 1) >> 12);
    dword_t *page_table = (dword_t *) P_MEM_PAGE_TABLE;
    for(int i = first_page; i <= last_page; i++) {
        page_table[i] = ((dword_t)pm_zero_page | VM_FLAG_PRESENT | VM_FLAG_READ_ONLY | VM_FLAG_USER | VM_FLAG_WRITE_BACK | VM_FLAG_TYPE_LAZY_ALLOCATED);
    }

    return allocation_start;
}

void *kpalloc(dword_t phy_start, dword_t size, dword_t type_of_memory) {
    // align phy start to page boundary
    if((phy_start & 0xFFF) != 0) {
        size += (phy_start & 0xFFF);
        phy_start &= 0xFFFFF000;
    }

    // prepare allocation
    void *allocation_start = prepare_alloc(size);
    if((dword_t)allocation_start == INVALID) {
        return (void *) INVALID;
    }

    // allocate pages
    dword_t first_page = ((dword_t)allocation_start >> 12);
    dword_t last_page = (((dword_t)allocation_start + size - 1) >> 12);
    dword_t *page_table = (dword_t *) P_MEM_PAGE_TABLE;
    for(int i = first_page; i <= last_page; i++, phy_start += PAGE_SIZE) {
        page_table[i] = ((dword_t)phy_start | VM_FLAG_PRESENT | VM_FLAGS_KERNEL_RW | type_of_memory);
    }

    return allocation_start;
}

// TODO: add expanding/shrinking from same position
void *krealloc(void *mem, size_t size) {
    // allocate new size
    void *new_mem = kmalloc(size);

    // get exact size of previous allocation
    command_info_t *command_info = (command_info_t *) P_MEM_COMMAND_INFO;
    virtual_memory_allocator_blocks_metadata_t *vma_metadata = (virtual_memory_allocator_blocks_metadata_t *) command_info->vm_of_vma_metadata;
    dword_t number_of_blocks = vma_metadata->allocated_blocks[((dword_t)mem >> 17)];
    dword_t old_mem_size = ((number_of_blocks - 1) * SIZE_OF_VM_BLOCK); // all blocks except for last one must be fully allocated
    dword_t first_page = (((dword_t)mem + old_mem_size) >> 12);
    dword_t last_page = (first_page + (SIZE_OF_VM_BLOCK / PAGE_SIZE));
    dword_t *page_table = (dword_t *) P_MEM_PAGE_TABLE;
    for(int i = first_page; i <= last_page; i++) {
        if((page_table[i] & VM_FLAG_PRESENT) == 0) {
            break; // we are at end of allocation
        }
        else {
            old_mem_size += PAGE_SIZE;
        }
    }

    // copy data
    memmove(new_mem, mem, old_mem_size);

    // free previous allocation
    free(mem);

    // return new allocation
    return new_mem;
}

void unmap(void *start) {
    // take size from metadata
    command_info_t *command_info = (command_info_t *) P_MEM_COMMAND_INFO;
    virtual_memory_allocator_bitmap_t *vma_bitmap = (virtual_memory_allocator_bitmap_t *) command_info->vm_of_vma_bitmap;
    virtual_memory_allocator_blocks_metadata_t *vma_metadata = (virtual_memory_allocator_blocks_metadata_t *) command_info->vm_of_vma_metadata;
    dword_t first_block = ((dword_t)start >> 17);
    dword_t number_of_blocks = vma_metadata->allocated_blocks[first_block];
    dword_t size = (number_of_blocks * SIZE_OF_VM_BLOCK);

    // update bitmap
    for(dword_t i = first_block; i < (first_block + number_of_blocks); i++) {
        vma_bitmap->bitmap[(i >> 3)] &= ~(1 << (i & 0x7));
    }

    // update metadata
    vma_metadata->allocated_blocks[first_block] = 0;

    // unmap pages
    dword_t first_page = ((dword_t)start >> 12);
    dword_t last_page = (((dword_t)start + size - 1) >> 12);
    dword_t *page_table = (dword_t *) P_MEM_PAGE_TABLE;
    for(int i = first_page; i <= last_page; i++) {
        if((page_table[i] & VM_FLAG_PRESENT) == 0) {
            break; // we are at end of allocation
        }
        else {
            page_table[i] = VM_FLAG_TYPE_ERROR_BY_ACCESS;
        }
    }

    // deallocate page tables
    dword_t first_page_table = ((dword_t)start >> 22);
    dword_t last_page_table = (((dword_t)start + size - 1) >> 22);
    dword_t *page_directory = (dword_t *) P_MEM_PAGE_DIRECTORY;
    dword_t *page_directory_in_bitmap = (dword_t *) (&vma_bitmap->bitmap[first_page_table*4]);
    for(int i = first_page_table; i <= last_page_table; i++, page_directory_in_bitmap++) {
        if(*page_directory_in_bitmap == 0x00000000) {
            pm_free_page((void *)(page_directory[i] & 0xFFFFF000));
            page_directory[i] = 0;
        }
    }

    // flush TLB
    write_cr3(kernel_attr->pm_of_loaded_page_directory);
}

void free(void *start) {
    // take size from metadata
    command_info_t *command_info = (command_info_t *) P_MEM_COMMAND_INFO;
    virtual_memory_allocator_bitmap_t *vma_bitmap = (virtual_memory_allocator_bitmap_t *) command_info->vm_of_vma_bitmap;
    virtual_memory_allocator_blocks_metadata_t *vma_metadata = (virtual_memory_allocator_blocks_metadata_t *) command_info->vm_of_vma_metadata;
    dword_t first_block = ((dword_t)start >> 17);
    dword_t number_of_blocks = vma_metadata->allocated_blocks[first_block];
    dword_t size = (number_of_blocks * SIZE_OF_VM_BLOCK);

    // update bitmap
    for(dword_t i = first_block; i < (first_block + number_of_blocks); i++) {
        vma_bitmap->bitmap[(i >> 3)] &= ~(1 << (i & 0x7));
    }

    // update metadata
    vma_metadata->allocated_blocks[first_block] = 0;

    // deallocate pages
    dword_t first_page = ((dword_t)start >> 12);
    dword_t last_page = (((dword_t)start + size - 1) >> 12);
    dword_t *page_table = (dword_t *) P_MEM_PAGE_TABLE;
    for(int i = first_page; i <= last_page; i++) {
        if((page_table[i] & VM_FLAG_PRESENT) == 0) {
            break; // we are at end of allocation
        }
        else {
            pm_free_page((void *)(page_table[i] & 0xFFFFF000));
            page_table[i] = VM_FLAG_TYPE_ERROR_BY_ACCESS;
        }
    }

    // deallocate page tables
    dword_t first_page_table = ((dword_t)start >> 22);
    dword_t last_page_table = (((dword_t)start + size - 1) >> 22);
    dword_t *page_directory = (dword_t *) P_MEM_PAGE_DIRECTORY;
    dword_t *page_directory_in_bitmap = (dword_t *) (&vma_bitmap->bitmap[first_page_table*4]);
    for(int i = first_page_table; i <= last_page_table; i++, page_directory_in_bitmap++) {
        if(*page_directory_in_bitmap == 0x00000000) {
            pm_free_page((void *)(page_directory[i] & 0xFFFFF000));
            page_directory[i] = 0;
        }
    }

    // flush TLB
    write_cr3(kernel_attr->pm_of_loaded_page_directory);
}