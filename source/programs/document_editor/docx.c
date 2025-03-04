//BleskOS

/*
* MIT License
* Copyright (c) 2023-2025 BleskOS developers
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
 if(document_xml_file_number==ZIP_FILE_NOT_FOUNDED) {
  log("\nDOCX: word/document.xml not founded");
  return STATUS_ERROR;
 }
 dword_t document_raw_xml_file_memory = zip_extract_file(docx_memory, docx_size, document_xml_file_number);
 if(document_raw_xml_file_memory==STATUS_ERROR) {
  log("\nDOCX: error with extracting word/document.xml");
  return STATUS_ERROR;
 }
 dword_t document_xml_file_memory = prepare_xml_file(document_raw_xml_file_memory, zip_extracted_file_size, XML_NO_SPECIAL_ATTRIBUTES);
 free((void *)document_raw_xml_file_memory);

 //count how many memory we will need for DMF
 word_t *document_xml = (word_t *) (document_xml_file_memory);
 word_t *docx_body_tag = (word_t *) (0);
 dword_t dmf_size = DMF_SFCH_ENTRY_LENGTH_IN_BYTES;
 dmf_number_of_chars_in_document = 0;
 while(*document_xml!=0) {
  if(*document_xml=='<') { //tag
   if(xml_is_tag(document_xml, "w:p")==STATUS_TRUE || xml_is_tag(document_xml, "w:r")==STATUS_TRUE || xml_is_tag(document_xml, "w:t")==STATUS_TRUE || xml_is_tag(document_xml, "w:br")==STATUS_TRUE || xml_is_tag(document_xml, "/w:r")==STATUS_TRUE || xml_is_tag(document_xml, "/w:t")==STATUS_TRUE || xml_is_tag(document_xml, "/w:sectPr")==STATUS_TRUE) {
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
  free((void *)document_xml_file_memory);
  return STATUS_ERROR;
 }

 //allocate memory for DMF
 dmf_size+=2;
 dword_t dmf_memory = (dword_t) calloc(dmf_size);

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

 //"Normal" style
 document_editor_style_stack_number_of_entries = 0;
 dmf_character_size = 9;
 dmf_character_emphasis = 0;
 dmf_character_color = BLACK;
 dmf_character_background_color = TRANSPARENT_COLOR;
 dmf_paragraph_alignment = DMF_SFCH_PARAGRAPH_DESCRIPTION_ALIGNMENT_LEFT;
 dmf_actual_paragraph_description = 0;
 dmf_paragraph_top_border = 0;
 dmf_paragraph_bottom_border = 0;
 dmf_paragraph_left_border = 0;
 dmf_paragraph_right_border = 0;
 dmf_paragaph_list_entry = 0;
 document_editor_add_style_to_stack();

 //set variables
 byte_t is_this_paragraph_list_entry = STATUS_FALSE, is_text_output = STATUS_FALSE;
 dword_t list_entry_level = 0, list_entry_type = 0;
 dmf_page_break = STATUS_FALSE;
 document_editor_stack_of_lists_number_of_entries = 0;
 byte_t *docx_list_of_list_types = (byte_t *) (calloc(DOCX_MAX_NUMBER_OF_LISTS));
 dword_t docx_list_of_list_types_pointer = 0;
 document_xml = docx_body_tag;

 //read numbering.xml
 dword_t numbering_xml_file_number = search_for_file_in_zip(docx_memory, docx_size, "word/numbering.xml");
 if(numbering_xml_file_number!=ZIP_FILE_NOT_FOUNDED) {
  dword_t numbering_raw_xml_file_memory = zip_extract_file(docx_memory, docx_size, numbering_xml_file_number);
  if(numbering_raw_xml_file_memory!=STATUS_ERROR) {
   dword_t numbering_xml_file_memory = prepare_xml_file(numbering_raw_xml_file_memory, zip_extracted_file_size, XML_NO_SPECIAL_ATTRIBUTES);
   free((void *)numbering_raw_xml_file_memory);

   //scan numbering.xml
   word_t *numbering_xml = (word_t *) (numbering_xml_file_memory);
   while(*numbering_xml!=0) {
    if(*numbering_xml=='<') { //tag
     if(xml_is_tag(numbering_xml, "w:abstractNum")==STATUS_TRUE) { //get number of list
      docx_list_of_list_types_pointer = 0;
      if(xml_find_tag_attribute("w:abstractNumId")==STATUS_TRUE) {
       docx_list_of_list_types_pointer = xml_get_attribute_number();
       if(docx_list_of_list_types_pointer>=DOCX_MAX_NUMBER_OF_LISTS) {
        docx_list_of_list_types_pointer = 0;
       }
      }
     }
     else if(xml_is_tag(numbering_xml, "/w:abstractNum")==STATUS_TRUE) { //save list type
      docx_list_of_list_types[docx_list_of_list_types_pointer] = list_entry_type;
     }
     else if(xml_is_tag(numbering_xml, "w:numFmt")==STATUS_TRUE) { //read list type TODO: not only type of last list level
      list_entry_type = DOCUMENT_EDITOR_LIST_ORDERED;

      if(xml_find_tag_attribute("w:val")==STATUS_TRUE) {
       if(xml_is_attribute("bullet")==STATUS_TRUE) {
        list_entry_type = DOCUMENT_EDITOR_LIST_UNORDERED;
       }
      }
     }

     //skip tag
     while(numbering_xml[0]!='>' && numbering_xml[1]!=0) {
      numbering_xml++;
     }
     numbering_xml++;
    }
    else {
     numbering_xml++;
    }
   }
   free((void *)numbering_xml_file_memory);
  }
  else {
   log("\nDOCX: error with extracting word/numbering.xml");
  }
 }

 //convert DOCX to DMF
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

     if(xml_is_tag(document_xml, "w:t")==STATUS_TRUE) {
      is_text_output = STATUS_FALSE;
     }
    }
    //TAG </w:pPr>
    else if(xml_is_tag(document_xml, "w:pPr")==STATUS_TRUE) {
     dmf_actual_paragraph_description = (dmf_paragraph_alignment<<DMF_SFCH_PARAGRAPH_DESCRIPTION_ALIGNMENT_SHIFT);

     //list entry
     if(is_this_paragraph_list_entry==STATUS_TRUE) {
      dmf_actual_paragraph_description |= DMF_SFCH_PARAGRAPH_DESCRIPTION_LIST_ENTRY;

      if(document_editor_stack_of_lists_pointer[document_editor_stack_of_lists_number_of_entries-1].type_of_list==DOCUMENT_EDITOR_LIST_UNORDERED) { //unordered list
       dmf_paragaph_list_entry = DMF_SFCH_PARAGRAPH_LIST_ENTRY_DOT;
      }
      else { //ordered list
       dmf_paragaph_list_entry = document_editor_stack_of_lists_pointer[document_editor_stack_of_lists_number_of_entries-1].number;
       document_editor_stack_of_lists_pointer[document_editor_stack_of_lists_number_of_entries-1].number++;
      }

      is_this_paragraph_list_entry = STATUS_FALSE;
     }
     else {
      document_editor_stack_of_lists_number_of_entries = 0;
     }

     //add all properties to paragraph format change section
     if(dmf[-1]==DMF_SECTION_FORMAT_CHANGE_SIGNATURE) { 
      dmf_add_section_format_change(((dword_t)dmf)-DMF_SFCH_ENTRY_LENGTH_IN_BYTES, dmf_character_size, dmf_character_emphasis, dmf_character_color, dmf_character_background_color);
      dmf_add_section_new_paragraph(((dword_t)dmf)-DMF_SFCH_ENTRY_LENGTH_IN_BYTES, dmf_actual_paragraph_description, dmf_paragraph_top_border, dmf_paragraph_bottom_border, dmf_paragraph_left_border, dmf_paragraph_right_border, document_editor_stack_of_lists_number_of_entries, dmf_paragaph_list_entry);
     }
    }
    //TAG </w:numPr>
    else if(xml_is_tag(document_xml, "w:numPr")==STATUS_TRUE) {
     if(list_entry_type==DOCUMENT_EDITOR_NO_LIST) {
      is_this_paragraph_list_entry = STATUS_FALSE;
     }
     else {
      if(list_entry_level>document_editor_stack_of_lists_number_of_entries) { //this is new list
       document_editor_stack_of_lists_pointer[document_editor_stack_of_lists_number_of_entries].type_of_list = list_entry_type;
       document_editor_stack_of_lists_pointer[document_editor_stack_of_lists_number_of_entries].number = 1;
       document_editor_stack_of_lists_number_of_entries++;
      }
      else if(list_entry_level<document_editor_stack_of_lists_number_of_entries) { //previous list was finished
       document_editor_stack_of_lists_number_of_entries--;
      }
      //list_entry_level==document_editor_stack_of_lists_number_of_entries mean this is next entry of same list

      is_this_paragraph_list_entry = STATUS_TRUE;
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
      dmf_add_section_new_paragraph(((dword_t)dmf)-DMF_SFCH_ENTRY_LENGTH_IN_BYTES, dmf_actual_paragraph_description, dmf_paragraph_top_border, dmf_paragraph_bottom_border, dmf_paragraph_left_border, dmf_paragraph_right_border, document_editor_stack_of_lists_number_of_entries, dmf_paragaph_list_entry);
     }
     else {
      //new format change section
      dmf_add_section_format_change(((dword_t)dmf), dmf_character_size, dmf_character_emphasis, dmf_character_color, dmf_character_background_color);
      dmf_add_section_new_paragraph(((dword_t)dmf), dmf_actual_paragraph_description, dmf_paragraph_top_border, dmf_paragraph_bottom_border, dmf_paragraph_left_border, dmf_paragraph_right_border, document_editor_stack_of_lists_number_of_entries, dmf_paragaph_list_entry);
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

     if(xml_is_tag(document_xml, "w:t")==STATUS_TRUE) {
      is_text_output = STATUS_TRUE;
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
       dmf_paragraph_left_border = 0;
       dmf_paragraph_right_border = 0;
       dmf_paragraph_top_border = 0;
       dmf_paragraph_bottom_border = 0;

       dmf_actual_paragraph_description = (dmf_paragraph_alignment<<DMF_SFCH_PARAGRAPH_DESCRIPTION_ALIGNMENT_SHIFT);
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

     if(xml_find_tag_attribute("w:val")==STATUS_TRUE) {
      if(xml_is_attribute("none")==STATUS_TRUE || xml_is_attribute("false")==STATUS_TRUE || xml_is_attribute("0")==STATUS_TRUE) {
       dmf_character_emphasis &= ~SF_EMPHASIS_BOLD;
      }
     }
    }
    //TAG <w:u>
    else if(xml_is_tag(document_xml, "w:u")==STATUS_TRUE) {
     dmf_character_emphasis |= SF_EMPHASIS_UNDERLINE;

     if(xml_find_tag_attribute("w:val")==STATUS_TRUE) {
      if(xml_is_attribute("none")==STATUS_TRUE || xml_is_attribute("false")==STATUS_TRUE) {
       dmf_character_emphasis &= ~SF_EMPHASIS_UNDERLINE;
      }
     }
    }
    //TAG <w:strike>
    else if(xml_is_tag(document_xml, "w:strike")==STATUS_TRUE) {
     dmf_character_emphasis |= SF_EMPHASIS_STRIKE;

     if(xml_find_tag_attribute("w:val")==STATUS_TRUE) {
      if(xml_is_attribute("none")==STATUS_TRUE || xml_is_attribute("false")==STATUS_TRUE) {
       dmf_character_emphasis &= ~SF_EMPHASIS_STRIKE;
      }
     }
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
    //TAG <w:jc>
    else if(xml_is_tag(document_xml, "w:jc")==STATUS_TRUE) {
     if(xml_find_tag_attribute("w:val")==STATUS_TRUE) {
      dmf_paragraph_alignment = DMF_SFCH_PARAGRAPH_DESCRIPTION_ALIGNMENT_LEFT; //"start" "left"
      if(xml_is_attribute("center")==STATUS_TRUE) {
       dmf_paragraph_alignment = DMF_SFCH_PARAGRAPH_DESCRIPTION_ALIGNMENT_CENTER;
      }
      else if(xml_is_attribute("end")==STATUS_TRUE || xml_is_attribute("right")==STATUS_TRUE) {
       dmf_paragraph_alignment = DMF_SFCH_PARAGRAPH_DESCRIPTION_ALIGNMENT_RIGHT;
      }
      //TODO: "both" "justify"
     }
    }
    //TAG <w:ind>
    else if(xml_is_tag(document_xml, "w:ind")==STATUS_TRUE) {
     if(xml_find_tag_attribute("w:left")==STATUS_TRUE) {
      dmf_paragraph_left_border = (xml_get_attribute_number()/15); //TODO: is this method always working?
     }
     if(xml_find_tag_attribute("w:right")==STATUS_TRUE) {
      dmf_paragraph_right_border = (xml_get_attribute_number()/15); //TODO: is this method always working?
     }
    }
    //TAG <w:spacing>
    else if(xml_is_tag(document_xml, "w:spacing")==STATUS_TRUE) {
     if(xml_find_tag_attribute("w:before")==STATUS_TRUE) {
      dmf_paragraph_top_border = (9*xml_get_attribute_number()/240); //TODO: is this method always working?
     }
     if(xml_find_tag_attribute("w:after")==STATUS_TRUE) {
      dmf_paragraph_bottom_border = (9*xml_get_attribute_number()/240); //TODO: is this method always working?
     }
    }
    //TAG <w:ilvl>
    else if(xml_is_tag(document_xml, "w:ilvl")==STATUS_TRUE) {
     if(xml_find_tag_attribute("w:val")==STATUS_TRUE) {
      list_entry_level = (xml_get_attribute_number()+1); //levels starts from 0
     }
    }
    //TAG <w:numId>
    else if(xml_is_tag(document_xml, "w:numId")==STATUS_TRUE) {
     if(xml_find_tag_attribute("w:val")==STATUS_TRUE) {
      dword_t list_number = xml_get_attribute_number();
      if(list_number==0) {
       list_entry_type = STATUS_FALSE;
      }
      else {
       list_entry_type = docx_list_of_list_types[xml_get_attribute_number()];
      }
     }
    }
    //TAG <w:pgSz>
    else if(xml_is_tag(document_xml, "w:pgSz")==STATUS_TRUE) {
     if(xml_find_tag_attribute("w:w")==STATUS_TRUE) {
      dmf_page_width = (xml_get_attribute_number()/15); //TODO: is this method always working?
     }
     if(xml_find_tag_attribute("w:h")==STATUS_TRUE) {
      dmf_page_height = (xml_get_attribute_number()/15); //TODO: is this method always working?
     }

     //WRITE OUTPUT TO DMF
     //TODO: change only one page layout
     dmf_add_section_new_page(dmf_memory, dmf_page_width, dmf_page_height, dmf_page_top_border, dmf_page_bottom_border, dmf_page_left_border, dmf_page_right_border);
    }
    //TAG <w:pgMar>
    else if(xml_is_tag(document_xml, "w:pgMar")==STATUS_TRUE) {
     if(xml_find_tag_attribute("w:left")==STATUS_TRUE) {
      dmf_page_left_border = (xml_get_attribute_number()/15); //TODO: is this method always working?
     }
     if(xml_find_tag_attribute("w:right")==STATUS_TRUE) {
      dmf_page_right_border = (xml_get_attribute_number()/15); //TODO: is this method always working?
     }
     if(xml_find_tag_attribute("w:top")==STATUS_TRUE) {
      dmf_page_top_border = (xml_get_attribute_number()/15); //TODO: is this method always working?
     }
     if(xml_find_tag_attribute("w:bottom")==STATUS_TRUE) {
      dmf_page_bottom_border = (xml_get_attribute_number()/15); //TODO: is this method always working?
     }

     //WRITE OUTPUT TO DMF
     //TODO: change only one page layout
     dmf_add_section_new_page(dmf_memory, dmf_page_width, dmf_page_height, dmf_page_top_border, dmf_page_bottom_border, dmf_page_left_border, dmf_page_right_border);
    }
    //TAG <w:br>
    else if(xml_is_tag(document_xml, "w:br")==STATUS_TRUE) {
     if(xml_find_tag_attribute("w:type")==STATUS_TRUE) {
      if(xml_is_attribute("page")==STATUS_TRUE) { //page break
       dmf_add_section_page_break(((dword_t)dmf));
       dmf = (word_t *) (((dword_t)dmf)+DMF_SFCH_ENTRY_LENGTH_IN_BYTES);
      }
     }
     else { //line break
      *dmf = 0xA;
      dmf++;
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
   if(is_text_output==STATUS_TRUE) {
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
   }
   
   document_xml++;
  }
 }

 free((void *)docx_list_of_list_types);
 free((void *)document_xml_file_memory);
 return dmf_memory;
}