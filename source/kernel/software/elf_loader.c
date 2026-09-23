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
#include <syslib.h>
#include <kernel/hardware/groups/logging/logging.h>
#include <kernel/libc/string.h>
#include <kernel/hardware/devices/memory/physical_memory.h>
#include <kernel/hardware/devices/memory/virtual_memory.h>
#include <kernel/hardware/devices/memory/memory_allocators.h>
#include <kernel/software/syscall.h>
#include <kernel/software/spawning_template.h>
#include <kernel/software/ramdisk.h>
#include <kernel/hardware/devices/cpu/commands.h>

/* local variables */
uint8_t close_thread_function[] = {
    0xB8, 0x05, 0x00, 0x00, 0x00, // mov eax, 5 ; DEMAND_TYPE_CLOSE_THREAD
    0xCD, 0xD0                    // int 0xD0
};

/* functions */
spawning_template_t load_elf32_to_spawning_template(void *elf_data, void prepare_memory(void)) {
    spawning_template_t template = {0};
    if(elf_data == (void *)0xFFFFFFFF) {
        log("\nInvalid ELF file location");
        return template;
    }

    // validation of ELF header
    Elf32_Ehdr *header = (Elf32_Ehdr *) elf_data;
    if(*(uint32_t *)header->e_ident != ELF_MAGIC) {
        log("\nInvalid ELF file");
        return template;
    }

    // create new virtual address space
    uint32_t original_page_directory = read_cr3();
    lock_core();
    template.page_directory = vm_create_new_userspace();

    // read program headers
    Elf32_Phdr *ph = (Elf32_Phdr *)((uint8_t *)elf_data + header->e_phoff);
    uint32_t lowest_used_memory = 0xFFFFFFFF;
    uint32_t highest_used_memory = 0;
    for(int i = 0; i < header->e_phnum; i++, ph++) {
        if(ph->p_type != PT_LOAD) {
            continue;
        }

        log("\nLoading segment: vaddr=0x%x, memsz=%d, filesz=%d, flags:", ph->p_vaddr, ph->p_memsz, ph->p_filesz);
        if((ph->p_flags & PF_R) == PF_R) {
            log(" readable");
        }
        if((ph->p_flags & PF_W) == PF_W) {
            log(" writeable");
        }
        if((ph->p_flags & PF_X) == PF_X) {
            log(" executable");
        }
        if(ph->p_memsz < ph->p_filesz) {
            log("\nInvalid segment: memsz < filesz");
            free_virtual_space(template.page_directory);
            template.page_directory = 0;
            unlock_core();
            return template;
        }

        // check if segment is in valid memory
        uint32_t segment_end = (ph->p_vaddr + ph->p_memsz);
        if(ph->p_vaddr == 0 || ph->p_vaddr < VM_USER_SPACE_START || ph->p_vaddr >= segment_end || segment_end > VM_USER_SPACE_END) {
            log("\nSegment is not in valid memory");
            free_virtual_space(template.page_directory);
            template.page_directory = 0;
            unlock_core();
            return template;
        }
        if(PAGE_MASK(ph->p_vaddr) < lowest_used_memory) {
            lowest_used_memory = PAGE_MASK(ph->p_vaddr);
        }
        if((PAGE_MASK(ph->p_vaddr) + PAGE_SIZE) > highest_used_memory) {
            highest_used_memory = (PAGE_MASK(ph->p_vaddr) + PAGE_SIZE);
        }

        // copy program segment to its virtual address
        memcpy((void *) ph->p_vaddr, (void *) ((uint8_t *)elf_data + ph->p_offset), ph->p_filesz);

        // clear remaining memory if memsz > filesz
        if(ph->p_memsz > ph->p_filesz) {
            memset((void *)(ph->p_vaddr + ph->p_filesz), 0, ph->p_memsz - ph->p_filesz);
        }

        // update flags of pages in this segment
        uint32_t *page_table_entry = (uint32_t *) (VM_PAGE_TABLES + ((ph->p_vaddr >> 12) * 4));
        uint32_t first_page = (ph->p_vaddr >> 12);
        uint32_t last_page = ((ph->p_vaddr + ph->p_memsz - 1) >> 12);
        log(" first page: 0x%x, last page: 0x%x", first_page, last_page);
        for(uint32_t j = first_page; j <= last_page; j++, page_table_entry++) {
            *page_table_entry &= ~VM_FLAG_READ_WRITE; // everything is read only
            if(ph->p_flags & PF_W) {
                *page_table_entry = (*page_table_entry & ~VM_FLAGS_TYPE) | VM_COW_ALLOCATION;
            }
            else {
                *page_table_entry = (*page_table_entry & ~VM_FLAGS_TYPE) | VM_SPAWN_TEMPLATE;
            }
        }
    }
    if(highest_used_memory <= lowest_used_memory) {
        log("\n[ELF] Unable to load ELF file, no valid segments found");
        free_virtual_space(template.page_directory);
        template.page_directory = 0;
        unlock_core();
        return template;
    }

    // read entry point
    template.entry_point = header->e_entry;

    // prepare memory in new user space
    if(prepare_memory != NULL) {
        prepare_memory();
    }

    // load system libraries
    if(load_static_elf32_to_memory(get_ramdisk_file_ptr("userspace_library.elf")) == ERROR) {
        log("\n[ELF] Unable to load userspace library");
        free_virtual_space(template.page_directory);
        template.page_directory = 0;
        unlock_core();
        return template;
    }
    template.syslib_got = get_elf_symbol_addr(get_ramdisk_file_ptr("userspace_library.elf"), "syslib_got");
    log("\n[ELF] syslib_got: %x", template.syslib_got);

    // set up virtual hardware interface
    template.virtual_hardware = (void *) VM_VIRTUAL_HARDWARE_INTERFACE;
    virtual_hardware_t *virtual_hardware_interface = (virtual_hardware_t *) (uint32_t)(template.virtual_hardware);

    // set up functions
    void *close_thread_function_ptr = (uint32_t *) (VM_USER_SPACE_END - sizeof(close_thread_function));
    memcpy(close_thread_function_ptr, close_thread_function, sizeof(close_thread_function));
    uint32_t *functions_page_table_entry = (uint32_t *) (VM_PAGE_TABLES + (((VM_USER_SPACE_END - PAGE_SIZE) >> 12) * 4));
    *functions_page_table_entry = (*functions_page_table_entry & ~(VM_FLAGS_TYPE | VM_FLAG_READ_WRITE)) | VM_SPAWN_TEMPLATE;

    // set up user stack
    uint32_t *stack_pointer = (uint32_t *) (VM_USER_SPACE_END - PAGE_SIZE);
    stack_pointer -= 3;
    stack_pointer[0] = (uint32_t) (close_thread_function_ptr); // pointer to function to close thread in user space
    stack_pointer[1] = (uint32_t) (template.syslib_got); // pointer to syslib functions
    stack_pointer[2] = (uint32_t) (template.virtual_hardware); // pointer to virtual hardware interface
    template.user_stack = stack_pointer;
    uint32_t *stack_page_table_entry = (uint32_t *) (VM_PAGE_TABLES + (((VM_USER_SPACE_END - PAGE_SIZE - PAGE_SIZE) >> 12) * 4));
    *stack_page_table_entry = (*stack_page_table_entry & ~(VM_FLAGS_TYPE | VM_FLAG_READ_WRITE)) | VM_COW_ALLOCATION;

    load_page_directory(original_page_directory);
    unlock_core();
    return template;
}

