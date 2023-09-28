//BleskOS

/*
* MIT License
* Copyright (c) 2023 Vendelín Slezák
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include "draw_webpage.h"
#include "process_html.h"
#include "process_css.h"

#define INTERNET_BROWSER_WEBPAGE_START_LINE 32

#define INTERNET_BROWSER_FILE_TEXT_AREA_MEMORY (PROGRAM_INTERFACE_FILE_PROGRAM_AREA+0)
#define INTERNET_BROWSER_FILE_STATUS (PROGRAM_INTERFACE_FILE_PROGRAM_AREA+1)
#define INTERNET_BROWSER_FILE_WEBPAGE_MEMORY (PROGRAM_INTERFACE_FILE_PROGRAM_AREA+2)
#define INTERNET_BROWSER_FILE_WEBPAGE_LENGTH (PROGRAM_INTERFACE_FILE_PROGRAM_AREA+3)
#define INTERNET_BROWSER_FILE_WEBPAGE_FIRST_SHOW_LINE (PROGRAM_INTERFACE_FILE_PROGRAM_AREA+4)
#define INTERNET_BROWSER_FILE_VERTICAL_SCROLLBAR_RIDER_POSITION (PROGRAM_INTERFACE_FILE_PROGRAM_AREA+5)
#define INTERNET_BROWSER_FILE_VERTICAL_SCROLLBAR_RIDER_SIZE (PROGRAM_INTERFACE_FILE_PROGRAM_AREA+6)
#define INTERNET_BROWSER_FILE_HTML_MEMORY (PROGRAM_INTERFACE_FILE_PROGRAM_AREA+7)
#define INTERNET_BROWSER_FILE_URL_HISTORY_MEMORY (PROGRAM_INTERFACE_FILE_PROGRAM_AREA+8)

#define INTERNET_BROWSER_CLICK_ZONE_SCROLLBAR 100
#define INTERNET_BROWSER_CLICK_ZONE_TEXT_AREA 101
#define INTERNET_BROWSER_CLICK_ZONE_PREVIOUS_PAGE_BUTTON 102

#define INTERNET_BROWSER_FILE_STATUS_NOTHING 0xFFFFFFFF

dword_t internet_browser_program_interface_memory = 0;
dword_t internet_browser_webpage_heigth = 0, internet_browser_webpage_width = 0, internet_browser_first_show_line = 0, internet_browser_last_show_line = 0, internet_browser_first_show_column = 0, internet_browser_last_show_column = 0;
dword_t internet_browser_url_mem = 0, internet_browser_message_line = 0;

void initalize_internet_browser(void);
void internet_browser(void);
void draw_internet_browser(void);
void internet_browser_print_message(byte_t *string);
void internet_browser_open_file(void);
void internet_browser_new_file(void);
void internet_browser_close_file(void);
void internet_browser_key_up_event(void);
void internet_browser_key_down_event(void);
void internet_browser_key_home_event(void);
void internet_browser_key_end_event(void);
void internet_browser_key_enter_event(void);
void internet_browser_key_p_event(void);
void internet_browser_vertical_scrollbar_event(void);
void internet_browser_load_webpage_from_url_in_text_area(void);