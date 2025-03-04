//BleskOS

/*
* MIT License
* Copyright (c) 2023-2025 BleskOS developers
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

void redraw_text_mode_screen(void) {
 if(text_mode_show_changes_on_screen == STATUS_FALSE) {
  return;
 }
 
 dword_t x = text_mode_x_on_screen;
 dword_t y = text_mode_y_on_screen;

 for(dword_t line=0; line<text_mode_screen_lines; line++) {
  for(dword_t column=0; column<text_mode_screen_columns; column++) {
   draw_full_square(x, y, 8, 10, text_mode_screen[line*text_mode_screen_columns+column].background_color);
   draw_char(text_mode_screen[line*text_mode_screen_columns+column].character, x, y+2, text_mode_screen[line*text_mode_screen_columns+column].character_color);
   if(text_mode_cursor_is_visible==STATUS_TRUE && line==text_mode_cursor_line && column==text_mode_cursor_column) {
    draw_full_square(x, y+8, 8, 2, text_mode_screen[line*text_mode_screen_columns+column].character_color);
   }
   x += 8;
  }
  x = text_mode_x_on_screen;
  y += 10;
 }

 (*redraw_part_of_framebuffer)(text_mode_x_on_screen, text_mode_y_on_screen, text_mode_screen_columns*8,text_mode_screen_lines*10);
}

void redraw_text_mode_pixel(dword_t column, dword_t line) {
 if(text_mode_show_changes_on_screen == STATUS_FALSE) {
  return;
 }
 
 draw_full_square(text_mode_x_on_screen+column*8, text_mode_y_on_screen+line*10, 8, 10, text_mode_screen[line*text_mode_screen_columns+column].background_color);
 draw_char(text_mode_screen[line*text_mode_screen_columns+column].character, text_mode_x_on_screen+column*8, text_mode_y_on_screen+line*10+2, text_mode_screen[line*text_mode_screen_columns+column].character_color);
 if(text_mode_cursor_is_visible==STATUS_TRUE && line==text_mode_cursor_line && column==text_mode_cursor_column) {
  draw_full_square(text_mode_x_on_screen+column*8, text_mode_y_on_screen+line*10+8, 8, 2, text_mode_screen[line*text_mode_screen_columns+column].character_color);
 }
 (*redraw_part_of_framebuffer)(text_mode_x_on_screen+column*8, text_mode_y_on_screen+line*10, 8, 10);
}