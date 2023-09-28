//BleskOS

/*
* MIT License
* Copyright (c) 2023 Vendelín Slezák
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#define FD_TYPE_OPEN 0
#define FD_TYPE_SAVE 1

#define FD_DISPLAY_FILES_LIST 0
#define FD_DISPLAY_FILES_ICONS 1

dword_t file_dialog_folder_first_displayed_file_num=0, file_dialog_highlighted_file=0, file_dialog_list_num_of_files_on_screen=0, file_dialog_list_num_of_chars_of_name=0, file_dialog_icons_files_lines=0, file_dialog_icons_files_columns=0;
byte_t file_dialog_window_type = 0, file_dialog_type_of_displaying_files=0;

void file_dialog_draw_background(void);
void file_dialog_draw_devices(void);
void file_dialog_draw_files(void);
void draw_file_dialog(void);
void redraw_file_dialog(void);
