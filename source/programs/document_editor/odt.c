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
 dword_t content_xml_file_memory = zip_extract_file(odt_memory, odt_size, content_xml_file_number);
 if(content_xml_file_memory==STATUS_ERROR) {
  log("\nODT: error with extracting content.xml");
  return STATUS_ERROR;
 }
 content_xml_file_memory = realloc(content_xml_file_memory, zip_extracted_file_size+2); //add zero to end

 //TODO: support for more coding than UTF-8
 dword_t content_xml_unicode_memory = calloc(zip_extracted_file_size*2);
 convert_utf_8_to_unicode(content_xml_file_memory, content_xml_unicode_memory, zip_extracted_file_size);
 word_t *content_xml = (word_t *) (content_xml_unicode_memory);

 //count how many memory we will need for DMF
 dword_t dmf_size = DMF_SFCH_ENTRY_LENGTH_IN_BYTES;
 dmf_number_of_chars_in_document = 0;
 while(*content_xml!=0) {
  if(*content_xml=='<') { //tag
   content_xml++;

   if(are_equal_b_string_w_string("text:p", content_xml)==STATUS_TRUE) {
    dmf_size+=DMF_SFCH_ENTRY_LENGTH_IN_BYTES;
   }
   else if(are_equal_b_string_w_string("text:line-break", content_xml)==STATUS_TRUE) {
    dmf_size+=2;
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
 content_xml = (word_t *) (content_xml_unicode_memory);
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
 dword_t count = 0;
 while(*content_xml!=0) {
  if(*content_xml=='<') { //tag
   content_xml++;

   if(are_equal_b_string_w_string("text:p", content_xml)==STATUS_TRUE) {
    dmf_add_section_format_change(((dword_t)dmf), DMF_SFCH_DESCRIPTION_NEW_PARAGRAPH, 10, DMF_SFCH_ENTRY_NO_CHANGE, DMF_SFCH_COLOR_NO_CHANGE, DMF_SFCH_COLOR_NO_CHANGE);
    dmf_add_section_paragraph_change(((dword_t)dmf), 0, 0, 0, 0, 0, 0);
    dmf = (word_t *) (((dword_t)dmf)+DMF_SFCH_ENTRY_LENGTH_IN_BYTES);
   }
   else if(are_equal_b_string_w_string("text:line-break", content_xml)==STATUS_TRUE) {
    *dmf = 0xA;
    dmf++;
   }

   //skip tag
   while(content_xml[0]!='>' && content_xml[1]!=0) {
    content_xml++;
   }
   content_xml++;
  }
  else { //char
   if(*content_xml>=32) { //skip unprintable characters
    *dmf = *content_xml;
    dmf++;
    count++;
   }
   content_xml++;
  }
 }

 return dmf_memory;
}