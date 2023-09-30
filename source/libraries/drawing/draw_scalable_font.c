//BleskOS

/*
* MIT License
* Copyright (c) 2023 Vendelín Slezák
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

void initalize_scalable_font(void) {
 scalable_font_pixel_distance_mem = malloc(256*2);
 scalable_font_pixel_distance = (word_t *) scalable_font_pixel_distance_mem;
 scalable_font_char_board_mem = malloc(256*256*4);
 scalable_font_char_mem = malloc(10*10*4);
 set_scalable_char_size(10);
 scalable_font_char_emphasis = SF_EMPHASIS_NONE;

 //predraw scalable font in size 10 for speeding up performance of drawing
 scalable_font_predraw_size_10_mem = malloc(11*20*4*128);

 //clear space
 dword_t *predraw8 = (dword_t *) (scalable_font_predraw_size_10_mem);
 for(int i=0; i<(11*20*128); i++) {
  *predraw8 = TRANSPARENT_COLOR;
  predraw8++;
 }

 //draw chars
 screen_save_variabiles();
 graphic_screen_x = 11;
 graphic_screen_y = 20*128;
 screen_bytes_per_line = 11*4;
 screen_mem = scalable_font_predraw_size_10_mem;
 scalable_font_char_size = 0; //draw_scalable_char_without_emphasis will not try to draw character from predrawed memory
 for(int i=32; i<128; i++) {
  draw_scalable_char_without_emphasis(i, 0, i*20, BLACK);
 }
 scalable_font_char_size = 10;
 screen_restore_variables();
}

void set_scalable_char_size(dword_t size) {
 scalable_font_char_size = size;
 
 free(scalable_font_char_mem);
 scalable_font_char_mem = calloc(scalable_font_char_size*scalable_font_char_size*4);
 
 for(int i=0; i<256; i++) {
  scalable_font_pixel_distance[i] = (i*size/100);
 }
}

void draw_scalable_char_without_emphasis(word_t char_val, dword_t x, dword_t y, dword_t color) {
 extern dword_t bleskos_scalable_font[512];
 
 if(char_val>512) {
  char_val = 0;
 }
 
 if(bleskos_scalable_font[char_val]==0) {
  return;
 }

 if(scalable_font_char_size==10 && char_val<128) {
  //we can draw char from predrawed memory
  copy_raw_image_data(scalable_font_predraw_size_10_mem, 11, 0, char_val*20, 11, 15, screen_mem, graphic_screen_x, x, y);
  return;
 }
 
 byte_t *char_data = (byte_t *) ((dword_t)bleskos_scalable_font[char_val]);
 
 for(int i=0; i<1000; i++) {
  if(*char_data==SF_CHAR_LINE) {
   draw_line(x+scalable_font_pixel_distance[char_data[1]], y+scalable_font_pixel_distance[char_data[2]], x+scalable_font_pixel_distance[char_data[3]], y+scalable_font_pixel_distance[char_data[4]], color);
   char_data += 5;
  }
  else if(*char_data==SF_CHAR_BEZIER) {
   draw_quadratic_bezier(x+scalable_font_pixel_distance[char_data[1]], y+scalable_font_pixel_distance[char_data[2]], x+scalable_font_pixel_distance[char_data[3]], y+scalable_font_pixel_distance[char_data[4]], x+scalable_font_pixel_distance[char_data[5]], y+scalable_font_pixel_distance[char_data[6]], color);
   char_data += 7;
  }
  else if(*char_data==SF_CHAR_FULL_ELLIPSE) {
   draw_full_ellipse(x+scalable_font_pixel_distance[char_data[1]], y+scalable_font_pixel_distance[char_data[2]], x+scalable_font_pixel_distance[char_data[1]]+scalable_font_pixel_distance[char_data[3]], y+scalable_font_pixel_distance[char_data[2]]+scalable_font_pixel_distance[char_data[4]], color);
   char_data += 5;
  }
  else if(*char_data==SF_CHAR_EMPTY_ELLIPSE) {
   draw_full_ellipse(x+scalable_font_pixel_distance[char_data[1]], y+scalable_font_pixel_distance[char_data[2]], x+scalable_font_pixel_distance[char_data[1]]+scalable_font_pixel_distance[char_data[3]], y+scalable_font_pixel_distance[char_data[2]]+scalable_font_pixel_distance[char_data[4]], color);
   char_data += 5;
  }
  else if(*char_data==SF_CHAR_JUMP) {
   dword_t *char_data32 = (dword_t *) ((dword_t)char_data+1);
   char_data = (byte_t *) (*char_data32);
  }
  else { //SF_CHAR_END or unknown command
   break;
  }
 }
}

void draw_scalable_char(word_t char_val, dword_t x, dword_t y, dword_t color) {
 draw_scalable_char_without_emphasis(char_val, x, y, color);
 if((scalable_font_char_emphasis & SF_EMPHASIS_BOLD)==SF_EMPHASIS_BOLD) {
  draw_scalable_char_without_emphasis(char_val, x+1, y, color);
 }
 if((scalable_font_char_emphasis & SF_EMPHASIS_UNDERLINE)==SF_EMPHASIS_UNDERLINE) {
  draw_line(x, y+scalable_font_pixel_distance[125], x+scalable_font_char_size, y+scalable_font_pixel_distance[125], color);
 }
 if((scalable_font_char_emphasis & SF_EMPHASIS_STRIKE)==SF_EMPHASIS_STRIKE) {
  draw_line(x, y+scalable_font_pixel_distance[60], x+scalable_font_char_size, y+scalable_font_pixel_distance[60], color);
 }
}

void draw_part_of_scalable_char(word_t char_val, dword_t x, dword_t y, dword_t char_column, dword_t char_line, dword_t char_width, dword_t char_heigth, dword_t color) {
 dword_t *char_board = (dword_t *) scalable_font_char_board_mem;

 screen_save_variabiles();
 for(int i=0; i<(scalable_font_char_size+scalable_font_char_size/2)*scalable_font_char_size; i++) {
  *char_board = TRANSPARENT_COLOR;
  char_board++;
 }
 
 screen_mem = scalable_font_char_board_mem;
 graphic_screen_x = scalable_font_char_size;
 graphic_screen_y = scalable_font_char_size+scalable_font_char_size/2;
 screen_bytes_per_line = scalable_font_char_size*4;
 
 draw_scalable_char(char_val, 0, 0, color);
 
 screen_restore_variables();

 copy_raw_image_data(scalable_font_char_board_mem, scalable_font_char_size, char_column, char_line, char_width, char_heigth, screen_mem, graphic_screen_x, x, y);
}

void scalable_font_print(byte_t *string, dword_t x, dword_t y, dword_t color) { 
 scalable_font_x = x;
 scalable_font_y = y;
 while(*string!=0) {
  if(*string<128) {
   draw_scalable_char(*string, scalable_font_x, scalable_font_y, color);
   string++;
  }
  else {
   draw_scalable_char(((string[1] & 0x3F) | ((string[0] & 0x1F)<<6)), scalable_font_x, scalable_font_y, color);
   string+=2;
  }
  scalable_font_x += scalable_font_char_size;
 }
}