int load_static_elf32_to_memory(void *elf_data) {
    Elf32_Ehdr *header = (Elf32_Ehdr *) elf_data;
    Elf32_Phdr *ph = (Elf32_Phdr *)((uint8_t *)elf_data + header->e_phoff);

    for(int i = 0; i < header->e_phnum; i++, ph++) {
        if(ph->p_type != PT_LOAD) {
            continue;
        }

        // check if segment has vaild length
        if(ph->p_memsz < ph->p_filesz) {
            log("\nInvalid segment: memsz < filesz");
            return ERROR;
        }

        // load segment to memory
        log("\nLoading segment: vaddr=0x%x, memsz=0x%x, filesz=0x%x, flags:", ph->p_vaddr, ph->p_memsz, ph->p_filesz);
        if((ph->p_flags & PF_R) == PF_R) {
            log(" readable");
        }
        if((ph->p_flags & PF_W) == PF_W) {
            log(" writeable");
        }
        if((ph->p_flags & PF_X) == PF_X) {
            log(" executable");
        }
        memcpy((void *) ph->p_vaddr, (void *) ((uint8_t *)elf_data + ph->p_offset), ph->p_filesz);
        if(ph->p_memsz > ph->p_filesz) {
            memset((void *)(ph->p_vaddr + ph->p_filesz), 0, ph->p_memsz - ph->p_filesz);
        }

        // update flags of pages in this segment
        uint32_t *page_table_entry = (uint32_t *) (VM_PAGE_TABLES + ((ph->p_vaddr >> 12) * 4));
        uint32_t first_page = (ph->p_vaddr >> 12);
        uint32_t last_page = ((ph->p_vaddr + ph->p_memsz - 1) >> 12);
        for(uint32_t j = first_page; j <= last_page; j++, page_table_entry++) {
            *page_table_entry &= ~VM_FLAG_READ_WRITE;
            if(ph->p_flags & PF_W) {
                *page_table_entry = (*page_table_entry & ~VM_FLAGS_TYPE) | VM_COW_ALLOCATION;
            }
            else {
                *page_table_entry = (*page_table_entry & ~VM_FLAGS_TYPE) | VM_SPAWN_TEMPLATE;
            }
        }
    }

    return SUCCESS;
}

void *get_elf_symbol_addr(void *elf_data, const uint8_t *symbol_name) {
    Elf32_Ehdr *header = (Elf32_Ehdr *) elf_data;
    Elf32_Shdr *sections = (Elf32_Shdr *) ((uintptr_t)elf_data + header->e_shoff);
    Elf32_Sym *symtab = NULL;
    char *strtab = NULL;
    uint32_t symbol_count = 0;

    for(int i = 0; i < header->e_shnum; i++) {
        if(sections[i].sh_type == SHT_SYMTAB) {
            symtab = (Elf32_Sym *) ((uintptr_t)elf_data + sections[i].sh_offset);
            symbol_count = sections[i].sh_size / sizeof(Elf32_Sym);
            Elf32_Shdr *strtab_section = &sections[sections[i].sh_link];
            strtab = (char *)((uintptr_t)elf_data + strtab_section->sh_offset);
            break;
        }
    }
    if(!symtab || !strtab) {
        return NULL;
    }

    for(uint32_t i = 0; i < symbol_count; i++) {
        char *name = strtab + symtab[i].st_name;
        if(strcmp(name, symbol_name) == 0) {
            return (void *) symtab[i].st_value;
        }
    }
    return NULL;
}