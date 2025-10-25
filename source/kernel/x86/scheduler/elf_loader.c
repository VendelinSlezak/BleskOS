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
#include <hardware/groups/logging/logging.h>
#include <kernel/x86/memory/pm_allocator.h>
#include <kernel/x86/memory/vm_allocator.h>
#include <kernel/x86/entities/entities.h>
#include <kernel/x86/libc/stdlib.h>
#include <kernel/x86/libc/string.h>

/* functions */
dword_t load_elf(byte_t *name, void *elf) {
    // TODO: check elf signature
    Elf32_Ehdr_t *ehdr = (Elf32_Ehdr_t *) elf;
    if(ehdr->e_type != ELF_ET_EXEC) {
        log("\n[ELF] ERROR invalid type: %d", ehdr->e_type);
        return INVALID;
    }

    // create entity
    word_t entity_number = create_entity(name, E_TYPE_USER_RING);
    entity_t *entity = entity_get_ptr(entity_number);
    vm_map_page(P_MEM_CE_VM_BITMAP, entity->pm_page_of_vm_bitmap, VM_FLAGS_KERNEL_RW);
    vm_map_page(P_MEM_CE_PAGE_DIRECTORY, entity->pm_page_of_page_directory, VM_FLAGS_PAGE_DIRECTORY);
    dword_t *elf_page_directory = (dword_t *) P_MEM_CE_PAGE_DIRECTORY;
    dword_t *elf_page_table = (dword_t *) P_MEM_CE_PAGE_TABLE;

    // load ELF to virtual memory of entity
    Elf32_Phdr_t *phdr = (Elf32_Phdr_t *) (elf + ehdr->e_phoff);
    for(int i = 0; i < ehdr->e_phnum; i++) {
        if(phdr[i].p_type == ELF_PT_LOAD) {
            log("\nSegment %d:", i);
            log("\n  Offset in file : 0x%x", phdr[i].p_offset);
            log("\n  Virtual addr   : 0x%x", phdr[i].p_vaddr);
            log("\n  Physical addr  : 0x%x", phdr[i].p_paddr);
            log("\n  File size      : %d bytes", phdr[i].p_filesz);
            log("\n  Memory size    : %d bytes", phdr[i].p_memsz);
            log("\n  Flags          : 0x%x", phdr[i].p_flags);

            // load segment to virtual memory
            void *segment_mem = kmalloc(phdr[i].p_memsz);
            memcpy(segment_mem, elf + phdr[i].p_offset, phdr[i].p_filesz);
            memset(segment_mem + phdr[i].p_filesz, 0, phdr[i].p_memsz - phdr[i].p_filesz);

            // copy segment to virtual memory of ELF entity
            dword_t loaded_page_directory_entry = INVALID;
            dword_t *page_table_entry = (dword_t *) (P_MEM_PAGE_TABLE + ((dword_t)segment_mem >> 12)*4); // pointer to pages of segment in virtual memory
            dword_t vm_offset = phdr[i].p_vaddr;
            dword_t number_of_pages = (phdr[i].p_memsz >> 12);
            if((phdr[i].p_memsz & 0xFFF) != 0) {
                number_of_pages++;
            }
            dword_t page_flags = (VM_FLAG_PRESENT | VM_FLAG_USER | VM_FLAG_WRITE_BACK);
            if(phdr[i].p_flags & ELF_PF_W) {
                page_flags |= VM_FLAG_READ_WRITE;
            }
            for(int j = 0; j < number_of_pages; j++, vm_offset += PAGE_SIZE) {
                // load page table
                dword_t vm_offset_pde = (vm_offset >> 22);
                if(loaded_page_directory_entry != vm_offset_pde) {
                    if((elf_page_directory[vm_offset_pde] & VM_FLAG_PRESENT) == 0) {
                        log("\nallocating page table %d", vm_offset_pde);
                        elf_page_directory[vm_offset_pde] = ((dword_t)pm_alloc_page() | VM_FLAG_PRESENT | VM_FLAG_READ_WRITE | VM_FLAG_USER);
                    }
                    log("\nmapping page table %d %x", vm_offset_pde, (elf_page_directory[vm_offset_pde] & 0xFFFFF000));
                    vm_map_page(P_MEM_CE_PAGE_TABLE, (elf_page_directory[vm_offset_pde] & 0xFFFFF000), VM_FLAGS_PAGE_TABLE);
                    loaded_page_directory_entry = vm_offset_pde;
                }

                // copy page
                dword_t vm_offset_pte = ((vm_offset >> 12) & 0x3FF);
                elf_page_table[vm_offset_pte] = (page_table_entry[j] & 0xFFFFF000) | page_flags;
                log("\ncopying page %d on VM 0x%x at PDE %d PTE %d as 0x%x", j, vm_offset, vm_offset_pde, vm_offset_pte, elf_page_table[vm_offset_pte]);
            }

            // update ELF entity bitmap
            dword_t first_block = (phdr[i].p_vaddr >> 17);
            dword_t last_block = ((phdr[i].p_vaddr + phdr[i].p_memsz) >> 17);
            virtual_memory_allocator_bitmap_t *vma_bitmap = (virtual_memory_allocator_bitmap_t *) P_MEM_CE_VM_BITMAP;
            for(dword_t block = first_block; block <= last_block; block++) {
                vma_bitmap->bitmap[(block >> 3)] |= (1 << (block & 0x7));
            }

            // unmap segment
            unmap(segment_mem);
        }
    }

    // add recursive mapping to page directory
    elf_page_directory[1023] = (entity->pm_page_of_page_directory | VM_FLAGS_PAGE_DIRECTORY);

    // update bump pointer to bitmap
    vm_map_page(P_MEM_CE_VM_BITMAP, entity->pm_page_of_vm_bitmap, VM_FLAGS_KERNEL_RW);
    bitmap_search_for_free_block((virtual_memory_allocator_bitmap_t *)P_MEM_CE_VM_BITMAP, 1);

    // TODO: check if there is valid pointer

    // start of ELF executable data
    // entity->functions[0] = (void *) ehdr->e_entry; // TODO: add to attributes

    return entity_number;
}