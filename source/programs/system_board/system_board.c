//BleskOS

/*
* MIT License
* Copyright (c) 2023-2025 BleskOS developers
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

void system_board_print_hardware_item(byte_t *string) {
 print(string, 20, system_board_draw_line, BLACK);
 system_board_draw_line += 30;
}

void system_board_redraw(void) {
 clear_screen(0xA04000);
 draw_empty_square(10+28*8, monitors[0].height-10-5-8-5, 29*8, 18, BLACK);
 print("[P] Open performance rating", 10+29*8, monitors[0].height-10-5-8, BLACK);
 draw_empty_square(monitors[0].width-22*8, monitors[0].height-10-5-8-5, 21*8, 18, BLACK);
 print("[L] Open system log", monitors[0].width-21*8, monitors[0].height-10-5-8, BLACK);
 draw_empty_square(10, monitors[0].height-10-5-8-5, 27*8, 18, BLACK);
 print("[Esc] Back to Main window", 18, monitors[0].height-10-5-8, BLACK);
}

void system_board(void) {
 system_board_selected_item = 0;
 system_item_variable = 0;

 redraw:
 system_board_redraw();
 redraw_screen();

 while(1) {
  wait_for_user_input();
  move_mouse_cursor();

  if(keyboard_code_of_pressed_key==KEY_ESC || (mouse_click_button_state==MOUSE_CLICK && is_mouse_in_zone(monitors[0].height-10-5-8-5, monitors[0].height-10, 10, 10+27*8)==STATUS_TRUE)) {
   return;
  }

  if(keyboard_code_of_pressed_key==KEY_L) {
   developer_program_log();
   goto redraw;
  }

  if(keyboard_code_of_pressed_key==KEY_P) {
   performance_rating();
   goto redraw;
  }

  if(mouse_click_button_state==MOUSE_CLICK) {
   if(is_mouse_in_zone(monitors[0].height-10-5-8-5, monitors[0].height-10, monitors[0].width-22*8, monitors[0].width-8)==STATUS_TRUE) {
    developer_program_log();
    goto redraw;
   }
   else if(is_mouse_in_zone(monitors[0].height-10-5-8-5,monitors[0].height-10, 10+28*8, 10+28*8+29*8)==STATUS_TRUE) {
    performance_rating();
    goto redraw;
   }
  }
 }
}
