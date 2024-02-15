//BleskOS

/*
* MIT License
* Copyright (c) 2023-2024 Vendelín Slezák
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include "document_llmf.h"
#include "document_mf.h"
#include "odt.h"

#define DOCUMENT_EDITOR_FILE_DMF_MEMORY (PROGRAM_INTERFACE_FILE_PROGRAM_AREA+0)
#define DOCUMENT_EDITOR_FILE_DLLMF_MEMORY (PROGRAM_INTERFACE_FILE_PROGRAM_AREA+1)
#define DOCUMENT_EDITOR_FILE_DOCUMENT_HEIGHT (PROGRAM_INTERFACE_FILE_PROGRAM_AREA+2)
#define DOCUMENT_EDITOR_FILE_FIRST_SHOW_LINE (PROGRAM_INTERFACE_FILE_PROGRAM_AREA+3)
#define DOCUMENT_EDITOR_FILE_FIRST_SHOW_COLUMN (PROGRAM_INTERFACE_FILE_PROGRAM_AREA+4)
#define DOCUMENT_EDITOR_FILE_VERTICAL_SCROLLBAR_RIDER_POSITION (PROGRAM_INTERFACE_FILE_PROGRAM_AREA+5)
#define DOCUMENT_EDITOR_FILE_VERTICAL_SCROLLBAR_RIDER_SIZE (PROGRAM_INTERFACE_FILE_PROGRAM_AREA+6)

#define DOCUMENT_EDITOR_CLICK_ZONE_VERTICAL_SCROLLBAR 1000

dword_t document_editor_program_interface_memory = 0;
dword_t document_editor_vertical_scrollbar_height = 0;

void initalize_document_editor(void);
void document_editor(void);
void draw_document_editor(void);
void document_editor_open_file(void);
void document_editor_close_file(void);
void document_editor_key_up_event(void);
void document_editor_key_down_event(void);
void document_editor_key_page_up_event(void);
void document_editor_key_page_down_event(void);
void document_editor_key_home_event(void);
void document_editor_key_end_event(void);
void document_editor_vertical_scrollbar_event(void);
void document_editor_recalculate_scrollbars(void);