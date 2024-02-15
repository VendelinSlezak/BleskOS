//BleskOS

/*
* MIT License
* Copyright (c) 2023-2024 Vendelín Slezák
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#define DLLMF_PAGE_ENTRY_SIZE 8
#define DLLMF_NUM_OF_PAGE_ENTRIES 1000
#define DLLMF_PAGE_ENTRY_WIDTH_OFFSET 0
#define DLLMF_PAGE_ENTRY_HEIGHT_OFFSET 1

#define DLLMF_CHAR_ENTRY_LENGTH_IN_BYTES 20
#define DLLMF_CHAR_ENTRY_CHAR_NUMBER_OFFSET 0
#define DLLMF_CHAR_ENTRY_COLUMN_OFFSET 1
#define DLLMF_CHAR_ENTRY_LINE_OFFSET 2
#define DLLMF_CHAR_ENTRY_COLOR_OFFSET 3
#define DLLMF_CHAR_ENTRY_SIZE_OFFSET 15
#define DLLMF_CHAR_ENTRY_BACKGROUND_COLOR_OFFSET 4
#define DLLMF_CHAR_ENTRY_EMPHASIS_OFFSET 19

#define DLLMF_PAGE_CONTENT_END 0xFFFFFFFF
#define DLLMF_DOCUMENT_CONTENT_END 0x00000000

#define DLLMF_SPACE_BETWEEN_DOCUMENTS 20

dword_t dllmf_screen_first_column = 0, dllmf_screen_first_line = 0;
dword_t dllmf_draw_first_column = 0, dllmf_draw_last_column = 0, dllmf_draw_first_line = 0, dllmf_draw_last_line = 0, dllmf_draw_width = 0, dllmf_draw_height = 0;
dword_t dllmf_square_x = 0, dllmf_square_y = 0, dllmf_square_width = 0, dllmf_square_height = 0, dllmf_square_draw_column = 0, dllmf_square_draw_line = 0;

void draw_dllmf(dword_t dllmf_mem);
void dllmf_calculate_draw_square(dword_t column, dword_t line, dword_t width, dword_t height);
dword_t dllmf_get_document_height(dword_t dllmf_memory);