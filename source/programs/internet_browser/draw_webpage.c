//BleskOS

/*
* MIT License
* Copyright (c) 2023-2024 Vendelín Slezák
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

void internet_browser_redraw_webpage(dword_t webpage_mem) {
 word_t *webpage = (word_t *) webpage_mem;
 dword_t *webpage32;
 
 //set starting variables
 ib_draw_column = 0;
 ib_draw_line = 0;
 ib_font_size = 10;
 ib_font_color = BLACK;
 ib_font_emphasis = SF_EMPHASIS_NONE;
 ib_line_size = 10;
 
 webpage_element_visibility = STATUS_TRUE;
 webpage_url_pointer_mem = 0;
 
 //set font
 set_pen_width(1);
 set_scalable_char_size(ib_font_size);

 //clear click board
 add_zone_to_click_board(0, INTERNET_BROWSER_WEBPAGE_START_LINE, internet_browser_webpage_width, internet_browser_webpage_height, 0);
 
 //draw webpage
 while(*webpage!=0) {
  //skip unprintable characters
  if(*webpage<' ') {
   webpage++;
   continue;
  }
 
  //we will print only characters to 512, so anything above have to be some command (0xFFF0-0xFFF8)
  if(*webpage>512) {
   if(*webpage==WEBPAGE_COMMAND_MOVE_DRAW_POSITION) {
    webpage32 = (dword_t *) ((dword_t)webpage+WEBPAGE_COMMAND_MOVE_DRAW_POSITION_BYTE_OFFSET_COLUMN);
    ib_draw_column = *webpage32;
    webpage32 = (dword_t *) ((dword_t)webpage+WEBPAGE_COMMAND_MOVE_DRAW_POSITION_BYTE_OFFSET_LINE);
    ib_draw_line = *webpage32;
    
    webpage_element_visibility = STATUS_TRUE;
    if(ib_draw_line>internet_browser_last_show_line) {
     webpage_element_visibility = STATUS_FALSE; //not in screen
    }
    if(ib_draw_column>internet_browser_last_show_column) {
     webpage_element_visibility = STATUS_FALSE; //not in screen
    }  
  
    webpage+=5;
    continue;
   }
   else if(*webpage==WEBPAGE_COMMAND_MOVE_TO_LINE) {    
    webpage32 = (dword_t *) ((dword_t)webpage+WEBPAGE_COMMAND_MOVE_TO_LINE_BYTE_OFFSET_COLUMN);
    ib_draw_column = *webpage32;
    webpage32 = (dword_t *) ((dword_t)webpage+WEBPAGE_COMMAND_MOVE_TO_LINE_BYTE_OFFSET_LINE);
    ib_draw_line = *webpage32;
    ib_line_size = webpage[WEBPAGE_COMMAND_MOVE_TO_LINE_OFFSET_LINE_SIZE];
    
    webpage_element_visibility = STATUS_TRUE;
    if(ib_draw_line>internet_browser_last_show_line) {
     webpage_element_visibility = STATUS_FALSE; //not in screen
    }
    if(ib_draw_column>internet_browser_last_show_column) {
     webpage_element_visibility = STATUS_FALSE; //not in screen
    }  
  
    webpage+=6;
    continue;
   }
   else if(*webpage==WEBPAGE_COMMAND_CHANGE_TEXT_SIZE) {
    ib_font_size = webpage[WEBPAGE_COMMAND_CHANGE_TEXT_SIZE_OFFSET_TEXT_SIZE];
    set_scalable_char_size(ib_font_size);
  
    webpage+=2;
    continue;
   }
   else if(*webpage==WEBPAGE_COMMAND_CHANGE_TEXT_EMPHASIS) {
    ib_font_emphasis = webpage[WEBPAGE_COMMAND_CHANGE_TEXT_EMPHASIS_OFFSET_TEXT_EMPHASIS];
    scalable_font_char_emphasis = ib_font_emphasis;
  
    webpage+=2;
    continue;
   }
   else if(*webpage==WEBPAGE_COMMAND_CHANGE_TEXT_COLOR) {
    webpage32 = (dword_t *) ((dword_t)webpage+WEBPAGE_COMMAND_CHANGE_TEXT_COLOR_BYTE_OFFSET_COLOR);
    ib_font_color = *webpage32;
  
    webpage+=3;
    continue;
   }
   else if(*webpage==WEBPAGE_COMMAND_DRAW_BOX) {
    //draw top border
    if(webpage[WEBPAGE_COMMAND_DRAW_BOX_OFFSET_TOP_BORDER_SIZE]!=0) {
     if(webpage[WEBPAGE_COMMAND_DRAW_BOX_OFFSET_TOP_BORDER_TYPE]==WEBPAGE_BORDER_TYPE_SOLID) {
      internet_browser_calculate_draw_square(ib_draw_column, ib_draw_line, webpage[WEBPAGE_COMMAND_DRAW_BOX_OFFSET_LEFT_BORDER_SIZE]+webpage[WEBPAGE_COMMAND_DRAW_BOX_OFFSET_WIDTH]+webpage[WEBPAGE_COMMAND_DRAW_BOX_OFFSET_RIGHT_BORDER_SIZE], webpage[WEBPAGE_COMMAND_DRAW_BOX_OFFSET_TOP_BORDER_SIZE]);
      if(ib_square_height!=0) {
       webpage32 = (dword_t *) ((dword_t)webpage+WEBPAGE_COMMAND_DRAW_BOX_BYTE_OFFSET_TOP_BORDER_COLOR);
       draw_full_square(ib_square_x, ib_square_y, ib_square_width, ib_square_height, *webpage32);
      }
     }
     //TODO: more types of border
    }
    
    //draw bottom border
    if(webpage[WEBPAGE_COMMAND_DRAW_BOX_OFFSET_BOTTOM_BORDER_SIZE]!=0) {
     if(webpage[WEBPAGE_COMMAND_DRAW_BOX_OFFSET_BOTTOM_BORDER_TYPE]==WEBPAGE_BORDER_TYPE_SOLID) {
      internet_browser_calculate_draw_square(ib_draw_column, ib_draw_line+webpage[WEBPAGE_COMMAND_DRAW_BOX_OFFSET_TOP_BORDER_SIZE]+webpage[WEBPAGE_COMMAND_DRAW_BOX_OFFSET_HEIGTH], webpage[WEBPAGE_COMMAND_DRAW_BOX_OFFSET_LEFT_BORDER_SIZE]+webpage[WEBPAGE_COMMAND_DRAW_BOX_OFFSET_WIDTH]+webpage[WEBPAGE_COMMAND_DRAW_BOX_OFFSET_RIGHT_BORDER_SIZE], webpage[WEBPAGE_COMMAND_DRAW_BOX_OFFSET_BOTTOM_BORDER_SIZE]);
      if(ib_square_height!=0) {
       webpage32 = (dword_t *) ((dword_t)webpage+WEBPAGE_COMMAND_DRAW_BOX_BYTE_OFFSET_BOTTOM_BORDER_COLOR);
       draw_full_square(ib_square_x, ib_square_y, ib_square_width, ib_square_height, *webpage32);
      }
     }
     //TODO: more types of border
    }
    
    //draw left border
    if(webpage[WEBPAGE_COMMAND_DRAW_BOX_OFFSET_LEFT_BORDER_SIZE]!=0) {
     if(webpage[WEBPAGE_COMMAND_DRAW_BOX_OFFSET_LEFT_BORDER_TYPE]==WEBPAGE_BORDER_TYPE_SOLID) {
      internet_browser_calculate_draw_square(ib_draw_column, ib_draw_line, webpage[WEBPAGE_COMMAND_DRAW_BOX_OFFSET_LEFT_BORDER_SIZE], webpage[WEBPAGE_COMMAND_DRAW_BOX_OFFSET_TOP_BORDER_SIZE]+webpage[WEBPAGE_COMMAND_DRAW_BOX_OFFSET_HEIGTH]+webpage[WEBPAGE_COMMAND_DRAW_BOX_OFFSET_BOTTOM_BORDER_SIZE]);
      if(ib_square_height!=0) {
       webpage32 = (dword_t *) ((dword_t)webpage+WEBPAGE_COMMAND_DRAW_BOX_BYTE_OFFSET_LEFT_BORDER_COLOR);
       draw_full_square(ib_square_x, ib_square_y, ib_square_width, ib_square_height, *webpage32);
      }
     }
     //TODO: more types of border
    }
    
    //draw right border
    if(webpage[WEBPAGE_COMMAND_DRAW_BOX_OFFSET_RIGHT_BORDER_SIZE]!=0) {
     if(webpage[WEBPAGE_COMMAND_DRAW_BOX_OFFSET_RIGHT_BORDER_TYPE]==WEBPAGE_BORDER_TYPE_SOLID) {
      internet_browser_calculate_draw_square(ib_draw_column+webpage[WEBPAGE_COMMAND_DRAW_BOX_OFFSET_LEFT_BORDER_SIZE]+webpage[WEBPAGE_COMMAND_DRAW_BOX_OFFSET_WIDTH], ib_draw_line, webpage[WEBPAGE_COMMAND_DRAW_BOX_OFFSET_RIGHT_BORDER_SIZE], webpage[WEBPAGE_COMMAND_DRAW_BOX_OFFSET_TOP_BORDER_SIZE]+webpage[WEBPAGE_COMMAND_DRAW_BOX_OFFSET_HEIGTH]+webpage[WEBPAGE_COMMAND_DRAW_BOX_OFFSET_BOTTOM_BORDER_SIZE]);
      if(ib_square_height!=0) {
       webpage32 = (dword_t *) ((dword_t)webpage+WEBPAGE_COMMAND_DRAW_BOX_BYTE_OFFSET_RIGHT_BORDER_COLOR);
       draw_full_square(ib_square_x, ib_square_y, ib_square_width, ib_square_height, *webpage32);
      }
     }
     //TODO: more types of border
    }
    
    //skip border
    ib_draw_column += webpage[WEBPAGE_COMMAND_DRAW_BOX_OFFSET_LEFT_BORDER_SIZE];
    ib_draw_line += webpage[WEBPAGE_COMMAND_DRAW_BOX_OFFSET_TOP_BORDER_SIZE];
    
    //draw background
    internet_browser_calculate_draw_square(ib_draw_column, ib_draw_line, webpage[WEBPAGE_COMMAND_DRAW_BOX_OFFSET_WIDTH], webpage[WEBPAGE_COMMAND_DRAW_BOX_OFFSET_HEIGTH]);
    if(ib_square_height!=0) {
     webpage32 = (dword_t *) ((dword_t)webpage+WEBPAGE_COMMAND_DRAW_BOX_BYTE_OFFSET_BACKGROUND_COLOR);
     if(*webpage32!=WEBPAGE_EMPTY_ENTRY_32) {
      draw_full_square(ib_square_x, ib_square_y, ib_square_width, ib_square_height, *webpage32);
     }
     
     //TODO: background with image
    }
    
    webpage+=21;
    continue;
   }
   else if(*webpage==WEBPAGE_COMMAND_DRAW_BORDER_LINE) {
    if(webpage[WEBPAGE_COMMAND_DRAW_BORDER_LINE_OFFSET_BORDER_TYPE]==WEBPAGE_BORDER_TYPE_SOLID) {
     internet_browser_calculate_draw_square(ib_draw_column, ib_draw_line, webpage[WEBPAGE_COMMAND_DRAW_BORDER_LINE_OFFSET_BORDER_WIDTH], webpage[WEBPAGE_COMMAND_DRAW_BORDER_LINE_OFFSET_BORDER_HEIGTH]);
     if(ib_square_height!=0) {
      webpage32 = (dword_t *) ((dword_t)webpage+WEBPAGE_COMMAND_DRAW_BORDER_LINE_BYTE_OFFSET_BORDER_COLOR);
      draw_full_square(ib_square_x, ib_square_y, ib_square_width, ib_square_height, *webpage32);
     }
    }
    //TODO: more types of border
  
    webpage+=6;
    continue;
   }
   else if(*webpage==WEBPAGE_COMMAND_DRAW_IMAGE) {
    //TODO:
    webpage+=1;
    continue;
   }
   else if(*webpage==WEBPAGE_COMMAND_DRAW_REPEATING_IMAGE) {
    //TODO:
    webpage+=1;
    continue;
   }
   else if(*webpage==WEBPAGE_COMMAND_URL) {
    webpage32 = (dword_t *) ((dword_t)webpage+WEBPAGE_COMMAND_URL_BYTE_OFFSET_POINTER);
    webpage_url_pointer_mem = *webpage32;
  
    webpage+=3;
    continue;
   }
  }
  
  //draw char
  if(webpage_element_visibility==STATUS_TRUE) {   
   internet_browser_calculate_draw_square(ib_draw_column, ib_draw_line+ib_line_size-ib_font_size, ib_font_size, ib_font_size+ib_font_size/2);
   if(ib_square_width==ib_font_size && ib_square_height==ib_font_size+ib_font_size/2) {
    draw_scalable_char(*webpage, ib_square_x, ib_square_y, ib_font_color);

    add_zone_to_click_board(ib_square_x, ib_square_y, ib_square_width, ib_square_height, webpage_url_pointer_mem);
   }
   else if(ib_square_width!=0 && ib_square_height!=0) {
    draw_part_of_scalable_char(*webpage, ib_square_x, ib_square_y, ib_square_draw_column, ib_square_draw_line, ib_square_width, ib_square_height, ib_font_color);
   
    add_zone_to_click_board(ib_square_x, ib_square_y, ib_square_width, ib_square_height, webpage_url_pointer_mem);
   }
   if(ib_square_width==0 || ib_square_height==0) {
    webpage_element_visibility = STATUS_FALSE;
   }
  }
  ib_draw_column += ib_font_size;
  
  //next char
  webpage++;
 }

}

void internet_browser_calculate_draw_square(dword_t column, dword_t line, dword_t width, dword_t height) {
 ib_square_x = 0;
 ib_square_y = 0;
 ib_square_width = 0;
 ib_square_height = 0;
 ib_square_draw_column = 0;
 ib_square_draw_line = 0;
 
 //test if something of square is on screen
 if(line>internet_browser_last_show_line) {
  return; //not in screen
 }
 else if((line+height)<internet_browser_first_show_line) {
  return; //not in screen
 }
 if(column>internet_browser_last_show_column) {
  return; //not in screen
 }
 else if((column+width)<internet_browser_first_show_column) {
  return; //not in screen
 }
 
 //calculate Y related variables
 if(line>internet_browser_first_show_line) {
  ib_square_y = INTERNET_BROWSER_WEBPAGE_START_LINE+(line-internet_browser_first_show_line);
  ib_square_draw_line = 0;
  if((line+height)<=internet_browser_last_show_line) {
   ib_square_height = height;
  }
  else {
   ib_square_height = (height-(line+height-internet_browser_last_show_line));
  }
 }
 else {
  ib_square_y = INTERNET_BROWSER_WEBPAGE_START_LINE;
  ib_square_draw_line = (height-(line+height-internet_browser_first_show_line));
  if((height-ib_square_draw_line)>=internet_browser_webpage_height) {
   ib_square_height = internet_browser_webpage_height;
  }
  else {
   ib_square_height = (line+height-internet_browser_first_show_line);
  }
 }
 
 //calculate X realted variables
 if(column>internet_browser_first_show_column) {
  ib_square_x = (column-internet_browser_first_show_column);
  ib_square_draw_column = 0;
  if((column+width)<=internet_browser_last_show_column) {
   ib_square_width = width;
  }
  else {
   ib_square_width = (width-(column+width-internet_browser_last_show_column));
  }
 }
 else {
  ib_square_x = 0;
  ib_square_width = (column+width-internet_browser_first_show_column);
  ib_square_draw_column = (width-ib_square_width);
 }
}
