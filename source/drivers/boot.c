//BleskOS

/*
* MIT License
* Copyright (c) 2023-2025 BleskOS developers
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

void boot_fundamental_interface(void) {
    // RAM memory
    create_physical_memory_allocator();
    components = (struct components_info_t *) calloc(sizeof(struct components_info_t));
    read_ram_info();

    // initalize logging
    detect_e9_debug_device();
    initalize_logging();
    log_ram_info();

 /* this code is not rewritten yet */
    // CPU
 initalize_mtrr();

    // ACPI
 read_acpi_tables();

    // CMOS
 read_time_format();

    // set interrupts
    initalize_interrupts();

    // timers
 set_pit();
 initalize_hpet();

    // scheduler
 initalize_scheduler();
}

void boot_devices(void) {
 /* this code is not rewritten yet */

    /* DETECT ALL DEVICES */
 scan_pci();

 // wait for any changes made during PCI initalization to take place
 wait(200);

    /* INITALIZE DEVICES */
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