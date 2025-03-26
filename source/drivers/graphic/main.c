//BleskOS

/*
* MIT License
* Copyright (c) 2023-2025 BleskOS developers
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

void initalize_graphic(void) {
    // clear list of monitors
    clear_memory((dword_t)&monitors, sizeof(monitors));

    /* initalize graphic cards and find all usable monitors */

    // get actual monitor mode from VBE
    vesa_read_graphic_mode_info();

    // Bochs Graphic Adapter
    if(components->p_bga == 1) {
        initalize_bga();
    }
    // Driver for graphic card present
    else if(components->p_graphic_card == 1) {
        // run VMware driver
        for(dword_t i = 0; i < components->n_vmware_graphic_card; i++) {
            initalize_vmware_graphic_card(i);
        }

        // run Intel driver
        for(dword_t i = 0; i < components->n_intel_graphic_card; i++) {
            initalize_intel_graphic_card(i);
        }
    }
    // No driver for graphic card
    else {
        // read EDID info loaded by bootloader on 0x2000
        is_bootloader_edid_present = parse_edid_data(0x2000);
        if(is_bootloader_edid_present == STATUS_FALSE) {
            logf("\n\nBootloader did not load EDID");
        }
        else {
            logf("\n\nBOOTLOADER EDID");
            log_edid_data();
        }
    }

    //check if we have valid graphic mode
    if(monitors[0].connected == STATUS_FALSE
        || (dword_t)monitors[0].linear_frame_buffer==0
        || monitors[0].linear_frame_buffer_bpl==0
        || monitors[0].width==0
        || monitors[0].height==0
        || (monitors[0].bpp!=16 && monitors[0].bpp!=24 && monitors[0].bpp!=32)) {
        //ERROR: we do not have valid graphic mode
        
        // if is monitor in VGA mode, clear screen with red color and print "Error 1"
        vga_text_mode_clear_screen(0x40);
        vga_text_mode_print(1, 1, "Error 1");

        // inform user about error by sound from PC speaker
        pc_speaker_beep(500);

        // halt forever
        while(1) { asm("hlt"); }
    }

 // set linear frame buffer to be Write-Combined to maximize performance
 mtrr_set_memory_type(monitors[0].linear_frame_buffer, monitors[0].height*monitors[0].linear_frame_buffer_bpl, MTRR_MEMORY_TYPE_WRITE_COMBINING);

 // allocate memory for double buffer of screen
 monitors[0].double_buffer = (byte_t *) malloc(monitors[0].width*monitors[0].height*4);

 // initalize other variables for graphic mode
 monitors[0].double_buffer_bpl = (monitors[0].width*4);
 monitors[0].x_center = (monitors[0].width/2);
 monitors[0].y_center = (monitors[0].height/2);
 debug_line = 0;

 // initalize redraw functions of graphic mode
 if(monitors[0].bpp==32) {
  if(monitors[0].linear_frame_buffer_bpl==(monitors[0].width*4)) {
   redraw_framebuffer = (&redraw_framebuffer_32_bpp_without_padding);
  }
  else {
   redraw_framebuffer = (&redraw_framebuffer_32_bpp);
  }
  redraw_part_of_framebuffer = (&redraw_part_of_framebuffer_32_bpp);
 }
 else if(monitors[0].bpp==24) {
  if(monitors[0].linear_frame_buffer_bpl==(monitors[0].width*3)) {
   redraw_framebuffer = (&redraw_framebuffer_24_bpp_without_padding);
  }
  else {
   redraw_framebuffer = (&redraw_framebuffer_24_bpp);
  }
  redraw_part_of_framebuffer = (&redraw_part_of_framebuffer_24_bpp);
 }
 else { // monitors[0].bpp = 16, other value is impossible, because we already tested this variable in if() about valid graphic mode
  if(monitors[0].linear_frame_buffer_bpl==(monitors[0].width*2)) {
   redraw_framebuffer = (&redraw_framebuffer_16_bpp_without_padding);
  }
  else {
   redraw_framebuffer = (&redraw_framebuffer_16_bpp);
  }
  redraw_part_of_framebuffer = (&redraw_part_of_framebuffer_16_bpp);
 }

 // initalize fonts
 extern dword_t *bleskos_font;
 binary_font_memory = (dword_t) (&bleskos_font);
 initalize_scalable_font();

 // initalize variables of drawing
 set_pen_width(1, BLACK);
 fill_first_stack = (dword_t) malloc((monitors[0].width*2+monitors[0].height*2)*32+8);
 fill_second_stack = (dword_t) malloc((monitors[0].width*2+monitors[0].height*2)*32+8);

 // allocate memory for mouse cursor background
 mouse_cursor_background = (dword_t) malloc(MOUSE_CURSOR_WIDTH*MOUSE_CURSOR_HEIGHT*4);

 // set initial position of mouse cursor
 mouse_cursor_x = 0;
 mouse_cursor_y = 0;

    // log monitor info
    logf("\n\nMONITOR INFO");
    for(dword_t i = 0; i < MAX_NUMBER_OF_MONITORS; i++) {
        if(monitors[i].connected == STATUS_TRUE) {
            logf("\nMonitor %d\n Mode: %dx%dx%d\n Linear frame buffer: 0x%x",
                i+1,
                monitors[i].width,
                monitors[i].height,
                monitors[i].bpp,
                monitors[i].linear_frame_buffer);
        }
    }
}

