//BleskOS

/*
* MIT License
* Copyright (c) 2023-2024 Vendelín Slezák
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

/*
 Document Low Level Memory Format

 This format consists of chars with computed final position on page. It is last layer between document and output on screen.

 First 1000 entries of 8 bytes are for pages. First dword defines width of page and second height of page.

 Every char have this format:
 dword = char number
 dword = pointer to memory of DMF where is this char
 dword = column
 dword = line
 3 bytes = RGB color of char
 byte = char size
 3 bytes = RGB color of char background
 7 bits = emphasis
 1 bit = 0 - tranparent background 1 - color background

 Char number DLLMF_PAGE_CONTENT_END mean this is end of page. Immediately after it continues content of next page.
 Char number DLLMF_DOCUMENT_CONTENT_END mean this is end of document and points to end of DMF.
*/

/*
 Example document with char 'A' printed on first page

 ;page entries
 dd 500
 dd 800
 times 999 dq 0

 ;document data
 dd 'A'
 dd 0x400036
 dd 20
 dd 20
 dd (10<<24) | (0x000000)
 dd (0x00<<24) | (0x000000)

 ;end of document
 dd DLLMF_DOCUMENT_CONTENT_END
 dd 0x400038
 dd 30
 dd 20
 dd (10<<24) | (0x000000)
 dd (0x00<<24) | (0x000000)
*/

void initalize_dllmf(void) {
 dllmf_screen_first_column = 0;
 dllmf_screen_first_line = PROGRAM_INTERFACE_TOP_LINE_HEIGHT;
 dllmf_draw_width = screen_width;
 dllmf_draw_height = screen_height-PROGRAM_INTERFACE_TOP_LINE_HEIGHT-PROGRAM_INTERFACE_BOTTOM_LINE_HEIGHT;

 dllmf_draw_first_line = 0;
 dllmf_draw_last_line = (dllmf_draw_first_line+dllmf_draw_height);
 dllmf_draw_first_column = 0;
 dllmf_draw_last_column = (dllmf_draw_first_column+dllmf_draw_width);

 dllmf_selected_area = 0;
}

