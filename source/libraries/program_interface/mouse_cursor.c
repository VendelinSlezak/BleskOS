//BleskOS

/*
* MIT License
* Copyright (c) 2023 Vendelín Slezák
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

void draw_mouse_cursor(dword_t x, dword_t y) {
 if(x>graphic_screen_x || y>graphic_screen_y) {
  return;
 }
 
 dword_t *screen;
 dword_t first_line_pixel_pointer = (screen_mem + (y*screen_bytes_per_line) + (x<<2));
 dword_t old_x = x;
 dword_t *mouse_cursor_img_ptr = (dword_t *) mouse_cursor_img;
 dword_t *mouse_cursor_background_ptr = (dword_t *) mouse_cursor_background;
 
 for(int i=0; i<MOUSE_CURSOR_HEIGTH; i++) {
  screen = (dword_t *) first_line_pixel_pointer;
  for(int j=0; j<MOUSE_CURSOR_WIDTH; j++) {
   if(*mouse_cursor_img_ptr!=TRANSPARENT_COLOR && x<graphic_screen_x && y<graphic_screen_y) {
    *mouse_cursor_background_ptr = *screen;
    *screen = *mouse_cursor_img_ptr;
   }
   screen++;
   mouse_cursor_img_ptr++;
   mouse_cursor_background_ptr++;
   x++;
  }
  first_line_pixel_pointer += screen_bytes_per_line;
  y++;
  x = old_x;
 }
}

void redraw_mouse_cursor(void) {
 redraw_part_of_screen(mouse_cursor_x, mouse_cursor_y, MOUSE_CURSOR_WIDTH, MOUSE_CURSOR_HEIGTH);
}

void mouse_cursor_restore_background(dword_t x, dword_t y) {
 if(x>graphic_screen_x || y>graphic_screen_y) {
  return;
 }
 
 dword_t *screen;
 dword_t first_line_pixel_pointer = (screen_mem + (y*screen_bytes_per_line) + (x<<2));
 dword_t old_x = x;
 dword_t *mouse_cursor_img_ptr = (dword_t *) mouse_cursor_img;
 dword_t *mouse_cursor_background_ptr = (dword_t *) mouse_cursor_background;
 
 for(int i=0; i<MOUSE_CURSOR_HEIGTH; i++) {
  screen = (dword_t *) first_line_pixel_pointer;
  for(int j=0; j<MOUSE_CURSOR_WIDTH; j++) {
   if(*mouse_cursor_img_ptr!=TRANSPARENT_COLOR && x<graphic_screen_x && y<graphic_screen_y) {
    *screen = *mouse_cursor_background_ptr;
   }
   screen++;
   mouse_cursor_img_ptr++;
   mouse_cursor_background_ptr++;
   x++;
  }
  first_line_pixel_pointer += screen_bytes_per_line;
  y++;
  x = old_x;
 }
}

void move_mouse_cursor(void) {
 dword_t old_mouse_cursor_x = mouse_cursor_x, old_mouse_cursor_y = mouse_cursor_y;
 
 //do not move if there was no movement
 if(mouse_movement_x==0 && mouse_movement_y==0) {
  return;
 }
 
 //remove cursor from screen
 mouse_cursor_restore_background(old_mouse_cursor_x, old_mouse_cursor_y);

 //X movement
 if(mouse_movement_x<0x80000000) {
  mouse_cursor_x += mouse_movement_x;
  if(mouse_cursor_x>=graphic_screen_x) {
   mouse_cursor_x = (graphic_screen_x-1);
  }
 }
 else {
  mouse_cursor_x -= ((0xFFFFFFFF-mouse_movement_x)+1);
  if(mouse_cursor_x>graphic_screen_x) {
   mouse_cursor_x = 0;
  }
 }
 mouse_movement_x = 0;
 
 //Y movement
 if(mouse_movement_y<0x80000000) {
  mouse_cursor_y += mouse_movement_y;
  if(mouse_cursor_y>=graphic_screen_y) {
   mouse_cursor_y = (graphic_screen_y-1);
  }
 }
 else {
  mouse_cursor_y -= ((0xFFFFFFFF-mouse_movement_y)+1);
  if(mouse_cursor_y>graphic_screen_y) {
   mouse_cursor_y = 0;
  }
 }
 mouse_movement_y = 0;
 
 //set mouse cursor variabiles
 if(mouse_drag_and_drop==MOUSE_NO_DRAG) {
  mouse_cursor_x_previous_dnd = mouse_cursor_x;
  mouse_cursor_y_previous_dnd = mouse_cursor_y;
  mouse_cursor_x_dnd = mouse_cursor_x;
  mouse_cursor_y_dnd = mouse_cursor_y;
 }
 else if(mouse_drag_and_drop==MOUSE_CLICK) {
  mouse_cursor_x_click = mouse_cursor_x;
  mouse_cursor_y_click = mouse_cursor_y;
  mouse_cursor_x_previous_dnd = mouse_cursor_x;
  mouse_cursor_y_previous_dnd = mouse_cursor_y;
  mouse_cursor_x_dnd = mouse_cursor_x;
  mouse_cursor_y_dnd = mouse_cursor_y;
 }
 else if(mouse_drag_and_drop==MOUSE_DRAG) {
  mouse_cursor_x_previous_dnd = mouse_cursor_x_dnd;
  mouse_cursor_y_previous_dnd = mouse_cursor_y_dnd;
  mouse_cursor_x_dnd = mouse_cursor_x;
  mouse_cursor_y_dnd = mouse_cursor_y;
 }
 
 // draw + save background cursor 
 draw_mouse_cursor(mouse_cursor_x, mouse_cursor_y);
 
 //show cursor change on screen
 redraw_part_of_screen(mouse_cursor_x, mouse_cursor_y, MOUSE_CURSOR_WIDTH, MOUSE_CURSOR_HEIGTH);
 redraw_part_of_screen(old_mouse_cursor_x, old_mouse_cursor_y, MOUSE_CURSOR_WIDTH, MOUSE_CURSOR_HEIGTH);
}

byte_t is_mouse_in_zone(dword_t up, dword_t down, dword_t left, dword_t right) {
 if(mouse_cursor_x>=left && mouse_cursor_x<=right && mouse_cursor_y>=up && mouse_cursor_y<=down) {
  return STATUS_TRUE;
 }
 else {
  return STATUS_FALSE;
 }
}

dword_t get_mouse_cursor_pixel_color(void) {
 dword_t *mouse_cursor_background_ptr = (dword_t *) mouse_cursor_background;
 return (*mouse_cursor_background_ptr);
}
