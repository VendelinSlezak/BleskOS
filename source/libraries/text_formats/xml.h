//BleskOS

/*
* MIT License
* Copyright (c) 2023-2024 Vendelín Slezák
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#define XML_NO_SPECIAL_ATTRIBUTES 0
#define XML_EAT_SPACES 0x1

word_t *xml_tag_content_memory;
word_t *xml_tag_attribute_content_memory;

dword_t prepare_xml_file(dword_t raw_xml_memory, dword_t raw_xml_size, dword_t special_attributes);
word_t xml_get_escape_sequence_character(word_t *xml_mem);
byte_t xml_is_tag(word_t *xml_mem, byte_t *tag_name);
byte_t xml_does_tag_have_pair(word_t *xml_mem);
byte_t xml_find_tag_attribute(byte_t *attribute_name);
byte_t xml_is_attribute(byte_t *attribute_content);
byte_t xml_compare_attribute_and_attribute(word_t *attribute_1, word_t *attribute_2);
dword_t xml_get_attribute_number(void);
dword_t xml_get_attribute_number_in_px(void);
dword_t xml_get_attribute_hex_number(void);
dword_t xml_get_attribute_color_number(void);