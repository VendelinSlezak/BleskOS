//BleskOS

/*
* MIT License
* Copyright (c) 2023-2024 Vendelín Slezák
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

/*
 Document Memory Format

 This format is layer between formats like DOC or ODT and DLLMF.
 It consists of word chars. When there is DMF_SECTION_FORMAT_CHANGE_SIGNATURE it mean that follows this block
 that specifies what changes:

 word = DMF_SECTION_FORMAT_CHANGE_SIGNATURE
 word = length of section in bytes with signatures
 word = what changes in this block, set bit mean present
  bit 0 = new page
  bit 1 = new paragraph

 word = width of new page
 word = height of new page
 word = page top border size
 word = page bottom border size
 word = page left border size
 word = page right border size

 word = paragraph description, set bit mean present
  bit 0 = paragraph top border collapse
  bit 1 = paragraph bottom border collapse
  bit 2 = this paragraph is list entry
  bit 3+4 = alignment 0b00 = left 0b01 = center 0b10 = right TODO: code for this
 word = paragraph top border size
 word = paragraph bottom border size
 word = paragraph left border size
 word = paragraph right border size
 word = list entry char, DMF_SFCH_PARAGRAPH_LIST_ENTRY_DOT = dot, other = number to print

 word = char size
 word = char emphasis
 dword = char color
 dword = char background color

 word = length of section in bytes with signatures
 word = DMF_SECTION_FORMAT_CHANGE_SIGNATURE
*/

