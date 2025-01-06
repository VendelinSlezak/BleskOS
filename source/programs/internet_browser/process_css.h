//BleskOS

/*
* MIT License
* Copyright (c) 2023-2025 Vendelín Slezák
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

dword_t css_margin_top, css_margin_bottom, css_margin_left, css_margin_right;
dword_t css_top_border_type, css_top_border_size, css_top_border_color;
dword_t css_bottom_border_type, css_bottom_border_size, css_bottom_border_color;
dword_t css_left_border_type, css_left_border_size, css_left_border_color;
dword_t css_right_border_type, css_right_border_size, css_right_border_color;
dword_t css_padding_top, css_padding_bottom, css_padding_left, css_padding_right;

dword_t css_background_color;

dword_t css_font_size, css_font_color, css_font_alignment, css_font_emphasis, css_font_spacing;

dword_t css_width, css_height, css_left_auto_margin, css_right_auto_margin;

word_t *css;

dword_t css_entries_tag_list_mem, css_entries_tag_list_pointer, css_entries_id_list_mem, css_entries_id_list_pointer, css_entries_class_list_mem, css_entries_class_list_pointer, *css_entries_list;

void css_parse_file(void);
void css_parse_declaration_block(void);
void css_parse_declaration(void);
dword_t css_read_number_value(dword_t percent_base);
dword_t css_read_color_value(void);
void css_skip_spaces(void);
void css_skip_property(void);
void css_skip_text(void);
void css_skip_after_char(word_t value);
byte_t css_compare_property_string(byte_t *string);
byte_t css_compare_string(byte_t *string);
