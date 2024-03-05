//BleskOS

/*
* MIT License
* Copyright (c) 2023-2024 Vendelín Slezák
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

void clear_screen(dword_t color) {
 dword_t *screen = (dword_t *) (screen_double_buffer_memory_pointer);
 
 for(int i=0; i<(screen_width*screen_height); i++) {
  *screen = color;
  screen++;
 }
}

void set_pen_width(dword_t width, dword_t color) {
 if(pen_width==width) {
  return;
 }
 
 pen_width = width;
 
 if(pen_img_pointer!=0) {
  free(pen_img_pointer);
 }
 pen_img_pointer = malloc(width*width*4);
 
 if(pen_width==0 || pen_width==1) {
  return;
 }

 //clear memory
 dword_t *pen_memory = (dword_t *) pen_img_pointer;
 for(int i=0; i<(width*width); i++) {
  *pen_memory = TRANSPARENT_COLOR;
  pen_memory++;
 }
 
 set_pixel_shape_circle(color);
}

void set_pixel_shape_circle(dword_t color) {
 dword_t save_lfb, save_x, save_y, save_bpl, width;
 
 //clear memory
 dword_t *pen_memory = (dword_t *) pen_img_pointer;
 for(int i=0; i<(pen_width*pen_width); i++) {
  *pen_memory = TRANSPARENT_COLOR;
  pen_memory++;
 }
 
 //set width 2 manually
 pen_memory = (dword_t *) pen_img_pointer;
 if(pen_width==2) {
  pen_memory = (dword_t *) pen_img_pointer;
  pen_memory[0] = BLACK;
  pen_memory[1] = BLACK;
  pen_memory[2] = BLACK;
  pen_memory[3] = BLACK;
  return;
 }
 
 //save screen variables
 save_lfb = (dword_t) screen_double_buffer_memory_pointer;
 save_x = screen_width;
 save_y = screen_height;
 save_bpl = screen_double_buffer_bytes_per_line;
 width = pen_width;

 //draw circle to pen image
 screen_double_buffer_memory_pointer = (byte_t *) pen_img_pointer;
 screen_width = pen_width;
 screen_height = width;
 screen_double_buffer_bytes_per_line = (width*4);
 pen_width = 1;
 draw_full_circle((width/2), (width/2), ((width-1)/2), color);
 if((width & 0x1) == 0) {
  draw_full_circle((width/2)-1, (width/2), ((width-1)/2), color);
  draw_full_circle((width/2), (width/2)-1, ((width-1)/2), color);
  draw_full_circle((width/2)-1, (width/2)-1, ((width-1)/2), color);
 }
 pen_width = width;
 
 //restore screen variables
 screen_double_buffer_memory_pointer = (byte_t *) save_lfb;
 screen_width = save_x;
 screen_height = save_y;
 screen_double_buffer_bytes_per_line = save_bpl;
}

void set_pixel_shape_square(dword_t color) {
 dword_t *pen_memory = (dword_t *) pen_img_pointer;
 for(int i=0; i<(pen_width*pen_width); i++) {
  *pen_memory = BLACK;
  pen_memory++;
 }
}

void draw_pixel(dword_t x, dword_t y, dword_t color) {
 dword_t *screen = (dword_t *) ((dword_t)screen_double_buffer_memory_pointer + (y*screen_double_buffer_bytes_per_line) + (x<<2));
 
 if(pen_width==1 || pen_width==0) {
  if(x<screen_width && y<screen_height) {
   *screen = color;
  }
  return;
 }
 
 x = (x-(pen_width/2)) + ~(pen_width | 0xFFFFFFFE);
 y = (y-(pen_width/2)) + ~(pen_width | 0xFFFFFFFE);
 dword_t first_line_pixel_pointer = ((dword_t)screen_double_buffer_memory_pointer + (y*screen_double_buffer_bytes_per_line) + (x<<2));
 dword_t old_x = x;
 dword_t *pen_img = (dword_t *) pen_img_pointer;
 for(int i=0; i<pen_width; i++) {
  screen = (dword_t *) first_line_pixel_pointer;
  for(int j=0; j<pen_width; j++) {
   if(*pen_img!=TRANSPARENT_COLOR && x<screen_width && y<screen_height) {
    *screen = color;
   }
   screen++;
   pen_img++;
   x++;
  }
  first_line_pixel_pointer += screen_double_buffer_bytes_per_line;
  y++;
  x = old_x;
 }
}

void draw_straigth_line(dword_t x, dword_t y, dword_t length, dword_t color) {
 for(int i=0; i<length; i++) {
  draw_pixel(x, y, color);
  x++;
 }
}

void draw_straigth_column(dword_t x, dword_t y, dword_t height, dword_t color) {
 for(int i=0; i<height; i++) {
  draw_pixel(x, y, color);
  y++;
 }
}

void draw_line(int x0, int y0, int x1, int y1, dword_t color) {
 int dx =  abs(x1-x0), sx = (x0<x1 ? 1 : -1), dy = -abs(y1-y0), sy = (y0<y1 ? 1 : -1), err = (dx+dy), e2;
 
 while(1) {
  draw_pixel(x0,y0, color);
  if (x0==x1 && y0==y1) {
   return;
  }
  if(x0>screen_width || y0>screen_height) {
   return;
  }
 
  e2 = 2*err;
  if (e2 >= dy) { 
   err += dy; 
   x0 += sx; 
  }
  if (e2 <= dx) {
   err += dx;
   y0 += sy;
  }
 }
}

void plotQuadBezierSeg(int x0, int y0, int x1, int y1, int x2, int y2, dword_t color) {                            
  int sx = x2-x1, sy = y2-y1;
  long long xx = x0-x1, yy = y0-y1, xy;         /* relative values for checks */
  double dx, dy, err, cur = xx*sy-yy*sx;                    /* curvature */

  if (sx*(long)sx+sy*(long)sy > xx*xx+yy*yy) { /* begin with longer part */ 
    x2 = x0; x0 = sx+x1; y2 = y0; y0 = sy+y1; cur = -cur;  /* swap P0 P2 */
  }  
  if (cur != 0) {                                    /* no straight line */
    xx += sx; xx *= sx = x0 < x2 ? 1 : -1;           /* x step direction */
    yy += sy; yy *= sy = y0 < y2 ? 1 : -1;           /* y step direction */
    xy = 2*xx*yy; xx *= xx; yy *= yy;          /* differences 2nd degree */
    if (cur*sx*sy < 0) {                           /* negated curvature? */
      xx = -xx; yy = -yy; xy = -xy; cur = -cur;
    }
    dx = 4.0*sy*cur*(x1-x0)+xx-xy;             /* differences 1st degree */
    dy = 4.0*sx*cur*(y0-y1)+yy-xy;
    xx += xx; yy += yy; err = dx+dy+xy;                /* error 1st step */
    dword_t cycles = 0;   
    do {                              
      draw_pixel(x0,y0, color);                                     /* plot curve */
      if (x0 == x2 && y0 == y2) return;  /* last pixel -> curve finished */
      y1 = 2*err < dx;                  /* save value for test of y step */
      if (2*err > dy) { x0 += sx; dx -= xy; err += dy += yy; } /* x step */
      if (    y1    ) { y0 += sy; dy -= xy; err += dx += xx; } /* y step */
      cycles++;
    } while (dy < dx && cycles<10000);           /* gradient negates -> algorithm fails */
  }
  draw_line(x0, y0, x2, y2, color);                  /* plot remaining part to end */
}