void convert_dmf_to_dllmf(dword_t dmf_memory, dword_t dllmf_memory) {
 word_t *dmf = (word_t *) (dmf_memory);
 dword_t *dllmf_page_entries = (dword_t *) (dllmf_memory);
 dword_t *dllmf_data = (dword_t *) (dllmf_memory+DLLMF_NUM_OF_PAGE_ENTRIES*DLLMF_PAGE_ENTRY_SIZE);

 dword_t dmf_actual_x_position = 0, dmf_actual_y_position = 0, dmf_number_of_pages = 0, dmf_was_last_character_char = STATUS_FALSE, dmf_go_to_next_line = STATUS_FALSE;
 dmf_page_width = 0;
 dmf_page_height = 0;
 dmf_page_top_border = 0;
 dmf_page_bottom_border = 0;
 dmf_page_left_border = 0;
 dmf_page_right_border = 0;

 dmf_actual_paragraph_description = 0;
 dmf_paragraph_top_border = 0;
 dmf_paragraph_bottom_border = 0;
 dmf_paragraph_left_border = 0;
 dmf_paragraph_right_border = 0;

 dmf_page_actual_left_border = 0;
 dmf_page_actual_right_border = 0;

 dmf_character_size = 0;
 dmf_character_spacing = 0;
 dmf_bottom_line_of_characters = 0;
 dmf_character_emphasis = 0;
 dmf_character_color = 0;
 dmf_character_background_color = TRANSPARENT_COLOR;
 dmf_paragraph_alignment = DMF_SFCH_PARAGRAPH_DESCRIPTION_ALIGNMENT_LEFT;

 while(*dmf!=0) {
  if(*dmf==DMF_SECTION_FORMAT_CHANGE_SIGNATURE) {
   //ADD PAGE
   if((dmf[DMF_SFCH_DESCRIPTION_OFFSET] & DMF_SFCH_DESCRIPTION_NEW_PAGE)==DMF_SFCH_DESCRIPTION_NEW_PAGE) {
    if(dmf_number_of_pages>=DLLMF_NUM_OF_PAGE_ENTRIES) {
     return; //too much pages
    }

    if(dmf_number_of_pages!=0) {
     *dllmf_data = DLLMF_PAGE_CONTENT_END;
     dllmf_data++;
    }
    dllmf_page_entries[DLLMF_PAGE_ENTRY_WIDTH_OFFSET] = dmf[DMF_SFCH_NEW_PAGE_WIDTH_OFFSET];
    dllmf_page_entries[DLLMF_PAGE_ENTRY_HEIGHT_OFFSET] = dmf[DMF_SFCH_NEW_PAGE_HEIGHT_OFFSET];
    dllmf_page_entries = (dword_t *) (((dword_t)dllmf_page_entries)+DLLMF_PAGE_ENTRY_SIZE);
    dmf_number_of_pages++;

    dmf_page_width = dmf[DMF_SFCH_NEW_PAGE_WIDTH_OFFSET];
    dmf_page_height = dmf[DMF_SFCH_NEW_PAGE_HEIGHT_OFFSET];
    dmf_page_top_border = dmf[DMF_SFCH_NEW_PAGE_TOP_BORDER_OFFSET];
    dmf_page_bottom_border = (dmf_page_height-dmf[DMF_SFCH_NEW_PAGE_BOTTOM_BORDER_OFFSET]);
    dmf_page_left_border = dmf[DMF_SFCH_NEW_PAGE_LEFT_BORDER_OFFSET];
    dmf_page_right_border = (dmf_page_width-dmf[DMF_SFCH_NEW_PAGE_RIGHT_BORDER_OFFSET]);

    dmf_page_actual_left_border = dmf_page_left_border;
    dmf_page_actual_right_border = dmf_page_right_border;

    dmf_actual_x_position = dmf_get_first_column_of_line((dword_t)dmf, dmf_paragraph_alignment, dmf[DMF_SFCH_INLINE_CHANGE_SIZE_OFFSET]);
    dmf_actual_y_position = dmf_page_top_border;
    dword_t biggest_char_size = dmf_get_biggest_char_size_of_line((dword_t)dmf, dmf[DMF_SFCH_INLINE_CHANGE_SIZE_OFFSET]);
    dmf_character_spacing = (biggest_char_size+(biggest_char_size/2));
    dmf_bottom_line_of_characters = (dmf_actual_y_position+biggest_char_size);
   }

   //ADD PARAGRAPH
   if((dmf[DMF_SFCH_DESCRIPTION_OFFSET] & DMF_SFCH_DESCRIPTION_NEW_PARAGRAPH)==DMF_SFCH_DESCRIPTION_NEW_PARAGRAPH) {
    //add char to DLLMF
    dllmf_data[DLLMF_CHAR_ENTRY_CHAR_NUMBER_OFFSET] = 0xA;
    dllmf_data[DLLMF_CHAR_ENTRY_CHAR_DMF_MEMORY_POINTER_OFFSET] = ((dword_t)dmf);
    dllmf_data[DLLMF_CHAR_ENTRY_COLUMN_OFFSET] = dmf_actual_x_position;
    dllmf_data[DLLMF_CHAR_ENTRY_LINE_OFFSET] = (dmf_bottom_line_of_characters-dmf_character_size);
    dllmf_data[DLLMF_CHAR_ENTRY_COLOR_OFFSET] = ((dmf_character_size)<<24);
    dllmf_data[DLLMF_CHAR_ENTRY_BACKGROUND_COLOR_OFFSET] = 0;
    dllmf_data = (dword_t *) (((dword_t)dllmf_data)+DLLMF_CHAR_ENTRY_LENGTH_IN_BYTES);

    //calculate how many space to skip after previous paragraph
    dword_t paragraph_how_many_space_skip = (dmf_paragraph_bottom_border+dmf[DMF_SFCH_PARAGRAPH_TOP_BORDER_OFFSET]);
    if((dmf_actual_paragraph_description & DMF_SFCH_PARAGRAPH_DESCRIPTION_BOTTOM_BORDER_COLLAPSE)==DMF_SFCH_PARAGRAPH_DESCRIPTION_BOTTOM_BORDER_COLLAPSE || (dmf[DMF_SFCH_PARAGRAPH_DESCRIPTION_OFFSET] & DMF_SFCH_PARAGRAPH_DESCRIPTION_TOP_BORDER_COLLAPSE)==DMF_SFCH_PARAGRAPH_DESCRIPTION_TOP_BORDER_COLLAPSE) {
     if(dmf_paragraph_bottom_border>dmf[DMF_SFCH_PARAGRAPH_TOP_BORDER_OFFSET]) {
      paragraph_how_many_space_skip = dmf_paragraph_bottom_border;
     }
     else {
      paragraph_how_many_space_skip = dmf[DMF_SFCH_PARAGRAPH_TOP_BORDER_OFFSET];
     }
    }

    //read variables
    dmf_actual_paragraph_description = dmf[DMF_SFCH_PARAGRAPH_DESCRIPTION_OFFSET];
    dmf_paragraph_top_border = dmf[DMF_SFCH_PARAGRAPH_TOP_BORDER_OFFSET];
    dmf_paragraph_bottom_border = dmf[DMF_SFCH_PARAGRAPH_BOTTOM_BORDER_OFFSET];
    dmf_paragraph_left_border = dmf[DMF_SFCH_PARAGRAPH_LEFT_BORDER_OFFSET];
    dmf_paragraph_right_border = dmf[DMF_SFCH_PARAGRAPH_RIGHT_BORDER_OFFSET];

    //move to right position
    if((dmf_actual_y_position+dmf_character_spacing+paragraph_how_many_space_skip)>dmf_page_bottom_border) {
     //new page
     if(dmf_number_of_pages>=DLLMF_NUM_OF_PAGE_ENTRIES) {
      return; //too much pages
     }

     *dllmf_data = DLLMF_PAGE_CONTENT_END;
     dllmf_data++;
     dmf_number_of_pages++;
     dllmf_page_entries[DLLMF_PAGE_ENTRY_WIDTH_OFFSET] = dmf_page_width;
     dllmf_page_entries[DLLMF_PAGE_ENTRY_HEIGHT_OFFSET] = dmf_page_height;
     dllmf_page_entries = (dword_t *) (((dword_t)dllmf_page_entries)+DLLMF_PAGE_ENTRY_SIZE);

     dmf_page_actual_left_border = (dmf_page_left_border+dmf[DMF_SFCH_PARAGRAPH_LEFT_BORDER_OFFSET]);
     dmf_actual_y_position = (dmf_page_top_border+dmf[DMF_SFCH_PARAGRAPH_TOP_BORDER_OFFSET]);
     dword_t biggest_char_size = dmf_get_biggest_char_size_of_line((dword_t)dmf, dmf[DMF_SFCH_INLINE_CHANGE_SIZE_OFFSET]);
     dmf_character_spacing = (biggest_char_size+(biggest_char_size/2));
     dmf_bottom_line_of_characters = (dmf_actual_y_position+biggest_char_size);
    }
    else {
     dmf_page_actual_left_border = (dmf_page_left_border+dmf[DMF_SFCH_PARAGRAPH_LEFT_BORDER_OFFSET]);
     dmf_actual_y_position += (dmf_character_spacing+paragraph_how_many_space_skip);     
     dword_t biggest_char_size = dmf_get_biggest_char_size_of_line((dword_t)dmf, dmf[DMF_SFCH_INLINE_CHANGE_SIZE_OFFSET]);
     dmf_character_spacing = (biggest_char_size+(biggest_char_size/2));
     dmf_bottom_line_of_characters = (dmf_actual_y_position+biggest_char_size);
    }
    dmf_page_actual_right_border = (dmf_page_right_border-dmf[DMF_SFCH_PARAGRAPH_RIGHT_BORDER_OFFSET]);
    dmf_actual_x_position = dmf_get_first_column_of_line((dword_t)dmf, dmf_paragraph_alignment, dmf[DMF_SFCH_INLINE_CHANGE_SIZE_OFFSET]);

    //print list entry
    if((dmf[DMF_SFCH_PARAGRAPH_DESCRIPTION_OFFSET] & DMF_SFCH_PARAGRAPH_DESCRIPTION_LIST_ENTRY)==DMF_SFCH_PARAGRAPH_DESCRIPTION_LIST_ENTRY) {
     if(dmf[DMF_SFCH_PARAGRAPH_LIST_ENTRY_OFFSET]==DMF_SFCH_PARAGRAPH_LIST_ENTRY_DOT) { //unordered list
      dllmf_data[DLLMF_CHAR_ENTRY_CHAR_NUMBER_OFFSET] = SF_DOT_IN_MIDDLE_OF_LINE;
      dllmf_data[DLLMF_CHAR_ENTRY_COLUMN_OFFSET] = dmf_page_actual_left_border-10;
      dllmf_data[DLLMF_CHAR_ENTRY_LINE_OFFSET] = (dmf_bottom_line_of_characters-dmf_character_size);
      dllmf_data[DLLMF_CHAR_ENTRY_COLOR_OFFSET] = (BLACK | ((dmf_character_size)<<24));
      dllmf_data[DLLMF_CHAR_ENTRY_BACKGROUND_COLOR_OFFSET] = 0;
      dllmf_data = (dword_t *) (((dword_t)dllmf_data)+DLLMF_CHAR_ENTRY_LENGTH_IN_BYTES);
     }
     else { //ordered list
      clear_memory(((dword_t)&number_string), MATH_LENGTH_OF_NUMBER_STRING_ARRAY);
      covert_number_to_byte_string(dmf[DMF_SFCH_PARAGRAPH_LIST_ENTRY_OFFSET], ((dword_t)&number_string));
      dword_t num_of_digits = get_number_of_chars_in_ascii_string(number_string);
      number_string[num_of_digits] = '.';
      num_of_digits++;
      for(dword_t i=0; i<num_of_digits; i++) {
       dllmf_data[DLLMF_CHAR_ENTRY_CHAR_NUMBER_OFFSET] = number_string[i];
       dllmf_data[DLLMF_CHAR_ENTRY_COLUMN_OFFSET] = dmf_page_actual_left_border-10-((num_of_digits-i-1)*dmf_character_size);
       dllmf_data[DLLMF_CHAR_ENTRY_LINE_OFFSET] = (dmf_bottom_line_of_characters-dmf_character_size);
       dllmf_data[DLLMF_CHAR_ENTRY_COLOR_OFFSET] = (BLACK | ((dmf_character_size)<<24));
       dllmf_data[DLLMF_CHAR_ENTRY_BACKGROUND_COLOR_OFFSET] = 0;
       dllmf_data = (dword_t *) (((dword_t)dllmf_data)+DLLMF_CHAR_ENTRY_LENGTH_IN_BYTES);
      }
     }
    }
   }

   //INLINE CHARACTER CHANGE
   dmf_character_size = dmf[DMF_SFCH_INLINE_CHANGE_SIZE_OFFSET];
   dmf_character_emphasis = dmf[DMF_SFCH_INLINE_CHANGE_EMPHASIS_OFFSET];
   dmf_character_color = (dmf[DMF_SFCH_INLINE_CHANGE_COLOR_OFFSET] | (dmf[DMF_SFCH_INLINE_CHANGE_COLOR_OFFSET+1]<<16));
   dmf_character_background_color = (dmf[DMF_SFCH_INLINE_CHANGE_BACKGROUND_COLOR_OFFSET] | (dmf[DMF_SFCH_INLINE_CHANGE_BACKGROUND_COLOR_OFFSET+1]<<16));

   //skip
   dmf = (word_t *) (((dword_t)dmf)+dmf[DMF_SFCH_LENGTH_OFFSET_1]);
  }
  else { //normal character
   //find if we need to go to next line
   dmf_go_to_next_line = STATUS_FALSE;
   if(*dmf!=0 && *dmf!=' ' && *dmf!=SF_NBSP) {
    if(dmf_was_last_character_char==STATUS_FALSE) {
     dmf_was_last_character_char = STATUS_TRUE;
     if((dmf_actual_x_position+dmf_get_size_of_word(((dword_t)dmf), dmf_character_size))>dmf_page_actual_right_border) {
      dmf_go_to_next_line = STATUS_TRUE;
     }
    }
   }
   else {
    dmf_was_last_character_char = STATUS_FALSE;
   }
   if((dmf_actual_x_position+dmf_character_size)>dmf_page_actual_right_border) {
    dmf_go_to_next_line = STATUS_TRUE;
   }
   if(*dmf==0xA) {
    dmf_go_to_next_line = STATUS_TRUE;
   }

   //go to next line
   if(dmf_go_to_next_line==STATUS_TRUE) {
    //add end line char to DLLMF
    if(*dmf==0xA) {
     dllmf_data[DLLMF_CHAR_ENTRY_CHAR_NUMBER_OFFSET] = 0xA;
    }
    else if(*dmf==' ') {
     dllmf_data[DLLMF_CHAR_ENTRY_CHAR_NUMBER_OFFSET] = DLLMF_HIDDEN_SPACE;
    }
    else {
     dllmf_data[DLLMF_CHAR_ENTRY_CHAR_NUMBER_OFFSET] = 0xD;
    }
    dllmf_data[DLLMF_CHAR_ENTRY_CHAR_DMF_MEMORY_POINTER_OFFSET] = ((dword_t)dmf);
    dllmf_data[DLLMF_CHAR_ENTRY_COLUMN_OFFSET] = dmf_actual_x_position;
    dllmf_data[DLLMF_CHAR_ENTRY_LINE_OFFSET] = (dmf_bottom_line_of_characters-dmf_character_size);
    dllmf_data[DLLMF_CHAR_ENTRY_COLOR_OFFSET] = ((dmf_character_size)<<24);
    dllmf_data[DLLMF_CHAR_ENTRY_BACKGROUND_COLOR_OFFSET] = 0;
    dllmf_data = (dword_t *) (((dword_t)dllmf_data)+DLLMF_CHAR_ENTRY_LENGTH_IN_BYTES);

    if((dmf_actual_y_position+dmf_character_spacing)>dmf_page_bottom_border) {
     //new page
     if(dmf_number_of_pages>=DLLMF_NUM_OF_PAGE_ENTRIES) {
      return; //too much pages
     }

     *dllmf_data = DLLMF_PAGE_CONTENT_END;
     dllmf_data++;
     dmf_number_of_pages++;
     dllmf_page_entries[DLLMF_PAGE_ENTRY_WIDTH_OFFSET] = dmf_page_width;
     dllmf_page_entries[DLLMF_PAGE_ENTRY_HEIGHT_OFFSET] = dmf_page_height;
     dllmf_page_entries = (dword_t *) (((dword_t)dllmf_page_entries)+DLLMF_PAGE_ENTRY_SIZE);

     dmf_actual_x_position = dmf_get_first_column_of_line((dword_t)dmf, dmf_paragraph_alignment, dmf_character_size);
     dmf_actual_y_position = dmf_page_top_border;
     dword_t biggest_char_size = dmf_get_biggest_char_size_of_line((dword_t)dmf, dmf_character_size);
     dmf_character_spacing = (biggest_char_size+(biggest_char_size/2));
     dmf_bottom_line_of_characters = (dmf_actual_y_position+biggest_char_size);
    }
    else {
     //new line
     dmf_actual_x_position = dmf_get_first_column_of_line((dword_t)dmf, dmf_paragraph_alignment, dmf_character_size);
     dmf_actual_y_position += dmf_character_spacing;
     dword_t biggest_char_size = dmf_get_biggest_char_size_of_line((dword_t)dmf, dmf_character_size);
     dmf_character_spacing = (biggest_char_size+(biggest_char_size/2));
     dmf_bottom_line_of_characters = (dmf_actual_y_position+biggest_char_size);
    }

    //if this is space at end of line, go to next character
    if(*dmf==' ') {
     dmf++;
     continue;
    }
   }

   if(*dmf!=0xA) {
    //add char to DLLMF
    dllmf_data[DLLMF_CHAR_ENTRY_CHAR_NUMBER_OFFSET] = *dmf;
    dllmf_data[DLLMF_CHAR_ENTRY_CHAR_DMF_MEMORY_POINTER_OFFSET] = ((dword_t)dmf);
    dllmf_data[DLLMF_CHAR_ENTRY_COLUMN_OFFSET] = dmf_actual_x_position;
    dllmf_data[DLLMF_CHAR_ENTRY_LINE_OFFSET] = (dmf_bottom_line_of_characters-dmf_character_size);
    dllmf_data[DLLMF_CHAR_ENTRY_COLOR_OFFSET] = ((dmf_character_color & 0xFFFFFF) | ((dmf_character_size)<<24));
    dllmf_data[DLLMF_CHAR_ENTRY_BACKGROUND_COLOR_OFFSET] = ((dmf_character_background_color & 0xFFFFFF) | ((dmf_character_emphasis)<<24));
    if(dmf_character_background_color!=TRANSPARENT_COLOR) {
     dllmf_data[DLLMF_CHAR_ENTRY_BACKGROUND_COLOR_OFFSET] |= (1<<31);
    }
    dllmf_data = (dword_t *) (((dword_t)dllmf_data)+DLLMF_CHAR_ENTRY_LENGTH_IN_BYTES);

    //move position if same line
    dmf_actual_x_position+=dmf_character_size;
   }

   //skip
   dmf++;
  }
 }

 //add last zero char to DLLMF
 dllmf_data[DLLMF_CHAR_ENTRY_CHAR_NUMBER_OFFSET] = DLLMF_DOCUMENT_CONTENT_END;
 dllmf_data[DLLMF_CHAR_ENTRY_CHAR_DMF_MEMORY_POINTER_OFFSET] = ((dword_t)dmf);
 dllmf_data[DLLMF_CHAR_ENTRY_COLUMN_OFFSET] = dmf_actual_x_position;
 dllmf_data[DLLMF_CHAR_ENTRY_LINE_OFFSET] = (dmf_bottom_line_of_characters-dmf_character_size);
 dllmf_data[DLLMF_CHAR_ENTRY_COLOR_OFFSET] = ((dmf_character_color & 0xFFFFFF) | ((dmf_character_size)<<24));
 dllmf_data[DLLMF_CHAR_ENTRY_BACKGROUND_COLOR_OFFSET] = ((dmf_character_background_color & 0xFFFFFF) | ((dmf_character_emphasis)<<24));
}

