//BleskOS

/*
* MIT License
* Copyright (c) 2023 Vendelín Slezák
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

void css_parse_file(void) {
 dword_t selector_pointer, selector_declaration_block_pointer;

 //skip @charset
 if(css_compare_string("@charset")==STATUS_TRUE) {
  css_skip_after_char(';');
 }
 
 //process body of css
 next_css_selector:
 while(*css!=0 && *css!='<' && *css!='>') {
  css_skip_spaces();
  
  if(*css=='#') { //id
   if(css_entries_id_list_pointer>=10000) { //too many entries
    css_skip_after_char('}');
    continue;
   }
   
   //find declaration block pointer
   dword_t css_memory = (dword_t)css;
   css_skip_after_char('{');
   if(*css==0 || *css=='"' || *css=='>') {
    return;
   }
   selector_declaration_block_pointer = ((dword_t)css);
   css = (word_t *) ((dword_t)css_memory);
   
   //test if we will add this entry to list
   css++; //skip #
   while(*css!='{' && *css!=0 && *css!='"' && *css!='>') {
    selector_pointer = ((dword_t)css);
    while((*css>='a' && *css<='z') || (*css>='A' && *css<='Z') || (*css>='0' && *css<='9') || *css=='-') {
     *css++;
    }
    css_skip_spaces();
    if(*css=='{' || *css==',') {
     //add entry to list
     css_entries_list = (dword_t *) css_entries_id_list_mem;
     css_entries_list[css_entries_id_list_pointer*2+0] = selector_pointer;
     css_entries_list[css_entries_id_list_pointer*2+1] = selector_declaration_block_pointer;
     css_entries_id_list_pointer++;
    
     if(*css=='{') {
      break;
     }
     else {
      css++; //skip ,
      goto next_css_selector;
     }
    }
    else {
     break;
    }
   }
   
   //go to next declaration block
   css_skip_after_char('}');
   continue;
  }
  else if(*css=='.') { //class
   if(css_entries_class_list_pointer>=10000) { //too many entries
    css_skip_after_char('}');
    continue;
   }
   
   //find declaration block pointer
   dword_t css_memory = (dword_t)css;
   css_skip_after_char('{');
   if(*css==0 || *css=='"' || *css=='>') {
    return;
   }
   selector_declaration_block_pointer = ((dword_t)css);
   css = (word_t *) ((dword_t)css_memory);
   
   //test if we will add this entry to list
   css++; //skip .
   while(*css!='{' && *css!=0 && *css!='"' && *css!='>') {
    selector_pointer = ((dword_t)css);
    while((*css>='a' && *css<='z') || (*css>='A' && *css<='Z') || (*css>='0' && *css<='9') || *css=='-') {
     *css++;
    }
    css_skip_spaces();
    if(*css=='{' || *css==',') {
     //add entry to list
     css_entries_list = (dword_t *) css_entries_class_list_mem;
     css_entries_list[css_entries_class_list_pointer*2+0] = selector_pointer;
     css_entries_list[css_entries_class_list_pointer*2+1] = selector_declaration_block_pointer;
     css_entries_class_list_pointer++;
    
     if(*css=='{') {
      break;
     }
     else {
      css++; //skip ,
      goto next_css_selector;
     }
    }
    else {
     break;
    }
   }
   
   //go to next declaration block
   css_skip_after_char('}');
   continue;
  }
  else if(*css>='a' && *css<='z') { //tag  
   if(css_entries_tag_list_pointer>=10000) { //too many entries
    css_skip_after_char('}');
    continue;
   }  
   
   //find declaration block pointer
   dword_t css_memory = (dword_t)css;
   css_skip_after_char('{');
   if(*css==0 || *css=='"' || *css=='>') {
    return;
   }
   selector_declaration_block_pointer = ((dword_t)css);
   css = (word_t *) ((dword_t)css_memory);
   
   //test if we will add this entry to list
   while(*css!='{' && *css!=0 && *css!='"' && *css!='>') {
    selector_pointer = ((dword_t)css);
    while((*css>='a' && *css<='z') || (*css>='A' && *css<='Z') || (*css>='0' && *css<='9') || *css=='-') {
     *css++;
    }
    css_skip_spaces();
    if(*css=='{' || *css==',') {
     //add entry to list
     css_entries_list = (dword_t *) css_entries_tag_list_mem;
     css_entries_list[css_entries_tag_list_pointer*2+0] = selector_pointer;
     css_entries_list[css_entries_tag_list_pointer*2+1] = selector_declaration_block_pointer;
     css_entries_tag_list_pointer++;
    
     if(*css=='{') {
      break;
     }
     else {
      css++; //skip ,
      goto next_css_selector;
     }
    }
    else {
     break;
    }
   }
   
   //go to next declaration block
   css_skip_after_char('}');
   continue;
  }
  else if(css[0]=='/' && css[1]=='*') { //comment
   //skip comment
   while(css[0]!='*' && css[1]!='/') {
    if(*css==0 || *css=='<' || *css=='>') {
     return;
    }
    css++;
   }
   css+=2;
  }
  else if(*css=='@') {
   css++;
   if(css_compare_string("import")==STATUS_TRUE) { //TODO:
    css_skip_after_char(';');
   }
   else if(css_compare_string("font-face")==STATUS_TRUE || css_compare_string("keyframes")==STATUS_TRUE) {
    css_skip_after_char('}');
   }
   else if(css_compare_string("media")==STATUS_TRUE) { //TODO:
    css_skip_after_char('{');
   }
  }
  else if(*css=='{') {
   css_skip_after_char('}');
   continue;
  }
  
  css++;
 }
}

void css_parse_declaration_block(void) {
 css_width = 0; css_left_auto_margin = 0; css_right_auto_margin = 0;
 
 while(*css!=0 && *css!='}' && *css!='"' && *css!='>') {  
  //parse declaration
  css_parse_declaration();
  
  //go to next declaration
  while(*css!=';' && *css!='}' && *css!='"' && *css!='>') {
   css++;
  }
  if(*css==';') {
   css++;
  }
 }
 
 //process auto margin
 if(css_width!=0) {
  if(css_left_auto_margin==STATUS_TRUE && css_right_auto_margin==STATUS_TRUE) {
   css_margin_left = (ib_line_width-css_width)/2;
   css_margin_right = (ib_line_width-css_margin_left-css_width);
  }
  else if(css_left_auto_margin==STATUS_TRUE) {
   css_margin_left = (ib_line_width-css_width-css_margin_right);
  }
  else if(css_right_auto_margin==STATUS_TRUE) {
   css_margin_right = (ib_line_width-css_width-css_margin_left);
  }
 }
}

void css_parse_declaration(void) {
 dword_t value = 0;
 
 css_skip_spaces();
 
 if(*css==';' || *css=='}' || *css=='>') {
  return;
 }
 
 if(*css=='b') {
  if(css_compare_property_string("background")==STATUS_TRUE) {
   css_skip_property();

   css_background_color = css_read_color_value();
  }
  else if(css_compare_property_string("background-color")==STATUS_TRUE) {
   css_skip_property();

   css_background_color = css_read_color_value();
  }
  else if(css_compare_property_string("border")==STATUS_TRUE) {
   css_skip_property();
   css_skip_spaces();
   
   css_top_border_size = css_read_number_value(1);
   css_bottom_border_size = css_top_border_size;
   css_left_border_size = css_top_border_size;
   css_right_border_size = css_top_border_size;
   css_skip_text();
   
   css_skip_spaces();
   //TODO: type of border
   css_top_border_type = WEBPAGE_BORDER_TYPE_SOLID;
   css_bottom_border_type = WEBPAGE_BORDER_TYPE_SOLID;
   css_left_border_type = WEBPAGE_BORDER_TYPE_SOLID;
   css_right_border_type = WEBPAGE_BORDER_TYPE_SOLID;
   css_skip_text();
   
   css_top_border_color = css_read_color_value();
   css_bottom_border_color = css_top_border_color;
   css_left_border_color = css_top_border_color;
   css_right_border_color = css_top_border_color;
  }
  else if(css_compare_property_string("border-top")==STATUS_TRUE) {
   css_skip_property();
   css_skip_spaces();
   
   css_top_border_size = css_read_number_value(1);
   css_skip_text();
   
   css_skip_spaces();
   //TODO: type of border
   css_top_border_type = WEBPAGE_BORDER_TYPE_SOLID;
   css_skip_text();
   
   css_top_border_color = css_read_color_value();
  }
  else if(css_compare_property_string("border-bottom")==STATUS_TRUE) {
   css_skip_property();
   css_skip_spaces();
   
   css_bottom_border_size = css_read_number_value(1);
   css_skip_text();
   
   css_skip_spaces();
   //TODO: type of border
   css_bottom_border_type = WEBPAGE_BORDER_TYPE_SOLID;
   css_skip_text();
   
   css_bottom_border_color = css_read_color_value();
  }
  else if(css_compare_property_string("border-left")==STATUS_TRUE) {
   css_skip_property();
   css_skip_spaces();
   
   css_left_border_size = css_read_number_value(1);
   css_skip_text();
   
   css_skip_spaces();
   //TODO: type of border
   css_left_border_type = WEBPAGE_BORDER_TYPE_SOLID;
   css_skip_text();
   
   css_left_border_color = css_read_color_value();
  }
  else if(css_compare_property_string("border-right")==STATUS_TRUE) {
   css_skip_property();
   css_skip_spaces();
   
   css_right_border_size = css_read_number_value(1);
   css_skip_text();
   
   css_skip_spaces();
   //TODO: type of border
   css_right_border_type = WEBPAGE_BORDER_TYPE_SOLID;
   css_skip_text();
   
   css_right_border_color = css_read_color_value();
  }
  if(css_compare_property_string("border-width")==STATUS_TRUE) {
   css_skip_property();
   
   value = css_read_number_value(1);
   css_skip_spaces();
   css_top_border_size = value;
   css_bottom_border_size = value;
   if(*css==';') {
    css_left_border_size = value;
    css_right_border_size = value;
    return;  
   }
   
   value = css_read_number_value(1);
   css_skip_spaces();
   css_left_border_size = value;
   css_right_border_size = value;
   if(*css==';') {
    return;  
   }

   value = css_read_number_value(1);
   css_skip_spaces();
   css_bottom_border_size = value;
   if(*css==';') {
    return;  
   }

   value = css_read_number_value(1);
   css_skip_spaces();
   css_left_border_size = value;
   return;
  }
  else if(css_compare_property_string("border-top-width")==STATUS_TRUE) {
   css_skip_property();
   css_top_border_size = css_read_number_value(1);
   css_skip_spaces();
  }
  else if(css_compare_property_string("border-bottom-width")==STATUS_TRUE) {
   css_skip_property();
   css_bottom_border_size = css_read_number_value(1);
   css_skip_spaces();
  }
  else if(css_compare_property_string("border-left-width")==STATUS_TRUE) {
   css_skip_property();
   css_left_border_size = css_read_number_value(1);
   css_skip_spaces();
  }
  else if(css_compare_property_string("border-right-width")==STATUS_TRUE) {
   css_skip_property();
   css_right_border_size = css_read_number_value(1);
   css_skip_spaces();
  }
  else if(css_compare_property_string("border-color")==STATUS_TRUE) {
   css_skip_property();
   
   css_top_border_color = css_read_color_value();
   css_bottom_border_color = css_top_border_color;
   css_left_border_color = css_top_border_color;
   css_right_border_color = css_top_border_color;
  }
 }
 else if(*css=='c') {
  if(css_compare_property_string("color")==STATUS_TRUE) {
   css_skip_property();
   
   css_font_color = css_read_color_value();
  }
 }
 else if(*css=='f') {
  if(css_compare_property_string("font-size")==STATUS_TRUE) {
   css_skip_property();
   css_skip_spaces();
   
   css_font_size = 10;
   if(*css>='0' && *css<='9') {   
    css_font_size = css_read_number_value(10);
   }
   else if(css_compare_string("larger")==STATUS_TRUE) {
    css_font_size = (ib_font_size*5/4);
   }
   else if(css_compare_string("smaller")==STATUS_TRUE) {
    css_font_size = (ib_font_size*3/4);
   }
   else if(css_compare_string("xx-small")==STATUS_TRUE) {
    css_font_size = 4;
   }
   else if(css_compare_string("x-small")==STATUS_TRUE) {
    css_font_size = 6;
   }
   else if(css_compare_string("small")==STATUS_TRUE) {
    css_font_size = 8;
   }
   else if(css_compare_string("middle")==STATUS_TRUE) {
    css_font_size = 10;
   }
   else if(css_compare_string("large")==STATUS_TRUE) {
    css_font_size = 15;
   }
   else if(css_compare_string("x-large")==STATUS_TRUE) {
    css_font_size = 20;
   }
   else if(css_compare_string("xx-large")==STATUS_TRUE) {
    css_font_size = 25;
   }
   
   css_font_spacing = (css_font_size*3/2);
  }
  else if(css_compare_property_string("font-weigth")==STATUS_TRUE) {
   css_skip_property();
   css_skip_spaces();
   
   if(css_compare_string("bold")==STATUS_TRUE) {
    css_font_emphasis = SF_EMPHASIS_BOLD;
   }
  }
 }
 else if(*css=='h') {
  if(css_compare_property_string("height")==STATUS_TRUE) {
   css_skip_property();
   
   css_heigth = css_read_number_value(1);
  }
 }
 else if(*css=='m') {
  if(css_compare_property_string("margin")==STATUS_TRUE) {
   css_skip_property();
   css_skip_spaces();
   
   if(css_compare_string("auto")==STATUS_TRUE) {
    css_margin_top = 0;
    css_margin_bottom = 0;
    css_left_auto_margin = STATUS_TRUE;
    css_right_auto_margin = STATUS_TRUE;
   }   
   value = css_read_number_value(1);
   css_skip_spaces();
   css_margin_top = value;
   css_margin_bottom = value;
   if(*css==';') {
    css_margin_left = value;
    css_margin_right = value;
    return;  
   }
   
   if(css_compare_string("auto")==STATUS_TRUE) {
    css_left_auto_margin = STATUS_TRUE;
    css_right_auto_margin = STATUS_TRUE;
   }  
   value = css_read_number_value(1);
   css_skip_spaces();
   css_margin_left = value;
   css_margin_right = value;
   if(*css==';') {
    return;  
   }

   value = css_read_number_value(1);
   css_skip_spaces();
   css_margin_bottom = value;
   if(*css==';') {
    return;  
   }

   if(css_compare_string("auto")==STATUS_TRUE) {
    css_left_auto_margin = STATUS_TRUE;
   }
   else { 
    value = css_read_number_value(1);
    css_skip_spaces();
    css_margin_left = value;
   }
   return;
  }
  else if(css_compare_property_string("margin-top")==STATUS_TRUE) {
   css_skip_property();
   css_margin_top = css_read_number_value(1);
   css_skip_spaces();
  }
  else if(css_compare_property_string("margin-bottom")==STATUS_TRUE) {
   css_skip_property();
   css_margin_bottom = css_read_number_value(1);
   css_skip_spaces();
  }
  else if(css_compare_property_string("margin-left")==STATUS_TRUE) {
   css_skip_property();
   css_skip_spaces();
   if(css_compare_string("auto")==STATUS_TRUE) {
    css_left_auto_margin = STATUS_TRUE;
    return;
   } 
   css_margin_left = css_read_number_value(1);
   css_skip_spaces();
  }
  else if(css_compare_property_string("margin-right")==STATUS_TRUE) {
   css_skip_property();
   css_skip_spaces();
   if(css_compare_string("auto")==STATUS_TRUE) {
    css_right_auto_margin = STATUS_TRUE;
    return;
   } 
   css_margin_right = css_read_number_value(1);
   css_skip_spaces();
  }
 }
 else if(*css=='p') {
  if(css_compare_property_string("padding")==STATUS_TRUE) {
   css_skip_property();
   
   value = css_read_number_value(1);
   css_skip_spaces();
   css_padding_top = value;
   css_padding_bottom = value;
   if(*css==';') {
    css_padding_left = value;
    css_padding_right = value;
    return;  
   }
   
   value = css_read_number_value(1);
   css_skip_spaces();
   css_padding_left = value;
   css_padding_right = value;
   if(*css==';') {
    return;  
   }

   value = css_read_number_value(1);
   css_skip_spaces();
   css_padding_bottom = value;
   if(*css==';') {
    return;  
   }
   
   value = css_read_number_value(1);
   css_skip_spaces();
   css_padding_left = value;
   return;
  }
  else if(css_compare_property_string("padding-top")==STATUS_TRUE) {
   css_skip_property();
   css_padding_top = css_read_number_value(1);
   css_skip_spaces();
  }
  else if(css_compare_property_string("padding-bottom")==STATUS_TRUE) {
   css_skip_property();
   css_padding_bottom = css_read_number_value(1);
   css_skip_spaces();
  }
  else if(css_compare_property_string("padding-left")==STATUS_TRUE) {
   css_skip_property();
   css_padding_left = css_read_number_value(1);
   css_skip_spaces();
  }
  else if(css_compare_property_string("padding-right")==STATUS_TRUE) {
   css_skip_property();
   css_padding_right = css_read_number_value(1);
   css_skip_spaces();
  }
 }
 else if(*css=='t') {
  if(css_compare_property_string("text-align")==STATUS_TRUE) {
   css_skip_property();
   css_skip_spaces();
   if(css_compare_string("left")==STATUS_TRUE) {
    css_font_alignment = WEBPAGE_LEFT_ALIGNMENT;
   }
   else if(css_compare_string("center")==STATUS_TRUE) {
    css_font_alignment = WEBPAGE_MIDDLE_ALIGNMENT;
   }
   else if(css_compare_string("right")==STATUS_TRUE) {
    css_font_alignment = WEBPAGE_RIGHT_ALIGNMENT;
   }
  }
 }
 else if(*css=='w') {
  if(css_compare_property_string("width")==STATUS_TRUE) {
   css_skip_property();
   
   css_width = css_read_number_value(ib_line_width);
  }
 }
}

dword_t css_read_number_value(dword_t percent_base) {
 float number;

 css_skip_spaces();
 if(*css<'0' || *css>'9') {
  return 0;
 }
 
 //convert number
 number = convert_word_string_to_float_number((dword_t)css);
 
 //skip text of number
 while(*css>='0' && *css<='9') {
  css++;
 }
 if(*css=='.') {
  css++;
  while(*css>='0' && *css<='9') {
   css++;
  }
 }
 
 //convert to px
 if(css[0]=='%') {
  number = (percent_base*number/100);
 }
 else if(css[0]=='e' && css[1]=='m') {
  number *= 16;
 }
 else if(css[0]=='e' && css[1]=='x') {
  number *= 10;
 }
 else if(css[0]=='p' && css[1]=='t') {
  number *= 1.3;
 }
 else if(css[0]=='c' && css[1]=='m') {
  number *= 37.8;
 }
 else if(css[0]=='m' && css[1]=='m') {
  number *= 3.7;
 }
 else if(css[0]=='i' && css[1]=='n') {
  number *= 96;
 }
 else if(css[0]=='p' && css[1]=='c') {
  number *= 0.1;
 }
 css_skip_text();
 
 return number;
}

dword_t css_read_color_value(void) { 
 dword_t color;

 css_skip_spaces();
 if(*css>='A' && *css<='Z') { //TODO: convert to small char
  css += 0x20;
 }
 
 //convert color in hex
 if(*css=='#') {
  css++;
  
  color = convert_hex_word_string_to_number((dword_t)css);
  
  if(is_hex_number(css[0])==STATUS_TRUE && is_hex_number(css[1])==STATUS_TRUE && is_hex_number(css[2])==STATUS_TRUE && is_hex_number(css[3])==STATUS_FALSE) {
   color = (((color>>8)<<20) | ((color>>8)<<16) | (((color>>4) & 0xF)<<12) | (((color>>4) & 0xF)<<8) | ((color & 0xF)<<4) | (color & 0xF)); //expand color
  }
  
  return color;
 }
 else if(css_compare_string("aqua")==STATUS_TRUE) {
  return 0x00FFFF;
 }
 else if(css_compare_string("black")==STATUS_TRUE) {
  return 0x000000;
 }
 else if(css_compare_string("blue")==STATUS_TRUE) {
  return 0x0000FF;
 }
 else if(css_compare_string("brown")==STATUS_TRUE) {
  return 0xA52A2A;
 }
 else if(css_compare_string("cyan")==STATUS_TRUE) {
  return 0x00FFFF;
 }
 else if(css_compare_string("fuschia")==STATUS_TRUE) {
  return 0xFF00FF;
 }
 else if(css_compare_string("gray")==STATUS_TRUE) {
  return 0x808080;
 }
 else if(css_compare_string("grey")==STATUS_TRUE) {
  return 0x808080;
 }
 else if(css_compare_string("green")==STATUS_TRUE) {
  return 0x008000;
 }
 else if(css_compare_string("lime")==STATUS_TRUE) {
  return 0x00FF00;
 }
 else if(css_compare_string("maroon")==STATUS_TRUE) {
  return 0x800000;
 }
 else if(css_compare_string("navy")==STATUS_TRUE) {
  return 0x000080;
 }
 else if(css_compare_string("olive")==STATUS_TRUE) {
  return 0x808000;
 }
 else if(css_compare_string("purple")==STATUS_TRUE) {
  return 0x800080;
 }
 else if(css_compare_string("red")==STATUS_TRUE) {
  return 0xFF0000;
 }
 else if(css_compare_string("silver")==STATUS_TRUE) {
  return 0xC0C0C0;
 }
 else if(css_compare_string("teal")==STATUS_TRUE) {
  return 0x008080;
 }
 else if(css_compare_string("transparent")==STATUS_TRUE) {
  return WEBPAGE_EMPTY_ENTRY_32;
 }
 else if(css_compare_string("white")==STATUS_TRUE) {
  return 0xFFFFFF;
 }
 else if(css_compare_string("yellow")==STATUS_TRUE) {
  return 0xFFFF00;
 }
 else {
  return WEBPAGE_EMPTY_ENTRY_32;
 }
}

void css_skip_spaces(void) {
 while(*css<=' ') {
  if(*css==0) {
   return;
  }
  
  css++;
 }
}

void css_skip_property(void) {
 while(*css!=' ' && *css!=':' && *css!=';' && *css!='}' && *css!='"' && *css!='>') {
  if(*css==0) {
   return;
  }
  
  css++;
 }
 
 if(*css==':') {
  css++;
 }
}

void css_skip_text(void) {
 while(*css!=' ' && *css!=';' && *css!='{' && *css!='}' && *css!='"' && *css!='<' && *css!='>') {
  if(*css==0) {
   return;
  }
  
  css++;
 }
}

void css_skip_after_char(word_t value) {
 while(*css!=value) {
  if(*css==0 || *css=='<' || *css=='>') {
   return;
  }
  css++;
 }
 css++;
}

byte_t css_compare_property_string(byte_t *string) {
 dword_t offset=0;

 while(*string!=0) {
  if(css[offset]!=*string) {
   return STATUS_FALSE;
  }
  
  offset++;
  string++;
 }
 
 if(css[offset]==':') {
  return STATUS_TRUE;
 }
 else {
  return STATUS_FALSE;
 }
}

byte_t css_compare_string(byte_t *string) {
 dword_t offset=0;

 while(*string!=0) {
  if(css[offset]!=*string) {
   return STATUS_FALSE;
  }
  
  offset++;
  string++;
 }

 return STATUS_TRUE;
}
