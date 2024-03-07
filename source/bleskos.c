//BleskOS

/*
* MIT License
* Copyright (c) 2023-2024 Vendelín Slezák
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include "bleskos.h"
#include "drivers/system/include.h"
#include "drivers/other/include.h"
#include "drivers/graphic/include.h"
#include "drivers/ps2/include.h"
#include "drivers/keyboard/keyboard.h"
#include "drivers/mouse/mouse.h"
#include "drivers/ide/include.h"
#include "drivers/ahci/include.h"
#include "drivers/filesystems/include.h"
#include "drivers/sound/include.h"
#include "drivers/ethernet/include.h"
#include "drivers/network/include.h"
#include "drivers/usb/include.h"
#include "libraries/basic/include.h"
#include "libraries/logging/logging.h"
#include "libraries/drawing/include.h"
#include "libraries/graphic_user_interface_elements/include.h"
#include "libraries/file_dialog/include.h"
#include "libraries/decoders/include.h"
#include "libraries/checksums/include.h"
#include "libraries/text_formats/include.h"
#include "libraries/graphic_formats/include.h"
#include "libraries/sound_formats/include.h"
#include "libraries/archive_formats/include.h"
#include "libraries/program_interface/include.h"
#include "programs/main_window/main_window.h"
#include "programs/system_board/system_board.h"
#include "programs/text_editor/text_editor.h"
#include "programs/graphic_editor/graphic_editor.h"
#include "programs/media_viewer/media_viewer.h"
#include "programs/internet_browser/internet_browser.h"
#include "programs/file_manager/file_manager.h"
#include "programs/document_editor/document_editor.h"
#include "programs/screenshooter/screenshooter.h"
#include "programs/performance_rating/performance_rating.h"
#include "programs/calculator/calculator.h"

#include "drivers/system/include.c"
#include "drivers/other/include.c"
#include "drivers/graphic/include.c"
#include "drivers/ps2/include.c"
#include "drivers/keyboard/keyboard.c"
#include "drivers/mouse/mouse.c"
#include "drivers/ide/include.c"
#include "drivers/ahci/include.c"
#include "drivers/filesystems/include.c"
#include "drivers/sound/include.c"
#include "drivers/ethernet/include.c"
#include "drivers/network/include.c"
#include "drivers/usb/include.c"
#include "libraries/basic/include.c"
#include "libraries/logging/logging.c"
#include "libraries/drawing/include.c"
#include "libraries/graphic_user_interface_elements/include.c"
#include "libraries/file_dialog/include.c"
#include "libraries/decoders/include.c"
#include "libraries/checksums/include.c"
#include "libraries/text_formats/include.c"
#include "libraries/graphic_formats/include.c"
#include "libraries/sound_formats/include.c"
#include "libraries/archive_formats/include.c"
#include "libraries/program_interface/include.c"
#include "programs/main_window/main_window.c"
#include "programs/system_board/system_board.c"
#include "programs/text_editor/text_editor.c"
#include "programs/graphic_editor/graphic_editor.c"
#include "programs/media_viewer/media_viewer.c"
#include "programs/internet_browser/internet_browser.c"
#include "programs/file_manager/file_manager.c"
#include "programs/document_editor/document_editor.c"
#include "programs/screenshooter/screenshooter.c"
#include "programs/performance_rating/performance_rating.c"
#include "programs/calculator/calculator.c"

void bleskos(dword_t bootloader_passed_value) {
 boot_options = bootloader_passed_value;

 bleskos_boot_debug_top_screen_color(0xFF0000); //red top of screen
 initalize_memory();

 bleskos_boot_debug_top_screen_color(0x00FF00); //green top of screen
 initalize_logging();
 bleskos_boot_debug_top_screen_color(0x0000FF); //blue top of screen
 log("BleskOS 2024 update 27\n\nPress F2 to save System log as TXT file");
 log_starting_memory();

 bleskos_boot_debug_top_screen_color(0xFFFF00); //yellow top of screen
 scan_pci();
 set_interrupts();
 set_pit();
 bleskos_boot_debug_top_screen_color(0xFF00FF); //pink top of screen

 initalize_graphic();
 mouse_cursor_x = 0;
 mouse_cursor_y = 0;
 clear_screen(0x00C000);
 set_scalable_char_size(64);
 scalable_font_print("BleskOS", screen_x_center-(64*7/2), screen_y_center-92, BLACK);
 print_to_message_window("Version 2024 update 27", screen_y_center);
 draw_empty_square(screen_x_center-161, screen_y_center+30, 322, 15, BLACK);
 number_of_start_screen_messages = 0;
 (*redraw_framebuffer)();

 bleskos_show_message_on_starting_screen("Reading time format...");
 read_time_format();
 bleskos_show_message_on_starting_screen("Reading ACPI tables...");
 read_acpi_tables();
 initalize_hpet();
 bleskos_boot_debug_log_message();
 
 bleskos_show_message_on_starting_screen("Initalizing PS/2 controller...");
 initalize_ps2_controller();
 initalize_ps2_keyboard();
 initalize_ps2_mouse();
 bleskos_show_message_on_starting_screen("Initalizing keyboard...");
 initalize_keyboard();
 bleskos_show_message_on_starting_screen("Initalizing mouse...");
 initalize_mouse();
 bleskos_boot_debug_log_message();

 hard_disk_size = 0;
 bleskos_show_message_on_starting_screen("Initalizing AHCI controllers...");
 initalize_ahci_controllers();
 bleskos_boot_debug_log_message();
 bleskos_show_message_on_starting_screen("Initalizing IDE controllers...");
 initalize_ide_controllers();
 bleskos_boot_debug_log_message();
 initalize_device_list();
 bleskos_boot_debug_log_message();
 
 bleskos_show_message_on_starting_screen("Initalizing sound card...");
 initalize_sound_card();
 bleskos_boot_debug_log_message();
 
 bleskos_show_message_on_starting_screen("Initalizing ethernet card...");
 initalize_network_cards();
 bleskos_boot_debug_log_message();
 initalize_network_stack();
 read_ethernet_cable_status();
 if(is_ethernet_cable_connected==STATUS_TRUE) {
  bleskos_show_message_on_starting_screen("Connecting to network...");
  connect_to_network();
  bleskos_boot_debug_log_message();
 }

 bleskos_show_message_on_starting_screen("Initalizing USB controllers...");
 initalize_usb_controllers();
 bleskos_boot_debug_log_message();
 
 bleskos_show_message_on_starting_screen("Initalizing libraries...");
 initalize_program_interface();
 initalize_click_board();
 initalize_image_operations();
 initalize_file_dialog();
 initalize_lzw();
 initalize_deflate();

 bleskos_show_message_on_starting_screen("Initalizing programs...");
 initalize_document_editor();
 initalize_text_editor();
 initalize_graphic_editor();
 initalize_media_viewer();
 initalize_internet_browser();
 initalize_file_manager();
 initalize_calculator();
 initalize_screenshooter();
 initalize_performance_rating();
 
 bleskos_show_message_on_starting_screen("Starting Graphic User Interface...");
 log("\nEND OF BOOTING\n");

 mouse_cursor_x = screen_x_center;
 mouse_cursor_y = screen_y_center;
 bleskos_main_window();
}

void bleskos_show_message_on_starting_screen(char *string) {
 if((boot_options & BOOT_OPTION_DEBUG_MESSAGES)==BOOT_OPTION_DEBUG_MESSAGES) {
  return;
 }

 draw_full_square(0, screen_y_center+65, screen_width, 8, 0x00C000);
 print_to_message_window(string, screen_y_center+65);
 number_of_start_screen_messages++;
 draw_full_square(screen_x_center-160, screen_y_center+31, (320*number_of_start_screen_messages/BLESKOS_NUMBER_OF_START_SCREEN_MESSAGES), 13, 0x0900FF);
 redraw_part_of_screen(0, screen_y_center+31, screen_width, 42);
}

void bleskos_boot_debug_top_screen_color(dword_t color) {
 if((boot_options & BOOT_OPTION_DEBUG_MESSAGES)==BOOT_OPTION_DEBUG_MESSAGES) {
  dword_t *vesa_lfb_pointer = (dword_t *) (0x3828);
  dword_t *monitor = (dword_t *) (*vesa_lfb_pointer);
  for(dword_t i=0; i<10000; i++) {
   monitor[i] = color;
  }
 }
}

void bleskos_boot_debug_log_message(void) {
 if((boot_options & BOOT_OPTION_DEBUG_MESSAGES)==BOOT_OPTION_DEBUG_MESSAGES) {
  show_log();
  wait(4000);
  skip_logs();
 }
}