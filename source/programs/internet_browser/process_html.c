//BleskOS

/*
* MIT License
* Copyright (c) 2023-2024 Vendelín Slezák
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

dword_t convert_html_to_bleskos_webpage(dword_t html_mem, dword_t html_length) {
 dword_t html_in_unicode_mem = calloc(html_length*2);
 convert_utf_8_to_unicode(html_mem, html_in_unicode_mem, html_length);
 word_t *html = (word_t *) html_in_unicode_mem;
 dword_t webpage_mem = calloc((html_length+1)*2);
 word_t *webpage = (word_t *) webpage_mem;
 word_t *webpage16;
 dword_t *webpage32;
 html_length = (html_in_unicode_mem+html_length*2);
 
 //prepare document
 byte_t is_space = 0;
 byte_t is_tag = 0;
 byte_t is_comment = 0;
 while(*html!=0 && (dword_t)html<html_length) {
  //remove chars that we will not print
  if(*html>512) {
   *html = 1;
  }
  
  //remove multiple spaces
  if(*html>=' ' && *html!='<' && is_tag==0 && is_comment==0 && *html!='>') {
   if(*html==' ') {
    if(is_space==0) {
     is_space = 1;
    }
    else {
     *html = 1;
    }
   }
   else {
    is_space = 0;
   }
  }
  
  //make all chars in tags lowercase
  if(*html=='<') { //start of tag
   is_tag = 1;
  }
  if(*html=='>') { //end of tag
   is_tag = 0;
  }
  if(is_tag==1 && *html>=0x41 && *html<=0x5A) { //content of tag
   *html += 0x20;
  }
  if(is_tag==1 && *html=='"') { //attribute
   is_tag = 2;
  }
  else if(is_tag==2 && *html=='"') { //end of attribute
   is_tag = 1;
  }
  
  //comments
  if(is_comment==1 && html[0]=='-' && html[1]=='-' && html[2]=='>') { //start of comment
   is_comment = 0;
  }
  if(html[0]=='<' && html[1]=='!' && html[2]=='-' && html[3]=='-') { //end of comment
   is_comment = 1;
  }
  
  //convert & chars
  if(*html=='&' && is_tag==0) {
   if(html[1]=='#') { //number
    if(html[2]=='x') { //hex number
     *html = convert_hex_word_string_to_number((dword_t)html+6);
    }
    else { //number
     *html = convert_word_string_to_number((dword_t)html+4);
    }
    if(*html>' ' && *html<512) {
     html++; //it is supported char number, so keep it
    }
    else if(*html==' ' && is_space==0) { //space
     html++;
     is_space = 1;
    }
   }
   else if(html[1]=='a') {
    if(html[2]=='m' && html[3]=='p' && html[4]==';') { //ampersand
     *html = '&';
     html++;
    }
   }
   else if(html[1]=='n') {
    if(html[2]=='b' && html[3]=='s' && html[4]=='p' && html[4]==';') { //non breakable space
     *html = ' '; //TODO: real non breakable space, not only space
     html++;
    }
   }
   //TODO: more chars
   
   //delete rest of char
   while(*html!=';') {
    if(*html==0) {
     break;
    }
     
    *html = 1;
    html++;
   }
    
   *html = 1; //delete ;
  }
  
  html++;
 }
 
 //process tags before <body>
 html = (word_t *) html_in_unicode_mem;
 clear_memory(html_title_memory, 1000*2);
 html_page_height = 0; 
 css_entries_tag_list_pointer = 0;
 css_entries_id_list_pointer = 0;
 css_entries_class_list_pointer = 0;
 html_list_of_downloaded_files_pointer = 0;
 while(*html!=0 && (dword_t)html<html_length) {
  if(*html=='<') {
   if(is_tag_equal((word_t *)html, "link")==STATUS_TRUE) {
    //TODO:
   }
   else if(is_tag_equal((word_t *)html, "style")==STATUS_TRUE) {
    //skip tag
    while(*html!='>') {
     if(*html==0) {
      goto page_is_finished;
     }
     
     html++;
    }
    html++;
    
    css = (word_t *) ((dword_t)html);
    css_parse_file();
   }
   else if(is_tag_equal((word_t *)html, "script")==STATUS_TRUE) {
    //TODO:
   }
   else if(is_tag_equal((word_t *)html, "title")==STATUS_TRUE) {
    //skip tag
    while(*html!='>') {
     if(*html==0) {
      goto page_is_finished;
     }
     
     html++;
    }
    html++;
    
    //copy title
    word_t *html_title = (word_t *) (html_title_memory);
    for(int i=0; i<999; i++) {
     if(html[i]=='<') {
      break;
     }
     if(html[i]>=' ') {
      *html_title = html[i];
      html_title++;
     }
    }
    
    continue;
   }
   else if(is_tag_equal((word_t *)html, "base")==STATUS_TRUE) {
    //TODO:
   }
   else if(is_tag_equal((word_t *)html, "body")==STATUS_TRUE) {
    break;
   }
  }
  
  html++;
 }
 if(*html==0 || (dword_t)html==html_length) { //no <body> tag
  goto page_is_finished;
 }
 
 //set variables for body
 html_page_height = 0;
 ib_line_width = internet_browser_webpage_width;
 ib_actual_line_width = 0;
 
 ib_draw_column = 0;
 ib_draw_line = 0;
 ib_line_biggest_spacing = 0;
 
 ib_font_size = 10;
 ib_font_spacing = 15;
 ib_font_emphasis = SF_EMPHASIS_NONE;
 ib_font_color = BLACK;
 ib_font_alignment = WEBPAGE_LEFT_ALIGNMENT;
 
 html_tag_css_list_pointer = 0;
 html_list_type_pointer = 0;
 
 //add command for movement to first line
 webpage_last_line_command_pointer = ((dword_t)webpage);
 webpage[0] = WEBPAGE_COMMAND_MOVE_TO_LINE;
 webpage32 = (dword_t *) ((dword_t)webpage+WEBPAGE_COMMAND_MOVE_TO_LINE_BYTE_OFFSET_COLUMN);
 *webpage32 = ib_draw_column;
 webpage32 = (dword_t *) ((dword_t)webpage+WEBPAGE_COMMAND_MOVE_TO_LINE_BYTE_OFFSET_LINE);
 *webpage32 = ib_draw_line;
 webpage[WEBPAGE_COMMAND_MOVE_TO_LINE_OFFSET_LINE_SIZE] = 0;
 webpage += 6;
 
 //process <body>
 html_last_char_is_list_start = STATUS_FALSE;
 while(*html!=0 && (dword_t)html<html_length) {
  if(*html<' ') { //skip unprintable characters
   html++;
   continue;
  }
 
  if(*html=='<') {
   //reset variables for tag
   html_not_pair_tag = STATUS_FALSE;
   html_list_tag = STATUS_FALSE;
   html_table_cell_width = 0;
   ib_tag_new_line = STATUS_FALSE; ib_tag_new_column = STATUS_FALSE;
   css_font_size = WEBPAGE_EMPTY_ENTRY_32; css_font_color = WEBPAGE_EMPTY_ENTRY_32; css_font_alignment = WEBPAGE_EMPTY_ENTRY_32; css_font_emphasis = ib_font_emphasis; css_font_spacing = WEBPAGE_EMPTY_ENTRY_32;
   css_margin_top = 0; css_margin_bottom = 0; css_margin_left = 0; css_margin_right = 0;
   css_top_border_type = WEBPAGE_BORDER_TYPE_NONE; css_top_border_size = 0; css_top_border_color = BLACK;
   css_bottom_border_type = WEBPAGE_BORDER_TYPE_NONE; css_bottom_border_size = 0; css_bottom_border_color = BLACK;
   css_left_border_type = WEBPAGE_BORDER_TYPE_NONE; css_left_border_size = 0; css_left_border_color = BLACK;
   css_right_border_type = WEBPAGE_BORDER_TYPE_NONE; css_right_border_size = 0; css_right_border_color = BLACK;
   css_padding_top = 0; css_padding_bottom = 0; css_padding_left = 0; css_padding_right = 0;
   css_background_color = WEBPAGE_EMPTY_ENTRY_32;
   css_width = 0; css_height = 0;
   
   html_tag_height_calculation_type = HTML_TAG_CSS_ENTRY_HEIGTH_CALCULATION_ADD;
   
   //ENDING TAG
   if(html[1]=='/') {
    html++;

    //if there is </body> tag, we just add height of last printed line and end processing of HTML
    if(is_tag_equal((word_t *)html, "body")==STATUS_TRUE) {
     html_tag_css_list = (dword_t *) html_tag_css_list_mem;     
     if(ib_actual_line_width!=0) {
      html_tag_css_list[HTML_TAG_CSS_ENTRY_OFFSET_HEIGTH] += ib_line_biggest_spacing;
      if(html_tag_css_list[HTML_TAG_CSS_ENTRY_OFFSET_DRAW_BOX_MEM]!=0) {
       webpage16 = (word_t *) (html_tag_css_list[HTML_TAG_CSS_ENTRY_OFFSET_DRAW_BOX_MEM]);
       webpage16[WEBPAGE_COMMAND_DRAW_BOX_OFFSET_HEIGTH] = html_tag_css_list[HTML_TAG_CSS_ENTRY_OFFSET_HEIGTH];
      }
     }
     
     //if is page smaller than view, change height to view height
     if(html_tag_css_list[HTML_TAG_CSS_ENTRY_OFFSET_HEIGTH]<internet_browser_webpage_height) {
      if(html_tag_css_list[HTML_TAG_CSS_ENTRY_OFFSET_DRAW_BOX_MEM]!=0) {
       webpage16 = (word_t *) (html_tag_css_list[HTML_TAG_CSS_ENTRY_OFFSET_DRAW_BOX_MEM]);
       webpage16[WEBPAGE_COMMAND_DRAW_BOX_OFFSET_HEIGTH] = internet_browser_webpage_height;
      }
     }
     
     html_page_height += html_tag_css_list[HTML_TAG_CSS_ENTRY_OFFSET_HEIGTH]; //save height of page
     goto page_is_finished;
    }
    
    //ending tag of list
    if(is_tag_equal((word_t *)html, "ul")==STATUS_TRUE || is_tag_equal((word_t *)html, "ol")==STATUS_TRUE) {
     if(html_list_type_pointer>0) {
      html_list_type_pointer--; //delete list
     }
    }
    
    //ending tag of table row
    if(is_tag_equal((word_t *)html, "tr")==STATUS_TRUE) {
     if(html_table_cell_width_list_pointer>0) {
      html_table_cell_width_list_pointer--; //delete
     }
    }

    //ending tag of table cell
    if(is_tag_equal((word_t *)html, "td")==STATUS_TRUE || is_tag_equal((word_t *)html, "th")==STATUS_TRUE) {
     html_table_last_cell_tag_ending = STATUS_TRUE;
    }

    //ending of link pointer TODO: more links inside
    if(is_tag_equal((word_t *)html, "a")==STATUS_TRUE) {
     webpage[0] = WEBPAGE_COMMAND_URL;
     webpage32 = (dword_t *) ((dword_t)webpage+WEBPAGE_COMMAND_URL_BYTE_OFFSET_POINTER);
     *webpage32 = 0;
     webpage+=3;
    }

    //ending tag of some tag that we processed
    //TODO: if this is ending not for last processed tag, like <p>something</h2>
    if(is_tag_equal((word_t *)html, "p")==STATUS_TRUE || is_tag_equal((word_t *)html, "b")==STATUS_TRUE || is_tag_equal((word_t *)html, "strong")==STATUS_TRUE || is_tag_equal((word_t *)html, "big")==STATUS_TRUE || is_tag_equal((word_t *)html, "small")==STATUS_TRUE || is_tag_equal((word_t *)html, "h1")==STATUS_TRUE || is_tag_equal((word_t *)html, "h2")==STATUS_TRUE || is_tag_equal((word_t *)html, "h3")==STATUS_TRUE || is_tag_equal((word_t *)html, "h4")==STATUS_TRUE || is_tag_equal((word_t *)html, "h5")==STATUS_TRUE || is_tag_equal((word_t *)html, "h6")==STATUS_TRUE || is_tag_equal((word_t *)html, "h6")==STATUS_TRUE || is_tag_equal((word_t *)html, "a")==STATUS_TRUE || is_tag_equal((word_t *)html, "ul")==STATUS_TRUE || is_tag_equal((word_t *)html, "ol")==STATUS_TRUE || is_tag_equal((word_t *)html, "li")==STATUS_TRUE || is_tag_equal((word_t *)html, "div")==STATUS_TRUE || is_tag_equal((word_t *)html, "table")==STATUS_TRUE || is_tag_equal((word_t *)html, "tr")==STATUS_TRUE || is_tag_equal((word_t *)html, "td")==STATUS_TRUE || is_tag_equal((word_t *)html, "th")==STATUS_TRUE) {
     goto remove_tag;
    }
    else { //skip all other tags, because we did not processed them
     goto skip_tag;
    }
    
    remove_tag:
    if(html_tag_css_list_pointer==1) { //invalid ending tag, this mean that we are only in <body> tag, and there is some tag ending
     goto skip_tag;
    }
    
    //set previous column and previous line width of tag
    html_tag_css_list = (dword_t *) (html_tag_css_list_mem+(html_tag_css_list_pointer-1)*HTML_TAG_CSS_ENTRY_SIZE);
    ib_draw_column = html_tag_css_list[HTML_TAG_CSS_ENTRY_OFFSET_STARTING_COLUMN];
    ib_line_width = html_tag_css_list[HTML_TAG_CSS_ENTRY_OFFSET_PREVIOUS_LINE_WIDTH];
    
    //add height of last printed line to ending tag
    if(ib_actual_line_width!=0) {
     html_tag_css_list[HTML_TAG_CSS_ENTRY_OFFSET_HEIGTH] += ib_line_biggest_spacing;
     
     if(html_tag_css_list[HTML_TAG_CSS_ENTRY_OFFSET_DRAW_BOX_MEM]!=0) {
      webpage16 = (word_t *) (html_tag_css_list[HTML_TAG_CSS_ENTRY_OFFSET_DRAW_BOX_MEM]);
      webpage16[WEBPAGE_COMMAND_DRAW_BOX_OFFSET_HEIGTH] = html_tag_css_list[HTML_TAG_CSS_ENTRY_OFFSET_HEIGTH];
     }
    }
    
    //go to next line or new column
    if(html_tag_css_list[HTML_TAG_CSS_ENTRY_OFFSET_WHERE_TO_MOVE_AT_END]==HTML_TAG_CSS_ENTRY_MOVE_TO_NEW_LINE) {  
     //move to next line 
     ib_draw_line = (html_tag_css_list[HTML_TAG_CSS_ENTRY_OFFSET_STARTING_LINE]+html_tag_css_list[HTML_TAG_CSS_ENTRY_OFFSET_HEIGTH]+html_tag_css_list[HTML_TAG_CSS_ENTRY_OFFSET_TOP_AND_BOTTOM_SKIP_SIZE]);
      
     //change height of all tags except for last one that is ending now
     dword_t add_height = html_tag_css_list[HTML_TAG_CSS_ENTRY_OFFSET_BOTTOM_SKIP_SIZE];
     if(ib_actual_line_width!=0) {
       add_height += ib_line_biggest_spacing;
     }
     for(int tag_entry=(html_tag_css_list_pointer-1); tag_entry>0; tag_entry--) {
      html_tag_css_list = (dword_t *) (html_tag_css_list_mem+(tag_entry-1)*HTML_TAG_CSS_ENTRY_SIZE);
    
      if(html_tag_css_list[HTML_TAG_CSS_ENTRY_OFFSET_HEIGTH_CALCULATION_TYPE]==HTML_TAG_CSS_ENTRY_HEIGTH_CALCULATION_ADD) {
       html_tag_css_list[HTML_TAG_CSS_ENTRY_OFFSET_HEIGTH] += add_height;
     
       if(html_tag_css_list[HTML_TAG_CSS_ENTRY_OFFSET_DRAW_BOX_MEM]!=0) {
        webpage16 = (word_t *) (html_tag_css_list[HTML_TAG_CSS_ENTRY_OFFSET_DRAW_BOX_MEM]);
        webpage16[WEBPAGE_COMMAND_DRAW_BOX_OFFSET_HEIGTH] = html_tag_css_list[HTML_TAG_CSS_ENTRY_OFFSET_HEIGTH];
       }
      }
      else if(html_tag_css_list[HTML_TAG_CSS_ENTRY_OFFSET_HEIGTH_CALCULATION_TYPE]==HTML_TAG_CSS_ENTRY_HEIGTH_CALCULATION_COMPARE) {
       if(html_tag_css_list[HTML_TAG_CSS_ENTRY_OFFSET_HEIGTH]<html_tag_css_list[HTML_TAG_CSS_ENTRY_NUM_OF_ENTRIES+HTML_TAG_CSS_ENTRY_OFFSET_HEIGTH]) {
        add_height = (html_tag_css_list[HTML_TAG_CSS_ENTRY_NUM_OF_ENTRIES+HTML_TAG_CSS_ENTRY_OFFSET_HEIGTH]-html_tag_css_list[HTML_TAG_CSS_ENTRY_OFFSET_HEIGTH]);
        html_tag_css_list[HTML_TAG_CSS_ENTRY_OFFSET_HEIGTH] = html_tag_css_list[HTML_TAG_CSS_ENTRY_NUM_OF_ENTRIES+HTML_TAG_CSS_ENTRY_OFFSET_HEIGTH];
       }
       else {
        break;
       }
      }
     }
    
     webpage_last_line_command_pointer = ((dword_t)webpage);
     webpage[0] = WEBPAGE_COMMAND_MOVE_TO_LINE;
     webpage32 = (dword_t *) ((dword_t)webpage+WEBPAGE_COMMAND_MOVE_TO_LINE_BYTE_OFFSET_COLUMN);
     *webpage32 = ib_draw_column;
     webpage32 = (dword_t *) ((dword_t)webpage+WEBPAGE_COMMAND_MOVE_TO_LINE_BYTE_OFFSET_LINE);
     *webpage32 = ib_draw_line;
     webpage[WEBPAGE_COMMAND_MOVE_TO_LINE_OFFSET_LINE_SIZE] = 0;
     webpage += 6;
     
     ib_font_spacing = 0;
     ib_actual_line_width = 0;
     ib_line_biggest_spacing = 0;
    }
    else if(html_tag_css_list[HTML_TAG_CSS_ENTRY_OFFSET_WHERE_TO_MOVE_AT_END]==HTML_TAG_CSS_ENTRY_MOVE_TO_NEW_COLUMN) {
     if((html_tag_css_list[HTML_TAG_CSS_ENTRY_OFFSET_WIDTH]+html_tag_css_list[HTML_TAG_CSS_ENTRY_OFFSET_LEFT_AND_RIGHT_SKIP_SIZE])!=0) { //we have to go to next column
      ib_draw_line = html_tag_css_list[HTML_TAG_CSS_ENTRY_OFFSET_STARTING_LINE];
      ib_draw_column = (html_tag_css_list[HTML_TAG_CSS_ENTRY_OFFSET_STARTING_COLUMN]+html_tag_css_list[HTML_TAG_CSS_ENTRY_OFFSET_WIDTH]+html_tag_css_list[HTML_TAG_CSS_ENTRY_OFFSET_LEFT_AND_RIGHT_SKIP_SIZE]);
      
      //change height of all tags except for last one that is ending now
      dword_t add_height = html_tag_css_list[HTML_TAG_CSS_ENTRY_OFFSET_BOTTOM_SKIP_SIZE];
      if(ib_actual_line_width!=0) {
        add_height += ib_line_biggest_spacing;
      }
      for(int tag_entry=(html_tag_css_list_pointer-1); tag_entry>0; tag_entry--) {
       html_tag_css_list = (dword_t *) (html_tag_css_list_mem+(tag_entry-1)*HTML_TAG_CSS_ENTRY_SIZE);
    
       if(html_tag_css_list[HTML_TAG_CSS_ENTRY_OFFSET_HEIGTH_CALCULATION_TYPE]==HTML_TAG_CSS_ENTRY_HEIGTH_CALCULATION_ADD) {
        html_tag_css_list[HTML_TAG_CSS_ENTRY_OFFSET_HEIGTH] += add_height;
      
        if(html_tag_css_list[HTML_TAG_CSS_ENTRY_OFFSET_DRAW_BOX_MEM]!=0) {
         webpage16 = (word_t *) (html_tag_css_list[HTML_TAG_CSS_ENTRY_OFFSET_DRAW_BOX_MEM]);
         webpage16[WEBPAGE_COMMAND_DRAW_BOX_OFFSET_HEIGTH] = html_tag_css_list[HTML_TAG_CSS_ENTRY_OFFSET_HEIGTH];
        }
       }
       else if(html_tag_css_list[HTML_TAG_CSS_ENTRY_OFFSET_HEIGTH_CALCULATION_TYPE]==HTML_TAG_CSS_ENTRY_HEIGTH_CALCULATION_COMPARE) {
        if(html_tag_css_list[HTML_TAG_CSS_ENTRY_OFFSET_HEIGTH]<html_tag_css_list[HTML_TAG_CSS_ENTRY_NUM_OF_ENTRIES+HTML_TAG_CSS_ENTRY_OFFSET_HEIGTH]) {
         add_height = (html_tag_css_list[HTML_TAG_CSS_ENTRY_NUM_OF_ENTRIES+HTML_TAG_CSS_ENTRY_OFFSET_HEIGTH]-html_tag_css_list[HTML_TAG_CSS_ENTRY_OFFSET_HEIGTH]);
         html_tag_css_list[HTML_TAG_CSS_ENTRY_OFFSET_HEIGTH] = html_tag_css_list[HTML_TAG_CSS_ENTRY_NUM_OF_ENTRIES+HTML_TAG_CSS_ENTRY_OFFSET_HEIGTH];
        }
        else {
         break;
        }
       }
      }
      
      webpage_last_line_command_pointer = ((dword_t)webpage);
      webpage[0] = WEBPAGE_COMMAND_MOVE_TO_LINE;
      webpage32 = (dword_t *) ((dword_t)webpage+WEBPAGE_COMMAND_MOVE_TO_LINE_BYTE_OFFSET_COLUMN);
      *webpage32 = ib_draw_column;
      webpage32 = (dword_t *) ((dword_t)webpage+WEBPAGE_COMMAND_MOVE_TO_LINE_BYTE_OFFSET_LINE);
      *webpage32 = ib_draw_line;
      webpage[WEBPAGE_COMMAND_MOVE_TO_LINE_OFFSET_LINE_SIZE] = 0;
      webpage += 6;
      
      ib_font_spacing = 0;
      ib_actual_line_width = 0;
      ib_line_biggest_spacing = 0;
     }
    }
    
    //delete tag
    html_tag_css_list_pointer--;
    
    //get actual CSS settings after we removed CSS of ending tag
    //default CSS values
    css_font_size = 10;
    css_font_spacing = 15;
    css_font_emphasis = SF_EMPHASIS_NONE;
    css_font_color = BLACK;
    css_font_alignment = WEBPAGE_LEFT_ALIGNMENT;
    //read CSS of all tags
    for(int tag_entry=0; tag_entry<html_tag_css_list_pointer; tag_entry++) {
     html_tag_css_list = (dword_t *) (html_tag_css_list_mem+tag_entry*HTML_TAG_CSS_ENTRY_SIZE);
     
     if(html_tag_css_list[HTML_TAG_CSS_ENTRY_OFFSET_FONT_SIZE]!=WEBPAGE_EMPTY_ENTRY_32) {
      css_font_size = html_tag_css_list[HTML_TAG_CSS_ENTRY_OFFSET_FONT_SIZE];
     }
     if(html_tag_css_list[HTML_TAG_CSS_ENTRY_OFFSET_FONT_COLOR]!=WEBPAGE_EMPTY_ENTRY_32) {
      css_font_color = html_tag_css_list[HTML_TAG_CSS_ENTRY_OFFSET_FONT_COLOR];
     }
     if(html_tag_css_list[HTML_TAG_CSS_ENTRY_OFFSET_FONT_EMPHASIS]!=WEBPAGE_EMPTY_ENTRY_32) {
      css_font_emphasis |= html_tag_css_list[HTML_TAG_CSS_ENTRY_OFFSET_FONT_EMPHASIS];
     }
     if(html_tag_css_list[HTML_TAG_CSS_ENTRY_OFFSET_FONT_SPACING]!=WEBPAGE_EMPTY_ENTRY_32) {
      css_font_spacing = html_tag_css_list[HTML_TAG_CSS_ENTRY_OFFSET_FONT_SPACING];
     }
     if(html_tag_css_list[HTML_TAG_CSS_ENTRY_OFFSET_FONT_ALIGNMENT]!=WEBPAGE_EMPTY_ENTRY_32) {
      css_font_alignment = html_tag_css_list[HTML_TAG_CSS_ENTRY_OFFSET_FONT_ALIGNMENT];
     }
    }
    
    //apply changes
    goto process_css_of_tag;
   }
   
   //TAG
   //here we set dafault CSS values of tags
   if(is_tag_equal((word_t *)html, "p")==STATUS_TRUE) { //<p>
    ib_tag_new_line = STATUS_TRUE;
    css_margin_top = 10;
    css_margin_bottom = 10;
   }
   else if(is_tag_equal((word_t *)html, "br")==STATUS_TRUE) { //<br>
    ib_tag_new_line = STATUS_TRUE;
    html_not_pair_tag = STATUS_TRUE;
    goto process_css_of_tag; //this is not pair tag
   }
   else if(is_tag_equal((word_t *)html, "b")==STATUS_TRUE) { //<b>
    css_font_emphasis |= SF_EMPHASIS_BOLD;
   }
   else if(is_tag_equal((word_t *)html, "strong")==STATUS_TRUE) { //<strong>
    css_font_emphasis |= SF_EMPHASIS_BOLD;
   }
   else if(is_tag_equal((word_t *)html, "big")==STATUS_TRUE) { //<big>
    css_font_size = (ib_font_size*5/4);
   }
   else if(is_tag_equal((word_t *)html, "small")==STATUS_TRUE) { //<small>
    css_font_size = (ib_font_size*3/4);
   }
   else if(is_tag_equal((word_t *)html, "h1")==STATUS_TRUE) { //<h1>
    ib_tag_new_line = STATUS_TRUE;
    css_font_emphasis |= SF_EMPHASIS_BOLD;
    css_font_size = 25;
    css_font_spacing = 37;
    
    css_padding_top = 12;
    css_padding_bottom = 12;
   }
   else if(is_tag_equal((word_t *)html, "h2")==STATUS_TRUE) { //<h2>
    ib_tag_new_line = STATUS_TRUE;
    css_font_emphasis |= SF_EMPHASIS_BOLD;
    css_font_size = 22;
    css_font_spacing = 33;
    
    css_padding_top = 11;
    css_padding_bottom = 11;
   }
   else if(is_tag_equal((word_t *)html, "h3")==STATUS_TRUE) { //<h3>
    ib_tag_new_line = STATUS_TRUE;
    css_font_emphasis |= SF_EMPHASIS_BOLD;
    css_font_size = 19;
    css_font_spacing = 28;
    
    css_padding_top = 9;
    css_padding_bottom = 9;
   }
   else if(is_tag_equal((word_t *)html, "h4")==STATUS_TRUE) { //<h4>
    ib_tag_new_line = STATUS_TRUE;
    css_font_emphasis |= SF_EMPHASIS_BOLD;
    css_font_size = 16;
    css_font_spacing = 24;
    
    css_padding_top = 8;
    css_padding_bottom = 8;
   }
   else if(is_tag_equal((word_t *)html, "h5")==STATUS_TRUE) { //<h5>
    ib_tag_new_line = STATUS_TRUE;
    css_font_emphasis |= SF_EMPHASIS_BOLD;
    css_font_size = 13;
    css_font_spacing = 19;
    
    css_padding_top = 6;
    css_padding_bottom = 6;
   }
   else if(is_tag_equal((word_t *)html, "h6")==STATUS_TRUE) { //<h6>
    ib_tag_new_line = STATUS_TRUE;
    css_font_emphasis |= SF_EMPHASIS_BOLD;
    css_font_size = 10;
    css_font_spacing = 15;
    
    css_padding_top = 5;
    css_padding_bottom = 5;
   }
   else if(is_tag_equal((word_t *)html, "a")==STATUS_TRUE) { //<a>
    css_font_color = 0x0000FF;
    
    //save pointer to link
    dword_t style_attribute_mem = find_tag_attribute((dword_t)html, "href");
    if(style_attribute_mem!=STATUS_FALSE) {
     webpage[0] = WEBPAGE_COMMAND_URL;
     webpage32 = (dword_t *) ((dword_t)webpage+WEBPAGE_COMMAND_URL_BYTE_OFFSET_POINTER);
     *webpage32 = style_attribute_mem;
     webpage+=3;
    }
   }
   else if(is_tag_equal((word_t *)html, "ul")==STATUS_TRUE) { //<ul>
    if(html_list_type_pointer>=1000) {
     log("IB: too many lists\n");
    }
    
    html_list_type_pointer++;
    html_list_type[html_list_type_pointer] = 0xFFFFFFFF;
    
    ib_tag_new_line = STATUS_TRUE;
    css_padding_left = 20;
    css_margin_top = 10;
    css_margin_bottom = 10;
   }
   else if(is_tag_equal((word_t *)html, "ol")==STATUS_TRUE) { //<ol>
    if(html_list_type_pointer>=1000) {
     log("IB: too many lists\n");
    }
    
    html_list_type_pointer++;
    html_list_type[html_list_type_pointer] = 1;
    
    ib_tag_new_line = STATUS_TRUE;
    css_padding_left = 20;
    css_margin_top = 10;
    css_margin_bottom = 10;
    
    goto skip_tag;
   }
   else if(is_tag_equal((word_t *)html, "li")==STATUS_TRUE) { //<li>
    ib_tag_new_line = STATUS_TRUE;
    
    if(html_list_type[html_list_type_pointer]==0xFFFFFFFF) {
     html_list_tag = 0xFFFFFFFF;
    }
    else if(html_list_type[html_list_type_pointer]!=0) {
     html_list_tag = html_list_type[html_list_type_pointer];
     
     html_list_type[html_list_type_pointer]++;
    }
   }
   else if(is_tag_equal((word_t *)html, "div")==STATUS_TRUE) { //<div>
    ib_tag_new_line = STATUS_TRUE;
   }
   else if(is_tag_equal((word_t *)html, "table")==STATUS_TRUE) { //<table>
    ib_tag_new_line = STATUS_TRUE;
   }
   else if(is_tag_equal((word_t *)html, "tr")==STATUS_TRUE) { //<tr>
    ib_tag_new_line = STATUS_TRUE;
    html_tag_height_calculation_type = HTML_TAG_CSS_ENTRY_HEIGTH_CALCULATION_COMPARE;
    
    //calculate width of every table cell
    html_num_of_tags_to_skip = 0;
    dword_t number_of_cells = 0;
    word_t *html2 = (word_t *) ((dword_t)html);
    do {
     if(*html2==0) {
      break;
     }
     
     if(*html2=='<') {
      if(html2[1]=='/') {
       html2+=2;
       
       //skip spaces
       while(*html2<=' ' && *html2!=0) {
        html2++;
       }
       
       if(html2[0]=='t' && html2[1]=='r') {
        html_num_of_tags_to_skip--;
       }
      }
      else if(is_tag_equal((word_t *)html2, "tr")==STATUS_TRUE) {
       html_num_of_tags_to_skip++;
      }
      else if(html_num_of_tags_to_skip==1 && (is_tag_equal((word_t *)html2, "td")==STATUS_TRUE || is_tag_equal((word_t *)html2, "th")==STATUS_TRUE)) {
       dword_t style_attribute_mem = find_tag_attribute((dword_t)html2, "colspan");
       if(style_attribute_mem!=STATUS_FALSE) {
        number_of_cells += convert_word_string_to_number(style_attribute_mem);
       }
       else {
        number_of_cells++;
       }
      }
      
      //skip tag
      while(*html2!='>') {
       if(*html2==0) {
        break;
       }
       html2++;
      }
      html2++;
      continue;
     }
     
     html2++;
    } while(html_num_of_tags_to_skip>0);
    
    //add cell width to table
    html_table_cell_width_list[html_table_cell_width_list_pointer] = (ib_line_width/number_of_cells);
    html_table_cell_width_list_pointer++;
    
    html_table_last_cell_tag_ending = STATUS_TRUE;
   }
   else if(is_tag_equal((word_t *)html, "td")==STATUS_TRUE) { //<td>
    if(html_table_last_cell_tag_ending==STATUS_FALSE) { //handle if last cell was not ended
     goto remove_tag;
    }
   
    ib_tag_new_column = STATUS_TRUE;

    if(html_table_cell_width_list_pointer>0) {
     dword_t style_attribute_mem = find_tag_attribute((dword_t)html, "colspan");
     if(style_attribute_mem!=STATUS_FALSE) {
      html_table_cell_width = (html_table_cell_width_list[html_table_cell_width_list_pointer-1]*convert_word_string_to_number(style_attribute_mem));
     }
     else {
      html_table_cell_width = html_table_cell_width_list[html_table_cell_width_list_pointer-1];
     }
    }
    else {
     html_table_cell_width = ib_line_width;
    }
    
    html_table_last_cell_tag_ending = STATUS_FALSE;
   }
   else if(is_tag_equal((word_t *)html, "th")==STATUS_TRUE) { //<th>
    if(html_table_last_cell_tag_ending==STATUS_FALSE) { //handle if last cell was not ended
     goto remove_tag;
    }
    
    ib_tag_new_column = STATUS_TRUE;
    css_font_alignment = WEBPAGE_MIDDLE_ALIGNMENT;

    if(html_table_cell_width_list_pointer>0) {
     dword_t style_attribute_mem = find_tag_attribute((dword_t)html, "colspan");
     if(style_attribute_mem!=STATUS_FALSE) {
      html_table_cell_width = (html_table_cell_width_list[html_table_cell_width_list_pointer-1]*convert_word_string_to_number(style_attribute_mem));
     }
     else {
      html_table_cell_width = html_table_cell_width_list[html_table_cell_width_list_pointer-1];
     }
    }
    else {
     html_table_cell_width = ib_line_width;
    }
    
    html_table_last_cell_tag_ending = STATUS_FALSE;
   }
   else if(is_tag_equal((word_t *)html, "style")==STATUS_TRUE) { //<style>
    //skip tag
    while(*html!='>') {
     if(*html==0) {
      goto page_is_finished;
     }
     
     html++;
    }
    html++;
    
    css = (word_t *) ((dword_t)html);
    css_parse_file();
   
    //skip everything in this tag - find ending tag
    while(is_tag_equal((word_t *)html, "/style")==STATUS_FALSE) {
     if(*html==0) {
      goto page_is_finished;
     }
     
     html++;
    }
    
    //skip </style> tag
    goto skip_tag;
   }
   else if(is_tag_equal((word_t *)html, "script")==STATUS_TRUE) { //<script>
    //skip everything in this tag - find ending tag
    while(is_tag_equal((word_t *)html, "/script")==STATUS_FALSE) {
     if(*html==0) {
      goto page_is_finished;
     }
     
     html++;
    }
    
    //skip </script> tag
    goto skip_tag;
   }
   else if(html[1]=='!' && html[2]=='-' && html[3]=='-') { //comment
    html+=4;
    
    //skip everything in this tag
    while(html[0]!='-' || html[1]!='-' || html[2]!='>') {
     if(*html==0) {
      goto page_is_finished;
     }
         
     html++;
    }
    
    html += 3;
    continue;
   }
   else if(is_tag_equal((word_t *)html, "body")==STATUS_TRUE) { //<body>
    //no CSS, just add tag to list
   }
   else { //skip any other tag
    goto skip_tag;
   }
   
   //GET CSS FROM TAG STYLE
   css_entries_list = (dword_t *) css_entries_tag_list_mem;
   html++; //skip <
   for(int i=0, is_tag_founded=STATUS_FALSE; i<css_entries_tag_list_pointer; i++, css_entries_list+=2) {
    //test if this entry contain actual tag
    word_t *entry_string = (word_t *) (css_entries_list[0]);
    
    for(int i=0; i<1000; i++) {
     if(entry_string[i]=='{' || entry_string[i]==' ' || entry_string[i]==',') {
      if(html[i]==' ' || html[i]=='>') {
       is_tag_founded = STATUS_TRUE;      
      }
      break;
     }
     
     if(html[i]!=entry_string[i]) {
      break; //this is not style for this tag
     }
     
    }
    
    //this is style for this tag
    if(is_tag_founded==STATUS_TRUE) {
     css = (word_t *) (css_entries_list[1]);
     css_parse_declaration_block();
     break;
    }
   }
   
   //GET CSS FROM CLASS STYLES
   dword_t style_attribute_mem = find_tag_attribute((dword_t)html, "class");
   if(style_attribute_mem!=STATUS_FALSE) {
    word_t *class_string = (word_t *) style_attribute_mem;
    
    while((*class_string>='a' && *class_string<='z') || (*class_string>='A' && *class_string<='Z') || (*class_string>='0' && *class_string<='9')) {
     css_entries_list = (dword_t *) css_entries_class_list_mem;
     
     for(int i=0, is_class_founded=STATUS_FALSE; i<css_entries_class_list_pointer; i++, css_entries_list+=2) {
      //test if this entry contain actual tag
      word_t *entry_string = (word_t *) (css_entries_list[0]);
    
      for(int i=0; i<1000; i++) {
       if(entry_string[i]=='{' || entry_string[i]==' ' || entry_string[i]==',') {
        if(class_string[i]==' ' || class_string[i]=='"') {
         is_class_founded = STATUS_TRUE;      
        }
        break;
       }
     
       if(class_string[i]!=entry_string[i]) {
        break; //this is not style for this tag
       }
     
      }
    
      //this is style for this tag
      if(is_class_founded==STATUS_TRUE) {
       css = (word_t *) (css_entries_list[1]);
       css_parse_declaration_block();
       break;
      }
    
     }
     
     //skip to next class name
     while(*class_string>' ' && *class_string!='"' && *class_string!='>') {
      class_string++;
     }
     while(*class_string<=' ' && *class_string!=0) {
      class_string++;
     }
    }
   }
   
   //GET CSS FROM ID STYLE   
   style_attribute_mem = find_tag_attribute((dword_t)html, "id");
   if(style_attribute_mem!=STATUS_FALSE) {
    css_entries_list = (dword_t *) css_entries_id_list_mem;
    word_t *id_string = (word_t *) style_attribute_mem;
    
    for(int i=0, is_id_founded=STATUS_FALSE; i<css_entries_id_list_pointer; i++, css_entries_list+=2) {
     //test if this entry contain actual tag
     word_t *entry_string = (word_t *) (css_entries_list[0]);
    
     for(int i=0; i<1000; i++) {
      if(entry_string[i]=='{' || entry_string[i]==' ' || entry_string[i]==',') {
       if(id_string[i]=='"' || id_string[i]==' ') {
        is_id_founded = STATUS_TRUE;      
       }
       break;
      }
     
      if(id_string[i]!=entry_string[i]) {
       break; //this is not style for this tag
      }
     
     }
    
     //this is style for this tag
     if(is_id_founded==STATUS_TRUE) {
      css = (word_t *) (css_entries_list[1]);
      css_parse_declaration_block();
      break;
     }
    
    }
   }
   
   //GET CSS FROM ATTRIBUTE style
   style_attribute_mem = find_tag_attribute((dword_t)html, "style");
   if(style_attribute_mem!=STATUS_FALSE) {
    css = (word_t *) style_attribute_mem;
    css_parse_declaration_block();
   }
   
   //SET NEW LINE VARIABILE
   if((css_margin_top+css_margin_left+css_margin_bottom+css_margin_right+css_top_border_size+css_left_border_size+css_bottom_border_size+css_right_border_size+css_padding_top+css_padding_left+css_padding_bottom+css_padding_right)!=0) {
    ib_tag_new_line = STATUS_TRUE;
   }
   
   //ADD TAG TO LIST
   if(html_tag_css_list_pointer>=1000) {
    log("IB error: more than 1000 tags inside\n");
    goto skip_tag;
   }
   html_tag_css_list = (dword_t *) (html_tag_css_list_mem+html_tag_css_list_pointer*HTML_TAG_CSS_ENTRY_SIZE);
   html_tag_css_list[HTML_TAG_CSS_ENTRY_OFFSET_WHERE_TO_MOVE_AT_END] = 0;
   if(ib_tag_new_line==STATUS_TRUE) {
    html_tag_css_list[HTML_TAG_CSS_ENTRY_OFFSET_WHERE_TO_MOVE_AT_END] = HTML_TAG_CSS_ENTRY_MOVE_TO_NEW_LINE;
   }
   else if(ib_tag_new_column==STATUS_TRUE) {
    html_tag_css_list[HTML_TAG_CSS_ENTRY_OFFSET_WHERE_TO_MOVE_AT_END] = HTML_TAG_CSS_ENTRY_MOVE_TO_NEW_COLUMN;
   }
   html_tag_css_list[HTML_TAG_CSS_ENTRY_OFFSET_HEIGTH] = (css_padding_top+css_padding_bottom);
   html_tag_css_list[HTML_TAG_CSS_ENTRY_OFFSET_WIDTH] = (ib_line_width-(css_margin_left+css_left_border_size+css_right_border_size+css_margin_right));
   html_tag_css_list[HTML_TAG_CSS_ENTRY_OFFSET_DRAW_BOX_MEM] = 0;
   html_tag_css_list[HTML_TAG_CSS_ENTRY_OFFSET_FONT_SIZE] = css_font_size;
   html_tag_css_list[HTML_TAG_CSS_ENTRY_OFFSET_FONT_COLOR] = css_font_color;
   html_tag_css_list[HTML_TAG_CSS_ENTRY_OFFSET_FONT_EMPHASIS] = css_font_emphasis;
   html_tag_css_list[HTML_TAG_CSS_ENTRY_OFFSET_FONT_ALIGNMENT] = css_font_alignment;
   html_tag_css_list[HTML_TAG_CSS_ENTRY_OFFSET_FONT_SPACING] = css_font_spacing;
   html_tag_css_list[HTML_TAG_CSS_ENTRY_OFFSET_STARTING_LINE] = (ib_draw_line+ib_line_biggest_spacing); //skip actual line
   html_tag_css_list[HTML_TAG_CSS_ENTRY_OFFSET_STARTING_COLUMN] = ib_draw_column;
   html_tag_css_list[HTML_TAG_CSS_ENTRY_OFFSET_PREVIOUS_LINE_WIDTH] = ib_line_width;
   html_tag_css_list[HTML_TAG_CSS_ENTRY_OFFSET_BOTTOM_SKIP_SIZE] = (css_padding_bottom+css_bottom_border_size+css_margin_bottom);
   html_tag_css_list[HTML_TAG_CSS_ENTRY_OFFSET_TOP_AND_BOTTOM_SKIP_SIZE] = (css_margin_top+css_top_border_size+css_bottom_border_size+css_margin_bottom);
   html_tag_css_list[HTML_TAG_CSS_ENTRY_OFFSET_LEFT_AND_RIGHT_SKIP_SIZE] = (css_margin_left+css_left_border_size+css_padding_left+css_padding_right+css_right_border_size+css_margin_right);
   html_tag_css_list[HTML_TAG_CSS_ENTRY_OFFSET_HEIGTH_CALCULATION_TYPE] = html_tag_height_calculation_type;
   html_tag_css_list_pointer++;
   
   if(is_tag_equal((word_t *)html, "body")==STATUS_TRUE) { //<body>
    html_page_height += (css_margin_top+css_top_border_size+css_bottom_border_size+css_margin_bottom);
   }
   
   if(html_table_cell_width!=0) {
    ib_line_width = html_table_cell_width;
    html_tag_css_list[HTML_TAG_CSS_ENTRY_OFFSET_WIDTH] = (ib_line_width-(css_margin_left+css_left_border_size+css_right_border_size+css_margin_right));
   }
   
   //PROCESS CSS OF TAG
   process_css_of_tag:
   //draw box
   if(css_background_color!=WEBPAGE_EMPTY_ENTRY_32 || css_top_border_type!=WEBPAGE_BORDER_TYPE_NONE || css_left_border_type!=WEBPAGE_BORDER_TYPE_NONE || css_bottom_border_type!=WEBPAGE_BORDER_TYPE_NONE || css_right_border_type!=WEBPAGE_BORDER_TYPE_NONE) {     
    //move to position from where box will be drawed   
    webpage[0] = WEBPAGE_COMMAND_MOVE_DRAW_POSITION;
    webpage32 = (dword_t *) ((dword_t)webpage+WEBPAGE_COMMAND_MOVE_DRAW_POSITION_BYTE_OFFSET_COLUMN);
    *webpage32 = (ib_draw_column+css_margin_left);
    webpage32 = (dword_t *) ((dword_t)webpage+WEBPAGE_COMMAND_MOVE_DRAW_POSITION_BYTE_OFFSET_LINE);
    *webpage32 = (ib_draw_line+ib_line_biggest_spacing+css_margin_top);
    webpage += 5;
    
    //save draw box command position
    html_tag_css_list[HTML_TAG_CSS_ENTRY_OFFSET_DRAW_BOX_MEM] = ((dword_t)webpage);
    
    //draw box command
    webpage[0] = WEBPAGE_COMMAND_DRAW_BOX;
    webpage[WEBPAGE_COMMAND_DRAW_BOX_OFFSET_WIDTH] = html_tag_css_list[HTML_TAG_CSS_ENTRY_OFFSET_WIDTH];
    webpage[WEBPAGE_COMMAND_DRAW_BOX_OFFSET_HEIGTH] = html_tag_css_list[HTML_TAG_CSS_ENTRY_OFFSET_HEIGTH];
    webpage32 = (dword_t *) ((dword_t)webpage+WEBPAGE_COMMAND_DRAW_BOX_BYTE_OFFSET_BACKGROUND_COLOR);
    *webpage32 = css_background_color;
    webpage[WEBPAGE_COMMAND_DRAW_BOX_OFFSET_TOP_BORDER_TYPE] = css_top_border_type;
    webpage[WEBPAGE_COMMAND_DRAW_BOX_OFFSET_TOP_BORDER_SIZE] = css_top_border_size;
    webpage32 = (dword_t *) ((dword_t)webpage+WEBPAGE_COMMAND_DRAW_BOX_BYTE_OFFSET_TOP_BORDER_COLOR);
    *webpage32 = css_top_border_color;
    webpage[WEBPAGE_COMMAND_DRAW_BOX_OFFSET_BOTTOM_BORDER_TYPE] = css_bottom_border_type;
    webpage[WEBPAGE_COMMAND_DRAW_BOX_OFFSET_BOTTOM_BORDER_SIZE] = css_bottom_border_size;
    webpage32 = (dword_t *) ((dword_t)webpage+WEBPAGE_COMMAND_DRAW_BOX_BYTE_OFFSET_BOTTOM_BORDER_COLOR);
    *webpage32 = css_bottom_border_color;
    webpage[WEBPAGE_COMMAND_DRAW_BOX_OFFSET_LEFT_BORDER_TYPE] = css_left_border_type;
    webpage[WEBPAGE_COMMAND_DRAW_BOX_OFFSET_LEFT_BORDER_SIZE] = css_left_border_size;
    webpage32 = (dword_t *) ((dword_t)webpage+WEBPAGE_COMMAND_DRAW_BOX_BYTE_OFFSET_LEFT_BORDER_COLOR);
    *webpage32 = css_left_border_color;
    webpage[WEBPAGE_COMMAND_DRAW_BOX_OFFSET_RIGHT_BORDER_TYPE] = css_right_border_type;
    webpage[WEBPAGE_COMMAND_DRAW_BOX_OFFSET_RIGHT_BORDER_SIZE] = css_right_border_size;
    webpage32 = (dword_t *) ((dword_t)webpage+WEBPAGE_COMMAND_DRAW_BOX_BYTE_OFFSET_RIGHT_BORDER_COLOR);
    *webpage32 = css_right_border_color;
    webpage += 21;
   
    //if there is box, we have to go to next line also with text
    ib_tag_new_line = STATUS_TRUE;
   }
   //go to next text line
   if(ib_tag_new_line==STATUS_TRUE) {
    //skip last printed line and move to inside of box
    ib_draw_column += (css_margin_left+css_left_border_size+css_padding_left);
    if(ib_actual_line_width!=0) {
     ib_draw_line += ib_line_biggest_spacing;
    }
    ib_draw_line += (css_margin_top+css_top_border_size+css_padding_top);
    
    //change height of all tags, except for last tag, because height of last tag is 0 because it has no content now
    dword_t tag_list_num_of_cycles = (html_tag_css_list_pointer-1);
    if(html_not_pair_tag==STATUS_TRUE) {
     tag_list_num_of_cycles++; //if last tag is not pair (like <br>), we will process all tags
    }
    dword_t add_height = (css_margin_top+css_top_border_size+css_padding_top);
    if(ib_actual_line_width!=0) {
      add_height += ib_line_biggest_spacing;
     }
    for(int tag_entry=tag_list_num_of_cycles; tag_entry>0; tag_entry--) {
     html_tag_css_list = (dword_t *) (html_tag_css_list_mem+(tag_entry-1)*HTML_TAG_CSS_ENTRY_SIZE);
    
     if(html_tag_css_list[HTML_TAG_CSS_ENTRY_OFFSET_HEIGTH_CALCULATION_TYPE]==HTML_TAG_CSS_ENTRY_HEIGTH_CALCULATION_ADD) {
      html_tag_css_list[HTML_TAG_CSS_ENTRY_OFFSET_HEIGTH] += add_height;
     
      if(html_tag_css_list[HTML_TAG_CSS_ENTRY_OFFSET_DRAW_BOX_MEM]!=0) {
       webpage16 = (word_t *) (html_tag_css_list[HTML_TAG_CSS_ENTRY_OFFSET_DRAW_BOX_MEM]);
       webpage16[WEBPAGE_COMMAND_DRAW_BOX_OFFSET_HEIGTH] = html_tag_css_list[HTML_TAG_CSS_ENTRY_OFFSET_HEIGTH];
      }
     }
     else if(html_tag_css_list[HTML_TAG_CSS_ENTRY_OFFSET_HEIGTH_CALCULATION_TYPE]==HTML_TAG_CSS_ENTRY_HEIGTH_CALCULATION_COMPARE) {
      if(html_tag_css_list[HTML_TAG_CSS_ENTRY_OFFSET_HEIGTH]<html_tag_css_list[HTML_TAG_CSS_ENTRY_NUM_OF_ENTRIES+HTML_TAG_CSS_ENTRY_OFFSET_HEIGTH]) {
       add_height = (html_tag_css_list[HTML_TAG_CSS_ENTRY_NUM_OF_ENTRIES+HTML_TAG_CSS_ENTRY_OFFSET_HEIGTH]-html_tag_css_list[HTML_TAG_CSS_ENTRY_OFFSET_HEIGTH]);
       html_tag_css_list[HTML_TAG_CSS_ENTRY_OFFSET_HEIGTH] = html_tag_css_list[HTML_TAG_CSS_ENTRY_NUM_OF_ENTRIES+HTML_TAG_CSS_ENTRY_OFFSET_HEIGTH];
      }
      else {
       break;
      }
     }
    }
    
    //move to actual text line
    webpage_last_line_command_pointer = ((dword_t)webpage);
    webpage[0] = WEBPAGE_COMMAND_MOVE_TO_LINE;
    webpage32 = (dword_t *) ((dword_t)webpage+WEBPAGE_COMMAND_MOVE_TO_LINE_BYTE_OFFSET_COLUMN);
    *webpage32 = ib_draw_column;
    webpage32 = (dword_t *) ((dword_t)webpage+WEBPAGE_COMMAND_MOVE_TO_LINE_BYTE_OFFSET_LINE);
    *webpage32 = ib_draw_line;
    webpage[WEBPAGE_COMMAND_MOVE_TO_LINE_OFFSET_LINE_SIZE] = 0;
    webpage += 6;

    //change variables
    if(html_not_pair_tag==STATUS_FALSE) {
     ib_line_width -= (css_margin_left+css_left_border_size+css_padding_left+css_padding_right+css_right_border_size+css_margin_right);
    }
    ib_actual_line_width = 0;
    ib_line_biggest_spacing = 0;
   }
   //set font size
   if(css_font_size!=WEBPAGE_EMPTY_ENTRY_32 && ib_font_size!=css_font_size) {
    webpage[0] = WEBPAGE_COMMAND_CHANGE_TEXT_SIZE;
    webpage[WEBPAGE_COMMAND_CHANGE_TEXT_SIZE_OFFSET_TEXT_SIZE] = css_font_size;
    webpage+=2;
    
    ib_font_size = css_font_size;
   }
   //set font color
   if(css_font_color!=WEBPAGE_EMPTY_ENTRY_32 && ib_font_size!=css_font_color) {
    webpage[0] = WEBPAGE_COMMAND_CHANGE_TEXT_COLOR;
    webpage32 = (dword_t *) ((dword_t)webpage+WEBPAGE_COMMAND_CHANGE_TEXT_COLOR_BYTE_OFFSET_COLOR);
    *webpage32 = css_font_color;
    webpage+=3;
    
    ib_font_color = css_font_color;
   }
   //set font emphasis
   if(ib_font_emphasis!=css_font_emphasis) {
    webpage[0] = WEBPAGE_COMMAND_CHANGE_TEXT_EMPHASIS;
    webpage[WEBPAGE_COMMAND_CHANGE_TEXT_EMPHASIS_OFFSET_TEXT_EMPHASIS] = css_font_emphasis;
    webpage+=2;
    
    ib_font_emphasis = css_font_emphasis;
   }
   //save font spacing
   if(css_font_spacing!=WEBPAGE_EMPTY_ENTRY_32) {
    ib_font_spacing = css_font_spacing;
   }
   //save font alignment
   if(css_font_alignment!=WEBPAGE_EMPTY_ENTRY_32) {
    ib_font_alignment = css_font_alignment;
   }
   
   //add char/chars before tag <li>
   if(html_list_tag!=0) {
    if(html_list_tag==0xFFFFFFFF) { //unordered list
     *webpage = 127; //circle
     webpage++;
     ib_actual_line_width += ib_font_size;
     html_last_char_is_list_start = STATUS_TRUE;
     
     //move column according to alignment
     if(ib_font_alignment==WEBPAGE_MIDDLE_ALIGNMENT) {
      webpage32 = (dword_t *) (webpage_last_line_command_pointer+WEBPAGE_COMMAND_MOVE_TO_LINE_BYTE_OFFSET_COLUMN);
      *webpage32 = (ib_draw_column+ib_line_width/2-ib_actual_line_width/2);
     }
     else if(ib_font_alignment==WEBPAGE_RIGHT_ALIGNMENT) {
      webpage32 = (dword_t *) (webpage_last_line_command_pointer+WEBPAGE_COMMAND_MOVE_TO_LINE_BYTE_OFFSET_COLUMN);
      *webpage32 = (ib_draw_column+ib_line_width-ib_actual_line_width);
     }
     
     //set line spacing
     if(ib_font_spacing>ib_line_biggest_spacing) {
      ib_line_biggest_spacing = ib_font_spacing;
     }
     
     //move last line command according to font size
     webpage16 = (word_t *) webpage_last_line_command_pointer;
     if(webpage16[WEBPAGE_COMMAND_MOVE_TO_LINE_OFFSET_LINE_SIZE]<ib_font_size) {
      webpage16[WEBPAGE_COMMAND_MOVE_TO_LINE_OFFSET_LINE_SIZE] = ib_font_size;
     }
    }
    else { //ordered list
     convert_number_to_word_string(html_list_tag, (dword_t)webpage);
     while(*webpage>='0' && *webpage<='9') {
      webpage++;
      ib_actual_line_width += ib_font_size;
     }
     *webpage = '.';
     webpage++;
     ib_actual_line_width += ib_font_size;
     html_last_char_is_list_start = STATUS_TRUE;
     
     //move column according to alignment
     if(ib_font_alignment==WEBPAGE_MIDDLE_ALIGNMENT) {
      webpage32 = (dword_t *) (webpage_last_line_command_pointer+WEBPAGE_COMMAND_MOVE_TO_LINE_BYTE_OFFSET_COLUMN);
      *webpage32 = (ib_draw_column+ib_line_width/2-ib_actual_line_width/2);
     }
     else if(ib_font_alignment==WEBPAGE_RIGHT_ALIGNMENT) {
      webpage32 = (dword_t *) (webpage_last_line_command_pointer+WEBPAGE_COMMAND_MOVE_TO_LINE_BYTE_OFFSET_COLUMN);
      *webpage32 = (ib_draw_column+ib_line_width-ib_actual_line_width);
     }
     
     //set line spacing
     if(ib_font_spacing>ib_line_biggest_spacing) {
      ib_line_biggest_spacing = ib_font_spacing;
     }
     
     //move last line command according to font size
     webpage16 = (word_t *) webpage_last_line_command_pointer;
     if(webpage16[WEBPAGE_COMMAND_MOVE_TO_LINE_OFFSET_LINE_SIZE]<ib_font_size) {
      webpage16[WEBPAGE_COMMAND_MOVE_TO_LINE_OFFSET_LINE_SIZE] = ib_font_size;
     }
    }
   }
   
   //SKIP TAG
   skip_tag:
   while(*html!='>' && *html!=0) {
    html++;
   }
   html++;
   continue;
  }
  
  //CALCULATE HOW MANY CHARS IN WORD
  dword_t chars_in_word = 0, size_of_word = 0;
  if(html_last_char_is_list_start==STATUS_TRUE && *html!=' ') { //add space before list entry inside
   *webpage = ' ';
   webpage++;
   ib_actual_line_width += ib_font_size;
  }
  for(int i=0; i<1000; i++) {
   if(html[i]>' ' && html[i]!='<') {
    chars_in_word++;
    size_of_word += ib_font_size;
   }
   else {
    break;
   }
  }
  //TODO: if word is bigger than line
  
  //IF NEEDED, GO TO NEXT LINE
  if((ib_actual_line_width+size_of_word)>ib_line_width) {
   //skip line
   ib_draw_line += ib_line_biggest_spacing;
   
   //change height of tags
   dword_t add_height = ib_line_biggest_spacing;
   for(int tag_entry=html_tag_css_list_pointer; tag_entry>0; tag_entry--) {
    html_tag_css_list = (dword_t *) (html_tag_css_list_mem+(tag_entry-1)*HTML_TAG_CSS_ENTRY_SIZE);
    
    if(html_tag_css_list[HTML_TAG_CSS_ENTRY_OFFSET_HEIGTH_CALCULATION_TYPE]==HTML_TAG_CSS_ENTRY_HEIGTH_CALCULATION_ADD) {
     html_tag_css_list[HTML_TAG_CSS_ENTRY_OFFSET_HEIGTH] += add_height;
     
     if(html_tag_css_list[HTML_TAG_CSS_ENTRY_OFFSET_DRAW_BOX_MEM]!=0) {
      webpage16 = (word_t *) (html_tag_css_list[HTML_TAG_CSS_ENTRY_OFFSET_DRAW_BOX_MEM]);
      webpage16[WEBPAGE_COMMAND_DRAW_BOX_OFFSET_HEIGTH] = html_tag_css_list[HTML_TAG_CSS_ENTRY_OFFSET_HEIGTH];
     }
    }
    else if(html_tag_css_list[HTML_TAG_CSS_ENTRY_OFFSET_HEIGTH_CALCULATION_TYPE]==HTML_TAG_CSS_ENTRY_HEIGTH_CALCULATION_COMPARE) {
     if(html_tag_css_list[HTML_TAG_CSS_ENTRY_OFFSET_HEIGTH]<html_tag_css_list[HTML_TAG_CSS_ENTRY_NUM_OF_ENTRIES+HTML_TAG_CSS_ENTRY_OFFSET_HEIGTH]) {
      add_height = (html_tag_css_list[HTML_TAG_CSS_ENTRY_NUM_OF_ENTRIES+HTML_TAG_CSS_ENTRY_OFFSET_HEIGTH]-html_tag_css_list[HTML_TAG_CSS_ENTRY_OFFSET_HEIGTH]);
      html_tag_css_list[HTML_TAG_CSS_ENTRY_OFFSET_HEIGTH] = html_tag_css_list[HTML_TAG_CSS_ENTRY_NUM_OF_ENTRIES+HTML_TAG_CSS_ENTRY_OFFSET_HEIGTH];
     }
     else {
      break;
     }
    }
   }

   //add movement to next line   
   webpage_last_line_command_pointer = ((dword_t)webpage);
   webpage[0] = WEBPAGE_COMMAND_MOVE_TO_LINE;
   webpage32 = (dword_t *) ((dword_t)webpage+WEBPAGE_COMMAND_MOVE_TO_LINE_BYTE_OFFSET_COLUMN);
   *webpage32 = ib_draw_column;
   webpage32 = (dword_t *) ((dword_t)webpage+WEBPAGE_COMMAND_MOVE_TO_LINE_BYTE_OFFSET_LINE);
   *webpage32 = ib_draw_line;
   webpage[WEBPAGE_COMMAND_MOVE_TO_LINE_OFFSET_LINE_SIZE] = 0;
   webpage += 6;
   
   //set variable
   ib_actual_line_width = 0;
   ib_line_biggest_spacing = 0;
  }
  
  //ADD CHAR TO WEBPAGE
  *webpage=*html;
  webpage++;
  ib_actual_line_width += ib_font_size;
  html_last_char_is_list_start = STATUS_FALSE;
  
  //move column according to alignment
  if(ib_font_alignment==WEBPAGE_MIDDLE_ALIGNMENT) {
   webpage32 = (dword_t *) (webpage_last_line_command_pointer+WEBPAGE_COMMAND_MOVE_TO_LINE_BYTE_OFFSET_COLUMN);
   *webpage32 = (ib_draw_column+ib_line_width/2-ib_actual_line_width/2);
  }
  else if(ib_font_alignment==WEBPAGE_RIGHT_ALIGNMENT) {
   webpage32 = (dword_t *) (webpage_last_line_command_pointer+WEBPAGE_COMMAND_MOVE_TO_LINE_BYTE_OFFSET_COLUMN);
   *webpage32 = (ib_draw_column+ib_line_width-ib_actual_line_width);
  }
  
  //set line spacing
  if(ib_font_spacing>ib_line_biggest_spacing) {
   ib_line_biggest_spacing = ib_font_spacing;
  }
  
  //move last line command according to font size
  webpage16 = (word_t *) webpage_last_line_command_pointer;
  if(webpage16[WEBPAGE_COMMAND_MOVE_TO_LINE_OFFSET_LINE_SIZE]<ib_font_size) {
   webpage16[WEBPAGE_COMMAND_MOVE_TO_LINE_OFFSET_LINE_SIZE] = ib_font_size;
  }
  
  //next char
  html++;
 }
 
 page_is_finished:

 //free all downloaded files
 for(int i=0; i<html_list_of_downloaded_files_pointer; i++) {
  free(html_list_of_downloaded_files[i]);
 }
 
 free(html_in_unicode_mem);
 return webpage_mem;
}

byte_t is_tag_equal(word_t *string1, byte_t *string2) { 
 string1++; //skip <

 while(1) {
  if(*string2==0) {
   if(*string1=='>' || *string1==' ') {
    return STATUS_TRUE;
   }
   else {
    return STATUS_FALSE;
   }
  }
  if(*string1!=*string2) {
   return STATUS_FALSE;
  }
  
  string1++;
  string2++;
 }
}

dword_t find_tag_attribute(dword_t html_mem, byte_t *string) {
 word_t *html = (word_t *) (html_mem);
 
 while(*html!='>' && *html!=0) { 
  //attribute
  if(*html=='=') {
   html++;
   
   //skip intro of attribute
   if(*html!='"') { //attribute=value
    while(*html!=' ') {
     if(*html==0) {
      return STATUS_FALSE;
     }
     
     html++;
    }
    continue;
   }
   else { //attribute="value/values"
    html++;
    
    while(*html!='"') {
     if(*html==0) {
      return STATUS_FALSE;
     }
     
     html++;
    }
    html++;
    continue;
   }
  }
 
  //test if we did not found our attribute
  for(int i=0; i<100; i++) {
   if(string[i]==0) {
    if(html[i]=='=') {
     if(html[i+1]=='"') {
      return ((dword_t)html+i*2+2+2); //return memory to attribute after attribute="
     }
     else {
      return ((dword_t)html+i*2+2); //return memory to attribute after attribute=
     }
    }
    else {
     break;
    }
   }
   else if(html[i]!=string[i]) {
    break;
   }
  }
  
  //skip char
  html++;
 }
 
 return STATUS_FALSE;
}

void convert_relative_url_to_full_url(dword_t url_mem) {
 word_t *url = (word_t *) (url_mem);
 byte_t *url_base = (byte_t *) (url_base_mem);
 byte_t *url_input = (byte_t *) (url_input_mem);
 byte_t *url_output = (byte_t *) (url_output_mem);
 dword_t url_input_pointer = 0, url_output_pointer = 0;

 //copy URL from source to URL input array
 clear_memory(url_input_mem, 2048);
 clear_memory(url_output_mem, 2048);
 url_input_pointer = 0;
 for(int i=0; i<2048; i++) {
  if(url[i]=='"' || url[i]=='#' || url[i]==' ' || url[i]=='>' || url[i]==0) {
   break;
  }
  if(url[i]=='&' && url[i+1]=='a' && url[i+2]=='m' && url[i+3]=='p' && url[i+4]==';') {
   url_input[url_input_pointer] = '&';
   url_input_pointer++;
   i+=4;
  }
  else {
   url_input[url_input_pointer] = url[i];
   url_input_pointer++;
  }
 }

 //if full URL, copy it
 if(are_equal_b_string_b_string(url_input, "http://")==STATUS_TRUE || are_equal_b_string_b_string(url_input, "https://")==STATUS_TRUE) {
  for(int i=0; i<2048; i++) {
   url_output[i]=url_input[i];
  }
  return;
 }
 if(are_equal_b_string_b_string(url_input, "//")==STATUS_TRUE) {
  //copy protocol
  url_output_pointer = 0;
  if(are_equal_b_string_b_string(url_base, "http://")==STATUS_TRUE) {
   for(int i=0; i<7; i++) {
    url_output[url_output_pointer]=url_base[i];
    url_output_pointer++;
   }
  }
  else if(are_equal_b_string_b_string(url_base, "https://")==STATUS_TRUE) {
   for(int i=0; i<8; i++) {
    url_output[url_output_pointer]=url_base[i];
    url_output_pointer++;
   }
  }

  //copy rest of url
  for(int i=2; i<2048; i++) {
   if(url_input[i]==0) {
    break;
   }
   url_output[url_output_pointer]=url_input[i];
   url_output_pointer++;
  }

  return;
 }

 //remove last / from base URL
 for(int i=2047; i>0; i--) {
  if(url_base[i]!=0) {
   if(url_base[i]=='/') {
    url_base[i]=0;
   }
   break;
  }
 }

 //find base values
 dword_t num_end_of_domain_name = 0;
 dword_t num_start_of_path = 0;
 dword_t num_start_of_actual_folder = 0;
 if(are_equal_b_string_b_string(url_base, "http://")==STATUS_TRUE) {
  num_end_of_domain_name=7;
 }
 else if(are_equal_b_string_b_string(url_base, "https://")==STATUS_TRUE) {
  num_end_of_domain_name=8;
 }
 for(int i=num_end_of_domain_name; i<2048; i++) {
  if(url_base[i]==0) {
   num_end_of_domain_name = i;
   break;
  }
  if(url_base[i]=='/') {
   num_end_of_domain_name = i;
   num_start_of_path = i;

   //find start char of actual folder
   for(int i=2048; i>0; i--) {
    if(url_base[i]=='/') {
     num_start_of_actual_folder = i;
     break;
    }
   }

   break;
  }
 }

 //convert URL to same folder
 if(are_equal_b_string_b_string(url_input, "./")==STATUS_TRUE) {
  //only domain name in base
  if(num_start_of_path==0) {
   //copy base url
   url_output_pointer = 0;
   for(int i=0; i<2048; i++) {
    if(url_base[i]==0) {
     break;
    }
    url_output[url_output_pointer]=url_base[i];
    url_output_pointer++;
   }

   //copy rest of url
   for(int i=1; i<2048; i++) {
    if(url_input[i]==0) {
     break;
    }
    url_output[url_output_pointer]=url_input[i];
    url_output_pointer++;
   }

   return;
  }

  //copy base url
  url_output_pointer = 0;
  for(int i=0; i<num_start_of_actual_folder; i++) {
   url_output[url_output_pointer]=url_base[i];
   url_output_pointer++;
  }  
  
  //copy rest of url
  for(int i=1; i<2048; i++) {
   if(url_input[i]==0) {
    break;
   }
   url_output[url_output_pointer]=url_input[i];
   url_output_pointer++;
  }
  return;
 }
 if(is_char(*url_input)==STATUS_TRUE) {
  //only domain name in base
  if(num_start_of_path==0) {
   //copy base url
   url_output_pointer = 0;
   for(int i=0; i<2048; i++) {
    if(url_base[i]==0) {
     break;
    }
    url_output[url_output_pointer]=url_base[i];
    url_output_pointer++;
   }

   //copy rest of url
   url_output[url_output_pointer]='/';
   url_output_pointer++;
   for(int i=0; i<2048; i++) {
    if(url_input[i]==0) {
     break;
    }
    url_output[url_output_pointer]=url_input[i];
    url_output_pointer++;
   }

   return;
  }

  //copy base url
  url_output_pointer = 0;
  for(int i=0; i<num_start_of_actual_folder; i++) {
   url_output[url_output_pointer]=url_base[i];
   url_output_pointer++;
  }  
  
  //copy rest of url
  url_output[url_output_pointer]='/';
  url_output_pointer++;
  for(int i=0; i<2048; i++) {
   if(url_input[i]==0) {
    break;
   }
   url_output[url_output_pointer]=url_input[i];
   url_output_pointer++;
  }
  return;
 }

 //convert URL from base
 if(*url_input=='/') {
  //copy base url
  url_output_pointer = 0;
  for(int i=0; i<num_end_of_domain_name; i++) {
   url_output[url_output_pointer]=url_base[i];
   url_output_pointer++;
  }

  //copy rest of url
  for(int i=0; i<2048; i++) {
   if(url_input[i]==0) {
    break;
   }
   url_output[url_output_pointer]=url_input[i];
   url_output_pointer++;
  }

  return;
 }

 //convert URL to previous folder
 if(are_equal_b_string_b_string(url_input, "../")==STATUS_TRUE) {
  if(num_start_of_actual_folder==0) { //only domain
   //copy base url
   url_output_pointer = 0;
   for(int i=0; i<num_end_of_domain_name; i++) {
    if(url_base[i]==0) {
     break;
    }
    url_output[url_output_pointer]=url_base[i];
    url_output_pointer++;
   }

   //copy rest of url
   for(int i=2; i<2048; i++) {
    if(url_input[i]==0) {
     break;
    }
    url_output[url_output_pointer]=url_input[i];
    url_output_pointer++;
   }

   return;
  }
  if(num_start_of_actual_folder==num_end_of_domain_name) { //only one folder
   //copy base url
   url_output_pointer = 0;
   for(int i=0; i<num_end_of_domain_name; i++) {
    if(url_base[i]==0) {
     break;
    }
    url_output[url_output_pointer]=url_base[i];
    url_output_pointer++;
   }

   //copy rest of url
   for(int i=2; i<2048; i++) {
    if(url_input[i]==0) {
     break;
    }
    url_output[url_output_pointer]=url_input[i];
    url_output_pointer++;
   }

   return;
  }

  //find previous folder
  for(int i=(num_start_of_actual_folder-1); i>0; i--) {
   if(url_base[i]=='/') {
    //copy base url
    url_output_pointer = 0;
    for(int j=0; j<i; j++) {
     url_output[url_output_pointer]=url_base[j];
     url_output_pointer++;
    }

    //copy rest of url
    for(int j=2; j<2048; j++) {
     if(url_input[j]==0) {
      break;
     }
     url_output[url_output_pointer]=url_input[j];
     url_output_pointer++;
    }

    return;
   }
  }
 }
}