void draw_quadratic_bezier(int x0, int y0, int x1, int y1, int x2, int y2, dword_t color) {
 int x = x0-x1, y = y0-y1;
 double t = x0-2*x1+x2, r;
 
 if ((long)x*(x2-x1) > 0) {
  // if ((long)y*(y2-y1) > 0) {
   if (fabs((y0-2*y1+y2)/t*x) > abs(y)) {           
    x0 = x2;
    x2 = x+x1;
    y0 = y2;
    y2 = y+y1;
   }
  // }

   t = (x0-x1)/t;
   r = (1-t)*((1-t)*y0+2.0*t*y1)+t*t*y2;
   t = (x0*x2-x1*x1)*t/(x0-x1);
   x = floor(t+0.5);
   y = floor(r+0.5);
   r = (y1-y0)*(t-x0)/(x1-x0)+y0;
   plotQuadBezierSeg(x0,y0, x,floor(r+0.5), x,y, color);
   r = (y1-y2)*(t-x2)/(x1-x2)+y2;
   x0 = x1 = x;
   y0 = y;
   y1 = floor(r+0.5);
 }
 
 if ((long)(y0-y1)*(y2-y1) > 0) {
  t = y0-2*y1+y2;
  t = (y0-y1)/t;
  r = (1-t)*((1-t)*x0+2.0*t*x1)+t*t*x2;
  t = (y0*y2-y1*y1)*t/(y0-y1);
  x = floor(r+0.5);
  y = floor(t+0.5);
  r = (x1-x0)*(t-y0)/(y1-y0)+x0;
  plotQuadBezierSeg(x0,y0, floor(r+0.5),y, x,y, color);
  r = (x1-x2)*(t-y2)/(y1-y2)+x2;
  x0 = x;
  x1 = floor(r+0.5);
  y0 = y1 = y;
 }
 
 plotQuadBezierSeg(x0, y0, x1, y1, x2, y2, color);
}

