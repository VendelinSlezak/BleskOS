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
 dword_t content_xml_file_memory = prepare_xml_file(content_raw_xml_file_memory, zip_extracted_file_size, XML_NO_SPECIAL_ATTRIBUTES);
 free(content_raw_xml_file_memory);

 //count how many memory we will need for DMF and find all available styles
 word_t *content_xml = (word_t *) (content_xml_file_memory);
 word_t *odt_body_tag = (word_t *) (0);
 dword_t dmf_size = DMF_SFCH_ENTRY_LENGTH_IN_BYTES;
 dmf_number_of_chars_in_document = 0;
 document_editor_list_of_styles_number_of_entries = 0;
 while(*content_xml!=0) {
  if(*content_xml=='<') { //tag
   if(xml_is_tag(content_xml, "text:p")==STATUS_TRUE || xml_is_tag(content_xml, "/text:p")==STATUS_TRUE) {
    dmf_size+=(DMF_SFCH_ENTRY_LENGTH_IN_BYTES*2);
   }
   else if(xml_is_tag(content_xml, "text:h")==STATUS_TRUE || xml_is_tag(content_xml, "/text:h")==STATUS_TRUE) {
    dmf_size+=(DMF_SFCH_ENTRY_LENGTH_IN_BYTES*2);
   }
   else if(xml_is_tag(content_xml, "text:span")==STATUS_TRUE || xml_is_tag(content_xml, "/text:span")==STATUS_TRUE) {
    dmf_size+=(DMF_SFCH_ENTRY_LENGTH_IN_BYTES*2);
   }
   else if(xml_is_tag(content_xml, "text:line-break")==STATUS_TRUE) {
    dmf_size+=2; //0xA
   }
   else if(xml_is_tag(content_xml, "style:style")==STATUS_TRUE) {
    if(xml_find_tag_attribute("style:name")==STATUS_TRUE) {
     while(content_xml[0]!='>' && content_xml[1]!=0) { //skip tag
      content_xml++;
     }
     document_editor_add_style_to_list((dword_t)xml_tag_attribute_content_memory, ((dword_t)content_xml)+2); //+2 = skip char >
    }
   }
   else if(xml_is_tag(content_xml, "text:list-style")==STATUS_TRUE) {
    if(xml_find_tag_attribute("style:name")==STATUS_TRUE) {
     while(content_xml[0]!='>' && content_xml[1]!=0) { //skip tag
      content_xml++;
     }
     document_editor_add_style_to_list((dword_t)xml_tag_attribute_content_memory, ((dword_t)content_xml)+2); //+2 = skip char >
    }
   }
   else if(xml_is_tag(content_xml, "office:body")==STATUS_TRUE) {
    odt_body_tag = content_xml;
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
 if(((dword_t)odt_body_tag)==0) {
  log("\nODT: no <office:body>");
  free(content_xml_file_memory);
  return STATUS_ERROR;
 }

 //allocate memory for DMF
 dmf_size+=4; //ending
 dword_t dmf_memory = calloc(dmf_size);

 //default page layout - A4 with 2 cm margins
 dmf_page_width = 794;
 dmf_page_height = 1123;
 dmf_page_top_border = 75;
 dmf_page_bottom_border = 75;
 dmf_page_left_border = 75;
 dmf_page_right_border = 75;

 //read styles.xml
 dword_t styles_xml_file_number = search_for_file_in_zip(odt_memory, odt_size, "styles.xml");
 if(styles_xml_file_number!=ZIP_FILE_NOT_FOUNDED) {
  dword_t styles_raw_xml_file_memory = zip_extract_file(odt_memory, odt_size, styles_xml_file_number);
  if(styles_raw_xml_file_memory!=STATUS_ERROR) {
   dword_t styles_xml_file_memory = prepare_xml_file(styles_raw_xml_file_memory, zip_extracted_file_size, XML_NO_SPECIAL_ATTRIBUTES);
   free(styles_raw_xml_file_memory);

   //scan styles.xml
   word_t *styles_xml = (word_t *) (styles_xml_file_memory);
   while(*styles_xml!=0) {
    if(*styles_xml=='<') { //tag
     //page layout
     if(xml_is_tag(styles_xml, "style:page-layout-properties")==STATUS_TRUE) {
      if(xml_find_tag_attribute("fo:page-width")==STATUS_TRUE) {
       dmf_page_width = xml_get_attribute_number_in_px();
      }
      if(xml_find_tag_attribute("fo:page-height")==STATUS_TRUE) {
       dmf_page_height = xml_get_attribute_number_in_px();
      }
      if(xml_find_tag_attribute("fo:margin-top")==STATUS_TRUE) {
       dmf_page_top_border = xml_get_attribute_number_in_px();
      }
      if(xml_find_tag_attribute("fo:margin-bottom")==STATUS_TRUE) {
       dmf_page_bottom_border = xml_get_attribute_number_in_px();
      }
      if(xml_find_tag_attribute("fo:margin-left")==STATUS_TRUE) {
       dmf_page_left_border = xml_get_attribute_number_in_px();
      }
      if(xml_find_tag_attribute("fo:margin-right")==STATUS_TRUE) {
       dmf_page_right_border = xml_get_attribute_number_in_px();
      }
     }

     //skip tag
     while(styles_xml[0]!='>' && styles_xml[1]!=0) {
      styles_xml++;
     }
     styles_xml++;
    }
    else {
     styles_xml++;
    }
   }
   free(styles_xml_file_memory);
  }
  else {
   log("\nODT: error with extracting styles.xml");
  }
 }

 //add start entry
 word_t *dmf = (word_t *) (dmf_memory);
 dmf_add_section_format_change(((dword_t)dmf), 0, 0, BLACK, TRANSPARENT_COLOR);
 dmf_add_section_new_page(((dword_t)dmf), dmf_page_width, dmf_page_height, dmf_page_top_border, dmf_page_bottom_border, dmf_page_left_border, dmf_page_right_border);
 dmf = (word_t *) (((dword_t)dmf)+DMF_SFCH_ENTRY_LENGTH_IN_BYTES);

 //convert ODT to DMF
 content_xml = odt_body_tag;
 document_editor_style_stack_number_of_entries = 0;
 document_editor_stack_of_lists_number_of_entries = 0;

 //"Standard" style
 dmf_character_size = 9;
 dmf_character_emphasis = 0;
 dmf_character_color = BLACK;
 dmf_character_background_color = TRANSPARENT_COLOR;
 dmf_paragraph_alignment = DMF_SFCH_PARAGRAPH_DESCRIPTION_ALIGNMENT_LEFT;
 document_editor_add_style_to_stack();

 //list variables
 byte_t is_this_paragraph_list_entry = STATUS_FALSE;
 dmf_paragaph_list_entry = 0;
 while(*content_xml!=0) {
  if(*content_xml=='<') { //tag
   if(content_xml[1]=='/') { //ending tag
    content_xml++;
    //TAGS </text:p> </text:h> </text:span>
    if(xml_is_tag(content_xml, "text:p")==STATUS_TRUE || xml_is_tag(content_xml, "text:h")==STATUS_TRUE || xml_is_tag(content_xml, "text:span")==STATUS_TRUE) {
     //take style from stack
     document_editor_take_style_from_stack();

     //write output to DMF
     if(dmf[-1]==DMF_SECTION_FORMAT_CHANGE_SIGNATURE && dmf_is_section_format_change_only_span_change(((dword_t)dmf)-DMF_SFCH_ENTRY_LENGTH_IN_BYTES)==STATUS_TRUE) {
      //we can rewrite format change section
      dmf_add_section_format_change(((dword_t)dmf)-DMF_SFCH_ENTRY_LENGTH_IN_BYTES, dmf_character_size, dmf_character_emphasis, dmf_character_color, dmf_character_background_color);
     }
     else {
      //new format change section
      dmf_add_section_format_change(((dword_t)dmf), dmf_character_size, dmf_character_emphasis, dmf_character_color, dmf_character_background_color);
      dmf = (word_t *) (((dword_t)dmf)+DMF_SFCH_ENTRY_LENGTH_IN_BYTES);
     }
    }
    //TAG </text:list-item>
    else if(xml_is_tag(content_xml, "text:list-item")==STATUS_TRUE) {
     is_this_paragraph_list_entry = STATUS_FALSE;
    }
    //TAG </text:list>
    else if(xml_is_tag(content_xml, "text:list")==STATUS_TRUE) {
     if(document_editor_stack_of_lists_number_of_entries>0) {
      document_editor_stack_of_lists_number_of_entries--;
     }
    }
   }
   else { //normal tag
    //TAGS <text:p> <text:h> <text:span>
    if(xml_is_tag(content_xml, "text:p")==STATUS_TRUE || xml_is_tag(content_xml, "text:h")==STATUS_TRUE || xml_is_tag(content_xml, "text:span")==STATUS_TRUE) {
     //reset variables
     dmf_actual_paragraph_description = (dmf_paragraph_alignment<<DMF_SFCH_PARAGRAPH_DESCRIPTION_ALIGNMENT_SHIFT);
     dmf_paragraph_top_border = 0;
     dmf_paragraph_bottom_border = 0;
     dmf_paragraph_left_border = 0;
     dmf_paragraph_right_border = 0;
     dmf_paragaph_list_entry = 0;
     dmf_page_break = STATUS_FALSE;
     
     //read attributes
     if(xml_find_tag_attribute("text:style-name")==STATUS_TRUE) {
      //find type of style
      if(xml_is_attribute("Standard")==STATUS_TRUE) { //default style
       dmf_character_size = 9;
       dmf_character_emphasis = 0;
       dmf_character_color = BLACK;
       dmf_character_background_color = TRANSPARENT_COLOR;
      }
      else { //try to find style in style list
       for(dword_t i=0; i<document_editor_list_of_styles_number_of_entries; i++) {
        if(xml_compare_attribute_and_attribute((word_t *)(document_editor_list_of_styles_pointer[i].memory_of_style_name), xml_tag_attribute_content_memory)==STATUS_TRUE) {
         odt_read_style(document_editor_list_of_styles_pointer[i].memory_of_style_content); //read style attributes
        }
       }
      }

      //some results needs to be converted
      dmf_paragraph_alignment = ((dmf_actual_paragraph_description>>DMF_SFCH_PARAGRAPH_DESCRIPTION_ALIGNMENT_SHIFT) & 0b11);

      //ONLY <text:p> If this is paragraph in list, add list type
      if(xml_is_tag(content_xml, "text:p")==STATUS_TRUE) {
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
      }
      
     }

     //WRITE OUTPUT TO DMF
     if(dmf_page_break==STATUS_TRUE && ((dword_t)dmf)!=(dmf_memory+DMF_SFCH_ENTRY_LENGTH_IN_BYTES)) { //page break, page break in first paragraph is not performed
      dmf_add_section_page_break(((dword_t)dmf));
      dmf = (word_t *) (((dword_t)dmf)+DMF_SFCH_ENTRY_LENGTH_IN_BYTES);
     }
     //ONLY <text:span>
     if(xml_is_tag(content_xml, "text:span")==STATUS_TRUE) {
      if(dmf[-1]==DMF_SECTION_FORMAT_CHANGE_SIGNATURE && dmf_is_section_format_change_only_span_change(((dword_t)dmf)-DMF_SFCH_ENTRY_LENGTH_IN_BYTES)==STATUS_TRUE) {
       //we can rewrite format change section
       dmf_add_section_format_change(((dword_t)dmf)-DMF_SFCH_ENTRY_LENGTH_IN_BYTES, dmf_character_size, dmf_character_emphasis, dmf_character_color, dmf_character_background_color);
      }
      else {
       //new format change section
       dmf_add_section_format_change(((dword_t)dmf), dmf_character_size, dmf_character_emphasis, dmf_character_color, dmf_character_background_color);
       dmf = (word_t *) (((dword_t)dmf)+DMF_SFCH_ENTRY_LENGTH_IN_BYTES);
      }
     }
     else { //ONLY <text:p> and <text:h>
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

     //add style to stack
     if(xml_does_tag_have_pair(content_xml)==STATUS_TRUE) {
      document_editor_add_style_to_stack();
     }
     else {
      //get values before this paragraph
      document_editor_take_style_from_stack_wihout_moving();
     }
    }
    //TAG <text:line-break>
    else if(xml_is_tag(content_xml, "text:line-break")==STATUS_TRUE) {
     *dmf = 0xA;
     dmf++;
    }
    //TAG <text:list-item>
    else if(xml_is_tag(content_xml, "text:list-item")==STATUS_TRUE) {
     is_this_paragraph_list_entry = STATUS_TRUE;
    }
    //TAG <text:list>
    else if(xml_is_tag(content_xml, "text:list")==STATUS_TRUE) {
     if(document_editor_stack_of_lists_number_of_entries<DOCUMENT_EDITOR_NUMBER_OF_ENTRIES_IN_STACK_OF_LISTS) {
      if(document_editor_stack_of_lists_number_of_entries==0) {
       dmf_paragaph_list_entry = 1; //ordered list is default for first list
      }
      else {
       //otherwise default is previous list type
       if(document_editor_stack_of_lists_pointer[document_editor_stack_of_lists_number_of_entries-1].type_of_list==DOCUMENT_EDITOR_LIST_UNORDERED) {
        dmf_paragaph_list_entry = DMF_SFCH_PARAGRAPH_LIST_ENTRY_DOT;
       }
       else if(document_editor_stack_of_lists_pointer[document_editor_stack_of_lists_number_of_entries-1].type_of_list==DOCUMENT_EDITOR_LIST_ORDERED) {
        dmf_paragaph_list_entry = 1;
       }
      }

      //read attributes
      if(xml_find_tag_attribute("text:style-name")==STATUS_TRUE) {
       //try to find style in style list
       for(dword_t i=0; i<document_editor_list_of_styles_number_of_entries; i++) {
        if(xml_compare_attribute_and_attribute((word_t *)(document_editor_list_of_styles_pointer[i].memory_of_style_name), xml_tag_attribute_content_memory)==STATUS_TRUE) {
         odt_read_style(document_editor_list_of_styles_pointer[i].memory_of_style_content); //read style
        }
       }
      }

      //add to list
      if(dmf_paragaph_list_entry==DMF_SFCH_PARAGRAPH_LIST_ENTRY_DOT) {
       document_editor_stack_of_lists_pointer[document_editor_stack_of_lists_number_of_entries].type_of_list = DOCUMENT_EDITOR_LIST_UNORDERED;
      }
      else if(dmf_paragaph_list_entry==1) {
       document_editor_stack_of_lists_pointer[document_editor_stack_of_lists_number_of_entries].type_of_list = DOCUMENT_EDITOR_LIST_ORDERED;
       document_editor_stack_of_lists_pointer[document_editor_stack_of_lists_number_of_entries].number = 1;
      }
      document_editor_stack_of_lists_number_of_entries++;
     }
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
    *dmf = xml_get_escape_sequence_character(content_xml); //convert to char and add to DMF
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
    *dmf = *content_xml; //add to DMF
    dmf++;
   }
   content_xml++;
  }
 }

 free(content_xml_file_memory);
 return dmf_memory;
}

void odt_read_style(dword_t odt_style_memory) {
 word_t *content_xml = (word_t *) (odt_style_memory);
 if(content_xml[-1]=='>' && content_xml[-2]=='/') { //style with no tags
  return;
 }
 while(*content_xml!=0) {
  if(*content_xml=='<') { //tag
   if(xml_is_tag(content_xml, "/style:style")==STATUS_TRUE || xml_is_tag(content_xml, "/text:list-style")==STATUS_TRUE) { //end of style section
    return;
   }
   else if(xml_is_tag(content_xml, "style:text-properties")==STATUS_TRUE) { //text properties
    //font size
    if(xml_find_tag_attribute("fo:font-size")==STATUS_TRUE) {
     dmf_character_size = xml_get_attribute_number_in_px();
    }

    //font emphasis
    if(xml_find_tag_attribute("fo:font-weight")==STATUS_TRUE) {
     if(xml_is_attribute("none")==STATUS_TRUE || xml_is_attribute("normal")==STATUS_TRUE) {
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
   else if(xml_is_tag(content_xml, "style:paragraph-properties")==STATUS_TRUE) { //paragraph properties
    //paragraph alignment
    if(xml_find_tag_attribute("fo:text-align")==STATUS_TRUE) {
     dmf_paragraph_alignment = DMF_SFCH_PARAGRAPH_DESCRIPTION_ALIGNMENT_LEFT; //"start" "left"
     if(xml_is_attribute("center")==STATUS_TRUE) {
      dmf_paragraph_alignment = DMF_SFCH_PARAGRAPH_DESCRIPTION_ALIGNMENT_CENTER;
     }
     else if(xml_is_attribute("end")==STATUS_TRUE || xml_is_attribute("right")==STATUS_TRUE) {
      dmf_paragraph_alignment = DMF_SFCH_PARAGRAPH_DESCRIPTION_ALIGNMENT_RIGHT;
     }
     //TODO: "justify"

     dmf_actual_paragraph_description |= (dmf_paragraph_alignment<<DMF_SFCH_PARAGRAPH_DESCRIPTION_ALIGNMENT_SHIFT);
    }

    //paragraph margin
    if(xml_find_tag_attribute("fo:margin-top")==STATUS_TRUE) {
     dmf_paragraph_top_border = xml_get_attribute_number_in_px();
    }
    if(xml_find_tag_attribute("fo:margin-bottom")==STATUS_TRUE) {
     dmf_paragraph_bottom_border = xml_get_attribute_number_in_px();
    }
    if(xml_find_tag_attribute("fo:margin-left")==STATUS_TRUE) {
     dmf_paragraph_left_border = xml_get_attribute_number_in_px();
    }
    if(xml_find_tag_attribute("fo:margin-right")==STATUS_TRUE) {
     dmf_paragraph_right_border = xml_get_attribute_number_in_px();
    }

    //break
    if(xml_find_tag_attribute("fo:break-before")==STATUS_TRUE) {
     if(xml_is_attribute("page")==STATUS_TRUE) {
      dmf_page_break = STATUS_TRUE;
     }
    }
   }
   else if(xml_is_tag(content_xml, "text:list-level-style-bullet")==STATUS_TRUE) {
    dmf_paragaph_list_entry = DMF_SFCH_PARAGRAPH_LIST_ENTRY_DOT;
   }
   else if(xml_is_tag(content_xml, "text:list-level-style-number")==STATUS_TRUE) {
    dmf_paragaph_list_entry = 1;
   }
  }
  content_xml++;
 }
}

void convert_dmf_to_odt(dword_t dmf_memory) {
 word_t *dmf = (word_t *) (dmf_memory);

 //create META-INF/manifest.xml
 struct byte_stream_descriptor *file_manifest_xml_byte_stream = create_byte_stream(BYTE_STREAM_100_KB_BLOCK);
 add_string_to_byte_stream(file_manifest_xml_byte_stream, "<?xml version=\"1.0\" encoding=\"UTF-8\"?><manifest:manifest xmlns:manifest=\"urn:oasis:names:tc:opendocument:xmlns:manifest:1.0\" manifest:version=\"1.2\"><manifest:file-entry manifest:full-path=\"/\" manifest:version=\"1.2\" manifest:media-type=\"application/vnd.oasis.opendocument.text\"/><manifest:file-entry manifest:full-path=\"content.xml\" manifest:media-type=\"text/xml\"/></manifest:manifest>");

 //create mimetype
 struct byte_stream_descriptor *file_mimetype_byte_stream = create_byte_stream(BYTE_STREAM_100_KB_BLOCK);
 add_string_to_byte_stream(file_mimetype_byte_stream, "application/vnd.oasis.opendocument.text");

 //create content.xml
 struct byte_stream_descriptor *file_content_xml_byte_stream = create_byte_stream(BYTE_STREAM_100_KB_BLOCK);
 add_string_to_byte_stream(file_content_xml_byte_stream, "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
 add_string_to_byte_stream(file_content_xml_byte_stream, "<office:document-content xmlns:office=\"urn:oasis:names:tc:opendocument:xmlns:office:1.0\" xmlns:style=\"urn:oasis:names:tc:opendocument:xmlns:style:1.0\" xmlns:text=\"urn:oasis:names:tc:opendocument:xmlns:text:1.0\" xmlns:table=\"urn:oasis:names:tc:opendocument:xmlns:table:1.0\" xmlns:draw=\"urn:oasis:names:tc:opendocument:xmlns:drawing:1.0\" xmlns:fo=\"urn:oasis:names:tc:opendocument:xmlns:xsl-fo-compatible:1.0\" xmlns:xlink=\"http://www.w3.org/1999/xlink\" xmlns:dc=\"http://purl.org/dc/elements/1.1/\" xmlns:meta=\"urn:oasis:names:tc:opendocument:xmlns:meta:1.0\" xmlns:number=\"urn:oasis:names:tc:opendocument:xmlns:datastyle:1.0\" xmlns:svg=\"urn:oasis:names:tc:opendocument:xmlns:svg-compatible:1.0\" xmlns:chart=\"urn:oasis:names:tc:opendocument:xmlns:chart:1.0\" xmlns:dr3d=\"urn:oasis:names:tc:opendocument:xmlns:dr3d:1.0\" xmlns:math=\"http://www.w3.org/1998/Math/MathML\" xmlns:form=\"urn:oasis:names:tc:opendocument:xmlns:form:1.0\" xmlns:script=\"urn:oasis:names:tc:opendocument:xmlns:script:1.0\" xmlns:ooo=\"http://openoffice.org/2004/office\" xmlns:ooow=\"http://openoffice.org/2004/writer\" xmlns:oooc=\"http://openoffice.org/2004/calc\" xmlns:dom=\"http://www.w3.org/2001/xml-events\" xmlns:xforms=\"http://www.w3.org/2002/xforms\" xmlns:xsd=\"http://www.w3.org/2001/XMLSchema\" xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xmlns:rpt=\"http://openoffice.org/2005/report\" xmlns:of=\"urn:oasis:names:tc:opendocument:xmlns:of:1.2\" xmlns:xhtml=\"http://www.w3.org/1999/xhtml\" xmlns:grddl=\"http://www.w3.org/2003/g/data-view#\" xmlns:officeooo=\"http://openoffice.org/2009/office\" xmlns:tableooo=\"http://openoffice.org/2009/table\" xmlns:drawooo=\"http://openoffice.org/2010/draw\" xmlns:calcext=\"urn:org:documentfoundation:names:experimental:calc:xmlns:calcext:1.0\" xmlns:loext=\"urn:org:documentfoundation:names:experimental:office:xmlns:loext:1.0\" xmlns:field=\"urn:openoffice:names:experimental:ooo-ms-interop:xmlns:field:1.0\" xmlns:formx=\"urn:openoffice:names:experimental:ooxml-odf-interop:xmlns:form:1.0\" xmlns:css3t=\"http://www.w3.org/TR/css3-text/\" office:version=\"1.2\">");
 
 //"Standard" style
 dmf_character_size = 9;
 dmf_character_emphasis = 0;
 dmf_character_color = BLACK;
 dmf_character_background_color = TRANSPARENT_COLOR;
 dmf_page_break = STATUS_FALSE;

 //convert all styles in document
 dword_t number_of_paragraph_style = 1, number_of_text_style = 1;

 add_string_to_byte_stream(file_content_xml_byte_stream, "<office:automatic-styles>");
 while(*dmf!=0) {
  if(*dmf==DMF_SECTION_FORMAT_CHANGE_SIGNATURE) {
   //page break
   if((dmf[DMF_SFCH_DESCRIPTION_OFFSET] & DMF_SFCH_DESCRIPTION_PAGE_BREAK)==DMF_SFCH_DESCRIPTION_PAGE_BREAK) {
    dmf_page_break = STATUS_TRUE;

    //skip
    dmf = (word_t *) (((dword_t)dmf)+DMF_SFCH_ENTRY_LENGTH_IN_BYTES);
    continue;
   }

   //style of paragraph
   if((dmf[DMF_SFCH_DESCRIPTION_OFFSET] & DMF_SFCH_DESCRIPTION_NEW_PARAGRAPH)==DMF_SFCH_DESCRIPTION_NEW_PARAGRAPH) {
    //"Standard" style
    dmf_character_size = 9;
    dmf_character_emphasis = 0;
    dmf_character_color = BLACK;
    dmf_character_background_color = TRANSPARENT_COLOR;

    if(dmf_is_section_standard_style((dword_t)dmf)==STATUS_TRUE) { //this paragraph is "Standard" style, so it do not need its own style
     //skip
     dmf = (word_t *) (((dword_t)dmf)+DMF_SFCH_ENTRY_LENGTH_IN_BYTES);
     continue;
    }
    
    //create paragraph style entry
    add_string_to_byte_stream(file_content_xml_byte_stream, "<style:style style:name=\"P");
    add_number_to_byte_stream(file_content_xml_byte_stream, number_of_paragraph_style);
    add_string_to_byte_stream(file_content_xml_byte_stream, "\" style:family=\"paragraph\" style:parent-style-name=\"Standard\">");
    number_of_paragraph_style++;

    //paragraph-properties
    add_string_to_byte_stream(file_content_xml_byte_stream, "<style:paragraph-properties");

     //page break
     if(dmf_page_break==STATUS_TRUE) {
      add_string_to_byte_stream(file_content_xml_byte_stream, " fo:break-before=\"page\"");
      dmf_page_break = STATUS_FALSE;
     }

     //alignment
     dmf_paragraph_alignment = ((dmf[DMF_SFCH_PARAGRAPH_DESCRIPTION_OFFSET]>>DMF_SFCH_PARAGRAPH_DESCRIPTION_ALIGNMENT_SHIFT) & 0b11);
     if(dmf_paragraph_alignment!=DMF_SFCH_PARAGRAPH_DESCRIPTION_ALIGNMENT_LEFT) {
      if(dmf_paragraph_alignment==DMF_SFCH_PARAGRAPH_DESCRIPTION_ALIGNMENT_CENTER) {
       add_string_to_byte_stream(file_content_xml_byte_stream, " fo:text-align=\"center\"");
      }
      else if(dmf_paragraph_alignment==DMF_SFCH_PARAGRAPH_DESCRIPTION_ALIGNMENT_RIGHT) {
       add_string_to_byte_stream(file_content_xml_byte_stream, " fo:text-align=\"end\"");
      }
     }

     //margin
     if(dmf[DMF_SFCH_PARAGRAPH_TOP_BORDER_OFFSET]!=0) {
      add_string_to_byte_stream(file_content_xml_byte_stream, " fo:margin-top=\"");
      add_number_to_byte_stream(file_content_xml_byte_stream, dmf[DMF_SFCH_PARAGRAPH_TOP_BORDER_OFFSET]);
      add_string_to_byte_stream(file_content_xml_byte_stream, "px\"");
     }
     if(dmf[DMF_SFCH_PARAGRAPH_BOTTOM_BORDER_OFFSET]!=0) {
      add_string_to_byte_stream(file_content_xml_byte_stream, " fo:margin-bottom=\"");
      add_number_to_byte_stream(file_content_xml_byte_stream, dmf[DMF_SFCH_PARAGRAPH_BOTTOM_BORDER_OFFSET]);
      add_string_to_byte_stream(file_content_xml_byte_stream, "px\"");
     }
     if(dmf[DMF_SFCH_PARAGRAPH_LEFT_BORDER_OFFSET]!=0) {
      add_string_to_byte_stream(file_content_xml_byte_stream, " fo:margin-left=\"");
      add_number_to_byte_stream(file_content_xml_byte_stream, dmf[DMF_SFCH_PARAGRAPH_LEFT_BORDER_OFFSET]);
      add_string_to_byte_stream(file_content_xml_byte_stream, "px\"");
     }
     if(dmf[DMF_SFCH_PARAGRAPH_RIGHT_BORDER_OFFSET]!=0) {
      add_string_to_byte_stream(file_content_xml_byte_stream, " fo:margin-right=\"");
      add_number_to_byte_stream(file_content_xml_byte_stream, dmf[DMF_SFCH_PARAGRAPH_RIGHT_BORDER_OFFSET]);
      add_string_to_byte_stream(file_content_xml_byte_stream, "px\"");
     }

    add_string_to_byte_stream(file_content_xml_byte_stream, "/>");

    //text-properties
    add_string_to_byte_stream(file_content_xml_byte_stream, "<style:text-properties");

     //size
     if(dmf[DMF_SFCH_INLINE_CHANGE_SIZE_OFFSET]!=9) {
      add_string_to_byte_stream(file_content_xml_byte_stream, " fo:font-size=\"");
      add_number_to_byte_stream(file_content_xml_byte_stream, dmf[DMF_SFCH_INLINE_CHANGE_SIZE_OFFSET]*96/72); //convert to pt
      add_string_to_byte_stream(file_content_xml_byte_stream, "pt\"");
      dmf_character_size = dmf[DMF_SFCH_INLINE_CHANGE_SIZE_OFFSET];
     }

     //emphasis
     if(dmf[DMF_SFCH_INLINE_CHANGE_EMPHASIS_OFFSET]!=0) {
      if((dmf[DMF_SFCH_INLINE_CHANGE_EMPHASIS_OFFSET] & SF_EMPHASIS_BOLD)==SF_EMPHASIS_BOLD) {
       add_string_to_byte_stream(file_content_xml_byte_stream, " fo:font-weight=\"bold\"");
      }

      if((dmf[DMF_SFCH_INLINE_CHANGE_EMPHASIS_OFFSET] & SF_EMPHASIS_UNDERLINE)==SF_EMPHASIS_UNDERLINE) {
       add_string_to_byte_stream(file_content_xml_byte_stream, " style:text-underline-style=\"solid\" style:text-underline-width=\"auto\"");
      }

      if((dmf[DMF_SFCH_INLINE_CHANGE_EMPHASIS_OFFSET] & SF_EMPHASIS_STRIKE)==SF_EMPHASIS_STRIKE) {
       add_string_to_byte_stream(file_content_xml_byte_stream, " style:text-line-through-style=\"solid\" style:text-line-through-type=\"single\"");
      }

      dmf_character_emphasis = dmf[DMF_SFCH_INLINE_CHANGE_EMPHASIS_OFFSET];
     }

     //color
     if((dmf[DMF_SFCH_INLINE_CHANGE_COLOR_OFFSET] | dmf[DMF_SFCH_INLINE_CHANGE_COLOR_OFFSET+1]<<16)!=BLACK) {
      add_string_to_byte_stream(file_content_xml_byte_stream, " fo:color=\"#");
      add_hex_number_to_byte_stream(file_content_xml_byte_stream, (dmf[DMF_SFCH_INLINE_CHANGE_COLOR_OFFSET] | dmf[DMF_SFCH_INLINE_CHANGE_COLOR_OFFSET+1]<<16), 6);
      add_string_to_byte_stream(file_content_xml_byte_stream, "\"");
      dmf_character_color = (dmf[DMF_SFCH_INLINE_CHANGE_COLOR_OFFSET] | dmf[DMF_SFCH_INLINE_CHANGE_COLOR_OFFSET+1]<<16);
     }

     //background color
     if((dmf[DMF_SFCH_INLINE_CHANGE_BACKGROUND_COLOR_OFFSET] | dmf[DMF_SFCH_INLINE_CHANGE_BACKGROUND_COLOR_OFFSET+1]<<16)!=TRANSPARENT_COLOR) {
      add_string_to_byte_stream(file_content_xml_byte_stream, " fo:background-color=\"#");
      add_hex_number_to_byte_stream(file_content_xml_byte_stream, (dmf[DMF_SFCH_INLINE_CHANGE_BACKGROUND_COLOR_OFFSET] | dmf[DMF_SFCH_INLINE_CHANGE_BACKGROUND_COLOR_OFFSET+1]<<16), 6);
      add_string_to_byte_stream(file_content_xml_byte_stream, "\"");
      dmf_character_background_color = (dmf[DMF_SFCH_INLINE_CHANGE_BACKGROUND_COLOR_OFFSET] | dmf[DMF_SFCH_INLINE_CHANGE_BACKGROUND_COLOR_OFFSET+1]<<16);
     }

    add_string_to_byte_stream(file_content_xml_byte_stream, "/>");

    //close paragraph style
    add_string_to_byte_stream(file_content_xml_byte_stream, "</style:style>");
   }
   else if(dmf_is_section_format_change_only_span_change((dword_t)dmf)==STATUS_TRUE) { //style of text
    //add style of text
    add_string_to_byte_stream(file_content_xml_byte_stream, "<style:style style:name=\"T");
    add_number_to_byte_stream(file_content_xml_byte_stream, number_of_text_style);
    add_string_to_byte_stream(file_content_xml_byte_stream, "\" style:family=\"text\">");
    number_of_text_style++;

    //text-properties
    add_string_to_byte_stream(file_content_xml_byte_stream, "<style:text-properties");
    
     //size
     if(dmf_character_size!=dmf[DMF_SFCH_INLINE_CHANGE_SIZE_OFFSET]) {
      add_string_to_byte_stream(file_content_xml_byte_stream, " fo:font-size=\"");
      add_number_to_byte_stream(file_content_xml_byte_stream, dmf[DMF_SFCH_INLINE_CHANGE_SIZE_OFFSET]*96/72); //convert to pt
      add_string_to_byte_stream(file_content_xml_byte_stream, "pt\"");
     }

     //emphasis
     if((dmf[DMF_SFCH_INLINE_CHANGE_EMPHASIS_OFFSET] & SF_EMPHASIS_BOLD)!=(dmf_character_emphasis & SF_EMPHASIS_BOLD)) {
      add_string_to_byte_stream(file_content_xml_byte_stream, " fo:font-weight=\"");
      if((dmf[DMF_SFCH_INLINE_CHANGE_EMPHASIS_OFFSET] & SF_EMPHASIS_BOLD)==SF_EMPHASIS_BOLD) {
       add_string_to_byte_stream(file_content_xml_byte_stream, "bold");
      }
      else {
       add_string_to_byte_stream(file_content_xml_byte_stream, "none");
      }
      add_string_to_byte_stream(file_content_xml_byte_stream, "\"");
     }
     
     if((dmf[DMF_SFCH_INLINE_CHANGE_EMPHASIS_OFFSET] & SF_EMPHASIS_UNDERLINE)!=(dmf_character_emphasis & SF_EMPHASIS_UNDERLINE)) {
      add_string_to_byte_stream(file_content_xml_byte_stream, " style:text-underline-style=\"");
      if((dmf[DMF_SFCH_INLINE_CHANGE_EMPHASIS_OFFSET] & SF_EMPHASIS_UNDERLINE)==SF_EMPHASIS_UNDERLINE) {
       add_string_to_byte_stream(file_content_xml_byte_stream, "solid\" style:text-underline-width=\"auto");
      }
      else {
       add_string_to_byte_stream(file_content_xml_byte_stream, "none");
      }
      add_string_to_byte_stream(file_content_xml_byte_stream, "\"");
     }

     if((dmf[DMF_SFCH_INLINE_CHANGE_EMPHASIS_OFFSET] & SF_EMPHASIS_STRIKE)!=(dmf_character_emphasis & SF_EMPHASIS_STRIKE)) {
      add_string_to_byte_stream(file_content_xml_byte_stream, " style:text-line-through-style=\"");
      if((dmf[DMF_SFCH_INLINE_CHANGE_EMPHASIS_OFFSET] & SF_EMPHASIS_STRIKE)==SF_EMPHASIS_STRIKE) {
       add_string_to_byte_stream(file_content_xml_byte_stream, "solid\" style:text-line-through-type=\"single");
      }
      else {
       add_string_to_byte_stream(file_content_xml_byte_stream, "none");
      }
      add_string_to_byte_stream(file_content_xml_byte_stream, "\"");
     }

     //color
     if(dmf_character_color!=(dmf[DMF_SFCH_INLINE_CHANGE_COLOR_OFFSET] | dmf[DMF_SFCH_INLINE_CHANGE_COLOR_OFFSET+1]<<16)) {
      add_string_to_byte_stream(file_content_xml_byte_stream, " fo:color=\"#");
      add_hex_number_to_byte_stream(file_content_xml_byte_stream, (dmf[DMF_SFCH_INLINE_CHANGE_COLOR_OFFSET] | dmf[DMF_SFCH_INLINE_CHANGE_COLOR_OFFSET+1]<<16), 6);
      add_string_to_byte_stream(file_content_xml_byte_stream, "\"");
     }

     //background color
     if((dmf[DMF_SFCH_INLINE_CHANGE_BACKGROUND_COLOR_OFFSET] | dmf[DMF_SFCH_INLINE_CHANGE_BACKGROUND_COLOR_OFFSET+1]<<16)!=TRANSPARENT_COLOR) {
      add_string_to_byte_stream(file_content_xml_byte_stream, " fo:background-color=\"#");
      add_hex_number_to_byte_stream(file_content_xml_byte_stream, (dmf[DMF_SFCH_INLINE_CHANGE_BACKGROUND_COLOR_OFFSET] | dmf[DMF_SFCH_INLINE_CHANGE_BACKGROUND_COLOR_OFFSET+1]<<16), 6);
      add_string_to_byte_stream(file_content_xml_byte_stream, "\"");
     }
     
    add_string_to_byte_stream(file_content_xml_byte_stream, "/>");

    //close style of text
    add_string_to_byte_stream(file_content_xml_byte_stream, "</style:style>");
   }

   //skip
   dmf = (word_t *) (((dword_t)dmf)+DMF_SFCH_ENTRY_LENGTH_IN_BYTES);
  }
  else {
   //skip
   dmf++;
  }
 }
 add_string_to_byte_stream(file_content_xml_byte_stream, "</office:automatic-styles>");

 //convert all document data
 dmf = (word_t *) (dmf_memory);
 byte_t inside_of_paragraph_tag = STATUS_FALSE, inside_of_span_tag = STATUS_FALSE;
 number_of_paragraph_style = 1;
 number_of_text_style = 1;

 add_string_to_byte_stream(file_content_xml_byte_stream, "<office:body><office:text>");
 while(*dmf!=0) {
  if(*dmf==DMF_SECTION_FORMAT_CHANGE_SIGNATURE) {
   //process new paragraph
   if((dmf[DMF_SFCH_DESCRIPTION_OFFSET] & DMF_SFCH_DESCRIPTION_NEW_PARAGRAPH)==DMF_SFCH_DESCRIPTION_NEW_PARAGRAPH) {
    //close all previous tags
    if(inside_of_span_tag==STATUS_TRUE) {
     add_string_to_byte_stream(file_content_xml_byte_stream, "</text:span>");
     inside_of_span_tag = STATUS_FALSE;
    }
    if(inside_of_paragraph_tag==STATUS_TRUE) {
     add_string_to_byte_stream(file_content_xml_byte_stream, "</text:p>");
     inside_of_paragraph_tag = STATUS_FALSE;
    }

    //add paragraph tag
    add_string_to_byte_stream(file_content_xml_byte_stream, "<text:p text:style-name=\"");
    if(dmf_is_section_standard_style((dword_t)dmf)==STATUS_TRUE) {
     add_string_to_byte_stream(file_content_xml_byte_stream, "Standard");
    }
    else {
     add_string_to_byte_stream(file_content_xml_byte_stream, "P");
     add_number_to_byte_stream(file_content_xml_byte_stream, number_of_paragraph_style);
     number_of_paragraph_style++;
    }
    add_string_to_byte_stream(file_content_xml_byte_stream, "\">");

    inside_of_paragraph_tag = STATUS_TRUE;
   }
   else if(dmf_is_section_format_change_only_span_change((dword_t)dmf)==STATUS_TRUE) { //process new span
    //close previous span tag
    if(inside_of_span_tag==STATUS_TRUE) {
     add_string_to_byte_stream(file_content_xml_byte_stream, "</text:span>");
     inside_of_span_tag = STATUS_FALSE;
    }

    //add span tag
    add_string_to_byte_stream(file_content_xml_byte_stream, "<text:span text:style-name=\"T");
    add_number_to_byte_stream(file_content_xml_byte_stream, number_of_text_style);
    add_string_to_byte_stream(file_content_xml_byte_stream, "\">");
    number_of_text_style++;

    inside_of_span_tag = STATUS_TRUE;
   }

   //skip
   dmf = (word_t *) (((dword_t)dmf)+DMF_SFCH_ENTRY_LENGTH_IN_BYTES);
  }
  else if(*dmf==0xA) { //line break
   add_string_to_byte_stream(file_content_xml_byte_stream, "<text:line-break/>");

   //skip
   dmf++;
  }
  else {
   //convert Unicode char to UTF-8 and write it to stream
   if(*dmf<0x80) {
    add_byte_to_byte_stream(file_content_xml_byte_stream, *dmf);
   }
   else if(*dmf<0x800) {
    add_byte_to_byte_stream(file_content_xml_byte_stream, (((*dmf>>6) & 0x1F) | 0xC0));
    add_byte_to_byte_stream(file_content_xml_byte_stream, ((*dmf & 0x3F) | 0x80));
   }
   else {
    add_byte_to_byte_stream(file_content_xml_byte_stream, (((*dmf>>12) & 0x0F) | 0xE0));
    add_byte_to_byte_stream(file_content_xml_byte_stream, (((*dmf>>6) & 0x1F) | 0x80));
    add_byte_to_byte_stream(file_content_xml_byte_stream, ((*dmf & 0x3F) | 0x80));
   }

   //skip
   dmf++;
  }
 }
 if(inside_of_span_tag==STATUS_TRUE) {
  add_string_to_byte_stream(file_content_xml_byte_stream, "</text:span>");
 }
 if(inside_of_paragraph_tag==STATUS_TRUE) {
  add_string_to_byte_stream(file_content_xml_byte_stream, "</text:p>");
 }

 //end document data
 add_string_to_byte_stream(file_content_xml_byte_stream, "</office:text></office:body></office:document-content>");

 //create ZIP file from all these files
 new_odt_file_memory = create_zip_file(3, file_manifest_xml_byte_stream->size_of_stream+file_mimetype_byte_stream->size_of_stream+file_content_xml_byte_stream->size_of_stream);
 zip_add_file("META-INF/manifest.xml", file_manifest_xml_byte_stream->start_of_allocated_memory, file_manifest_xml_byte_stream->size_of_stream);
 zip_add_file("mimetype", file_mimetype_byte_stream->start_of_allocated_memory, file_mimetype_byte_stream->size_of_stream);
 zip_add_file("content.xml", file_content_xml_byte_stream->start_of_allocated_memory, file_content_xml_byte_stream->size_of_stream);
 zip_add_central_directory_file_header("META-INF/manifest.xml", file_manifest_xml_byte_stream->start_of_allocated_memory, file_manifest_xml_byte_stream->size_of_stream);
 zip_add_central_directory_file_header("mimetype", file_mimetype_byte_stream->start_of_allocated_memory, file_mimetype_byte_stream->size_of_stream);
 zip_add_central_directory_file_header("content.xml", file_content_xml_byte_stream->start_of_allocated_memory, file_content_xml_byte_stream->size_of_stream);
 new_odt_file_size = new_zip_file_size;

 //free all memory
 close_byte_stream(file_manifest_xml_byte_stream);
 close_byte_stream(file_mimetype_byte_stream);
 close_byte_stream(file_content_xml_byte_stream);
}