//BleskOS

/*
* MIT License
* Copyright (c) 2023-2025 Vendelín Slezák
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

dword_t prepare_xml_file(dword_t raw_xml_memory, dword_t raw_xml_size, dword_t special_attributes) {
 //convert file to unicode
 //TODO: support for more encodings than only UTF-8
 dword_t unicode_xml_memory = calloc(raw_xml_size*2+2);
 convert_utf_8_to_unicode(raw_xml_memory, unicode_xml_memory, raw_xml_size);

 //allocate memory for result file
 dword_t xml_memory = calloc(raw_xml_size*2+2);

 //convert file
 word_t *unicode_xml = (word_t *) (unicode_xml_memory);
 word_t *xml = (word_t *) (xml_memory);
 dword_t is_space_sequence = STATUS_FALSE;
 while(*unicode_xml!=0) {
  if(*unicode_xml=='<') { //tag
   if(are_equal_b_string_w_string("<!--", unicode_xml)==STATUS_TRUE) { //comment
    *unicode_xml+=4;

    //skip comment
    while(*unicode_xml!=0) {
     if(are_equal_b_string_w_string("-->", unicode_xml)==STATUS_TRUE) { //end of comment
      *unicode_xml+=3;
      break;
     }

     unicode_xml++;
    }
    continue;
   }
   else { //copy tag
    //copy <
    *xml = '<';
    xml++;
    unicode_xml++;
    
    //copy rest of tag
    while(unicode_xml[-1]!='>') {
     if(*unicode_xml==0) {
      free(unicode_xml_memory);
      return xml_memory;
     }
     *xml = *unicode_xml;
     xml++;
     unicode_xml++;
    }
   }
  }
  else if(special_attributes==XML_EAT_SPACES && *unicode_xml==' ') { 
   unicode_xml++;
   if(is_space_sequence==STATUS_TRUE) { //we need to eat more spaces afterward
    continue;
   }

   *xml = ' ';
   xml++;
   is_space_sequence = STATUS_TRUE;
  }
  else if((*unicode_xml>' ' && *unicode_xml<512) || (special_attributes!=XML_EAT_SPACES && *unicode_xml==' ')) { //printable characters
   *xml = *unicode_xml;
   xml++;
   unicode_xml++;
   is_space_sequence = STATUS_FALSE;
  }
  else { //skip unprintable characters
   unicode_xml++;
  }
 }

 free(unicode_xml_memory);
 return xml_memory;
}

word_t xml_get_escape_sequence_character(word_t *xml_mem) {
 //*xml_mem = &
 xml_mem++;
 if(are_equal_b_string_w_string("amp;", xml_mem)==STATUS_TRUE) {
  return '&';
 }
 else if(are_equal_b_string_w_string("lt;", xml_mem)==STATUS_TRUE) {
  return '<';
 }
 else if(are_equal_b_string_w_string("gt;", xml_mem)==STATUS_TRUE) {
  return '>';
 }
 else if(are_equal_b_string_w_string("qout;", xml_mem)==STATUS_TRUE) {
  return '"';
 }
 else if(are_equal_b_string_w_string("apos;", xml_mem)==STATUS_TRUE) {
  return '\'';
 }
 else { //unknown escape sequence
  return ' ';
 }
}

byte_t xml_is_tag(word_t *xml_mem, byte_t *tag_name) {
 //*xml_mem points to <
 xml_mem++;

 //compare
 word_t *xml_mem_2 = xml_mem;
 while(*tag_name!=0) {
  if(*tag_name!=*xml_mem_2) {
   return STATUS_FALSE;
  }

  tag_name++;
  xml_mem_2++;
 }
 if(*xml_mem_2!=' ' && *xml_mem_2!='>' && *xml_mem_2!='/') {
  return STATUS_FALSE;
 }

 //find tag content
 while(*xml_mem>' ' && *xml_mem!='>' && *xml_mem!=SF_NBSP) {
  xml_mem++;
 }
 xml_tag_content_memory = xml_mem;
 return STATUS_TRUE;
}

byte_t xml_does_tag_have_pair(word_t *xml_mem) {
 //*xml_mem points to <
 while(xml_mem[0]!='>' && xml_mem[1]!=0) {
  xml_mem++;
 }
 if(xml_mem[-1]=='/' || (xml_mem[-1]==' ' && xml_mem[-2]=='/')) {
  return STATUS_FALSE;
 }
 else {
  return STATUS_TRUE;
 }
}

byte_t xml_find_tag_attribute(byte_t *attribute_name) {
 //this method can be used only after xml_is_tag()==STATUS_TRUE  or with correct value in xml_tag_content_memory
 word_t *xml_tag_content = (word_t *) (xml_tag_content_memory);

 while(*xml_tag_content!='>' && *xml_tag_content!=0) {
  if(*xml_tag_content!=' ' && *xml_tag_content!=SF_NBSP) { //attribute
   if(are_equal_b_string_w_string(attribute_name, xml_tag_content)==STATUS_TRUE) { //we find attribute
    //find attribute content
    while(*xml_tag_content!='"') {
    if(*xml_tag_content=='>' || *xml_tag_content==0) {
     xml_tag_attribute_content_memory = (word_t *) 0; //no content
     return STATUS_TRUE;
    }
     xml_tag_content++;
    }
    xml_tag_content++; //skip "
    xml_tag_attribute_content_memory = xml_tag_content;
    return STATUS_TRUE;
   }

   //skip attribute
   while(*xml_tag_content!='"') {
    if(*xml_tag_content=='>' || *xml_tag_content==0) {
     return STATUS_FALSE;
    }
    xml_tag_content++;
   }
   xml_tag_content++; //skip "
   while(*xml_tag_content!='"') {
    if(*xml_tag_content=='>' || *xml_tag_content==0) {
     return STATUS_FALSE;
    }
    xml_tag_content++;
   }
   xml_tag_content++; //skip "
  }
  else { //skip spaces
   xml_tag_content++;
  }
 }
 return STATUS_FALSE;
}

byte_t xml_is_attribute(byte_t *attribute_content) {
 //this method can be used only after xml_find_tag_attribute()==STATUS_TRUE or with correct value in xml_tag_attribute_content_memory
 return are_equal_b_string_w_string(attribute_content, xml_tag_attribute_content_memory);
}

byte_t xml_compare_attribute_and_attribute(word_t *attribute_1, word_t *attribute_2) {
 while(*attribute_1!='"' && *attribute_1!=0 && *attribute_2!='"' && *attribute_2!=0) {
  if(*attribute_1!=*attribute_2) {
   return STATUS_FALSE;
  }

  attribute_1++;
  attribute_2++;
 }
 if(*attribute_1==*attribute_2) {
  return STATUS_TRUE;
 }
 else {
  return STATUS_FALSE;
 }
}

dword_t xml_get_attribute_number(void) {
 //this method can be used only after xml_find_tag_attribute()==STATUS_TRUE or with correct value in xml_tag_attribute_content_memory
 return convert_word_string_to_float_number((dword_t)xml_tag_attribute_content_memory);
}

dword_t xml_get_attribute_number_in_px(void) {
 //this method can be used only after xml_find_tag_attribute()==STATUS_TRUE or with correct value in xml_tag_attribute_content_memory
 float number = convert_word_string_to_float_number((dword_t)xml_tag_attribute_content_memory);

 //skip number
 word_t *xml_tag_attribute_content = (word_t *) (xml_tag_attribute_content_memory);
 while(is_number(*xml_tag_attribute_content)==STATUS_TRUE || *xml_tag_attribute_content=='.') {
  xml_tag_attribute_content++;
 }

 //recalculate number to px
 if(xml_tag_attribute_content[0]=='p' && xml_tag_attribute_content[1]=='t') {
  number = (72*number/96);
 }
 else if(xml_tag_attribute_content[0]=='i' && xml_tag_attribute_content[1]=='n') {
  number *= 96;
 }
 else if(xml_tag_attribute_content[0]=='c' && xml_tag_attribute_content[1]=='m') {
  number = (number*378/10);
 }
 //TODO: more

 //round result
 if((number-((dword_t)number))<0.5) {
  return ((dword_t)number);
 }
 else {
  return (((dword_t)number)+1);
 }
}

dword_t xml_get_attribute_hex_number(void) {
 word_t *xml_tag_attribute_content = (word_t *) (xml_tag_attribute_content_memory);
 if(*xml_tag_attribute_content=='#') {
  xml_tag_attribute_content++;
 }
 dword_t hex_number = 0;
 for(dword_t i=0; i<8; i++) {
  if(is_hex_number(*xml_tag_attribute_content)==STATUS_FALSE) {
   break;
  }

  hex_number <<= 4;
  if(is_number(*xml_tag_attribute_content)==STATUS_TRUE) {
   hex_number |= (*xml_tag_attribute_content-'0');
  }
  else {
   *xml_tag_attribute_content = get_big_char_value(*xml_tag_attribute_content); //convert all chars to big char
   hex_number |= (*xml_tag_attribute_content-'A'+10);
  }
  xml_tag_attribute_content++;
 }
 return hex_number;
}

dword_t xml_get_attribute_color_number(void) {
 extern dword_t xml_color_numbers_string_array[17*2];

 //scan array
 for(dword_t i=0; i<17; i++) {
  if(xml_is_attribute((byte_t *)(xml_color_numbers_string_array[i*2]))==STATUS_TRUE) {
   return xml_color_numbers_string_array[i*2+1];
  }
 }

 return xml_get_attribute_hex_number();

 if(xml_is_attribute("auto")==STATUS_TRUE || xml_is_attribute("black")==STATUS_TRUE) {
  return BLACK;
 }
 else if(xml_is_attribute("white")==STATUS_TRUE) {
  return WHITE;
 }
 else if(xml_is_attribute("red")==STATUS_TRUE) {
  return RED;
 }
 else if(xml_is_attribute("green")==STATUS_TRUE) {
  return 0x00FF00;
 }
 else if(xml_is_attribute("blue")==STATUS_TRUE) {
  return 0x0000FF;
 }
 else if(xml_is_attribute("yellow")==STATUS_TRUE) {
  return 0xFFFF00;
 }
 else if(xml_is_attribute("magneta")==STATUS_TRUE) {
  return 0xFF00FF;
 }
 else if(xml_is_attribute("cyan")==STATUS_TRUE) {
  return 0x00FFFF;
 }
 else if(xml_is_attribute("lightGrey")==STATUS_TRUE) {
  return 0xD3D3D3;
 }
 else if(xml_is_attribute("darkGrey")==STATUS_TRUE) {
  return 0xA9A9A9;
 }
 else if(xml_is_attribute("darkRed")==STATUS_TRUE) {
  return 0x8B0000;
 }
 else if(xml_is_attribute("darkGreen")==STATUS_TRUE) {
  return 0x006400;
 }
 else if(xml_is_attribute("darkBlue")==STATUS_TRUE) {
  return 0x00008B;
 }
 else if(xml_is_attribute("darkYellow")==STATUS_TRUE) {
  return 0xF6BE00;
 }
 else if(xml_is_attribute("darkMagneta")==STATUS_TRUE) {
  return 0x8B008B;
 }
 else if(xml_is_attribute("darkCyan")==STATUS_TRUE) {
  return 0x008B8B;
 }
 else {
  return xml_get_attribute_hex_number();
 }
}