void draw_empty_square(dword_t x, dword_t y, dword_t width, dword_t height, dword_t color) {
 draw_straigth_line(x, y, width, color);
 draw_straigth_column(x, y, height, color);
 draw_straigth_line(x, y+height-1, width, color);
 draw_straigth_column(x+width-1, y, height, color);
}

void draw_full_square(dword_t x, dword_t y, dword_t width, dword_t height, dword_t color) {
 dword_t first_line_pixel_pointer = ((dword_t)screen_double_buffer_memory_pointer + (y*screen_double_buffer_bytes_per_line) + (x<<2));
 dword_t *screen = (dword_t *) first_line_pixel_pointer;
 
 for(int i=0; i<height; i++) {
  for(int j=0; j<width; j++) {
   *screen = color;
   screen++;
  }
  
  first_line_pixel_pointer += screen_double_buffer_bytes_per_line;
  screen = (dword_t *) first_line_pixel_pointer;
 }
}

void draw_empty_circle_point(dword_t x, dword_t y, dword_t color) {
 draw_pixel(draw_x+x, draw_y+y, color);
 draw_pixel(draw_x+x, draw_y-y, color);
 draw_pixel(draw_x-x, draw_y+y, color);
 draw_pixel(draw_x-x, draw_y-y, color);
 draw_pixel(draw_x+y, draw_y+x, color);
 draw_pixel(draw_x+y, draw_y-x, color);
 draw_pixel(draw_x-y, draw_y+x, color);
 draw_pixel(draw_x-y, draw_y-x, color);
}

void draw_empty_circle(dword_t x, dword_t y, dword_t radius, dword_t color) {
 long z, z_right, z_right_down;

 draw_x = x;
 draw_y = y;
 x = 0;
 y = radius;
 z = 1 - radius;
 z_right = 3;
 z_right_down = ((-2*radius) + 5);
 draw_empty_circle_point(x, y, color);
 
 while(x < y) {
  if(z < 0) {
   z += z_right;
   z_right += 2;
   z_right_down += 2;
   x++;
  }
  else {
   z += z_right_down;
   z_right += 2;
   z_right_down += 4;
   x++;
   y--;
  }
  
  draw_empty_circle_point(x, y, color);
 }
}

void draw_full_circle_line(dword_t x, dword_t y, dword_t color) {
 draw_straigth_line(draw_x-x, draw_y-y, (x<<1), color);
 draw_straigth_line(draw_x-x, draw_y+y, (x<<1), color);
 draw_straigth_line(draw_x-y, draw_y-x, (y<<1), color);
 draw_straigth_line(draw_x-y, draw_y+x, (y<<1), color);
}

void draw_full_circle(dword_t x, dword_t y, dword_t radius, dword_t color) {
 long z, z_right, z_right_down;

 draw_x = x;
 draw_y = y;
 x = 0;
 y = radius;
 z = 1 - radius;
 z_right = 3;
 z_right_down = ((-2*radius) + 5);
 draw_full_circle_line(x, y, color);
 
 while(x < y) {
  if(z < 0) {
   z += z_right;
   z_right += 2;
   z_right_down += 2;
   x++;
  }
  else {
   z += z_right_down;
   z_right += 2;
   z_right_down += 4;
   x++;
   y--;
  }
  
  draw_full_circle_line(x, y, color);
 }
}