void draw_dllmf(dword_t dllmf_mem) {
 dword_t *page_entries = (dword_t *) (dllmf_mem);
 dword_t *document_data = (dword_t *) (dllmf_mem+DLLMF_NUM_OF_PAGE_ENTRIES*DLLMF_PAGE_ENTRY_SIZE);
 byte_t *document_data8 = (byte_t *) (dllmf_mem+DLLMF_NUM_OF_PAGE_ENTRIES*DLLMF_PAGE_ENTRY_SIZE);
 dword_t page_first_column = 0, page_first_line = DLLMF_SPACE_BETWEEN_DOCUMENTS, page_screen_width = 0, page_screen_height = 0;

 //draw pages
 for(dword_t i=0; i<DLLMF_NUM_OF_PAGE_ENTRIES; i++, page_entries+=2) {
  if(*page_entries==0) { //no more pages
   break;
  }

  //center page
  if(page_entries[DLLMF_PAGE_ENTRY_WIDTH_OFFSET]<(dllmf_draw_width-DLLMF_SPACE_BETWEEN_DOCUMENTS*2)) {
   page_first_column = (dllmf_screen_first_column+(dllmf_draw_width/2)-(page_entries[DLLMF_PAGE_ENTRY_WIDTH_OFFSET]/2));
  }
  else {
   page_first_column = (dllmf_screen_first_column+DLLMF_SPACE_BETWEEN_DOCUMENTS);
  }

  //draw page background
  dllmf_calculate_draw_square(page_first_column, page_first_line, page_entries[DLLMF_PAGE_ENTRY_WIDTH_OFFSET], page_entries[DLLMF_PAGE_ENTRY_HEIGHT_OFFSET]);
  if(dllmf_square_height!=0) {
   draw_full_square(dllmf_square_x, dllmf_square_y, dllmf_square_width, dllmf_square_height, WHITE);
  }

  //draw page content
  while(1) {
   //basic test if we need to draw this char
   dword_t char_size_with_spacing = (document_data8[DLLMF_CHAR_ENTRY_SIZE_OFFSET]+(document_data8[DLLMF_CHAR_ENTRY_SIZE_OFFSET]/2));
   if((page_first_line+document_data[DLLMF_CHAR_ENTRY_LINE_OFFSET])>dllmf_draw_last_line) {
    return;
   }
   else if((page_first_line+document_data[DLLMF_CHAR_ENTRY_LINE_OFFSET]+char_size_with_spacing)<dllmf_draw_first_line) {
    goto move_to_next_char;
   }

   //draw char entry
   dllmf_calculate_draw_square(page_first_column+document_data[DLLMF_CHAR_ENTRY_COLUMN_OFFSET], page_first_line+document_data[DLLMF_CHAR_ENTRY_LINE_OFFSET], document_data8[DLLMF_CHAR_ENTRY_SIZE_OFFSET], char_size_with_spacing);
   if(document_data[DLLMF_CHAR_ENTRY_CHAR_NUMBER_OFFSET]>=32) { //do not print unprintable characters
    //set size
    set_scalable_char_size(document_data8[DLLMF_CHAR_ENTRY_SIZE_OFFSET]);

    //set emphasis
    scalable_font_char_emphasis = (document_data8[DLLMF_CHAR_ENTRY_EMPHASIS_OFFSET] & 0x7F);

    //draw background
    if((document_data8[DLLMF_CHAR_ENTRY_EMPHASIS_OFFSET] & 0x80)==0x80) {
     draw_full_square(dllmf_square_x, dllmf_square_y, dllmf_square_width+1, dllmf_square_height, document_data[DLLMF_CHAR_ENTRY_BACKGROUND_COLOR_OFFSET]);
    }

    //draw background if this char is selected
    if(dllmf_selected_area!=0) { 
     if(dllmf_selected_area<dllmf_cursor) {
      if(((dword_t)document_data)>=dllmf_selected_area && ((dword_t)document_data)<dllmf_cursor) {
       draw_full_square(dllmf_square_x, dllmf_square_y, dllmf_square_width+1, dllmf_square_height, 0x00A0FF);
      }
     }
     else if(((dword_t)document_data)>=dllmf_cursor && ((dword_t)document_data)<dllmf_selected_area) {
      draw_full_square(dllmf_square_x, dllmf_square_y, dllmf_square_width+1, dllmf_square_height, 0x00A0FF);
     }
    }

    //draw char
    if(dllmf_square_width==document_data8[DLLMF_CHAR_ENTRY_SIZE_OFFSET] && dllmf_square_height==document_data8[DLLMF_CHAR_ENTRY_SIZE_OFFSET]) { 
     draw_scalable_char((document_data[DLLMF_CHAR_ENTRY_CHAR_NUMBER_OFFSET] & 0xFFFF), dllmf_square_x, dllmf_square_y, (document_data[DLLMF_CHAR_ENTRY_COLOR_OFFSET] & 0xFFFFFF));
    }
    else {
     draw_part_of_scalable_char((document_data[DLLMF_CHAR_ENTRY_CHAR_NUMBER_OFFSET] & 0xFFFF), dllmf_square_x, dllmf_square_y, dllmf_square_draw_column, dllmf_square_draw_line, dllmf_square_width, dllmf_square_height, (document_data[DLLMF_CHAR_ENTRY_COLOR_OFFSET] & 0xFFFFFF));
    }
   }

   //draw cursor
   dword_t cursor_draw_height = dllmf_square_height;
   if(cursor_draw_height>(document_data8[DLLMF_CHAR_ENTRY_SIZE_OFFSET]+2)) {
    cursor_draw_height = (document_data8[DLLMF_CHAR_ENTRY_SIZE_OFFSET]+2);
   }
   if(((dword_t)document_data)==dllmf_cursor) {
    draw_straigth_column(dllmf_square_x, dllmf_square_y, cursor_draw_height, BLACK);
   }

   //add click zone
   if(dllmf_square_draw_column<(document_data8[DLLMF_CHAR_ENTRY_SIZE_OFFSET]/2)) { //first half of character is visible
    if(document_data[DLLMF_CHAR_ENTRY_CHAR_NUMBER_OFFSET]<32) { //end of line
     add_zone_to_click_board(dllmf_square_x, dllmf_square_y, (dllmf_screen_first_column+dllmf_draw_width-dllmf_square_x), dllmf_square_height, ((dword_t)document_data));
     if(((dword_t)document_data)==(dllmf_mem+DLLMF_NUM_OF_PAGE_ENTRIES*DLLMF_PAGE_ENTRY_SIZE)) { //and also start of line
      add_zone_to_click_board(dllmf_screen_first_column, dllmf_square_y, (dllmf_square_x-dllmf_screen_first_column), dllmf_square_height, ((dword_t)document_data));
     }
     else if(document_data[-(DLLMF_CHAR_ENTRY_LENGTH_IN_BYTES/4)]<32) { //and also start of line
      add_zone_to_click_board(dllmf_screen_first_column, dllmf_square_y, (dllmf_square_x-dllmf_screen_first_column), dllmf_square_height, ((dword_t)document_data));
     }
    }
    else { //middle of line
     add_zone_to_click_board(dllmf_square_x, dllmf_square_y, ((document_data8[DLLMF_CHAR_ENTRY_SIZE_OFFSET]/2)-dllmf_square_draw_column), dllmf_square_height, ((dword_t)document_data));
     if(((dword_t)document_data)==(dllmf_mem+DLLMF_NUM_OF_PAGE_ENTRIES*DLLMF_PAGE_ENTRY_SIZE)) { //start of line
      add_zone_to_click_board(dllmf_screen_first_column, dllmf_square_y, (dllmf_square_x+dllmf_square_width-dllmf_screen_first_column), dllmf_square_height, ((dword_t)document_data));
     }
     else if(document_data[-(DLLMF_CHAR_ENTRY_LENGTH_IN_BYTES/4)]<32) { //start of line
      add_zone_to_click_board(dllmf_screen_first_column, dllmf_square_y, (dllmf_square_x+dllmf_square_width-dllmf_screen_first_column), dllmf_square_height, ((dword_t)document_data));
     }
     if(dllmf_square_width>=(document_data8[DLLMF_CHAR_ENTRY_SIZE_OFFSET]/2)) { //second half of character points to next char
      add_zone_to_click_board(dllmf_square_x+((document_data8[DLLMF_CHAR_ENTRY_SIZE_OFFSET]/2)-dllmf_square_draw_column), dllmf_square_y, dllmf_square_width-((document_data8[DLLMF_CHAR_ENTRY_SIZE_OFFSET]/2)-dllmf_square_draw_column), dllmf_square_height, ((dword_t)document_data)+DLLMF_CHAR_ENTRY_LENGTH_IN_BYTES);
     }
    }
   }
   else { //only second half of character is visible
    if(document_data[DLLMF_CHAR_ENTRY_CHAR_NUMBER_OFFSET]<32) { //end of line
     add_zone_to_click_board(dllmf_square_x, dllmf_square_y, (dllmf_screen_first_column+dllmf_draw_width-dllmf_square_x), dllmf_square_height, ((dword_t)document_data));
    }
    else { //middle of line
     add_zone_to_click_board(dllmf_square_x, dllmf_square_y, dllmf_square_width, dllmf_square_height, ((dword_t)document_data)+DLLMF_CHAR_ENTRY_LENGTH_IN_BYTES);
    }
   }

   //move to next char
   move_to_next_char:
   if(*document_data==DLLMF_PAGE_CONTENT_END) {
    document_data++;
    document_data8+=4;
    break;
   }
   else if(*document_data==DLLMF_DOCUMENT_CONTENT_END) {
    return;
   }
   else {
    document_data+=(DLLMF_CHAR_ENTRY_LENGTH_IN_BYTES/4);
    document_data8+=DLLMF_CHAR_ENTRY_LENGTH_IN_BYTES;
   }
  }

  //move variable value to next page with free space between
  page_first_line+=(page_entries[DLLMF_PAGE_ENTRY_HEIGHT_OFFSET]+DLLMF_SPACE_BETWEEN_DOCUMENTS);
 }
}

