//BleskOS

/*
* MIT License
* Copyright (c) 2023-2025 BleskOS developers
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

void draw_starting_screen_background(void) {
    clear_screen(0x00C000);

    set_scalable_char_size(64);
    scalable_font_print("BleskOS", monitors[0].x_center-(64*7/2), monitors[0].y_center-92, BLACK);
    print_to_message_window("Version 2025 update 8", monitors[0].y_center);

    draw_empty_square(monitors[0].x_center-161, monitors[0].y_center+30, 322, 15, BLACK);

    number_of_start_screen_messages = 0;
    
    (*redraw_framebuffer)();
}

void bleskos_show_message_on_starting_screen(char *string) {
    if((boot_options & BOOT_OPTION_DEEP_DEBUGGER) == BOOT_OPTION_DEEP_DEBUGGER) {
        vga_text_mode_clear_screen(0x20);
        vga_text_mode_print(1, 1, string);
        return;
    }

    if((boot_options & BOOT_OPTION_DEBUG_MESSAGES) != 0) {
        return;
    }

    draw_full_square(0, monitors[0].y_center+65, monitors[0].width, 8, 0x00C000);
    print_to_message_window(string, monitors[0].y_center+65);
    number_of_start_screen_messages++;
    draw_full_square(monitors[0].x_center-160, monitors[0].y_center+31, (320*number_of_start_screen_messages/BLESKOS_NUMBER_OF_START_SCREEN_MESSAGES), 13, 0x0900FF);
    redraw_part_of_screen(0, monitors[0].y_center+31, monitors[0].width, 42);
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