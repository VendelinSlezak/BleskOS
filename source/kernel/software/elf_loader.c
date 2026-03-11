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
#include <kernel/hardware/groups/logging/logging.h>
#include <libc/string.h>
#include <kernel/memory/physical_memory.h>
#include <kernel/memory/virtual_memory.h>
#include <kernel/memory/memory_allocators.h>
#include <kernel/software/ramdisk.h>
#include <kernel/software/syscall.h>

/* local variables */
syslib_t syslib;
uint8_t close_thread_function[] = {
    0xB8, 0x06, 0x00, 0x00, 0x00, // mov eax, 6
    0xCD, 0xD0                    // int 0xD0
};

/* functions */
void initialize_syslib(void) {
    syslib.version = 0x20260311;

    syslib.initialize = get_elf_symbol_addr(get_ramdisk_file_ptr("libraries.elf"), "syslib_initialize");

    syslib.log = get_elf_symbol_addr(get_ramdisk_file_ptr("libraries.elf"), "syslib_log");
    syslib.logf = get_elf_symbol_addr(get_ramdisk_file_ptr("libraries.elf"), "syslib_logf");

    syslib.get_time_in_microseconds = get_elf_symbol_addr(get_ramdisk_file_ptr("libraries.elf"), "syslib_get_time_in_microseconds");

    syslib.malloc = get_elf_symbol_addr(get_ramdisk_file_ptr("libraries.elf"), "syslib_malloc");
    syslib.calloc = get_elf_symbol_addr(get_ramdisk_file_ptr("libraries.elf"), "syslib_calloc");
    syslib.realloc = get_elf_symbol_addr(get_ramdisk_file_ptr("libraries.elf"), "syslib_realloc");
    syslib.free = get_elf_symbol_addr(get_ramdisk_file_ptr("libraries.elf"), "syslib_free");

    syslib.create_window = get_elf_symbol_addr(get_ramdisk_file_ptr("libraries.elf"), "syslib_create_window");
    syslib.redraw_window = get_elf_symbol_addr(get_ramdisk_file_ptr("libraries.elf"), "syslib_redraw_window");

    syslib.create_screen_buffer = get_elf_symbol_addr(get_ramdisk_file_ptr("libraries.elf"), "syslib_create_screen_buffer");
    syslib.destroy_screen_buffer = get_elf_symbol_addr(get_ramdisk_file_ptr("libraries.elf"), "syslib_destroy_screen_buffer");

    syslib.load_bitmap_font = get_elf_symbol_addr(get_ramdisk_file_ptr("libraries.elf"), "syslib_load_bitmap_font");
    syslib.draw_bitmap_char = get_elf_symbol_addr(get_ramdisk_file_ptr("libraries.elf"), "syslib_draw_bitmap_char");
    syslib.draw_bitmap_string = get_elf_symbol_addr(get_ramdisk_file_ptr("libraries.elf"), "syslib_draw_bitmap_string");
    syslib.destroy_bitmap_font = get_elf_symbol_addr(get_ramdisk_file_ptr("libraries.elf"), "syslib_destroy_bitmap_font");

    syslib.initialize_gui = get_elf_symbol_addr(get_ramdisk_file_ptr("libraries.elf"), "syslib_initialize_gui");
    syslib.add_canvas_component = get_elf_symbol_addr(get_ramdisk_file_ptr("libraries.elf"), "syslib_add_canvas_component");
    syslib.register_pressed_key_event_handler = get_elf_symbol_addr(get_ramdisk_file_ptr("libraries.elf"), "syslib_register_pressed_key_event_handler");
    syslib.redraw_gui = get_elf_symbol_addr(get_ramdisk_file_ptr("libraries.elf"), "syslib_redraw_gui");
}

