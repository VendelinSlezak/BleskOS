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
#include <kernel/software/elf_loader.h>
#include <kernel/software/ramdisk.h>

/* global variables */
syslib_t system_syslib;

/* functions */
void initialize_syslib(void) {
    system_syslib.version = 0x20260311;

    system_syslib.initialize = get_elf_symbol_addr(get_ramdisk_file_ptr("userspace_library.elf"), "syslib_initialize");

    system_syslib.log = get_elf_symbol_addr(get_ramdisk_file_ptr("userspace_library.elf"), "syslib_log");
    system_syslib.logf = get_elf_symbol_addr(get_ramdisk_file_ptr("userspace_library.elf"), "syslib_logf");

    system_syslib.get_time_in_microseconds = get_elf_symbol_addr(get_ramdisk_file_ptr("userspace_library.elf"), "syslib_get_time_in_microseconds");

    system_syslib.malloc = get_elf_symbol_addr(get_ramdisk_file_ptr("userspace_library.elf"), "syslib_malloc");
    system_syslib.calloc = get_elf_symbol_addr(get_ramdisk_file_ptr("userspace_library.elf"), "syslib_calloc");
    system_syslib.realloc = get_elf_symbol_addr(get_ramdisk_file_ptr("userspace_library.elf"), "syslib_realloc");
    system_syslib.free = get_elf_symbol_addr(get_ramdisk_file_ptr("userspace_library.elf"), "syslib_free");

    system_syslib.create_window = get_elf_symbol_addr(get_ramdisk_file_ptr("userspace_library.elf"), "syslib_create_window");
    system_syslib.redraw_window = get_elf_symbol_addr(get_ramdisk_file_ptr("userspace_library.elf"), "syslib_redraw_window");

    system_syslib.create_screen_buffer = get_elf_symbol_addr(get_ramdisk_file_ptr("userspace_library.elf"), "syslib_create_screen_buffer");
    system_syslib.destroy_screen_buffer = get_elf_symbol_addr(get_ramdisk_file_ptr("userspace_library.elf"), "syslib_destroy_screen_buffer");

    system_syslib.load_bitmap_font = get_elf_symbol_addr(get_ramdisk_file_ptr("userspace_library.elf"), "syslib_load_bitmap_font");
    system_syslib.draw_bitmap_char = get_elf_symbol_addr(get_ramdisk_file_ptr("userspace_library.elf"), "syslib_draw_bitmap_char");
    system_syslib.draw_bitmap_string = get_elf_symbol_addr(get_ramdisk_file_ptr("userspace_library.elf"), "syslib_draw_bitmap_string");
    system_syslib.destroy_bitmap_font = get_elf_symbol_addr(get_ramdisk_file_ptr("userspace_library.elf"), "syslib_destroy_bitmap_font");

    system_syslib.initialize_gui = get_elf_symbol_addr(get_ramdisk_file_ptr("userspace_library.elf"), "syslib_initialize_gui");
    system_syslib.add_canvas_component = get_elf_symbol_addr(get_ramdisk_file_ptr("userspace_library.elf"), "syslib_add_canvas_component");
    system_syslib.register_pressed_key_event_handler = get_elf_symbol_addr(get_ramdisk_file_ptr("userspace_library.elf"), "syslib_register_pressed_key_event_handler");
    system_syslib.redraw_gui = get_elf_symbol_addr(get_ramdisk_file_ptr("userspace_library.elf"), "syslib_redraw_gui");
}