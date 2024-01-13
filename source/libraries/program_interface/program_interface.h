//BleskOS

/*
* MIT License
* Copyright (c) 2023-2024 Vendelín Slezák
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#define CLICK_ZONE_PROGRAM_NAME 1
#define CLICK_ZONE_BACK 2
#define CLICK_ZONE_OPEN 3
#define CLICK_ZONE_SAVE 4
#define CLICK_ZONE_NEW 5
#define CLICK_ZONE_FIRST_FILE 10
#define CLICK_ZONE_LAST_FILE 19
#define CLICK_ZONE_CLOSE_FIRST_FILE 20
#define CLICK_ZONE_CLOSE_LAST_FILE 29

#define ELEMENT_VERTICAL_SCROLLBAR 1
#define ELEMENT_HORIZONTAL_SCROLLBAR 2
#define ELEMENT_TEXT_AREA 3

dword_t program_interface_keyboard_events_list_mem = 0, program_interface_click_zone_events_list_mem = 0, program_interface_elements_list_mem = 0;
dword_t program_interface_num_of_click_zone_events = 0, program_interface_num_of_elements = 0;
dword_t program_interface_memory_pointer = 0, program_interface_draw_method_pointer = 0, program_interface_text_area_info_memory = 0;
dword_t program_interface_element_with_focus = 0, program_interface_selected_element_number = 0, program_interface_text_area_selected_scrollbar = 0;

void initalize_program_interface(void);
void set_program_interface(dword_t program_interface_mem);
void clear_program_interface_before_drawing(void);
void program_interface_add_keyboard_event(dword_t key, dword_t method);
void program_interface_add_click_zone_event(dword_t click_zone, dword_t method);
void program_interface_add_vertical_scrollbar(dword_t click_zone, dword_t pointer_to_scrollbar_height, dword_t pointer_to_rider_position, dword_t pointer_to_rider_size, dword_t scrollbar_draw_method_pointer);
void program_interface_add_horizontal_scrollbar(dword_t click_zone, dword_t pointer_to_scrollbar_width, dword_t pointer_to_rider_position, dword_t pointer_to_rider_size, dword_t scrollbar_draw_method_pointer);
void program_interface_add_text_area(dword_t click_zone, dword_t text_area_info_mem);
void program_interface_redraw(void);
void program_interface_process_keyboard_event(void);
void program_interface_process_mouse_event(void);
byte_t dialog_yes_no(byte_t *string);
void error_window(byte_t *string);
dword_t get_number_of_clicked_item_from_menu_list(dword_t number_of_items);