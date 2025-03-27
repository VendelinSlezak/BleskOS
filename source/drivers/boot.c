//BleskOS

/*
* MIT License
* Copyright (c) 2023-2025 BleskOS developers
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

void boot_drivers(void) {
    /* PREPARE FUNDAMENTAL INTERFACE */

    // RAM memory
    create_physical_memory_allocator();
    components = (struct components_info_t *) calloc(sizeof(struct components_info_t));
    read_ram_info();

    // initalize logging
    check_presence_of_e9_debug_device();
    initalize_logging();
    log_ram_info();

    // CPU
    read_cpu_info();
    initalize_mtrr();
    // TODO: initalize_fpu();

    // Extended BIOS Data Area
    read_extended_bios_data_area();

    // ACPI
    read_acpi_tables();

    // CMOS
    read_cmos_data();

    // initalize IDT and PIC
    initalize_interrupts();

    // start timers
    initalize_timers();

    // scheduler
    initalize_scheduler();

    /* DETECT DEVICES */

    // detect devices connected to PCI bus
    scan_pci();

    // detect devices connected to fixed ports
    check_presence_of_bga();

    /* INITALIZE DEVICES */
 /* this code is not rewritten yet */
 // draw starting screen
 if((boot_options & BOOT_OPTION_DEEP_DEBUGGER) != BOOT_OPTION_DEEP_DEBUGGER) {
    initalize_graphic();
    draw_starting_screen_background();
 }

 bleskos_show_message_on_starting_screen("Initalizing keyboard...");
 initalize_keyboard();
 bleskos_show_message_on_starting_screen("Initalizing mouse...");
 initalize_mouse();
 
 bleskos_show_message_on_starting_screen("Initalizing PS/2 controller...");
 initalize_ps2_controller();
 initalize_ps2_keyboard();
 initalize_ps2_mouse();
 bleskos_boot_debug_log_message();

 bleskos_show_message_on_starting_screen("Initalizing storage controllers...");
 initalize_list_of_connected_partitions();
 initalize_drivers_of_filesystems();
 initalize_storage_controllers();
 bleskos_boot_debug_log_message();
 
 bleskos_show_message_on_starting_screen("Initalizing sound card...");
 initalize_sound_card();
 bleskos_boot_debug_log_message();

 bleskos_show_message_on_starting_screen("Initalizing network...");
 initalize_network_connection();

 bleskos_show_message_on_starting_screen("Initalizing USB controllers...");
 initalize_usb_controllers();
 bleskos_boot_debug_log_message();
}