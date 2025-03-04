//BleskOS

/*
* MIT License
* Copyright (c) 2023-2025 BleskOS developers
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

// #define NO_PROGRAMS

#include "bleskos.h"
#include "starting_screen.h"
#include "drivers/system/include.h"
#include "drivers/graphic/include.h"
#include "drivers/ps2/include.h"
#include "drivers/keyboard/keyboard.h"
#include "drivers/mouse/mouse.h"
#include "drivers/ide/include.h"
#include "drivers/ahci/include.h"
#include "drivers/storage/include.h"
#include "drivers/filesystems/include.h"
#include "drivers/sound/include.h"
#include "drivers/network/include.h"
#include "drivers/usb/include.h"
#include "drivers/components.h"
#include "drivers/boot.h"

#include "libraries/basic/include.h"
#include "libraries/logging/logging.h"
#include "libraries/drawing/include.h"
#include "libraries/graphic_user_interface_elements/include.h"
#include "libraries/decoders/include.h"
#include "libraries/checksums/include.h"
#include "libraries/text_formats/include.h"
#include "libraries/graphic_formats/include.h"
#include "libraries/sound_formats/include.h"
#include "libraries/archive_formats/include.h"
#include "libraries/program_interface/include.h"
#include "libraries/file_dialog/include.h"
#include "libraries/binary_programs_interface/include.h"

#ifndef NO_PROGRAMS
#include "programs/main_window/main_window.h"
#include "programs/system_board/system_board.h"
#include "programs/text_editor/text_editor.h"
#include "programs/graphic_editor/graphic_editor.h"
#include "programs/media_viewer/media_viewer.h"
#include "programs/internet_browser/internet_browser.h"
#include "programs/file_manager/file_manager.h"
#include "programs/document_editor/document_editor.h"
#include "programs/screenshooter/screenshooter.h"
#include "programs/calculator/calculator.h"
#endif
#include "programs/performance_rating/performance_rating.h"

#include "starting_screen.c"
#include "drivers/system/include.c"
#include "drivers/graphic/include.c"
#include "drivers/ps2/include.c"
#include "drivers/keyboard/keyboard.c"
#include "drivers/mouse/mouse.c"
#include "drivers/ide/include.c"
#include "drivers/ahci/include.c"
#include "drivers/storage/include.c"
#include "drivers/filesystems/include.c"
#include "drivers/sound/include.c"
#include "drivers/network/include.c"
#include "drivers/usb/include.c"
#include "drivers/boot.c"

#include "libraries/basic/include.c"
#include "libraries/logging/logging.c"
#include "libraries/drawing/include.c"
#include "libraries/graphic_user_interface_elements/include.c"
#include "libraries/decoders/include.c"
#include "libraries/checksums/include.c"
#include "libraries/text_formats/include.c"
#include "libraries/graphic_formats/include.c"
#include "libraries/sound_formats/include.c"
#include "libraries/archive_formats/include.c"
#include "libraries/program_interface/include.c"
#include "libraries/file_dialog/include.c"
#include "libraries/binary_programs_interface/include.c"

#ifndef NO_PROGRAMS
#include "programs/main_window/main_window.c"
#include "programs/system_board/system_board.c"
#include "programs/text_editor/text_editor.c"
#include "programs/graphic_editor/graphic_editor.c"
#include "programs/media_viewer/media_viewer.c"
#include "programs/internet_browser/internet_browser.c"
#include "programs/file_manager/file_manager.c"
#include "programs/document_editor/document_editor.c"
#include "programs/screenshooter/screenshooter.c"
#include "programs/calculator/calculator.c"
#endif
#include "programs/performance_rating/performance_rating.c"

void bleskos(dword_t bootloader_passed_value) {
    // save boot options passed from bootloader
    boot_options = bootloader_passed_value;

    // prepare interface for BleskOS to be able to run
    boot_fundamental_interface();

    // detect and initalize all inbuild devices
    boot_devices();
 
 /* this code is not rewritten yet */
 bleskos_show_message_on_starting_screen("Initalizing libraries...");
 initalize_program_interface();
 initalize_click_board();
 initalize_image_operations();
 initalize_file_dialog();
 initalize_lzw();
 initalize_deflate();
 initalize_mp3_decoder();
 initalize_binary_programs_interface();

 if((boot_options & BOOT_OPTION_DEEP_DEBUGGER) == BOOT_OPTION_DEEP_DEBUGGER) {
  deep_debugger();
 }

 #ifndef NO_PROGRAMS
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
 log("\n\nEND OF BOOTING\n");

 mouse_cursor_x = screen_x_center;
 mouse_cursor_y = screen_y_center;
 bleskos_main_window();
 #endif
}