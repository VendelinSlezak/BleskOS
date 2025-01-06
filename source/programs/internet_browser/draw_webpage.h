//BleskOS

/*
* MIT License
* Copyright (c) 2023-2025 Vendelín Slezák
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#define WEBPAGE_COMMAND_MOVE_DRAW_POSITION 0xFFF0 // 0xFFF0 // column // line
#define WEBPAGE_COMMAND_MOVE_DRAW_POSITION_BYTE_OFFSET_COLUMN 2
#define WEBPAGE_COMMAND_MOVE_DRAW_POSITION_BYTE_OFFSET_LINE 6

#define WEBPAGE_COMMAND_MOVE_TO_LINE 0xFFF1 // 0xFFF1 // column // line / line size
#define WEBPAGE_COMMAND_MOVE_TO_LINE_BYTE_OFFSET_COLUMN 2
#define WEBPAGE_COMMAND_MOVE_TO_LINE_BYTE_OFFSET_LINE 6
#define WEBPAGE_COMMAND_MOVE_TO_LINE_OFFSET_LINE_SIZE 5

#define WEBPAGE_COMMAND_CHANGE_TEXT_SIZE 0xFFF2 // 0xFFF2 / text size
#define WEBPAGE_COMMAND_CHANGE_TEXT_SIZE_OFFSET_TEXT_SIZE 1

#define WEBPAGE_COMMAND_CHANGE_TEXT_EMPHASIS 0xFFF3 // 0xFFF3 / text emphasis
#define WEBPAGE_COMMAND_CHANGE_TEXT_EMPHASIS_OFFSET_TEXT_EMPHASIS 1

#define WEBPAGE_COMMAND_CHANGE_TEXT_COLOR 0xFFF4 // 0xFFF4 // color
#define WEBPAGE_COMMAND_CHANGE_TEXT_COLOR_BYTE_OFFSET_COLOR 2

#define WEBPAGE_COMMAND_DRAW_BOX 0xFFF5 // 0xFFF5 / width / height // background color / top border type / top border size // top border color / bottom border type / bottom border size // bottom border color / left border type / left border size // left border color / right border type / right border size // right border color
#define WEBPAGE_COMMAND_DRAW_BOX_OFFSET_WIDTH 1
#define WEBPAGE_COMMAND_DRAW_BOX_OFFSET_HEIGHT 2
#define WEBPAGE_COMMAND_DRAW_BOX_BYTE_OFFSET_BACKGROUND_COLOR 6
#define WEBPAGE_COMMAND_DRAW_BOX_OFFSET_TOP_BORDER_TYPE 5
#define WEBPAGE_COMMAND_DRAW_BOX_OFFSET_TOP_BORDER_SIZE 6
#define WEBPAGE_COMMAND_DRAW_BOX_BYTE_OFFSET_TOP_BORDER_COLOR 14
#define WEBPAGE_COMMAND_DRAW_BOX_OFFSET_BOTTOM_BORDER_TYPE 9
#define WEBPAGE_COMMAND_DRAW_BOX_OFFSET_BOTTOM_BORDER_SIZE 10
#define WEBPAGE_COMMAND_DRAW_BOX_BYTE_OFFSET_BOTTOM_BORDER_COLOR 22
#define WEBPAGE_COMMAND_DRAW_BOX_OFFSET_LEFT_BORDER_TYPE 13
#define WEBPAGE_COMMAND_DRAW_BOX_OFFSET_LEFT_BORDER_SIZE 14
#define WEBPAGE_COMMAND_DRAW_BOX_BYTE_OFFSET_LEFT_BORDER_COLOR 30
#define WEBPAGE_COMMAND_DRAW_BOX_OFFSET_RIGHT_BORDER_TYPE 17
#define WEBPAGE_COMMAND_DRAW_BOX_OFFSET_RIGHT_BORDER_SIZE 18
#define WEBPAGE_COMMAND_DRAW_BOX_BYTE_OFFSET_RIGHT_BORDER_COLOR 38

#define WEBPAGE_COMMAND_DRAW_BORDER_LINE 0xFFF6 // 0xFFF6 / border type / border width / border height // border color
#define WEBPAGE_COMMAND_DRAW_BORDER_LINE_OFFSET_BORDER_TYPE 1
#define WEBPAGE_COMMAND_DRAW_BORDER_LINE_OFFSET_BORDER_WIDTH 2
#define WEBPAGE_COMMAND_DRAW_BORDER_LINE_OFFSET_BORDER_HEIGHT 3
#define WEBPAGE_COMMAND_DRAW_BORDER_LINE_BYTE_OFFSET_BORDER_COLOR 8

#define WEBPAGE_COMMAND_DRAW_IMAGE 0xFFF7
#define WEBPAGE_COMMAND_DRAW_REPEATING_IMAGE 0xFFF8

#define WEBPAGE_COMMAND_URL 0xFFF9
#define WEBPAGE_COMMAND_URL_BYTE_OFFSET_POINTER 2

#define WEBPAGE_EMPTY_ENTRY_32 0xFFFFFFFF

#define WEBPAGE_BORDER_TYPE_NONE 0
#define WEBPAGE_BORDER_TYPE_SOLID 1

#define WEBPAGE_LEFT_ALIGNMENT 0
#define WEBPAGE_MIDDLE_ALIGNMENT 1
#define WEBPAGE_RIGHT_ALIGNMENT 2

dword_t ib_draw_column, ib_draw_line, ib_line_size;
dword_t ib_font_size, ib_font_color, ib_font_emphasis, ib_font_spacing, ib_font_alignment;
dword_t ib_square_x, ib_square_y, ib_square_width, ib_square_height, ib_square_draw_column, ib_square_draw_line;
dword_t webpage_element_visibility, webpage_url_pointer_mem;

void internet_browser_redraw_webpage(dword_t webpage_mem);
void internet_browser_calculate_draw_square(dword_t column, dword_t line, dword_t width, dword_t height);
