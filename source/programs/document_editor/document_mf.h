//BleskOS

/*
* MIT License
* Copyright (c) 2023-2025 Vendelín Slezák
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#define DMF_SECTION_FORMAT_CHANGE_SIGNATURE 0xFFFF
#define DMF_SFCH_ENTRY_LENGTH_IN_BYTES 24*2
#define DMF_SFCH_SIGNATURE_OFFSET_1 0
#define DMF_SFCH_LENGTH_OFFSET_1 1
#define DMF_SFCH_LENGTH_OFFSET_2 22
#define DMF_SFCH_SIGNATURE_OFFSET_2 23

#define DMF_SFCH_DESCRIPTION_OFFSET 2
 #define DMF_SFCH_DESCRIPTION_PAGE_BREAK 0x1
 #define DMF_SFCH_DESCRIPTION_NEW_PAGE 0x2
 #define DMF_SFCH_DESCRIPTION_NEW_PARAGRAPH 0x4

#define DMF_SFCH_NEW_PAGE_WIDTH_OFFSET 3
#define DMF_SFCH_NEW_PAGE_HEIGHT_OFFSET 4
#define DMF_SFCH_NEW_PAGE_TOP_BORDER_OFFSET 5
#define DMF_SFCH_NEW_PAGE_BOTTOM_BORDER_OFFSET 6
#define DMF_SFCH_NEW_PAGE_LEFT_BORDER_OFFSET 7
#define DMF_SFCH_NEW_PAGE_RIGHT_BORDER_OFFSET 8

#define DMF_SFCH_PARAGRAPH_DESCRIPTION_OFFSET 9
 #define DMF_SFCH_PARAGRAPH_DESCRIPTION_LIST_ENTRY 0x1
 #define DMF_SFCH_PARAGRAPH_DESCRIPTION_ALIGNMENT_SHIFT 1
  #define DMF_SFCH_PARAGRAPH_DESCRIPTION_ALIGNMENT_LEFT 0b00
  #define DMF_SFCH_PARAGRAPH_DESCRIPTION_ALIGNMENT_CENTER 0b01
  #define DMF_SFCH_PARAGRAPH_DESCRIPTION_ALIGNMENT_RIGHT 0b10
#define DMF_SFCH_PARAGRAPH_TOP_BORDER_OFFSET 10
#define DMF_SFCH_PARAGRAPH_BOTTOM_BORDER_OFFSET 11
#define DMF_SFCH_PARAGRAPH_LEFT_BORDER_OFFSET 12
#define DMF_SFCH_PARAGRAPH_RIGHT_BORDER_OFFSET 13
#define DMF_SFCH_PARAGRAPH_LIST_LEVEL_OFFSET 14
#define DMF_SFCH_PARAGRAPH_LIST_ENTRY_OFFSET 15
 #define DMF_SFCH_PARAGRAPH_LIST_ENTRY_DOT 0xFFFF

#define DMF_SFCH_INLINE_CHANGE_SIZE_OFFSET 16
#define DMF_SFCH_INLINE_CHANGE_EMPHASIS_OFFSET 17
#define DMF_SFCH_INLINE_CHANGE_COLOR_OFFSET 18
#define DMF_SFCH_INLINE_CHANGE_BACKGROUND_COLOR_OFFSET 20

#define DMF_LIST_ENTRY_LEFT_MARGIN 45

dword_t dmf_page_width = 0, dmf_page_height = 0, dmf_page_top_border = 0, dmf_page_bottom_border = 0, dmf_page_left_border = 0, dmf_page_right_border = 0;
dword_t dmf_page_actual_left_border = 0, dmf_page_actual_right_border = 0;
dword_t dmf_actual_paragraph_description = 0, dmf_paragraph_top_border = 0, dmf_paragraph_bottom_border = 0, dmf_paragraph_left_border = 0, dmf_paragraph_right_border = 0, dmf_paragraph_alignment = 0, dmf_paragaph_list_entry = 0;
dword_t dmf_character_size = 0, dmf_character_spacing = 0, dmf_bottom_line_of_characters = 0, dmf_character_emphasis = 0, dmf_character_color = 0, dmf_character_background_color = 0;

dword_t dmf_number_of_chars_in_document = 0;

byte_t dmf_page_break = STATUS_FALSE;

void convert_dmf_to_dllmf(dword_t dmf_memory, dword_t dllmf_memory);
dword_t dmf_get_first_column_of_line(dword_t dmf_memory, dword_t alignment, dword_t actual_char_size);
dword_t dmf_get_biggest_char_size_of_line(dword_t dmf_memory, dword_t actual_char_size);
dword_t dmf_get_size_of_word(dword_t dmf_memory, dword_t actual_char_size);
void dmf_add_section_format_change(dword_t memory, dword_t size, dword_t emphasis, dword_t color, dword_t background_color);
void dmf_add_section_page_break(dword_t memory);
void dmf_add_section_new_page(dword_t memory, dword_t width, dword_t height, dword_t top, dword_t bottom, dword_t left, dword_t right);
void dmf_add_section_new_paragraph(dword_t memory, dword_t description, dword_t top, dword_t bottom, dword_t left, dword_t right, dword_t list_level, dword_t list_entry);
byte_t dmf_is_section_format_change_only_span_change(dword_t memory);
byte_t dmf_is_section_with_new_paragraph(dword_t memory);
byte_t dmf_is_section_standard_style(dword_t memory);