spawning_template_t load_elf32_to_spawning_template(void *elf_data, void prepare_memory(void)) {
    spawning_template_t template = {0};

    // validation of ELF header
    Elf32_Ehdr *header = (Elf32_Ehdr *) elf_data;
    if(*(uint32_t *)header->e_ident != ELF_MAGIC) {
        log("\nInvalid ELF file");
        return template;
    }

    // create new virtual address space
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

        log("\nLoading segment: vaddr=0x%x, memsz=0x%x, filesz=0x%x, flags=0x%x", ph->p_vaddr, ph->p_memsz, ph->p_filesz, ph->p_flags);
        if(ph->p_memsz < ph->p_filesz) {
            log("\nInvalid segment: memsz < filesz");
            free_virtual_space(template.page_directory);
            template.page_directory = 0;
            unlock_core();
            return template;
        }

        // check if segment is in valid memory
        if(ph->p_vaddr < user_space_allocation_start || (ph->p_vaddr + ph->p_memsz) > user_space_allocation_end) {
            log("\nSegment is not in valid memory");
            free_virtual_space(template.page_directory);
            template.page_directory = 0;
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
        uint32_t *page_table_entry = (uint32_t *) (P_MEM_PAGE_TABLE + ((ph->p_vaddr >> 12) * 4));
        uint32_t number_of_pages = ((ph->p_memsz + PAGE_SIZE - 1) >> 12);
        for(uint32_t i = 0; i < number_of_pages; i++, page_table_entry++) {
            *page_table_entry &= ~VM_FLAG_READ_WRITE;
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
    uint32_t userspace_size = PAGE_MASK(user_space_allocation_end - highest_used_memory);
    if(userspace_size < PAGE_SIZE) {
        log("\n[ELF] Unable to load ELF file, not enough userspace memory left (0x%x - 0x%x)", highest_used_memory, user_space_allocation_end);
        free_virtual_space(template.page_directory);
        template.page_directory = 0;
        unlock_core();
        return template;
    }
    log("\n[ELF] Userspace size: 0x%x", userspace_size);

    // read entry point
    template.entry_point = header->e_entry;

    // prepare memory in new user space
    if(prepare_memory != NULL) {
        prepare_memory();
    }

    // load system libraries
    load_static_elf32_to_memory(get_ramdisk_file_ptr("libraries.elf"));

    // set up syslib page
    syslib_t *template_syslib = (syslib_t *) 0x1000;
    memcpy(template_syslib, (void *) &syslib, sizeof(syslib_t));
    template_syslib->userspace_start = highest_used_memory;
    template_syslib->userspace_size = userspace_size;

    // set up functions
    memcpy((void *) 0x2000, close_thread_function, sizeof(close_thread_function));

    // set up user stack
    uint32_t *stack_pointer = (uint32_t *) user_space_allocation_start;
    stack_pointer--;
    *stack_pointer = (uint32_t) template_syslib; // pointer to library functions structure in user space
    stack_pointer--;
    *stack_pointer = 0x2000; // pointer to function to close thread in user space
    template.user_stack = stack_pointer;

    unlock_core();
    return template;
}

void load_static_elf32_to_memory(void *elf_data) {
    Elf32_Ehdr *header = (Elf32_Ehdr *) elf_data;
    Elf32_Phdr *ph = (Elf32_Phdr *)((uint8_t *)elf_data + header->e_phoff);

    for(int i = 0; i < header->e_phnum; i++) {
        // check if segment has vaild length
        if(ph[i].p_memsz < ph[i].p_filesz) {
            log("\nInvalid segment: memsz < filesz");
            return;
        }

        // load segment to memory
        if(ph[i].p_type == PT_LOAD) {
            log("\nLoading segment: vaddr=0x%x, memsz=0x%x, filesz=0x%x, flags=0x%x", ph[i].p_vaddr, ph[i].p_memsz, ph[i].p_filesz, ph[i].p_flags);
            memcpy((void *) ph[i].p_vaddr, (void *) ((uint8_t *)elf_data + ph[i].p_offset), ph[i].p_filesz);
            if(ph[i].p_memsz > ph[i].p_filesz) {
                memset((void *)(ph[i].p_vaddr + ph[i].p_filesz), 0, ph[i].p_memsz - ph[i].p_filesz);
            }
        }

        // update flags of pages in this segment
        uint32_t *page_table_entry = (uint32_t *) (P_MEM_PAGE_TABLE + ((ph[i].p_vaddr >> 12) * 4));
        uint32_t number_of_pages = ((ph[i].p_memsz + PAGE_SIZE - 1) >> 12);
        for(uint32_t j = 0; j < number_of_pages; j++, page_table_entry++) {
            *page_table_entry &= ~VM_FLAG_READ_WRITE;
            if(ph[i].p_flags & PF_W) {
                *page_table_entry = (*page_table_entry & ~VM_FLAGS_TYPE) | VM_COW_ALLOCATION;
            }
            else {
                *page_table_entry = (*page_table_entry & ~VM_FLAGS_TYPE) | VM_SPAWN_TEMPLATE;
            }
        }
    }
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