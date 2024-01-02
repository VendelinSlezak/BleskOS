//BleskOS

/*
* MIT License
* Copyright (c) 2023-2024 Vendelín Slezák
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#define TEXT_EDITOR_FILE_TEXT_AREA_MEMORY (PROGRAM_INTERFACE_FILE_PROGRAM_AREA+0)

#define TEXT_EDITOR_CLICK_ZONE_TEXT_AREA 100
#define TEXT_EDITOR_CLICK_ZONE_VERTICAL_SCROLLBAR 101
#define TEXT_EDITOR_CLICK_ZONE_HORIZONTAL_SCROLLBAR 102
#define TEXT_EDITOR_CLICK_ZONE_MORE 103
#define TEXT_EDITOR_CLICK_ZONE_UNDO 104
#define TEXT_EDITOR_CLICK_ZONE_REDO 105
#define TEXT_EDITOR_CLICK_ZONE_MORE_LIST 106
#define TEXT_EDITOR_CLICK_ZONE_TEXT_AREA_FIND 107
#define TEXT_EDITOR_CLICK_ZONE_TEXT_AREA_FIND_NEXT 108
#define TEXT_EDITOR_CLICK_ZONE_TEXT_AREA_FIND_PREVIOUS 109
#define TEXT_EDITOR_CLICK_ZONE_TEXT_AREA_REPLACE 110
#define TEXT_EDITOR_CLICK_ZONE_TEXT_AREA_REPLACE_ACTUAL_SELECTION 111
#define TEXT_EDITOR_CLICK_ZONE_TEXT_AREA_REPLACE_ALL 112

#define TEXT_EDITOR_STATE_TEXT 1
#define TEXT_EDITOR_STATE_FIND 2
#define TEXT_EDITOR_STATE_FIND_AND_REPLACE 3

dword_t text_editor_program_interface_memory = 0, text_editor_button_undo_on_screen = 0, text_editor_button_redo_on_screen = 0, text_editor_state = 0, text_editor_more_list_on_screen = 0;
dword_t text_editor_find_function_text_area_info_mem = 0, text_editor_replace_function_text_area_mem = 0, text_editor_go_to_line_text_area_mem = 0;

void initalize_text_editor(void);
void text_editor(void);
void draw_text_editor(void);
void text_editor_open_file(void);
void text_editor_save_file(void);
void text_editor_new_file(void);
void text_editor_close_file(void);
void text_editor_key_f7_event(void);
void text_editor_key_f8_event(void);
void text_editor_key_f9_event(void);
void text_editor_key_f10_event(void);
void text_editor_key_f11_event(void);
void text_editor_click_zone_undo_event(void);
void text_editor_click_zone_redo_event(void);
void text_editor_click_zone_more_list(void);
void text_area_unfocus_find_and_replace(void);
void text_area_unfocus_find(void);
void text_area_unfocus_replace(void);
void text_editor_find_next(void);
void text_editor_find_previous(void);
void text_editor_replace_actual_selection(void);
void text_editor_replace_all(void);