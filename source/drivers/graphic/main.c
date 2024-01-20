//BleskOS

/*
* MIT License
* Copyright (c) 2023-2024 Vendelín Slezák
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

void initalize_graphic(void) {
 extern dword_t *bleskos_font;

 //set best video mode according to graphic card
 vesa_init_graphic(); //currently there are no other graphic drivers

 //check if we have graphic mode
 if(graphic_screen_lfb==0 || graphic_screen_bytes_per_line==0 || graphic_screen_x==0 || graphic_screen_y==0 || (graphic_screen_bpp!=16 && graphic_screen_bpp!=24 && graphic_screen_bpp!=32)) {
  //ERROR: we do not have valid graphic mode
  
  //if is monitor still in VGA mode, this will clear screen with red color end print "Error 1"
  word_t *vga = (word_t *) (0xB8000);
  for(dword_t i=0; i<80*25; i++) {
   vga[i]=0x4020;
  }
  vga[0]=(0x40<<8 | 'E');
  vga[1]=(0x40<<8 | 'r');
  vga[2]=(0x40<<8 | 'r');
  vga[3]=(0x40<<8 | 'o');
  vga[4]=(0x40<<8 | 'r');
  vga[6]=(0x40<<8 | '1');

  pc_speaker_beep(500); //inform user about error also with sound from PC speaker

  while(1) { 
   asm("hlt"); //halt forever
  }
 }
 
 //initalize variables of graphic mode
 screen_mem = malloc(graphic_screen_x*graphic_screen_y*4);
 if(screen_mem==0) {
  memory_error_debug(0x0000FF);
 }
 screen_bytes_per_line = graphic_screen_x*4;
 graphic_screen_x_center = (graphic_screen_x>>1);
 graphic_screen_y_center = (graphic_screen_y>>1);
 debug_line = 0;
 global_color = BLACK;

 //initalize functions of graphic mode
 if(graphic_screen_bpp==32) {
  redraw_framebuffer = &redraw_framebuffer_32_bpp;
  redraw_part_of_framebuffer = &redraw_part_of_framebuffer_32_bpp;
 }
 else if(graphic_screen_bpp==24) {
  redraw_framebuffer = &redraw_framebuffer_24_bpp;
  redraw_part_of_framebuffer = &redraw_part_of_framebuffer_24_bpp;
 }
 else if(graphic_screen_bpp==16) {
  redraw_framebuffer = &redraw_framebuffer_16_bpp;
  redraw_part_of_framebuffer = &redraw_part_of_framebuffer_16_bpp;
 }

 //initalize fonts
 bleskos_boot_debug_top_screen_color(0xFFFFFF); //white top of screen
 binary_font_memory = (dword_t) &bleskos_font;
 initalize_scalable_font();
 bleskos_boot_debug_top_screen_color(0x888888); //grey top of screen

 //initalize drawing
 set_pen_width(1);
 fill_first_stack = malloc((graphic_screen_x*2+graphic_screen_y*2)*32+8);
 fill_second_stack = malloc((graphic_screen_x*2+graphic_screen_y*2)*32+8);

 //initalize mouse cursor
 mouse_cursor_background = malloc(MOUSE_CURSOR_WIDTH*MOUSE_CURSOR_HEIGTH*4);

 //read EDID
 parse_edid_informations();

 //log
 log("\nlinear frame buffer: ");
 log_hex_with_space(graphic_screen_lfb);
 log("screen x: ");
 log_var_with_space(graphic_screen_x);
 log("screen y: ");
 log_var_with_space(graphic_screen_y);
 log("screen bpp: ");
 log_var(graphic_screen_bpp);
 log("\n");
}

void redraw_framebuffer_32_bpp(void) {
 dword_t *screen32 = (dword_t *) (screen_mem);
 dword_t framebuffer_line_start = (graphic_screen_lfb);
 dword_t *framebuffer32 = (dword_t *) (framebuffer_line_start);

 for(int i=0; i<graphic_screen_y; i++) {
  framebuffer32 = (dword_t *) (framebuffer_line_start);

  for(int j=0; j<graphic_screen_x; j++) {
   *framebuffer32 = *screen32;
   framebuffer32++;
   screen32++;
  }

  framebuffer_line_start += graphic_screen_bytes_per_line;
 }
}

void redraw_framebuffer_24_bpp(void) {
 byte_t *screen8 = (byte_t *) (screen_mem);
 dword_t framebuffer_line_start = (graphic_screen_lfb);
 byte_t *framebuffer8 = (byte_t *) (framebuffer_line_start);

 for(int i=0; i<graphic_screen_y; i++) {
  framebuffer8 = (byte_t *) (framebuffer_line_start);

  for(int j=0; j<graphic_screen_x; j++) {
   *framebuffer8 = *screen8;
   framebuffer8++;
   screen8++;
   *framebuffer8 = *screen8;
   framebuffer8++;
   screen8++;
   *framebuffer8 = *screen8;
   framebuffer8++;
   screen8+=2;
  }

  framebuffer_line_start += graphic_screen_bytes_per_line;
 }
}

void redraw_framebuffer_16_bpp(void) {
 byte_t *screen8 = (byte_t *) (screen_mem);
 dword_t framebuffer_line_start = (graphic_screen_lfb);
 word_t *framebuffer16 = (word_t *) (framebuffer_line_start);

 for(int i=0; i<graphic_screen_y; i++) {
  framebuffer16 = (word_t *) (framebuffer_line_start);

  for(int j=0; j<graphic_screen_x; j++) {
   *framebuffer16 = (((screen8[2] & 0xF8)<<8) | ((screen8[1] & 0xFC)<<3) | ((screen8[0] & 0xF8)>>3));
   framebuffer16++;
   screen8+=4;
  }

  framebuffer_line_start += graphic_screen_bytes_per_line;
 }
}

void redraw_screen(void) {
 if(mouse_cursor_x<graphic_screen_x && mouse_cursor_y<graphic_screen_y) { //mouse is on screen
  //initalize variables
  dword_t *screen;
  dword_t first_line_pixel_pointer_start_value = (screen_mem + (mouse_cursor_y*screen_bytes_per_line) + (mouse_cursor_x<<2));
  dword_t first_line_pixel_pointer = first_line_pixel_pointer_start_value;
  dword_t mouse_height = MOUSE_CURSOR_HEIGTH;
  if((mouse_cursor_y+MOUSE_CURSOR_HEIGTH)>graphic_screen_y) {
   mouse_height = (graphic_screen_y-mouse_cursor_y);
  }
  dword_t mouse_width = MOUSE_CURSOR_WIDTH;
  if((mouse_cursor_x+MOUSE_CURSOR_WIDTH)>graphic_screen_x) {
   mouse_width = (graphic_screen_x-mouse_cursor_x);
  }

  //save background of mouse 
  dword_t *mouse_cursor_background_ptr = (dword_t *) (mouse_cursor_background);
  for(int i=0; i<mouse_height; i++) {
   screen = (dword_t *) first_line_pixel_pointer;
   for(int j=0; j<mouse_width; j++) {
    *mouse_cursor_background_ptr = *screen;
    screen++;
    mouse_cursor_background_ptr++;
   }
   first_line_pixel_pointer += screen_bytes_per_line;
  }

  //draw mouse to double buffer
  dword_t mouse_cursor_img_pointer = (dword_t)(mouse_cursor_img);
  dword_t *mouse_cursor_img = (dword_t *) (mouse_cursor_img_pointer);
  first_line_pixel_pointer = first_line_pixel_pointer_start_value;
  for(int i=0; i<mouse_height; i++) {
   screen = (dword_t *) (first_line_pixel_pointer);
   mouse_cursor_img = (dword_t *) (mouse_cursor_img_pointer);
   for(int j=0; j<mouse_width; j++) {
    if(*mouse_cursor_img!=TRANSPARENT_COLOR) {
     *screen = *mouse_cursor_img;
    }
    screen++;
    mouse_cursor_img++;
   }
   first_line_pixel_pointer += screen_bytes_per_line;
   mouse_cursor_img_pointer += MOUSE_CURSOR_WIDTH*4;
  }

  //redraw screen
  (*redraw_framebuffer)();

  //restore background of mouse
  mouse_cursor_background_ptr = (dword_t *) (mouse_cursor_background);
  first_line_pixel_pointer = first_line_pixel_pointer_start_value;
  for(int i=0; i<mouse_height; i++) {
   screen = (dword_t *) first_line_pixel_pointer;
   for(int j=0; j<mouse_width; j++) {
    *screen = *mouse_cursor_background_ptr;
    screen++;
    mouse_cursor_background_ptr++;
   }
   first_line_pixel_pointer += screen_bytes_per_line;
  }
 }
 else { //mouse is not on screen
  (*redraw_framebuffer)();
 }
}

void redraw_part_of_framebuffer_32_bpp(dword_t x, dword_t y, dword_t width, dword_t height) {
 dword_t *screen32 = (dword_t *) (screen_mem + (y*screen_bytes_per_line) + (x*4));
 dword_t framebuffer_line_start = (graphic_screen_lfb + (y*graphic_screen_bytes_per_line) + (x*4));
 dword_t *framebuffer32 = (dword_t *) (framebuffer_line_start);
 dword_t next_line = (graphic_screen_x-width);

 for(int i=0; i<height; i++) {
  framebuffer32 = (dword_t *) (framebuffer_line_start);

  for(int j=0; j<width; j++) {
   *framebuffer32 = *screen32;
   framebuffer32++;
   screen32++;
  }

  framebuffer_line_start+=graphic_screen_bytes_per_line;
  screen32+=next_line;
 }
}

void redraw_part_of_framebuffer_24_bpp(dword_t x, dword_t y, dword_t width, dword_t height) {
 byte_t *screen8 = (byte_t *) (screen_mem + (y*screen_bytes_per_line) + (x*4));
 dword_t next_line = ((graphic_screen_x-width)*4);
 dword_t framebuffer_line_start = (graphic_screen_lfb + (y*graphic_screen_bytes_per_line) + (x*3));
 byte_t *framebuffer8 = (byte_t *) (framebuffer_line_start);
 
 for(int i=0; i<height; i++) {
  framebuffer8 = (byte_t *) (framebuffer_line_start);

  for(int j=0; j<width; j++) {
   *framebuffer8 = *screen8;
   framebuffer8++;
   screen8++;
   *framebuffer8 = *screen8;
   framebuffer8++;
   screen8++;
   *framebuffer8 = *screen8;
   framebuffer8++;
   screen8+=2;
  }

  framebuffer_line_start+=graphic_screen_bytes_per_line;
  screen8+=next_line;
 }
}

void redraw_part_of_framebuffer_16_bpp(dword_t x, dword_t y, dword_t width, dword_t height) {
 byte_t *screen8 = (byte_t *) (screen_mem + (y*screen_bytes_per_line) + (x*4));
 dword_t next_line = ((graphic_screen_x-width)*4);
 dword_t framebuffer_line_start = (graphic_screen_lfb + (y*graphic_screen_bytes_per_line) + (x*2));
 word_t *framebuffer16 = (word_t *) (framebuffer_line_start);

 for(int i=0; i<height; i++) {
  framebuffer16 = (word_t *) (framebuffer_line_start);

  for(int j=0; j<width; j++) {
   *framebuffer16 = (((screen8[2] & 0xF8)<<8) | ((screen8[1] & 0xFC)<<3) | ((screen8[0] & 0xF8)>>3));
   framebuffer16++;
   screen8+=4;
  }

  framebuffer_line_start += graphic_screen_bytes_per_line;
  screen8+=next_line;
 }
}

void redraw_part_of_screen(dword_t x, dword_t y, dword_t width, dword_t height) {
 //calculate variables
 if(x>graphic_screen_x || y>graphic_screen_y) {
  return;
 }
 if((x+width)>graphic_screen_x) {
  width = (graphic_screen_x-x);
 }
 if((y+height)>graphic_screen_y) {
  height = (graphic_screen_y-y);
 }
 
 //redraw part of screen
 if((mouse_cursor_x+MOUSE_CURSOR_WIDTH)<x && mouse_cursor_x>(x+width) && (mouse_cursor_y+MOUSE_CURSOR_HEIGTH)<y && mouse_cursor_y>(y+height)) { //mouse is not on redrawed part of screen
  (*redraw_part_of_framebuffer)(x, y, width, height);
 }
 else { //mouse is on redrawed part of screen
  //initalize variables
  dword_t *screen;
  dword_t first_line_pixel_pointer_start_value = (screen_mem + (mouse_cursor_y*screen_bytes_per_line) + (mouse_cursor_x<<2));
  dword_t first_line_pixel_pointer = first_line_pixel_pointer_start_value;
  dword_t mouse_height = MOUSE_CURSOR_HEIGTH;
  if((mouse_cursor_y+MOUSE_CURSOR_HEIGTH)>graphic_screen_y) {
   mouse_height = (graphic_screen_y-mouse_cursor_y);
  }
  dword_t mouse_width = MOUSE_CURSOR_WIDTH;
  if((mouse_cursor_x+MOUSE_CURSOR_WIDTH)>graphic_screen_x) {
   mouse_width = (graphic_screen_x-mouse_cursor_x);
  }

  //save background of mouse 
  dword_t *mouse_cursor_background_ptr = (dword_t *) (mouse_cursor_background);
  for(int i=0; i<mouse_height; i++) {
   screen = (dword_t *) first_line_pixel_pointer;
   for(int j=0; j<mouse_width; j++) {
    *mouse_cursor_background_ptr = *screen;
    screen++;
    mouse_cursor_background_ptr++;
   }
   first_line_pixel_pointer += screen_bytes_per_line;
  }

  //draw mouse to double buffer
  dword_t mouse_cursor_img_pointer = (dword_t)(mouse_cursor_img);
  dword_t *mouse_cursor_img = (dword_t *) (mouse_cursor_img_pointer);
  first_line_pixel_pointer = first_line_pixel_pointer_start_value;
  for(int i=0; i<mouse_height; i++) {
   screen = (dword_t *) (first_line_pixel_pointer);
   mouse_cursor_img = (dword_t *) (mouse_cursor_img_pointer);
   for(int j=0; j<mouse_width; j++) {
    if(*mouse_cursor_img!=TRANSPARENT_COLOR) {
     *screen = *mouse_cursor_img;
    }
    screen++;
    mouse_cursor_img++;
   }
   first_line_pixel_pointer += screen_bytes_per_line;
   mouse_cursor_img_pointer += MOUSE_CURSOR_WIDTH*4;
  }

  //redraw part of screen
  (*redraw_part_of_framebuffer)(x, y, width, height);
  
  //restore background of mouse
  mouse_cursor_background_ptr = (dword_t *) (mouse_cursor_background);
  first_line_pixel_pointer = first_line_pixel_pointer_start_value;
  for(int i=0; i<mouse_height; i++) {
   screen = (dword_t *) first_line_pixel_pointer;
   for(int j=0; j<mouse_width; j++) {
    *screen = *mouse_cursor_background_ptr;
    screen++;
    mouse_cursor_background_ptr++;
   }
   first_line_pixel_pointer += screen_bytes_per_line;
  }
 }
}

void screen_save_variables(void) {
 save_screen_mem = screen_mem;
 save_screen_x = graphic_screen_x;
 save_screen_y = graphic_screen_y;
 save_screen_bytes_per_line = screen_bytes_per_line;
}

void screen_restore_variables(void) {
 screen_mem = save_screen_mem;
 graphic_screen_x = save_screen_x;
 graphic_screen_y = save_screen_y;
 screen_bytes_per_line = save_screen_bytes_per_line;
}