dword_t dmf_get_first_column_of_line(dword_t dmf_memory, dword_t alignment, dword_t actual_char_size) {
 if(alignment==DMF_SFCH_PARAGRAPH_DESCRIPTION_ALIGNMENT_LEFT) {
  return dmf_page_actual_left_border;
 }

 word_t *dmf = (word_t *) (dmf_memory);
 dword_t length_of_line = 0, dmf_was_last_character_char = 0;
 while(*dmf!=0) {
  if(*dmf==DMF_SECTION_FORMAT_CHANGE_SIGNATURE) {
   actual_char_size = dmf[DMF_SFCH_INLINE_CHANGE_SIZE_OFFSET];

   dmf = (word_t *) (((dword_t)dmf)+dmf[DMF_SFCH_LENGTH_OFFSET_1]);
  }
  else {
   //find if we need to go to next line
   if(*dmf!=0 && *dmf!=' ' && *dmf!=SF_NBSP) {
    if(dmf_was_last_character_char==STATUS_FALSE) {
     dmf_was_last_character_char = STATUS_TRUE;
     if((dmf_page_actual_left_border+length_of_line+dmf_get_size_of_word(((dword_t)dmf), actual_char_size))>dmf_page_actual_right_border) {
      break;
     }
    }
   }
   else {
    dmf_was_last_character_char = STATUS_FALSE;
   }
   if((dmf_page_actual_left_border+length_of_line+actual_char_size)>dmf_page_actual_right_border) {
    break;
   }
   if(*dmf==0xA) {
    break;
   }

   length_of_line += actual_char_size;

   dmf++;
  }
 }

 if(alignment==DMF_SFCH_PARAGRAPH_DESCRIPTION_ALIGNMENT_CENTER) {
  return dmf_page_actual_left_border+((dmf_page_actual_right_border-dmf_page_actual_left_border)/2)-(length_of_line/2);
 }
 if(alignment==DMF_SFCH_PARAGRAPH_DESCRIPTION_ALIGNMENT_RIGHT) {
  return (dmf_page_actual_right_border-length_of_line);
 }
}