void redraw_framebuffer_32_bpp(void) {
 cli();

 dword_t *screen32 = (dword_t *) (monitors[0].double_buffer);
 dword_t framebuffer_line_start = ((dword_t)monitors[0].linear_frame_buffer);
 dword_t *framebuffer32 = (dword_t *) (framebuffer_line_start);

 for(dword_t i=0; i<monitors[0].height; i++) {
  framebuffer32 = (dword_t *) (framebuffer_line_start);

  for(dword_t j=0; j<monitors[0].width; j++) {
   *framebuffer32 = *screen32;
   framebuffer32++;
   screen32++;
  }

  framebuffer_line_start += monitors[0].linear_frame_buffer_bpl;
 }

 sti();
}

void redraw_framebuffer_32_bpp_without_padding(void) {
 cli();

 dword_t *screen32 = (dword_t *) (monitors[0].double_buffer);
 dword_t *framebuffer32 = (dword_t *) (monitors[0].linear_frame_buffer);

 for(dword_t i=0; i<(monitors[0].height*monitors[0].width); i++) {
  *framebuffer32 = *screen32;
  framebuffer32++;
  screen32++;
 }

 sti();
}

void redraw_framebuffer_24_bpp(void) {
 cli();

 byte_t *screen8 = (byte_t *) (monitors[0].double_buffer);
 dword_t framebuffer_line_start = ((dword_t)monitors[0].linear_frame_buffer);
 byte_t *framebuffer8 = (byte_t *) (framebuffer_line_start);

 for(dword_t i=0; i<monitors[0].height; i++) {
  framebuffer8 = (byte_t *) (framebuffer_line_start);

  for(dword_t j=0; j<monitors[0].width; j++) {
   framebuffer8[0] = screen8[0];
   framebuffer8[1] = screen8[1];
   framebuffer8[2] = screen8[2];
   framebuffer8 += 3;
   screen8 += 4;
  }

  framebuffer_line_start += monitors[0].linear_frame_buffer_bpl;
 }

 sti();
}

