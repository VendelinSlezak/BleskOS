//BleskOS

/*
* MIT License
* Copyright (c) 2023-2025 Vendelín Slezák
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#define HTML_TAG_CSS_ENTRY_NUM_OF_ENTRIES 16
#define HTML_TAG_CSS_ENTRY_SIZE HTML_TAG_CSS_ENTRY_NUM_OF_ENTRIES*4
#define HTML_TAG_CSS_ENTRY_OFFSET_WHERE_TO_MOVE_AT_END 0
#define HTML_TAG_CSS_ENTRY_OFFSET_WIDTH 1
#define HTML_TAG_CSS_ENTRY_OFFSET_HEIGHT 2
#define HTML_TAG_CSS_ENTRY_OFFSET_DRAW_BOX_MEM 3
#define HTML_TAG_CSS_ENTRY_OFFSET_FONT_SIZE 4
#define HTML_TAG_CSS_ENTRY_OFFSET_FONT_COLOR 5
#define HTML_TAG_CSS_ENTRY_OFFSET_FONT_EMPHASIS 6
#define HTML_TAG_CSS_ENTRY_OFFSET_FONT_ALIGNMENT 7
#define HTML_TAG_CSS_ENTRY_OFFSET_FONT_SPACING 8
#define HTML_TAG_CSS_ENTRY_OFFSET_STARTING_LINE 9
#define HTML_TAG_CSS_ENTRY_OFFSET_STARTING_COLUMN 10
#define HTML_TAG_CSS_ENTRY_OFFSET_PREVIOUS_LINE_WIDTH 11
#define HTML_TAG_CSS_ENTRY_OFFSET_BOTTOM_SKIP_SIZE 12
#define HTML_TAG_CSS_ENTRY_OFFSET_TOP_AND_BOTTOM_SKIP_SIZE 13
#define HTML_TAG_CSS_ENTRY_OFFSET_LEFT_AND_RIGHT_SKIP_SIZE 14
#define HTML_TAG_CSS_ENTRY_OFFSET_HEIGHT_CALCULATION_TYPE 15

#define HTML_TAG_CSS_ENTRY_MOVE_TO_NEW_LINE 1
#define HTML_TAG_CSS_ENTRY_MOVE_TO_NEW_COLUMN 2

#define HTML_TAG_CSS_ENTRY_HEIGHT_CALCULATION_ADD 0
#define HTML_TAG_CSS_ENTRY_HEIGHT_CALCULATION_COMPARE 1

dword_t html_title_memory = 0, webpage_last_line_command_pointer = 0;
dword_t html_tag_css_list_mem = 0, html_tag_css_list_pointer = 0, *html_tag_css_list, *html_tag_css_list_2;
dword_t html_not_pair_tag = 0, html_tag_height_calculation_type = 0, html_num_of_tags_to_skip = 0, html_page_height = 0;
dword_t html_table_cell_width_list_mem = 0, html_table_cell_width_list_pointer = 0, *html_table_cell_width_list, html_table_cell_width = 0, html_table_last_cell_tag_ending = 0;
dword_t html_list_type_mem = 0, html_list_type_pointer = 0, *html_list_type, html_list_tag = 0, html_last_char_is_list_start = 0;
dword_t ib_line_width = 0, ib_actual_line_width = 0, ib_tag_new_line = 0, ib_tag_new_column = 0, ib_line_biggest_spacing = 0;

dword_t url_base_mem, url_input_mem, url_output_mem;
dword_t html_list_of_downloaded_files_mem, html_list_of_downloaded_files_pointer, *html_list_of_downloaded_files;

dword_t convert_html_to_bleskos_webpage(dword_t html_mem, dword_t html_length);
byte_t is_tag_equal(word_t *string1, byte_t *string2);
dword_t find_tag_attribute(dword_t html_mem, byte_t *string);
void convert_relative_url_to_full_url(dword_t url_mem);