dword_t dmf_get_biggest_char_size_of_line(dword_t dmf_memory, dword_t actual_char_size) {
 word_t *dmf = (word_t *) (dmf_memory);
 dword_t length_of_line = 0, biggest_char_size = actual_char_size, dmf_was_last_character_char = 0;
 while(*dmf!=0) {
  if(*dmf==DMF_SECTION_FORMAT_CHANGE_SIGNATURE) {
   //find if this is last paragraph
   if((dmf[DMF_SFCH_DESCRIPTION_OFFSET] & DMF_SFCH_DESCRIPTION_NEW_PAGE)==DMF_SFCH_DESCRIPTION_NEW_PAGE || (dmf[DMF_SFCH_DESCRIPTION_OFFSET] & DMF_SFCH_DESCRIPTION_NEW_PARAGRAPH)==DMF_SFCH_DESCRIPTION_NEW_PARAGRAPH) {
    break;
   }

   //get actual size
   actual_char_size = dmf[DMF_SFCH_INLINE_CHANGE_SIZE_OFFSET];
   if(actual_char_size>biggest_char_size) {
    biggest_char_size = actual_char_size;
   }

   dmf = (word_t *) (((dword_t)dmf)+dmf[DMF_SFCH_LENGTH_OFFSET_1]);
  }
  else {
   //find if we need to go to next line
   if(*dmf!=0 && *dmf!=' ' && *dmf!=SF_NBSP) {
    if(dmf_was_last_character_char==STATUS_FALSE) {
     dmf_was_last_character_char = STATUS_TRUE;
     if((dmf_page_actual_left_border+length_of_line+dmf_get_size_of_word(((dword_t)dmf), actual_char_size))>dmf_page_actual_right_border) {
      break;
     }
    }
   }
   else {
    dmf_was_last_character_char = STATUS_FALSE;
   }
   if((dmf_page_actual_left_border+length_of_line+actual_char_size)>dmf_page_actual_right_border) {
    break;
   }
   if(*dmf==0xA) {
    break;
   }

   length_of_line += actual_char_size;

   dmf++;
  }
 }

 return biggest_char_size;
}

