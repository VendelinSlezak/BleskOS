//BleskOS

/*
* MIT License
* Copyright (c) 2023-2025 BleskOS developers
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

void initalize_graphic(void) {
 //initalize variables for using graphic mode that was set by VBE
 vesa_read_graphic_mode_info();

 //read EDID info loaded by bootloader on 0x2000
 is_bootloader_edid_present = parse_edid_data(0x2000);
 if(is_bootloader_edid_present==STATUS_FALSE) {
  log("\n\nBootloader did not load EDID");
 }
 else {
  log("\n\nBOOTLOADER EDID");
  log_edid_data();
 }

 //set best graphic mode by driver of graphic card
 is_driver_for_graphic_card = STATUS_FALSE;
 if(is_bga_present() == STATUS_TRUE) { //check Bochs Graphic Adapter
  bga_set_graphic_mode();
 }
 else {
  //run driver of graphic card
  for(dword_t i=0; i<number_of_graphic_cards; i++) {
   if(graphic_cards_info[i].initalize != 0) {
    selected_graphic_card = i;
    is_driver_for_graphic_card = STATUS_TRUE;
    graphic_cards_info[i].initalize(i);
    break;
   }
  }
 }

 //check if we have valid graphic mode
 if((dword_t)monitor_screen_linear_frame_buffer_memory_pointer==0 || monitor_screen_bytes_per_line==0 || screen_width==0 || screen_height==0 || (screen_bpp!=16 && screen_bpp!=24 && screen_bpp!=32)) {
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

  //inform user about error also with sound from PC speaker
  pc_speaker_beep(500);

  //halt forever
  while(1) { 
   asm("hlt");
  }
 }

 //set linear frame buffer to be Write-Combined to maximize performance
 mtrr_set_memory_type(monitor_screen_linear_frame_buffer_memory_pointer, screen_height*monitor_screen_bytes_per_line, MTRR_MEMORY_TYPE_WRITE_COMBINING);

 //allocate memory for double buffer of screen
 screen_double_buffer_memory_pointer = (byte_t *) malloc(screen_width*screen_height*4);

 //initalize other variables for graphic mode
 screen_double_buffer_bytes_per_line = (screen_width*4);
 screen_x_center = (screen_width/2);
 screen_y_center = (screen_height/2);
 debug_line = 0;

 //initalize redraw functions of graphic mode
 if(screen_bpp==32) {
  if(monitor_screen_bytes_per_line==(screen_width*4)) {
   redraw_framebuffer = (&redraw_framebuffer_32_bpp_without_padding);
  }
  else {
   redraw_framebuffer = (&redraw_framebuffer_32_bpp);
  }
  redraw_part_of_framebuffer = (&redraw_part_of_framebuffer_32_bpp);
 }
 else if(screen_bpp==24) {
  if(monitor_screen_bytes_per_line==(screen_width*3)) {
   redraw_framebuffer = (&redraw_framebuffer_24_bpp_without_padding);
  }
  else {
   redraw_framebuffer = (&redraw_framebuffer_24_bpp);
  }
  redraw_part_of_framebuffer = (&redraw_part_of_framebuffer_24_bpp);
 }
 else { //screen_bpp = 16, other value is impossible, because we already tested this variable in if() about valid graphic mode
  if(monitor_screen_bytes_per_line==(screen_width*2)) {
   redraw_framebuffer = (&redraw_framebuffer_16_bpp_without_padding);
  }
  else {
   redraw_framebuffer = (&redraw_framebuffer_16_bpp);
  }
  redraw_part_of_framebuffer = (&redraw_part_of_framebuffer_16_bpp);
 }

 //initalize fonts
 bleskos_boot_debug_top_screen_color(0xFFFFFF); //white top of screen
 extern dword_t *bleskos_font;
 binary_font_memory = (dword_t) (&bleskos_font);
 initalize_scalable_font();
 bleskos_boot_debug_top_screen_color(0x888888); //grey top of screen

 //initalize variables of drawing
 set_pen_width(1, BLACK);
 fill_first_stack = (dword_t) malloc((screen_width*2+screen_height*2)*32+8);
 fill_second_stack = (dword_t) malloc((screen_width*2+screen_height*2)*32+8);

 //allocate memory for mouse cursor background
 mouse_cursor_background = (dword_t) malloc(MOUSE_CURSOR_WIDTH*MOUSE_CURSOR_HEIGHT*4);

 //set initial position of mouse cursor
 mouse_cursor_x = 0;
 mouse_cursor_y = 0;

 //log
 logf("\n\nGRAPHIC MODE INFO\nLinear frame buffer: %x\nScreen x: %d\nScreen y: %d\nScreen bpp: %d",
      (dword_t)monitor_screen_linear_frame_buffer_memory_pointer,
      screen_width,
      screen_height,
      screen_bpp);
}

void redraw_framebuffer_32_bpp(void) {
 cli();

 dword_t *screen32 = (dword_t *) (screen_double_buffer_memory_pointer);
 dword_t framebuffer_line_start = ((dword_t)monitor_screen_linear_frame_buffer_memory_pointer);
 dword_t *framebuffer32 = (dword_t *) (framebuffer_line_start);

 for(dword_t i=0; i<screen_height; i++) {
  framebuffer32 = (dword_t *) (framebuffer_line_start);

  for(dword_t j=0; j<screen_width; j++) {
   *framebuffer32 = *screen32;
   framebuffer32++;
   screen32++;
  }

  framebuffer_line_start += monitor_screen_bytes_per_line;
 }

 sti();
}

void redraw_framebuffer_32_bpp_without_padding(void) {
 cli();

 dword_t *screen32 = (dword_t *) (screen_double_buffer_memory_pointer);
 dword_t *framebuffer32 = (dword_t *) (monitor_screen_linear_frame_buffer_memory_pointer);

 for(dword_t i=0; i<(screen_height*screen_width); i++) {
  *framebuffer32 = *screen32;
  framebuffer32++;
  screen32++;
 }

 sti();
}

void redraw_framebuffer_24_bpp(void) {
 cli();

 byte_t *screen8 = (byte_t *) (screen_double_buffer_memory_pointer);
 dword_t framebuffer_line_start = ((dword_t)monitor_screen_linear_frame_buffer_memory_pointer);
 byte_t *framebuffer8 = (byte_t *) (framebuffer_line_start);

 for(dword_t i=0; i<screen_height; i++) {
  framebuffer8 = (byte_t *) (framebuffer_line_start);

  for(dword_t j=0; j<screen_width; j++) {
   framebuffer8[0] = screen8[0];
   framebuffer8[1] = screen8[1];
   framebuffer8[2] = screen8[2];
   framebuffer8 += 3;
   screen8 += 4;
  }

  framebuffer_line_start += monitor_screen_bytes_per_line;
 }

 sti();
}

void redraw_framebuffer_24_bpp_without_padding(void) {
 cli();

 dword_t *screen32 = (dword_t *) (screen_double_buffer_memory_pointer);
 dword_t *framebuffer32 = (dword_t *) (monitor_screen_linear_frame_buffer_memory_pointer);

 for(dword_t i=0; i<(screen_height*screen_width-1); i++) {
  *framebuffer32 = *screen32;
  framebuffer32 = (dword_t *) ((dword_t)framebuffer32+3);
  screen32++;
 }

 framebuffer32 = (dword_t *) ((dword_t)framebuffer32-1);
 *framebuffer32 = ((screen32[0] << 8) | ((screen32[-1] >> 16) & 0xFF));

 sti();
}

void redraw_framebuffer_16_bpp(void) {
 cli();

 byte_t *screen8 = (byte_t *) (screen_double_buffer_memory_pointer);
 dword_t framebuffer_line_start = ((dword_t)monitor_screen_linear_frame_buffer_memory_pointer);
 word_t *framebuffer16 = (word_t *) (framebuffer_line_start);

 for(dword_t i=0; i<screen_height; i++) {
  framebuffer16 = (word_t *) (framebuffer_line_start);

  for(dword_t j=0; j<screen_width; j++) {
   *framebuffer16 = (((screen8[2] & 0xF8)<<8) | ((screen8[1] & 0xFC)<<3) | ((screen8[0] & 0xF8)>>3));
   framebuffer16++;
   screen8+=4;
  }

  framebuffer_line_start += monitor_screen_bytes_per_line;
 }

 sti();
}

void redraw_framebuffer_16_bpp_without_padding(void) {
 cli();

 byte_t *screen8 = (byte_t *) (screen_double_buffer_memory_pointer);
 word_t *framebuffer16 = (word_t *) (monitor_screen_linear_frame_buffer_memory_pointer);

 for(dword_t i=0; i<(screen_height*screen_width); i++) {
  *framebuffer16 = (((screen8[2] & 0xF8)<<8) | ((screen8[1] & 0xFC)<<3) | ((screen8[0] & 0xF8)>>3));
  framebuffer16++;
  screen8+=4;
 }

 sti();
}

void redraw_screen(void) {
 if(mouse_cursor_x<screen_width && mouse_cursor_y<screen_height) { //mouse is on screen
  add_mouse_to_screen_double_buffer();
  (*redraw_framebuffer)();
  remove_mouse_from_screen_double_buffer();
 }
 else { //mouse is not on screen
  (*redraw_framebuffer)();
 }
}

void redraw_part_of_framebuffer_32_bpp(dword_t x, dword_t y, dword_t width, dword_t height) {
 dword_t *screen32 = (dword_t *) ((dword_t)screen_double_buffer_memory_pointer + (y*screen_double_buffer_bytes_per_line) + (x*4));
 dword_t framebuffer_line_start = ((dword_t)monitor_screen_linear_frame_buffer_memory_pointer + (y*monitor_screen_bytes_per_line) + (x*4));
 dword_t *framebuffer32 = (dword_t *) (framebuffer_line_start);
 dword_t next_line = (screen_width-width);

 for(dword_t i=0; i<height; i++) {
  framebuffer32 = (dword_t *) (framebuffer_line_start);

  for(dword_t j=0; j<width; j++) {
   *framebuffer32 = *screen32;
   framebuffer32++;
   screen32++;
  }

  framebuffer_line_start+=monitor_screen_bytes_per_line;
  screen32+=next_line;
 }
}

void redraw_part_of_framebuffer_24_bpp(dword_t x, dword_t y, dword_t width, dword_t height) {
 byte_t *screen8 = (byte_t *) ((dword_t)screen_double_buffer_memory_pointer + (y*screen_double_buffer_bytes_per_line) + (x*4));
 dword_t next_line = ((screen_width-width)*4);
 dword_t framebuffer_line_start = ((dword_t)monitor_screen_linear_frame_buffer_memory_pointer + (y*monitor_screen_bytes_per_line) + (x*3));
 byte_t *framebuffer8 = (byte_t *) (framebuffer_line_start);
 
 for(dword_t i=0; i<height; i++) {
  framebuffer8 = (byte_t *) (framebuffer_line_start);

  for(dword_t j=0; j<width; j++) {
   framebuffer8[0] = screen8[0];
   framebuffer8[1] = screen8[1];
   framebuffer8[2] = screen8[2];
   framebuffer8 += 3;
   screen8 += 4;
  }

  framebuffer_line_start+=monitor_screen_bytes_per_line;
  screen8+=next_line;
 }
}

void redraw_part_of_framebuffer_16_bpp(dword_t x, dword_t y, dword_t width, dword_t height) {
 byte_t *screen8 = (byte_t *) ((dword_t)screen_double_buffer_memory_pointer + (y*screen_double_buffer_bytes_per_line) + (x*4));
 dword_t next_line = ((screen_width-width)*4);
 dword_t framebuffer_line_start = ((dword_t)monitor_screen_linear_frame_buffer_memory_pointer + (y*monitor_screen_bytes_per_line) + (x*2));
 word_t *framebuffer16 = (word_t *) (framebuffer_line_start);

 for(dword_t i=0; i<height; i++) {
  framebuffer16 = (word_t *) (framebuffer_line_start);

  for(dword_t j=0; j<width; j++) {
   *framebuffer16 = (((screen8[2] & 0xF8)<<8) | ((screen8[1] & 0xFC)<<3) | ((screen8[0] & 0xF8)>>3));
   framebuffer16++;
   screen8+=4;
  }

  framebuffer_line_start += monitor_screen_bytes_per_line;
  screen8+=next_line;
 }
}

void redraw_part_of_screen(dword_t x, dword_t y, dword_t width, dword_t height) {
 //calculate variables
 if(x>screen_width || y>screen_height) {
  return;
 }
 if((x+width)>screen_width) {
  width = (screen_width-x);
 }
 if((y+height)>screen_height) {
  height = (screen_height-y);
 }
 
 //redraw part of screen
 if((mouse_cursor_x+MOUSE_CURSOR_WIDTH)<x && mouse_cursor_x>(x+width) && (mouse_cursor_y+MOUSE_CURSOR_HEIGHT)<y && mouse_cursor_y>(y+height)) { //mouse is not on redrawed part of screen
  (*redraw_part_of_framebuffer)(x, y, width, height);
 }
 else { //mouse is on redrawed part of screen
  add_mouse_to_screen_double_buffer();
  (*redraw_part_of_framebuffer)(x, y, width, height);
  remove_mouse_from_screen_double_buffer();
 }
}

void add_mouse_to_screen_double_buffer(void) {
 //initalize variables
 dword_t *screen;
 dword_t first_line_pixel_pointer_start_value = ((dword_t)screen_double_buffer_memory_pointer + (mouse_cursor_y*screen_double_buffer_bytes_per_line) + (mouse_cursor_x*4));
 dword_t first_line_pixel_pointer = first_line_pixel_pointer_start_value;
 dword_t mouse_height = MOUSE_CURSOR_HEIGHT;
 if((mouse_cursor_y+MOUSE_CURSOR_HEIGHT)>screen_height) {
  mouse_height = (screen_height-mouse_cursor_y);
 }
 dword_t mouse_width = MOUSE_CURSOR_WIDTH;
 if((mouse_cursor_x+MOUSE_CURSOR_WIDTH)>screen_width) {
  mouse_width = (screen_width-mouse_cursor_x);
 }

 //save background of mouse 
 dword_t *mouse_cursor_background_ptr = (dword_t *) (mouse_cursor_background);
 for(dword_t i=0; i<mouse_height; i++) {
  screen = (dword_t *) first_line_pixel_pointer;
  for(dword_t j=0; j<mouse_width; j++) {
   *mouse_cursor_background_ptr = *screen;
   screen++;
   mouse_cursor_background_ptr++;
  }
  first_line_pixel_pointer += screen_double_buffer_bytes_per_line;
 }

 //draw mouse to double buffer
 dword_t mouse_cursor_img_pointer = (dword_t)(mouse_cursor_img);
 dword_t *mouse_cursor_img = (dword_t *) (mouse_cursor_img_pointer);
 first_line_pixel_pointer = first_line_pixel_pointer_start_value;
 for(dword_t i=0; i<mouse_height; i++) {
  screen = (dword_t *) (first_line_pixel_pointer);
  mouse_cursor_img = (dword_t *) (mouse_cursor_img_pointer);
  for(dword_t j=0; j<mouse_width; j++) {
   if(*mouse_cursor_img!=TRANSPARENT_COLOR) {
    *screen = *mouse_cursor_img;
   }
   screen++;
   mouse_cursor_img++;
  }
  first_line_pixel_pointer += screen_double_buffer_bytes_per_line;
  mouse_cursor_img_pointer += MOUSE_CURSOR_WIDTH*4;
 }
}

void remove_mouse_from_screen_double_buffer(void) {
 //initalize variables
 dword_t mouse_height = MOUSE_CURSOR_HEIGHT;
 if((mouse_cursor_y+MOUSE_CURSOR_HEIGHT)>screen_height) {
  mouse_height = (screen_height-mouse_cursor_y);
 }
 dword_t mouse_width = MOUSE_CURSOR_WIDTH;
 if((mouse_cursor_x+MOUSE_CURSOR_WIDTH)>screen_width) {
  mouse_width = (screen_width-mouse_cursor_x);
 }
 dword_t *screen;
 dword_t *mouse_cursor_background_ptr = (dword_t *) (mouse_cursor_background);
 dword_t first_line_pixel_pointer = ((dword_t)screen_double_buffer_memory_pointer + (mouse_cursor_y*screen_double_buffer_bytes_per_line) + (mouse_cursor_x*4));
 
 //restore background of mouse
 for(dword_t i=0; i<mouse_height; i++) {
  screen = (dword_t *) first_line_pixel_pointer;
  for(dword_t j=0; j<mouse_width; j++) {
   *screen = *mouse_cursor_background_ptr;
   screen++;
   mouse_cursor_background_ptr++;
  }
  first_line_pixel_pointer += screen_double_buffer_bytes_per_line;
 }
}

void screen_save_variables(void) {
 save_screen_double_buffer_memory_pointer = screen_double_buffer_memory_pointer;
 save_screen_width = screen_width;
 save_screen_height = screen_height;
 save_screen_double_buffer_bytes_per_line = screen_double_buffer_bytes_per_line;
}

void screen_restore_variables(void) {
 screen_double_buffer_memory_pointer = save_screen_double_buffer_memory_pointer;
 screen_width = save_screen_width;
 screen_height = save_screen_height;
 screen_double_buffer_bytes_per_line = save_screen_double_buffer_bytes_per_line;
}

void monitor_change_backlight(byte_t value) {
 if(is_driver_for_graphic_card==STATUS_FALSE || can_graphic_card_driver_change_backlight==STATUS_FALSE) {
  return;
 }

 //set to good value
 if(value<10) {
  value = 10;
 }
 else if(value>100) {
  value = 100;
 }

 //change backlight
 (*graphic_card_driver_monitor_change_backlight)(value);
}