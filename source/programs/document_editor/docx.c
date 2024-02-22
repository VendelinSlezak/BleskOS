//BleskOS

/*
* MIT License
* Copyright (c) 2023-2024 Vendelín Slezák
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

dword_t convert_docx_to_dmf(dword_t docx_memory, dword_t docx_size) {
 if(is_this_zip(docx_memory, docx_size)==STATUS_FALSE) {
  log("\nDOCX: not zip file");
  return STATUS_ERROR;
 }
 dword_t document_xml_file_number = search_for_file_in_zip(docx_memory, docx_size, "word/document.xml");
 log("\n"); log_var(document_xml_file_number);
 if(document_xml_file_number==ZIP_FILE_NOT_FOUNDED) {
  log("\nDOCX: word/document.xml not founded");
  return STATUS_ERROR;
 }
 dword_t document_raw_xml_file_memory = zip_extract_file(docx_memory, docx_size, document_xml_file_number);
 if(document_raw_xml_file_memory==STATUS_ERROR) {
  log("\nDOCX: error with extracting word/document.xml");
  return STATUS_ERROR;
 }
 dword_t document_xml_file_memory = prepare_xml_file(document_raw_xml_file_memory, zip_extracted_file_size);
 free(document_raw_xml_file_memory);

 //count how many memory we will need for DMF
 word_t *document_xml = (word_t *) (document_xml_file_memory);
 word_t *docx_body_tag = (word_t *) (0);
 dword_t dmf_size = DMF_SFCH_ENTRY_LENGTH_IN_BYTES;
 dmf_number_of_chars_in_document = 0;
 while(*document_xml!=0) {
  if(*document_xml=='<') { //tag
   if(xml_is_tag(document_xml, "w:p")==STATUS_TRUE || xml_is_tag(document_xml, "w:r")==STATUS_TRUE || xml_is_tag(document_xml, "w:t")==STATUS_TRUE || xml_is_tag(document_xml, "/w:r")==STATUS_TRUE || xml_is_tag(document_xml, "/w:t")==STATUS_TRUE) {
    dmf_size += DMF_SFCH_ENTRY_LENGTH_IN_BYTES;
   }
   else if(xml_is_tag(document_xml, "w:body")==STATUS_TRUE) {
    docx_body_tag = document_xml;
   }

   //skip tag
   while(document_xml[0]!='>' && document_xml[1]!=0) {
    document_xml++;
   }
   document_xml++;
  }
  else { //char
   dmf_size+=2;
   document_xml++;
   dmf_number_of_chars_in_document++;
  }
 }
 if(((dword_t)docx_body_tag)==0) {
  log("\nDOCX: no <w:body>");
  free(document_xml_file_memory);
  return STATUS_ERROR;
 }

 //allocate memory for DMF
 dmf_size+=2;
 dword_t dmf_memory = calloc(dmf_size);

 //default page layout - A4 with 2 cm margins
 dmf_page_width = 794;
 dmf_page_height = 1123;
 dmf_page_top_border = 75;
 dmf_page_bottom_border = 75;
 dmf_page_left_border = 75;
 dmf_page_right_border = 75;

 //add start entry
 word_t *dmf = (word_t *) (dmf_memory);
 dmf_add_section_format_change(((dword_t)dmf), 0, 0, BLACK, TRANSPARENT_COLOR);
 dmf_add_section_new_page(((dword_t)dmf), dmf_page_width, dmf_page_height, dmf_page_top_border, dmf_page_bottom_border, dmf_page_left_border, dmf_page_right_border);
 dmf = (word_t *) (((dword_t)dmf)+DMF_SFCH_ENTRY_LENGTH_IN_BYTES);

 //convert ODT to DMF
 document_xml = docx_body_tag;
 document_editor_style_stack_number_of_entries = 0;
 document_editor_stack_of_lists_number_of_entries = 0;

 //"Normal" style
 dmf_character_size = 9;
 dmf_character_emphasis = 0;
 dmf_character_color = BLACK;
 dmf_character_background_color = TRANSPARENT_COLOR;
 dmf_paragraph_alignment = DMF_SFCH_PARAGRAPH_DESCRIPTION_ALIGNMENT_LEFT;
 document_editor_add_style_to_stack();

 //list variables
 byte_t is_this_paragraph_list_entry = STATUS_FALSE;
 dmf_paragaph_list_entry = 0;

 while(*document_xml!=0) {
  if(*document_xml=='<') { //tag
   if(document_xml[1]=='/') { //ending tag
    document_xml++;
    //TAGs </w:r> </w:t>
    if(xml_is_tag(document_xml, "w:r")==STATUS_TRUE || xml_is_tag(document_xml, "w:t")==STATUS_TRUE) {
     document_editor_take_style_from_stack();
     
     //WRITE OUTPUT TO DMF
     if(dmf[-1]==DMF_SECTION_FORMAT_CHANGE_SIGNATURE) {
      //we can rewrite format change section
      dmf_add_section_format_change(((dword_t)dmf)-DMF_SFCH_ENTRY_LENGTH_IN_BYTES, dmf_character_size, dmf_character_emphasis, dmf_character_color, dmf_character_background_color);
     }
     else {
      //new format change section
      dmf_add_section_format_change(((dword_t)dmf), dmf_character_size, dmf_character_emphasis, dmf_character_color, dmf_character_background_color);
      dmf = (word_t *) (((dword_t)dmf)+DMF_SFCH_ENTRY_LENGTH_IN_BYTES);
     }
    }
   }
   else { //normal tag
    //TAG <w:p>
    if(xml_is_tag(document_xml, "w:p")==STATUS_TRUE) {
     //WRITE OUTPUT TO DMF
     if(dmf[-1]==DMF_SECTION_FORMAT_CHANGE_SIGNATURE && dmf_is_section_format_change_only_span_change(((dword_t)dmf)-DMF_SFCH_ENTRY_LENGTH_IN_BYTES)==STATUS_TRUE) {
      //we can rewrite format change section
      dmf_add_section_format_change(((dword_t)dmf)-DMF_SFCH_ENTRY_LENGTH_IN_BYTES, dmf_character_size, dmf_character_emphasis, dmf_character_color, dmf_character_background_color);
      dmf_add_section_new_paragraph(((dword_t)dmf)-DMF_SFCH_ENTRY_LENGTH_IN_BYTES, dmf_actual_paragraph_description, dmf_paragraph_top_border, dmf_paragraph_bottom_border, dmf_paragraph_left_border, dmf_paragraph_right_border, dmf_paragaph_list_entry);
     }
     else {
      //new format change section
      dmf_add_section_format_change(((dword_t)dmf), dmf_character_size, dmf_character_emphasis, dmf_character_color, dmf_character_background_color);
      dmf_add_section_new_paragraph(((dword_t)dmf), dmf_actual_paragraph_description, dmf_paragraph_top_border, dmf_paragraph_bottom_border, dmf_paragraph_left_border, dmf_paragraph_right_border, dmf_paragaph_list_entry);
      dmf = (word_t *) (((dword_t)dmf)+DMF_SFCH_ENTRY_LENGTH_IN_BYTES);
     }
    }
    //TAGs <w:r> <w:t>
    else if(xml_is_tag(document_xml, "w:r")==STATUS_TRUE || xml_is_tag(document_xml, "w:t")==STATUS_TRUE) {
     document_editor_add_style_to_stack();

     //WRITE OUTPUT TO DMF
     if(dmf[-1]==DMF_SECTION_FORMAT_CHANGE_SIGNATURE) {
      //we can rewrite format change section
      dmf_add_section_format_change(((dword_t)dmf)-DMF_SFCH_ENTRY_LENGTH_IN_BYTES, dmf_character_size, dmf_character_emphasis, dmf_character_color, dmf_character_background_color);
     }
     else {
      //new format change section
      dmf_add_section_format_change(((dword_t)dmf), dmf_character_size, dmf_character_emphasis, dmf_character_color, dmf_character_background_color);
      dmf = (word_t *) (((dword_t)dmf)+DMF_SFCH_ENTRY_LENGTH_IN_BYTES);
     }
    }
    //TAG <w:pStyle>
    else if(xml_is_tag(document_xml, "w:pStyle")==STATUS_TRUE) {
     if(xml_find_tag_attribute("w:val")==STATUS_TRUE) {
      if(xml_is_attribute("Normal")==STATUS_TRUE) {
       dmf_character_size = 9;
       dmf_character_emphasis = 0;
       dmf_character_color = BLACK;
       dmf_character_background_color = TRANSPARENT_COLOR;
       dmf_paragraph_alignment = DMF_SFCH_PARAGRAPH_DESCRIPTION_ALIGNMENT_LEFT;
      }
     }
    }
    //TAG <w:sz>
    else if(xml_is_tag(document_xml, "w:sz")==STATUS_TRUE) {
     if(xml_find_tag_attribute("w:val")==STATUS_TRUE) {
      dmf_character_size = ((72*(xml_get_attribute_number()/2))/96); //TODO: is this method always working?
     }
    }
    //TAG <w:b>
    else if(xml_is_tag(document_xml, "w:b")==STATUS_TRUE) {
     dmf_character_emphasis |= SF_EMPHASIS_BOLD;
    }
    //TAG <w:u>
    else if(xml_is_tag(document_xml, "w:u")==STATUS_TRUE) {
     dmf_character_emphasis |= SF_EMPHASIS_UNDERLINE;

     if(xml_find_tag_attribute("w:val")==STATUS_TRUE) {
      if(xml_is_attribute("none")==STATUS_TRUE) {
       dmf_character_emphasis &= ~SF_EMPHASIS_UNDERLINE;
      }
     }
    }
    //TAG <w:strike>
    else if(xml_is_tag(document_xml, "w:strike")==STATUS_TRUE) {
     dmf_character_emphasis |= SF_EMPHASIS_STRIKE;
    }
    //TAG <w:color>
    else if(xml_is_tag(document_xml, "w:color")==STATUS_TRUE) {
     if(xml_find_tag_attribute("w:val")==STATUS_TRUE) {
      dmf_character_color = xml_get_attribute_hex_number();
     }
    }
    //TAG <w:highlight>
    else if(xml_is_tag(document_xml, "w:highlight")==STATUS_TRUE) {
     if(xml_find_tag_attribute("w:val")==STATUS_TRUE) {
      dmf_character_background_color = xml_get_attribute_color_number();
     }
    }
   }

   //skip tag
   while(document_xml[0]!='>' && document_xml[1]!=0) {
    document_xml++;
   }
   document_xml++;
  }
  else { //char
   if(*document_xml=='&') { //escape sequence
    *dmf = xml_get_escape_sequence_character(document_xml); //convert to char and add to DMF
    dmf++;

    //skip escape sequence
    while(document_xml[1]!=0) {
     if(*document_xml==';') {
      break;
     }
     document_xml++;
    }
   }
   else if(*document_xml>=' ') { //skip unprintable characters
    *dmf = *document_xml; //add to DMF
    dmf++;
   }
   document_xml++;
  }
 }

 free(document_xml_file_memory);
 return dmf_memory;
}