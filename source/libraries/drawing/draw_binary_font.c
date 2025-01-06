//BleskOS

/*
* MIT License
* Copyright (c) 2023-2025 Vendelín Slezák
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

void draw_char(word_t char_val, dword_t x, dword_t y, dword_t color) {
 byte_t *char_data_ptr = (byte_t *) (binary_font_memory + (char_val*8)); //char position
 dword_t first_line_pixel_pointer = ((dword_t)screen_double_buffer_memory_pointer + (y*screen_double_buffer_bytes_per_line) + (x<<2));
 dword_t *screen = (dword_t *) first_line_pixel_pointer;

 for(int i=0; i<8; i++) {
  for(int j=0, mask=(1<<7); j<8; j++, mask>>=1) {
   if((*char_data_ptr & mask)==mask) {
    *screen = color;
   }
   screen++;
  }
  
  first_line_pixel_pointer += screen_double_buffer_bytes_per_line;
  screen = (dword_t *) first_line_pixel_pointer;
  char_data_ptr++;
 }
 
}

void draw_part_of_char(word_t char_val, dword_t x, dword_t y, dword_t first_char_column, dword_t last_char_column, dword_t first_char_line, dword_t last_char_line, dword_t color) {
 byte_t *char_data_ptr = (byte_t *) (binary_font_memory + (char_val*8) + first_char_line); //char position
 dword_t first_line_pixel_pointer = ((dword_t)screen_double_buffer_memory_pointer + (y*screen_double_buffer_bytes_per_line) + (x<<2));
 dword_t *screen = (dword_t *) first_line_pixel_pointer;

 for(int i=first_char_line; i<last_char_line; i++) {
  for(int j=first_char_column, mask=(1<<(7-first_char_column)); j<last_char_column; j++, mask>>=1) {
   if((*char_data_ptr & mask)==mask) {
    *screen = color;
   }
   screen++;
  }
  
  first_line_pixel_pointer += screen_double_buffer_bytes_per_line;
  screen = (dword_t *) first_line_pixel_pointer;
  char_data_ptr++;
 }
 
}

void print(byte_t *string, dword_t x, dword_t y, dword_t color) {
 word_t unicode_char=0, original_x = x;

 while(*string!=0 && x<screen_width) {
  unicode_char = (word_t) *string;
  if(unicode_char=='\n') {
   y += 10;
   x = original_x;
   string++;
  }
  else if(unicode_char<0x80) {
   draw_char(unicode_char, x, y, color);
   string++;
   x += 8;
  }
  else if((unicode_char & 0xE0) == 0xC0){
   unicode_char = (((unicode_char & 0x1F) << 6) | (string[1] & 0x3F));
   draw_char(unicode_char, x, y, color);
   string+=2;
   x += 8;
  }
  else {
   string++;
  }
 }
}

void print_ascii(byte_t *string, dword_t x, dword_t y, dword_t color) {
 while(*string!=0 && x<screen_width) {
  draw_char((word_t) *string, x, y, color);
  x += 8;
  string++;
 }
}

void print_unicode(word_t *string, dword_t x, dword_t y, dword_t color) {
 while(*string!=0 && x<screen_width) {
  draw_char(*string, x, y, color);
  x += 8;
  string++;
 }
}

void print_var(dword_t value, dword_t x, dword_t y, dword_t color) {
 word_t number_string[11];
 dword_t number_digits = 0;
 
 for(int i=0; i<11; i++) {
  number_string[i]=0;
 }
 
 for(int i=9; i>0; i--) {
  number_string[i]=(value%10);
  value/=10;
  number_digits++;
  if(value==0) {
   break;
  }
 }
 
 for(int i=0; i<10; i++) {
  number_string[i]+='0';
 }
 
 print_unicode((word_t *) (number_string+(10-number_digits)), x, y, color);
}

void print_hex(dword_t value, dword_t x, dword_t y, dword_t color) {
 draw_char('0', x, y, color);
 x+=8;
 draw_char('x', x, y, color);
 x+=8;
 
 for(int i=0, digit=0, shift=28; i<8; i++, shift-=4) {
  digit = ((value>>shift) & 0xF);
  draw_char(((digit<10) ? (digit+'0') : (digit+'A'-10)), x, y, color);
  x+=8;
 }
}

void pstr(byte_t *string) {
 print(string, 0, debug_line, 0x888888);
 debug_line+=10;
 debug_line%=screen_height;
 redraw_screen();
}

void pvar(dword_t value) {
 print_var(value, 0, debug_line, 0x888888);
 debug_line+=10;
 debug_line%=screen_height;
 redraw_screen();
}

void phex(dword_t value) {
 print_hex(value, 0, debug_line, 0x888888);
 debug_line+=10;
 debug_line%=screen_height;
 redraw_screen();
}