dword_t dmf_get_size_of_word(dword_t dmf_memory, dword_t actual_char_size) {
 word_t *dmf = (word_t *) (dmf_memory);
 dword_t length_of_word = 0;
 while(*dmf!=' ' && *dmf!=0 && *dmf!=SF_NBSP) {
  if(*dmf==DMF_SECTION_FORMAT_CHANGE_SIGNATURE) {
   actual_char_size = dmf[DMF_SFCH_INLINE_CHANGE_SIZE_OFFSET];

   dmf = (word_t *) (((dword_t)dmf)+dmf[DMF_SFCH_LENGTH_OFFSET_1]);
  }
  else {
   length_of_word += actual_char_size;

   dmf++;
  }
 }

 return length_of_word;
}

void dmf_add_section_format_change(dword_t memory, dword_t description, dword_t size, dword_t emphasis, dword_t color, dword_t background_color) {
 word_t *dmf = (word_t *) (memory);

 dmf[DMF_SFCH_SIGNATURE_OFFSET_1] = DMF_SECTION_FORMAT_CHANGE_SIGNATURE;
 dmf[DMF_SFCH_SIGNATURE_OFFSET_2] = DMF_SECTION_FORMAT_CHANGE_SIGNATURE;
 dmf[DMF_SFCH_LENGTH_OFFSET_1] = DMF_SFCH_ENTRY_LENGTH_IN_BYTES;
 dmf[DMF_SFCH_LENGTH_OFFSET_2] = DMF_SFCH_ENTRY_LENGTH_IN_BYTES;
 dmf[DMF_SFCH_DESCRIPTION_OFFSET] = description;

 dmf[DMF_SFCH_INLINE_CHANGE_SIZE_OFFSET] = size;
 dmf[DMF_SFCH_INLINE_CHANGE_EMPHASIS_OFFSET] = emphasis;
 dmf[DMF_SFCH_INLINE_CHANGE_COLOR_OFFSET] = (color & 0xFFFF);
 dmf[DMF_SFCH_INLINE_CHANGE_COLOR_OFFSET+1] = (color >> 16);
 dmf[DMF_SFCH_INLINE_CHANGE_BACKGROUND_COLOR_OFFSET] = (background_color & 0xFFFF);
 dmf[DMF_SFCH_INLINE_CHANGE_BACKGROUND_COLOR_OFFSET+1] = (background_color >> 16);
}

void dmf_add_section_paragraph_change(dword_t memory, dword_t description, dword_t top, dword_t bottom, dword_t left, dword_t right, dword_t list_entry) {
 word_t *dmf = (word_t *) (memory);

 dmf[DMF_SFCH_PARAGRAPH_DESCRIPTION_OFFSET] = description;
 dmf[DMF_SFCH_PARAGRAPH_TOP_BORDER_OFFSET] = top;
 dmf[DMF_SFCH_PARAGRAPH_BOTTOM_BORDER_OFFSET] = bottom;
 dmf[DMF_SFCH_PARAGRAPH_LEFT_BORDER_OFFSET] = left;
 dmf[DMF_SFCH_PARAGRAPH_RIGHT_BORDER_OFFSET] = right;
 dmf[DMF_SFCH_PARAGRAPH_LIST_ENTRY_OFFSET] = list_entry;
}