void draw_empty_ellipse(int x0, int y0, int x1, int y1, dword_t color) {
 long long a = abs(x1-x0), b = abs(y1-y0), b1 = (b & 1);
 long long dx = 4*(1-a)*b*b, dy = 4*(b1+1)*a*a, err = dx+dy+b1*a*a, e2;

 if (x0 > x1) {
  x0 = x1;
  x1 += a;
 }
 if (y0 > y1) {
  y0 = y1;
 }
 y0 += (b+1)/2;
 y1 = y0-b1;
 a *= 8*a;
 b1 = 8*b*b;

 do {
  draw_pixel(x1, y0, color); /*   I. Quadrant */
  draw_pixel(x0, y0, color); /*  II. Quadrant */
  draw_pixel(x0, y1, color); /* III. Quadrant */
  draw_pixel(x1, y1, color); /*  IV. Quadrant */
  
  e2 = 2*err;
  if (e2 <= dy) { 
   y0++;
   y1--;
   err += dy += a;
  }  /* y step */ 
  if (e2 >= dx || 2*err > dy) {
   x0++;
   x1--;
   err += dx += b1;
  } /* x step */
 } while (x0 <= x1);
   
 while ((y0-y1) < b) {  /* too early stop of flat ellipses a=1 */
  draw_pixel(x0-1, y0, color); /* -> finish tip of ellipse */
  draw_pixel(x1+1, y0++, color); 
  draw_pixel(x0-1, y1, color);
  draw_pixel(x1+1, y1--, color); 
 }
}

void draw_parts_of_empty_ellipse(byte_t parts, int x0, int y0, int x1, int y1, dword_t color) {
 long long a = abs(x1-x0), b = abs(y1-y0), b1 = (b & 1);
 long long dx = 4*(1-a)*b*b, dy = 4*(b1+1)*a*a, err = dx+dy+b1*a*a, e2;

 if (x0 > x1) {
  x0 = x1;
  x1 += a;
 }
 if (y0 > y1) {
  y0 = y1;
 }
 y0 += (b+1)/2;
 y1 = y0-b1;
 a *= 8*a;
 b1 = 8*b*b;

 do {
  if((parts & 0x1)==0x1) {
  draw_pixel(x1, y0, color); /*   I. Quadrant */
  }
  if((parts & 0x2)==0x2) {
  draw_pixel(x0, y0, color); /*  II. Quadrant */
  }
  if((parts & 0x4)==0x4) {
  draw_pixel(x0, y1, color); /* III. Quadrant */
  }
  if((parts & 0x8)==0x8) {
  draw_pixel(x1, y1, color); /*  IV. Quadrant */
  }
  
  e2 = 2*err;
  if (e2 <= dy) { 
   y0++;
   y1--;
   err += dy += a;
  }  /* y step */ 
  if (e2 >= dx || 2*err > dy) {
   x0++;
   x1--;
   err += dx += b1;
  } /* x step */
 } while (x0 <= x1);
   
 /*while ((y0-y1) < b) {  // too early stop of flat ellipses a=1
  draw_pixel(x0-1, y0, color); // -> finish tip of ellipse
  draw_pixel(x1+1, y0++, color); 
  draw_pixel(x0-1, y1, color);
  draw_pixel(x1+1, y1--, color); 
 }*/
}

void draw_full_ellipse(int x0, int y0, int x1, int y1, dword_t color) {
 long long a = abs(x1-x0), b = abs(y1-y0), b1 = (b & 1);
 long long dx = 4*(1-a)*b*b, dy = 4*(b1+1)*a*a, err = dx+dy+b1*a*a, e2;

 if (x0 > x1) {
  x0 = x1;
  x1 += a;
 }
 if (y0 > y1) {
  y0 = y1;
 }
 y0 += (b+1)/2;
 y1 = y0-b1;
 a *= 8*a;
 b1 = 8*b*b;

 do {
  draw_straigth_line(x0, y1, (x1-x0), color);
  draw_straigth_line(x0, y0, (x1-x0), color);
  
  e2 = 2*err;
  if (e2 <= dy) { 
   y0++;
   y1--;
   err += dy += a;
  }  /* y step */ 
  if (e2 >= dx || 2*err > dy) {
   x0++;
   x1--;
   err += dx += b1;
  } /* x step */
 } while (x0 <= x1);
}