void dllmf_calculate_draw_square(dword_t column, dword_t line, dword_t width, dword_t height) {
 dllmf_square_x = 0;
 dllmf_square_y = 0;
 dllmf_square_width = 0;
 dllmf_square_height = 0;
 dllmf_square_draw_column = 0;
 dllmf_square_draw_line = 0;
 
 // test if something of square is on screen
 if(line>dllmf_draw_last_line) {
  return; //not in screen
 }
 else if((line+height)<dllmf_draw_first_line) {
  return; //not in screen
 }
 if(column>dllmf_draw_last_column) {
  return; //not in screen
 }
 else if((column+width)<dllmf_draw_first_column) {
  return; //not in screen
 }
 
 //calculate Y related variables
 if(line>dllmf_draw_first_line) {
  dllmf_square_y = dllmf_screen_first_line+(line-dllmf_draw_first_line);
  dllmf_square_draw_line = 0;
  if((line+height)<=dllmf_draw_last_line) {
   dllmf_square_height = height;
  }
  else {
   dllmf_square_height = (height-(line+height-dllmf_draw_last_line));
  }
 }
 else {
  dllmf_square_y = dllmf_screen_first_line;
  dllmf_square_draw_line = (height-(line+height-dllmf_draw_first_line));
  if((height-dllmf_square_draw_line)>=dllmf_draw_height) {
   dllmf_square_height = dllmf_draw_height;
  }
  else {
   dllmf_square_height = (line+height-dllmf_draw_first_line);
  }
 }
 
 //calculate X related variables
 if(column>dllmf_draw_first_column) {
  dllmf_square_x = (column-dllmf_draw_first_column);
  dllmf_square_draw_column = 0;
  if((column+width)<=dllmf_draw_last_column) {
   dllmf_square_width = width;
  }
  else {
   dllmf_square_width = (width-(column+width-dllmf_draw_last_column));
  }
 }
 else {
  dllmf_square_x = dllmf_screen_first_column;
  dllmf_square_draw_column = (width-(column+width-dllmf_draw_first_column));
  if((width-dllmf_square_draw_column)>=dllmf_draw_width) {
   dllmf_square_width = dllmf_draw_width;
  }
  else {
   dllmf_square_width = (column+width-dllmf_draw_first_column);
  }
 }
}

