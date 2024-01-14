//BleskOS

/*
* MIT License
* Copyright (c) 2023-2024 Vendelín Slezák
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

void initalize_performance_rating(void) {
}

void performance_rating(void) {
 redraw_performance_rating();

 while(1) {
  wait_for_user_input();
  move_mouse_cursor();

  dword_t click_value = get_mouse_cursor_click_board_value();

  //back
  if(keyboard_value==KEY_ESC) {
   return;
  }
 }
}

void redraw_performance_rating(void) {
 clear_screen(0xbb00dd);
 print("Test for Performance Rating", 20, 20, BLACK);
 draw_button_with_click_zone("[esc] Back", 20, graphic_screen_y-40, 200, 20, PERFORMANCE_CLICK_ZONE_BACK);
 redraw_screen();
}
