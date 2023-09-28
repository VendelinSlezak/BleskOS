//BleskOS

/*
* MIT License
* Copyright (c) 2023 Vendelín Slezák
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

void initalize_graphic(void) {
 extern dword_t *bleskos_font;

 //set best video mode according to graphic card
 vesa_init_graphic(); //currently there are no other graphic drivers

 //check if we have graphic mode
 if(graphic_screen_lfb==0 || graphic_screen_bytes_per_line==0 || graphic_screen_x==0 || graphic_screen_y==0 || graphic_screen_bpp==0) {
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
 
 //initalize variabiles of graphic mode
 screen_mem = malloc(graphic_screen_x*graphic_screen_y*4);
 if(screen_mem==0) {
  memory_error_debug(0x0000FF);
 }
 screen_bytes_per_line = graphic_screen_x*4;
 graphic_screen_x_center = (graphic_screen_x>>1);
 graphic_screen_y_center = (graphic_screen_y>>1);
 debug_line = 0;
 global_color = BLACK;

 //initalize fonts
 binary_font_memory = (dword_t) &bleskos_font;
 initalize_scalable_font();

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

void redraw_screen(void) {
 dword_t *screen32 = (dword_t *) screen_mem;
 dword_t framebuffer_line_start = graphic_screen_lfb;
 dword_t *framebuffer32 = (dword_t *) (framebuffer_line_start);

 if(graphic_screen_bpp==32) {
  for(int i=0; i<graphic_screen_y; i++) {
   framebuffer32 = (dword_t *) (framebuffer_line_start);

   for(int j=0; j<graphic_screen_x; j++) {
    *framebuffer32 = *screen32;
    framebuffer32++;
    screen32++;
   }

   framebuffer_line_start += graphic_screen_bytes_per_line;
  }

  return;
 }

 byte_t *screen8 = (byte_t *) screen_mem;
 byte_t *framebuffer8 = (byte_t *) (framebuffer_line_start);

 if(graphic_screen_bpp==24) {
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
}

void redraw_part_of_screen(dword_t x, dword_t y, dword_t width, dword_t heigth) {
 if(x>graphic_screen_x || y>graphic_screen_y) {
  return;
 }
 if((x+width)>graphic_screen_x) {
  width = (graphic_screen_x-x);
 }
 if((y+heigth)>graphic_screen_y) {
  heigth = (graphic_screen_y-y);
 }
 
 dword_t *screen32 = (dword_t *) (screen_mem + (y*screen_bytes_per_line) + (x*4));
 dword_t framebuffer_line_start = (graphic_screen_lfb + (y*graphic_screen_bytes_per_line) + (x*4));
 dword_t *framebuffer32 = (dword_t *) (framebuffer_line_start);
 dword_t next_line = (graphic_screen_x-width);
 
 if(graphic_screen_bpp==32) {
  for(int i=0; i<heigth; i++) {
   framebuffer32 = (dword_t *) (framebuffer_line_start);
   for(int j=0; j<width; j++) {
    *framebuffer32 = *screen32;
    framebuffer32++;
    screen32++;
   }
   framebuffer_line_start+=graphic_screen_bytes_per_line;
   screen32+=next_line;
  }
  return;
 }
 
 byte_t *screen8 = (byte_t *) (screen_mem + (y*screen_bytes_per_line) + (x*4));
 framebuffer_line_start = (graphic_screen_lfb + (y*graphic_screen_bytes_per_line) + (x*3));;
 byte_t *framebuffer8 = (byte_t *) (framebuffer_line_start);
 dword_t next_line_framebuffer = (graphic_screen_x*3-width*3);
 next_line *= 4;
 
 if(graphic_screen_bpp==24) {
  for(int i=0; i<heigth; i++) {
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
}

void screen_save_variabiles(void) {
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