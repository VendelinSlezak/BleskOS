//BleskOS

/*
* MIT License
* Copyright (c) 2023-2025 Vendelín Slezák
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include "document_llmf.h"
#include "document_mf.h"
#include "odt.h"
#include "docx.h"

#define DOCUMENT_EDITOR_FILE_DMF_MEMORY (PROGRAM_INTERFACE_FILE_PROGRAM_AREA+0)
#define DOCUMENT_EDITOR_FILE_DLLMF_MEMORY (PROGRAM_INTERFACE_FILE_PROGRAM_AREA+1)
#define DOCUMENT_EDITOR_FILE_DOCUMENT_HEIGHT (PROGRAM_INTERFACE_FILE_PROGRAM_AREA+2)
#define DOCUMENT_EDITOR_FILE_DOCUMENT_WIDTH (PROGRAM_INTERFACE_FILE_PROGRAM_AREA+3)
#define DOCUMENT_EDITOR_FILE_FIRST_SHOW_LINE (PROGRAM_INTERFACE_FILE_PROGRAM_AREA+4)
#define DOCUMENT_EDITOR_FILE_FIRST_SHOW_COLUMN (PROGRAM_INTERFACE_FILE_PROGRAM_AREA+5)
#define DOCUMENT_EDITOR_FILE_VERTICAL_SCROLLBAR_RIDER_POSITION (PROGRAM_INTERFACE_FILE_PROGRAM_AREA+6)
#define DOCUMENT_EDITOR_FILE_VERTICAL_SCROLLBAR_RIDER_SIZE (PROGRAM_INTERFACE_FILE_PROGRAM_AREA+7)
#define DOCUMENT_EDITOR_FILE_HORIZONTAL_SCROLLBAR_RIDER_POSITION (PROGRAM_INTERFACE_FILE_PROGRAM_AREA+8)
#define DOCUMENT_EDITOR_FILE_HORIZONTAL_SCROLLBAR_RIDER_SIZE (PROGRAM_INTERFACE_FILE_PROGRAM_AREA+9)
#define DOCUMENT_EDITOR_FILE_CURSOR (PROGRAM_INTERFACE_FILE_PROGRAM_AREA+10)
#define DOCUMENT_EDITOR_FILE_SELECTED_AREA (PROGRAM_INTERFACE_FILE_PROGRAM_AREA+11)

#define DOCUMENT_EDITOR_CLICK_ZONE_VERTICAL_SCROLLBAR 1000
#define DOCUMENT_EDITOR_CLICK_ZONE_HORIZONTAL_SCROLLBAR 1001

#define DOCUMENT_EDITOR_NUMBER_OF_ENTRIES_IN_STYLE_STACK 10000
struct document_editor_style_stack_entry {
 dword_t paragraph_alignment;
 dword_t paragraph_left_margin;
 dword_t paragraph_right_margin;
 dword_t paragraph_top_margin;
 dword_t paragraph_bottom_margin;
 dword_t character_size;
 dword_t character_emphasis;
 dword_t character_color;
 dword_t character_background_color;
}__attribute__((packed));
struct document_editor_style_stack_entry *document_editor_style_stack_pointer;
dword_t document_editor_style_stack_number_of_entries = 0;

#define DOCUMENT_EDITOR_NUMBER_OF_ENTRIES_IN_LIST_OF_STYLES 10000
struct document_editor_list_of_styles_entry {
 dword_t memory_of_style_name;
 dword_t memory_of_style_content;
}__attribute__((packed));
struct document_editor_list_of_styles_entry *document_editor_list_of_styles_pointer;
dword_t document_editor_list_of_styles_number_of_entries = 0;

#define DOCUMENT_EDITOR_NO_LIST 0
#define DOCUMENT_EDITOR_LIST_UNORDERED 1
#define DOCUMENT_EDITOR_LIST_ORDERED 2
#define DOCUMENT_EDITOR_NUMBER_OF_ENTRIES_IN_STACK_OF_LISTS 1000
struct document_editor_stack_of_lists_entry {
 dword_t type_of_list;
 dword_t number;
}__attribute__((packed));
struct document_editor_stack_of_lists_entry *document_editor_stack_of_lists_pointer;
dword_t document_editor_stack_of_lists_number_of_entries = 0;

dword_t document_editor_program_interface_memory = 0;
dword_t document_editor_vertical_scrollbar_height = 0, document_editor_horizontal_scrollbar_width = 0;

void initalize_document_editor(void);
void document_editor(void);
void draw_document_editor(void);
void document_editor_draw_document(void);
void document_editor_open_file(void);
void document_editor_save_file(void);
void document_editor_close_file(void);
void document_editor_key_up_event(void);
void document_editor_key_down_event(void);
void document_editor_key_left_event(void);
void document_editor_key_right_event(void);
void document_editor_key_page_up_event(void);
void document_editor_key_page_down_event(void);
void document_editor_key_home_event(void);
void document_editor_key_end_event(void);
void document_editor_vertical_scrollbar_event(void);
void document_editor_horizontal_scrollbar_event(void);
void document_editor_recalculate_scrollbars(void);
void document_editor_add_style_to_stack(void);
void document_editor_take_style_from_stack(void);
void document_editor_take_style_from_stack_wihout_moving(void);
void document_editor_add_style_to_list(dword_t memory_of_style_name, dword_t memory_of_style_content);