void fill_area(dword_t x, dword_t y, dword_t color) {
 dword_t *first_stack = (dword_t *) fill_first_stack;
 dword_t *second_stack = (dword_t *) fill_second_stack;
 dword_t *screen = (dword_t *) ((dword_t)screen_double_buffer_memory_pointer + (y*screen_double_buffer_bytes_per_line) + (x<<2));
 dword_t *screen_2;
 dword_t background_color;
 
 for(int i=0; i<((screen_width*2+screen_height*2)*8); i++) {
  first_stack[i]=0;
  second_stack[i]=0;
 }
 
 first_stack[0] = (dword_t) screen;
 first_stack[1] = ((y << 16) | (x));
 second_stack[0] = 0;
 
 //draw first pixel
 if(*screen==color) {
  return;
 }
 background_color = *screen;
 *screen = color;
 
 while(1) {
 
 first_stack = (dword_t *) fill_first_stack;
 second_stack = (dword_t *) fill_second_stack;
 second_stack[0] = 0;
 for(int i=0; i<(screen_width*2+screen_height*2); i++) {
  if(*first_stack==0) {
   if((dword_t) first_stack == (dword_t) fill_first_stack) {
    return;
   }
   break;
  }
  
  //load pixel values
  screen = (dword_t *) first_stack[0];
  x = (first_stack[1] & 0xFFFF);
  y = (first_stack[1] >> 16);
  
  //pixel above
  if(y > 0) {
   screen_2 = (dword_t *) screen;
   screen_2 -= screen_width;
   if(*screen_2==background_color) {
    *screen_2 = color;
    second_stack[0] = (dword_t) screen_2;
    second_stack[1] = (((y-1) << 16) | (x));
    second_stack += 2;
   }
  }
  
  //pixel below
  if(y < screen_height-1) {
   screen_2 = (dword_t *) screen;
   screen_2 += screen_width;
   if(*screen_2==background_color) {
    *screen_2 = color;
    second_stack[0] = (dword_t) screen_2;
    second_stack[1] = (((y+1) << 16) | (x));
    second_stack += 2;
   }
  }
  
  //pixel left
  if(x > 0) {
   screen_2 = (dword_t *) screen;
   screen_2--;
   if(*screen_2==background_color) {
    *screen_2 = color;
    second_stack[0] = (dword_t) screen_2;
    second_stack[1] = ((y << 16) | (x-1));
    second_stack += 2;
   }
  }
  
  //pixel right
  if(x < screen_width-1) {
   screen_2 = (dword_t *) screen;
   screen_2++;
   if(*screen_2==background_color) {
    *screen_2 = color;
    second_stack[0] = (dword_t) screen_2;
    second_stack[1] = ((y << 16) | (x+1));
    second_stack += 2;
   }
  }
  
  first_stack += 2;
 }
 second_stack[0]=0;
 
 first_stack = (dword_t *) fill_first_stack;
 second_stack = (dword_t *) fill_second_stack;
 first_stack[0] = 0;
 for(int i=0; i<(screen_width*2+screen_height*2); i++) {
  if(*second_stack==0) {
   if((dword_t) second_stack == (dword_t) fill_second_stack) {
    return;
   }
   break;
  }
  
  //load pixel values
  screen = (dword_t *) second_stack[0];
  x = (second_stack[1] & 0xFFFF);
  y = (second_stack[1] >> 16);
  
  //pixel above
  if(y > 0) {
   screen_2 = (dword_t *) screen;
   screen_2 -= screen_width;
   if(*screen_2==background_color) {
    *screen_2 = color;
    first_stack[0] = (dword_t) screen_2;
    first_stack[1] = (((y-1) << 16) | (x));
    first_stack += 2;
   }
  }
  
  //pixel below
  if(y < screen_height-1) {
   screen_2 = (dword_t *) screen;
   screen_2 += screen_width;
   if(*screen_2==background_color) {
    *screen_2 = color;
    first_stack[0] = (dword_t) screen_2;
    first_stack[1] = (((y+1) << 16) | (x));
    first_stack += 2;
   }
  }
  
  //pixel left
  if(x > 0) {
   screen_2 = (dword_t *) screen;
   screen_2--;
   if(*screen_2==background_color) {
    *screen_2 = color;
    first_stack[0] = (dword_t) screen_2;
    first_stack[1] = ((y << 16) | (x-1));
    first_stack += 2;
   }
  }
  
  //pixel right
  if(x < screen_width-1) {
   screen_2 = (dword_t *) screen;
   screen_2++;
   if(*screen_2==background_color) {
    *screen_2 = color;
    first_stack[0] = (dword_t) screen_2;
    first_stack[1] = ((y << 16) | (x+1));
    first_stack += 2;
   }
  }
  
  second_stack += 2;
 }
 first_stack[0]=0;
 
 }
}