dword_t dllmf_get_document_height(dword_t dllmf_memory) {
 dword_t *page_entries = (dword_t *) (dllmf_memory);
 dword_t document_height = DLLMF_SPACE_BETWEEN_DOCUMENTS;

 for(dword_t i=0; i<DLLMF_NUM_OF_PAGE_ENTRIES; i++, page_entries+=2) {
  if(*page_entries==0) { //no more pages
   break;
  }
  document_height += (page_entries[DLLMF_PAGE_ENTRY_HEIGHT_OFFSET]+DLLMF_SPACE_BETWEEN_DOCUMENTS);
 }

 return document_height;
}

dword_t dllmf_get_document_width(dword_t dllmf_memory) {
 dword_t *page_entries = (dword_t *) (dllmf_memory);
 dword_t document_width = 0;

 for(dword_t i=0; i<DLLMF_NUM_OF_PAGE_ENTRIES; i++, page_entries+=2) {
  if(*page_entries==0) { //no more pages
   break;
  }
  if((page_entries[DLLMF_PAGE_ENTRY_WIDTH_OFFSET]+DLLMF_SPACE_BETWEEN_DOCUMENTS*2)>document_width) {
   document_width = (page_entries[DLLMF_PAGE_ENTRY_WIDTH_OFFSET]+DLLMF_SPACE_BETWEEN_DOCUMENTS*2);
  }
 }

 return document_width;
}

dword_t dllmf_get_data_memory(dword_t dllmf_memory) {
 return (dllmf_memory+DLLMF_NUM_OF_PAGE_ENTRIES*DLLMF_PAGE_ENTRY_SIZE);
}