void redraw_framebuffer_24_bpp_without_padding(void) {
 cli();

 dword_t *screen32 = (dword_t *) (monitors[0].double_buffer);
 dword_t *framebuffer32 = (dword_t *) (monitors[0].linear_frame_buffer);

 for(dword_t i=0; i<(monitors[0].height*monitors[0].width-1); i++) {
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

 byte_t *screen8 = (byte_t *) (monitors[0].double_buffer);
 dword_t framebuffer_line_start = ((dword_t)monitors[0].linear_frame_buffer);
 word_t *framebuffer16 = (word_t *) (framebuffer_line_start);

 for(dword_t i=0; i<monitors[0].height; i++) {
  framebuffer16 = (word_t *) (framebuffer_line_start);

  for(dword_t j=0; j<monitors[0].width; j++) {
   *framebuffer16 = (((screen8[2] & 0xF8)<<8) | ((screen8[1] & 0xFC)<<3) | ((screen8[0] & 0xF8)>>3));
   framebuffer16++;
   screen8+=4;
  }

  framebuffer_line_start += monitors[0].linear_frame_buffer_bpl;
 }

 sti();
}

void redraw_framebuffer_16_bpp_without_padding(void) {
 cli();

 byte_t *screen8 = (byte_t *) (monitors[0].double_buffer);
 word_t *framebuffer16 = (word_t *) (monitors[0].linear_frame_buffer);

 for(dword_t i=0; i<(monitors[0].height*monitors[0].width); i++) {
  *framebuffer16 = (((screen8[2] & 0xF8)<<8) | ((screen8[1] & 0xFC)<<3) | ((screen8[0] & 0xF8)>>3));
  framebuffer16++;
  screen8+=4;
 }

 sti();
}

void redraw_screen(void) {
 if(mouse_cursor_x<monitors[0].width && mouse_cursor_y<monitors[0].height) { //mouse is on screen
  add_mouse_to_screen_double_buffer();
  (*redraw_framebuffer)();
  remove_mouse_from_screen_double_buffer();
 }
 else { //mouse is not on screen
  (*redraw_framebuffer)();
 }
}

void redraw_part_of_framebuffer_32_bpp(dword_t x, dword_t y, dword_t width, dword_t height) {
 dword_t *screen32 = (dword_t *) ((dword_t)monitors[0].double_buffer + (y*monitors[0].double_buffer_bpl) + (x*4));
 dword_t framebuffer_line_start = ((dword_t)monitors[0].linear_frame_buffer + (y*monitors[0].linear_frame_buffer_bpl) + (x*4));
 dword_t *framebuffer32 = (dword_t *) (framebuffer_line_start);
 dword_t next_line = (monitors[0].width-width);

 for(dword_t i=0; i<height; i++) {
  framebuffer32 = (dword_t *) (framebuffer_line_start);

  for(dword_t j=0; j<width; j++) {
   *framebuffer32 = *screen32;
   framebuffer32++;
   screen32++;
  }

  framebuffer_line_start+=monitors[0].linear_frame_buffer_bpl;
  screen32+=next_line;
 }
}

void redraw_part_of_framebuffer_24_bpp(dword_t x, dword_t y, dword_t width, dword_t height) {
 byte_t *screen8 = (byte_t *) ((dword_t)monitors[0].double_buffer + (y*monitors[0].double_buffer_bpl) + (x*4));
 dword_t next_line = ((monitors[0].width-width)*4);
 dword_t framebuffer_line_start = ((dword_t)monitors[0].linear_frame_buffer + (y*monitors[0].linear_frame_buffer_bpl) + (x*3));
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

  framebuffer_line_start+=monitors[0].linear_frame_buffer_bpl;
  screen8+=next_line;
 }
}

void redraw_part_of_framebuffer_16_bpp(dword_t x, dword_t y, dword_t width, dword_t height) {
 byte_t *screen8 = (byte_t *) ((dword_t)monitors[0].double_buffer + (y*monitors[0].double_buffer_bpl) + (x*4));
 dword_t next_line = ((monitors[0].width-width)*4);
 dword_t framebuffer_line_start = ((dword_t)monitors[0].linear_frame_buffer + (y*monitors[0].linear_frame_buffer_bpl) + (x*2));
 word_t *framebuffer16 = (word_t *) (framebuffer_line_start);

 for(dword_t i=0; i<height; i++) {
  framebuffer16 = (word_t *) (framebuffer_line_start);

  for(dword_t j=0; j<width; j++) {
   *framebuffer16 = (((screen8[2] & 0xF8)<<8) | ((screen8[1] & 0xFC)<<3) | ((screen8[0] & 0xF8)>>3));
   framebuffer16++;
   screen8+=4;
  }

  framebuffer_line_start += monitors[0].linear_frame_buffer_bpl;
  screen8+=next_line;
 }
}

