//BleskOS

/*
* MIT License
* Copyright (c) 2023 Vendelín Slezák
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

void draw_clear_window(byte_t *up_string, byte_t *down_string, dword_t background_color, dword_t border_color) {
 clear_screen(background_color);
 draw_full_square(0, 0, graphic_screen_x, 20, border_color);
 draw_full_square(0, graphic_screen_y-20, graphic_screen_x, 20, border_color);
 print(up_string, 10, 5, BLACK);
 print(down_string, 10, graphic_screen_y-14, BLACK);
}

void draw_message_window(dword_t width, dword_t heigth) {
 draw_full_square(graphic_screen_x_center-(width/2), graphic_screen_y_center-(heigth/2), width, heigth, 0xFF7000);
 set_pen_width(1);
 draw_empty_square(graphic_screen_x_center-(width/2), graphic_screen_y_center-(heigth/2), width, heigth, BLACK);
}

void redraw_message_window(dword_t width, dword_t heigth) {
 redraw_part_of_screen(graphic_screen_x_center-(width/2), graphic_screen_y_center-(heigth/2), width, heigth);
}

void print_to_message_window(byte_t *string, dword_t line) {
 for(int i=0, width=0; i<1000; i++) {
  if(string[i]==0) {
   print(string, graphic_screen_x_center-(width/2), line, BLACK);
   return;
  }
  width += 8;
 }
}

void message_window(byte_t *message) {
 //count chars
 dword_t chars = 0;
 for(int i=0; i<1000; i++) {
  if(message[i]==0) {
   break;
  }
  chars++;
 }
 
 draw_message_window(chars*8+16, 30);
 print(message, graphic_screen_x_center-chars*4, graphic_screen_y_center-4, BLACK);
}

void show_message_window(byte_t *message) {
 message_window(message);
 redraw_message_window(get_number_of_chars_in_ascii_string(message)*8+16, 30);
}

void show_system_message(byte_t *string) {
 system_message_background_mem = malloc(300*30*4);

 copy_raw_image_data(screen_mem, graphic_screen_x, 10, 10, 300, 30, system_message_background_mem, 300, 0, 0);
 draw_full_square(10, 10, 300, 30, WHITE);
 draw_empty_square(10, 10, 300, 30, BLACK);
 print(string, 20, 20, BLACK);
 redraw_part_of_screen(10, 10, 300, 30);
}

void remove_system_message(void) {
 copy_raw_image_data(system_message_background_mem, 300, 0, 0, 300, 30, screen_mem, graphic_screen_x, 10, 10);
 redraw_part_of_screen(10, 10, 300, 30);

 free(system_message_background_mem);
}