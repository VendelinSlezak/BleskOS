//BleskOS

/*
* MIT License
* Copyright (c) 2023-2024 Vendelín Slezák
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

dword_t convert_odt_to_dmf(dword_t odt_memory, dword_t odt_size) {
 if(is_this_zip(odt_memory, odt_size)==STATUS_FALSE) {
  log("\nODT: not zip file");
  return STATUS_ERROR;
 }
 dword_t content_xml_file_number = search_for_file_in_zip(odt_memory, odt_size, "content.xml");
 if(content_xml_file_number==ZIP_FILE_NOT_FOUNDED) {
  log("\nODT: content.xml not founded");
  return STATUS_ERROR;
 }
 dword_t content_raw_xml_file_memory = zip_extract_file(odt_memory, odt_size, content_xml_file_number);
 if(content_raw_xml_file_memory==STATUS_ERROR) {
  log("\nODT: error with extracting content.xml");
  return STATUS_ERROR;
 }
 dword_t content_xml_file_memory = prepare_xml_file(content_raw_xml_file_memory, zip_extracted_file_size);
 free(content_raw_xml_file_memory);

 //count how many memory we will need for DMF
 word_t *content_xml = (word_t *) (content_xml_file_memory);
 dword_t dmf_size = DMF_SFCH_ENTRY_LENGTH_IN_BYTES;
 dmf_number_of_chars_in_document = 0;
 document_editor_list_of_styles_number_of_entries = 0;
 while(*content_xml!=0) {
  if(*content_xml=='<') { //tag
   if(xml_is_tag(content_xml, "text:p")==STATUS_TRUE || xml_is_tag(content_xml, "/text:p")==STATUS_TRUE) {
    dmf_size+=DMF_SFCH_ENTRY_LENGTH_IN_BYTES;
   }
   else if(xml_is_tag(content_xml, "text:span")==STATUS_TRUE || xml_is_tag(content_xml, "/text:span")==STATUS_TRUE) {
    dmf_size+=DMF_SFCH_ENTRY_LENGTH_IN_BYTES;
   }
   else if(xml_is_tag(content_xml, "text:line-break")==STATUS_TRUE) {
    dmf_size+=2; //0xA
   }
   else if(xml_is_tag(content_xml, "style:style")==STATUS_TRUE) {
    if(xml_find_tag_attribute("style:name")==STATUS_TRUE) {
     while(content_xml[0]!='>' && content_xml[1]!=0) {
      content_xml++;
     }
     document_editor_add_style_to_list((dword_t)xml_tag_attribute_content_memory, ((dword_t)content_xml)+2); //+2 = skip char >
    }
   }

   //skip tag
   while(content_xml[0]!='>' && content_xml[1]!=0) {
    content_xml++;
   }
   content_xml++;
  }
  else { //char
   dmf_size+=2;
   content_xml++;
   dmf_number_of_chars_in_document++;
  }
 }

 //allocate memory for DMF
 dmf_size+=2;
 dword_t dmf_memory = calloc(dmf_size);

 //add start entry
 content_xml = (word_t *) (content_xml_file_memory);
 word_t *dmf = (word_t *) (dmf_memory);
 dmf_add_section_format_change(((dword_t)dmf), (DMF_SFCH_DESCRIPTION_NEW_PAGE), 0, 0, BLACK, TRANSPARENT_COLOR);
 //A4
 dmf[DMF_SFCH_NEW_PAGE_WIDTH_OFFSET] = 794; //21 cm
 dmf[DMF_SFCH_NEW_PAGE_HEIGHT_OFFSET] = 1123; //29.7 cm
 dmf[DMF_SFCH_NEW_PAGE_TOP_BORDER_OFFSET] = 75; //2 cm
 dmf[DMF_SFCH_NEW_PAGE_BOTTOM_BORDER_OFFSET] = 75; //2 cm
 dmf[DMF_SFCH_NEW_PAGE_LEFT_BORDER_OFFSET] = 75; //2 cm
 dmf[DMF_SFCH_NEW_PAGE_RIGHT_BORDER_OFFSET] = 75; //2 cm
 dmf = (word_t *) (((dword_t)dmf)+DMF_SFCH_ENTRY_LENGTH_IN_BYTES);

 //convert ODT to DMF
 document_editor_style_stack_number_of_entries = 0;
 dmf_character_size = 10, dmf_character_emphasis = 0, dmf_character_color = BLACK, dmf_character_background_color = TRANSPARENT_COLOR;
 document_editor_add_style_to_stack();
 while(*content_xml!=0) {
  if(*content_xml=='<') { //tag
   if(content_xml[1]=='/') { //ending tag
    content_xml++;
    //TAG </text:p>
    if(xml_is_tag(content_xml, "text:p")==STATUS_TRUE) {
     //take style from stack
     document_editor_take_style_from_stack();

     //write output to DMF
     dmf_add_section_format_change(((dword_t)dmf), 0, dmf_character_size, dmf_character_emphasis, dmf_character_color, dmf_character_background_color);
     dmf = (word_t *) (((dword_t)dmf)+DMF_SFCH_ENTRY_LENGTH_IN_BYTES);
    }
    //TAG </text:p>
    else if(xml_is_tag(content_xml, "text:span")==STATUS_TRUE) {
     //take style from stack
     document_editor_take_style_from_stack();

     //write output to DMF
     dmf_add_section_format_change(((dword_t)dmf), 0, dmf_character_size, dmf_character_emphasis, dmf_character_color, dmf_character_background_color);
     dmf = (word_t *) (((dword_t)dmf)+DMF_SFCH_ENTRY_LENGTH_IN_BYTES);
    }
   }
   else { //normal tag
    //TAG <text:p>
    if(xml_is_tag(content_xml, "text:p")==STATUS_TRUE) {
     //read attributes
     if(xml_find_tag_attribute("text:style-name")==STATUS_TRUE) {
      //try to find style in style list
      for(dword_t i=0; i<document_editor_list_of_styles_number_of_entries; i++) {
       if(xml_compare_attribute_and_attribute((word_t *)(document_editor_list_of_styles_pointer[i].memory_of_style_name), xml_tag_attribute_content_memory)==STATUS_TRUE) {
        odt_read_style(document_editor_list_of_styles_pointer[i].memory_of_style_content); //read style
       }
      }
     }

     //write output to DMF
     dmf_add_section_format_change(((dword_t)dmf), DMF_SFCH_DESCRIPTION_NEW_PARAGRAPH, dmf_character_size, dmf_character_emphasis, dmf_character_color, dmf_character_background_color);
     dmf_add_section_paragraph_change(((dword_t)dmf), 0, 0, 0, 0, 0, 0);
     dmf = (word_t *) (((dword_t)dmf)+DMF_SFCH_ENTRY_LENGTH_IN_BYTES);

     //add style to stack
     if(xml_does_tag_have_pair(content_xml)==STATUS_TRUE) {
      document_editor_add_style_to_stack();
     }
    }
    //TAG <text:span>
    else if(xml_is_tag(content_xml, "text:span")==STATUS_TRUE) {
     //read attributes
     if(xml_find_tag_attribute("text:style-name")==STATUS_TRUE) {
      //try to find style in style list
      for(dword_t i=0; i<document_editor_list_of_styles_number_of_entries; i++) {
       if(xml_compare_attribute_and_attribute((word_t *)(document_editor_list_of_styles_pointer[i].memory_of_style_name), xml_tag_attribute_content_memory)==STATUS_TRUE) {
        odt_read_style(document_editor_list_of_styles_pointer[i].memory_of_style_content); //read style
       }
      }
     }

     //write output to DMF
     dmf_add_section_format_change(((dword_t)dmf), 0, dmf_character_size, dmf_character_emphasis, dmf_character_color, dmf_character_background_color);
     dmf = (word_t *) (((dword_t)dmf)+DMF_SFCH_ENTRY_LENGTH_IN_BYTES);

     //add style to stack
     if(xml_does_tag_have_pair(content_xml)==STATUS_TRUE) {
      document_editor_add_style_to_stack();
     }
    }
    //TAG <text:line-break>
    else if(xml_is_tag(content_xml, "text:line-break")==STATUS_TRUE) {
     *dmf = 0xA;
     dmf++;
    }
   }

   //skip tag
   while(content_xml[0]!='>' && content_xml[1]!=0) {
    content_xml++;
   }
   content_xml++;
  }
  else { //char
   if(*content_xml=='&') { //escape sequence
    *dmf = xml_get_escape_sequence_character(content_xml);
    dmf++;

    //skip escape sequence
    while(content_xml[1]!=0) {
     if(*content_xml==';') {
      break;
     }
     content_xml++;
    }
   }
   else if(*content_xml>=' ') { //skip unprintable characters
    *dmf = *content_xml;
    dmf++;
   }
   content_xml++;
  }
 }

 return dmf_memory;
}

void odt_read_style(dword_t odt_style_memory) {
 word_t *content_xml = (word_t *) (odt_style_memory);
 while(*content_xml!=0) {
  if(*content_xml=='<') { //tag
   if(xml_is_tag(content_xml, "/style:style")==STATUS_TRUE) { //end of style section
    return;
   }
   else if(xml_is_tag(content_xml, "style:text-properties")==STATUS_TRUE) { //text properties
    //font size
    if(xml_find_tag_attribute("fo:font-size")==STATUS_TRUE) {
     dmf_character_size = xml_get_attribute_number_in_px();
    }

    //font emphasis
    if(xml_find_tag_attribute("fo:font-weight")==STATUS_TRUE) {
     if(xml_is_attribute("none")==STATUS_TRUE) {
      dmf_character_emphasis &= ~SF_EMPHASIS_BOLD;
     }
     else {
      dmf_character_emphasis |= SF_EMPHASIS_BOLD;
     }
    }
    if(xml_find_tag_attribute("style:text-underline-style")==STATUS_TRUE) {
     if(xml_is_attribute("none")==STATUS_TRUE) {
      dmf_character_emphasis &= ~SF_EMPHASIS_UNDERLINE;
     }
     else {
      dmf_character_emphasis |= SF_EMPHASIS_UNDERLINE;
     }
    }
    if(xml_find_tag_attribute("style:text-line-through-style")==STATUS_TRUE) {
     if(xml_is_attribute("none")==STATUS_TRUE) {
      dmf_character_emphasis &= ~SF_EMPHASIS_STRIKE;
     }
     else {
      dmf_character_emphasis |= SF_EMPHASIS_STRIKE;
     }
    }

    //font color
    if(xml_find_tag_attribute("fo:color")==STATUS_TRUE) {
     dmf_character_color = xml_get_attribute_hex_number();
    }

    //background color
    if(xml_find_tag_attribute("fo:background-color")==STATUS_TRUE) {
     if(xml_is_attribute("transparent")==STATUS_TRUE) {
      dmf_character_background_color = TRANSPARENT_COLOR;
     }
     else {
      dmf_character_background_color = xml_get_attribute_hex_number();
     }
    }
   }
  }
  content_xml++;
 }
}