void redraw_part_of_screen(dword_t x, dword_t y, dword_t width, dword_t height) {
 //calculate variables
 if(x>monitors[0].width || y>monitors[0].height) {
  return;
 }
 if((x+width)>monitors[0].width) {
  width = (monitors[0].width-x);
 }
 if((y+height)>monitors[0].height) {
  height = (monitors[0].height-y);
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
 dword_t first_line_pixel_pointer_start_value = ((dword_t)monitors[0].double_buffer + (mouse_cursor_y*monitors[0].double_buffer_bpl) + (mouse_cursor_x*4));
 dword_t first_line_pixel_pointer = first_line_pixel_pointer_start_value;
 dword_t mouse_height = MOUSE_CURSOR_HEIGHT;
 if((mouse_cursor_y+MOUSE_CURSOR_HEIGHT)>monitors[0].height) {
  mouse_height = (monitors[0].height-mouse_cursor_y);
 }
 dword_t mouse_width = MOUSE_CURSOR_WIDTH;
 if((mouse_cursor_x+MOUSE_CURSOR_WIDTH)>monitors[0].width) {
  mouse_width = (monitors[0].width-mouse_cursor_x);
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
  first_line_pixel_pointer += monitors[0].double_buffer_bpl;
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
  first_line_pixel_pointer += monitors[0].double_buffer_bpl;
  mouse_cursor_img_pointer += MOUSE_CURSOR_WIDTH*4;
 }
}

void remove_mouse_from_screen_double_buffer(void) {
 //initalize variables
 dword_t mouse_height = MOUSE_CURSOR_HEIGHT;
 if((mouse_cursor_y+MOUSE_CURSOR_HEIGHT)>monitors[0].height) {
  mouse_height = (monitors[0].height-mouse_cursor_y);
 }
 dword_t mouse_width = MOUSE_CURSOR_WIDTH;
 if((mouse_cursor_x+MOUSE_CURSOR_WIDTH)>monitors[0].width) {
  mouse_width = (monitors[0].width-mouse_cursor_x);
 }
 dword_t *screen;
 dword_t *mouse_cursor_background_ptr = (dword_t *) (mouse_cursor_background);
 dword_t first_line_pixel_pointer = ((dword_t)monitors[0].double_buffer + (mouse_cursor_y*monitors[0].double_buffer_bpl) + (mouse_cursor_x*4));
 
 //restore background of mouse
 for(dword_t i=0; i<mouse_height; i++) {
  screen = (dword_t *) first_line_pixel_pointer;
  for(dword_t j=0; j<mouse_width; j++) {
   *screen = *mouse_cursor_background_ptr;
   screen++;
   mouse_cursor_background_ptr++;
  }
  first_line_pixel_pointer += monitors[0].double_buffer_bpl;
 }
}

void screen_save_variables(void) {
 save_screen_double_buffer_memory_pointer = monitors[0].double_buffer;
 save_screen_width = monitors[0].width;
 save_screen_height = monitors[0].height;
 save_screen_double_buffer_bytes_per_line = monitors[0].double_buffer_bpl;
}

void screen_restore_variables(void) {
 monitors[0].double_buffer = save_screen_double_buffer_memory_pointer;
 monitors[0].width = save_screen_width;
 monitors[0].height = save_screen_height;
 monitors[0].double_buffer_bpl = save_screen_double_buffer_bytes_per_line;
}