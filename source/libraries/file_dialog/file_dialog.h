//BleskOS

/*
* MIT License
* Copyright (c) 2023-2024 Vendelín Slezák
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#define FILE_DIALOG_OPEN 0
#define FILE_DIALOG_SAVE 1

enum {
 CLICK_ZONE_FILE_DIALOG_BACK = 1,
 CLICK_ZONE_FILE_DIALOG_REFRESH_DEVICES,
 CLICK_ZONE_FILE_DIALOG_CHANGE_VIEW_WINDOW,
 CLICK_ZONE_FILE_DIALOG_PREVIEW_WINDOW,
 CLICK_ZONE_FILE_DIALOG_GO_TO_PREVIOUS_FOLDER,
 CLICK_ZONE_FILE_DIALOG_CREATE_NEW_FOLDER,
 CLICK_ZONE_FILE_DIALOG_READ_FILE_SKIPPING_ERRORS,
 CLICK_ZONE_FILE_DIALOG_SAVE_FILE_TO_FOLDER,
 CLICK_ZONE_FILE_DIALOG_SCROLLBAR,
 CLICK_ZONE_FILE_DIALOG_FIRST_CONNECTED_PARTITION,
 CLICK_ZONE_FILE_DIALOG_LAST_CONNECTED_PARTITION = (CLICK_ZONE_FILE_DIALOG_FIRST_CONNECTED_PARTITION+9),
 CLICK_ZONE_FILE_DIALOG_FIRST_ITEM,
};
enum {
 FILE_DIALOG_EVENT_EXIT_FILE_NOT_LOADED = PROGRAM_DEFINED_EVENTS,
 FILE_DIALOG_EVENT_EXIT_FILE_SUCCESSFULLY_LOADED,
 FILE_DIALOG_EVENT_EXIT_FILE_SUCCESSFULLY_SAVED,
 FILE_DIALOG_EVENT_REDRAW,
};

enum {
 CLICK_ZONE_FILE_DIALOG_VIEW_WINDOW_BACK = 1,
 CLICK_ZONE_FILE_DIALOG_VIEW_WINDOW_LIST,
 CLICK_ZONE_FILE_DIALOG_VIEW_WINDOW_ICONS,
 CLICK_ZONE_FILE_DIALOG_VIEW_WINDOW_SORT_BY_NAME,
 CLICK_ZONE_FILE_DIALOG_VIEW_WINDOW_SORT_BY_EXTENSION,
 CLICK_ZONE_FILE_DIALOG_VIEW_WINDOW_SORT_BY_SIZE,
 CLICK_ZONE_FILE_DIALOG_VIEW_WINDOW_SORT_BY_DATE_OF_CREATION,
};

enum {
 CLICK_ZONE_FILE_DIALOG_PREVIEW_WINDOW_BACK = 1,
 CLICK_ZONE_FILE_DIALOG_PREVIEW_WINDOW_LEFT,
 CLICK_ZONE_FILE_DIALOG_PREVIEW_WINDOW_ENTER,
 CLICK_ZONE_FILE_DIALOG_PREVIEW_WINDOW_RIGHT,
};

enum {
 CLICK_ZONE_FILE_DIALOG_NAME_INPUT_CANCEL = 1,
 CLICK_ZONE_FILE_DIALOG_NAME_INPUT_APPROVE,
};

#define FILE_DIALOG_WIDTH_OF_CONNECTED_PARTITIONS 22*8

#define FILE_DIALOG_NUMBER_OF_CHARS_IN_ICON_LINE 10
#define FILE_DIALOG_NUMBER_OF_PIXELS_IN_ICON_DIMENSION FILE_DIALOG_NUMBER_OF_CHARS_IN_ICON_LINE*8

struct folder_descriptor_t *file_dialog_folder_descriptor;
struct file_descriptor_t *file_dialog_file_descriptor;
byte_t *file_dialog_open_file_memory;

dword_t file_dialog_scrollbar_y, file_dialog_scrollbar_size, file_dialog_scrollbar_rider_size, file_dialog_scrollbar_rider_position;
dword_t file_dialog_number_of_lines_on_screen, file_dialog_number_of_columns_on_screen, file_dialog_number_of_items_on_screen;

byte_t file_dialog_type;
byte_t *file_dialog_extensions;
byte_t *file_dialog_file_memory;
dword_t file_dialog_file_size_in_bytes;

//TODO: rewrite later when code of text area will be rewritten
dword_t *file_dialog_text_area;

struct scrollbar_info_t file_dialog_scrollbar_info;

void initalize_file_dialog(void);
byte_t file_dialog_open(byte_t *extensions);
byte_t file_dialog_save(byte_t *file_memory, dword_t file_size_in_bytes, byte_t *extension);

byte_t file_dialog(byte_t type, byte_t *extensions);
void file_dialog_event_click_on_connected_partitions(void);
void file_dialog_event_key_page_up(void);
void file_dialog_event_key_page_down(void);
void file_dialog_event_key_up(void);
void file_dialog_event_key_down(void);
void file_dialog_event_key_left(void);
void file_dialog_event_key_right(void);
dword_t file_dialog_event_key_enter(void);
void file_dialog_event_go_to_previous_folder(void);
dword_t file_dialog_event_click_on_files(void);
void file_dialog_event_scrollbar_change(dword_t value);

void file_dialog_change_view_window(void);
dword_t file_dialog_preview_window(void);
dword_t file_dialog_read_file_skipping_errors(void);
dword_t file_dialog_create_folder_in_folder(void);
dword_t file_dialog_save_file_to_folder(void);

void redraw_file_dialog(void);

byte_t is_loaded_file_extension(byte_t *extension);

dword_t file_dialog_event_interface[] = { 0,
 //exit
 KEYBOARD_EVENT_PRESSED_KEY, KEY_ESC, 0, FILE_DIALOG_EVENT_EXIT_FILE_NOT_LOADED,
 MOUSE_EVENT_CLICK_ON_ZONE, CLICK_ZONE_FILE_DIALOG_BACK, 0, FILE_DIALOG_EVENT_EXIT_FILE_NOT_LOADED,

 //change of selected partition
 KEYBOARD_EVENT_PRESSED_KEY, KEY_PAGE_UP, (dword_t)(&file_dialog_event_key_page_up), NO_EVENT,
 KEYBOARD_EVENT_PRESSED_KEY, KEY_PAGE_DOWN, (dword_t)(&file_dialog_event_key_page_down), NO_EVENT,
 MOUSE_EVENT_CLICK_ON_ZONES, CLICK_ZONE_FILE_DIALOG_FIRST_CONNECTED_PARTITION, CLICK_ZONE_FILE_DIALOG_LAST_CONNECTED_PARTITION, (dword_t)(&file_dialog_event_click_on_connected_partitions), NO_EVENT,
 
 //refresh devices
 KEYBOARD_EVENT_PRESSED_KEY, KEY_F8, (dword_t)(&refresh_devices), FILE_DIALOG_EVENT_REDRAW,
 MOUSE_EVENT_CLICK_ON_ZONE, CLICK_ZONE_FILE_DIALOG_REFRESH_DEVICES, (dword_t)(&refresh_devices), FILE_DIALOG_EVENT_REDRAW,

 //events in file zone
 KEYBOARD_EVENT_PRESSED_KEY, KEY_UP, (dword_t)(&file_dialog_event_key_up), NO_EVENT,
 KEYBOARD_EVENT_PRESSED_KEY, KEY_DOWN, (dword_t)(&file_dialog_event_key_down), NO_EVENT,
 KEYBOARD_EVENT_PRESSED_KEY, KEY_LEFT, (dword_t)(&file_dialog_event_key_left), NO_EVENT,
 KEYBOARD_EVENT_PRESSED_KEY, KEY_RIGHT, (dword_t)(&file_dialog_event_key_right), NO_EVENT,
 KEYBOARD_EVENT_PRESSED_KEY, KEY_ENTER, (dword_t)(&file_dialog_event_key_enter), RETURN_EVENT_FROM_METHOD,
 MOUSE_EVENT_CLICK_ON_ZONES, CLICK_ZONE_FILE_DIALOG_FIRST_ITEM, 0xFFFFFFFF, (dword_t)(&file_dialog_event_click_on_files), RETURN_EVENT_FROM_METHOD,
 VERTICAL_SCROLLBAR_EVENT, CLICK_ZONE_FILE_DIALOG_SCROLLBAR, (dword_t)(&file_dialog_scrollbar_info), (dword_t)(&file_dialog_event_scrollbar_change),

 //go to previous folder
 KEYBOARD_EVENT_PRESSED_KEY, KEY_B, (dword_t)(&file_dialog_event_go_to_previous_folder), NO_EVENT,
 MOUSE_EVENT_CLICK_ON_ZONE, CLICK_ZONE_FILE_DIALOG_GO_TO_PREVIOUS_FOLDER, (dword_t)(&file_dialog_event_go_to_previous_folder), NO_EVENT,

 //change view
 KEYBOARD_EVENT_PRESSED_KEY, KEY_V, (dword_t)(&file_dialog_change_view_window), NO_EVENT,
 MOUSE_EVENT_CLICK_ON_ZONE, CLICK_ZONE_FILE_DIALOG_CHANGE_VIEW_WINDOW, (dword_t)(&file_dialog_change_view_window), NO_EVENT,

 //image preview
 KEYBOARD_EVENT_PRESSED_KEY, KEY_P, (dword_t)(&file_dialog_preview_window), RETURN_EVENT_FROM_METHOD,
 MOUSE_EVENT_CLICK_ON_ZONE, CLICK_ZONE_FILE_DIALOG_PREVIEW_WINDOW, (dword_t)(&file_dialog_preview_window), RETURN_EVENT_FROM_METHOD,

 //read file with skipping errors
 KEYBOARD_EVENT_PRESSED_KEY, KEY_R, (dword_t)(&file_dialog_read_file_skipping_errors), RETURN_EVENT_FROM_METHOD,
 MOUSE_EVENT_CLICK_ON_ZONE, CLICK_ZONE_FILE_DIALOG_READ_FILE_SKIPPING_ERRORS, (dword_t)(&file_dialog_read_file_skipping_errors), RETURN_EVENT_FROM_METHOD,

 //create folder
 KEYBOARD_EVENT_PRESSED_KEY, KEY_C, (dword_t)(&file_dialog_create_folder_in_folder), RETURN_EVENT_FROM_METHOD,
 MOUSE_EVENT_CLICK_ON_ZONE, CLICK_ZONE_FILE_DIALOG_CREATE_NEW_FOLDER, (dword_t)(&file_dialog_create_folder_in_folder), RETURN_EVENT_FROM_METHOD,

 //save file
 KEYBOARD_EVENT_PRESSED_KEY, KEY_S, (dword_t)(&file_dialog_save_file_to_folder), RETURN_EVENT_FROM_METHOD,
 MOUSE_EVENT_CLICK_ON_ZONE, CLICK_ZONE_FILE_DIALOG_SAVE_FILE_TO_FOLDER, (dword_t)(&file_dialog_save_file_to_folder), RETURN_EVENT_FROM_METHOD,

 END_OF_EVENTS,
};

dword_t file_dialog_view_window_event_interface[] = { 0,
 KEYBOARD_EVENT_PRESSED_KEY, KEY_ESC, 0, CLICK_ZONE_FILE_DIALOG_VIEW_WINDOW_BACK,
 MOUSE_EVENT_CLICK_ON_ZONE, CLICK_ZONE_FILE_DIALOG_VIEW_WINDOW_BACK, 0, CLICK_ZONE_FILE_DIALOG_VIEW_WINDOW_BACK,

 KEYBOARD_EVENT_PRESSED_KEY, KEY_L, 0, CLICK_ZONE_FILE_DIALOG_VIEW_WINDOW_LIST,
 MOUSE_EVENT_CLICK_ON_ZONE, CLICK_ZONE_FILE_DIALOG_VIEW_WINDOW_LIST, 0, CLICK_ZONE_FILE_DIALOG_VIEW_WINDOW_LIST,
 KEYBOARD_EVENT_PRESSED_KEY, KEY_I, 0, CLICK_ZONE_FILE_DIALOG_VIEW_WINDOW_ICONS,
 MOUSE_EVENT_CLICK_ON_ZONE, CLICK_ZONE_FILE_DIALOG_VIEW_WINDOW_ICONS, 0, CLICK_ZONE_FILE_DIALOG_VIEW_WINDOW_ICONS,

 KEYBOARD_EVENT_PRESSED_KEY, KEY_N, 0, CLICK_ZONE_FILE_DIALOG_VIEW_WINDOW_SORT_BY_NAME,
 MOUSE_EVENT_CLICK_ON_ZONE, CLICK_ZONE_FILE_DIALOG_VIEW_WINDOW_SORT_BY_NAME, 0, CLICK_ZONE_FILE_DIALOG_VIEW_WINDOW_SORT_BY_NAME,
 KEYBOARD_EVENT_PRESSED_KEY, KEY_E, 0, CLICK_ZONE_FILE_DIALOG_VIEW_WINDOW_SORT_BY_EXTENSION,
 MOUSE_EVENT_CLICK_ON_ZONE, CLICK_ZONE_FILE_DIALOG_VIEW_WINDOW_SORT_BY_EXTENSION, 0, CLICK_ZONE_FILE_DIALOG_VIEW_WINDOW_SORT_BY_EXTENSION,
 KEYBOARD_EVENT_PRESSED_KEY, KEY_S, 0, CLICK_ZONE_FILE_DIALOG_VIEW_WINDOW_SORT_BY_SIZE,
 MOUSE_EVENT_CLICK_ON_ZONE, CLICK_ZONE_FILE_DIALOG_VIEW_WINDOW_SORT_BY_SIZE, 0, CLICK_ZONE_FILE_DIALOG_VIEW_WINDOW_SORT_BY_SIZE,
 KEYBOARD_EVENT_PRESSED_KEY, KEY_D, 0, CLICK_ZONE_FILE_DIALOG_VIEW_WINDOW_SORT_BY_DATE_OF_CREATION,
 MOUSE_EVENT_CLICK_ON_ZONE, CLICK_ZONE_FILE_DIALOG_VIEW_WINDOW_SORT_BY_DATE_OF_CREATION, 0, CLICK_ZONE_FILE_DIALOG_VIEW_WINDOW_SORT_BY_DATE_OF_CREATION,

 END_OF_EVENTS,
};

dword_t file_dialog_preview_window_event_interface[] = { 0,
 KEYBOARD_EVENT_PRESSED_KEY, KEY_ESC, 0, CLICK_ZONE_FILE_DIALOG_PREVIEW_WINDOW_BACK,
 MOUSE_EVENT_CLICK_ON_ZONE, 0, 0, CLICK_ZONE_FILE_DIALOG_PREVIEW_WINDOW_BACK,

 KEYBOARD_EVENT_PRESSED_KEY, KEY_LEFT, 0, CLICK_ZONE_FILE_DIALOG_PREVIEW_WINDOW_LEFT,
 MOUSE_EVENT_CLICK_ON_ZONE, CLICK_ZONE_FILE_DIALOG_PREVIEW_WINDOW_LEFT, 0, CLICK_ZONE_FILE_DIALOG_PREVIEW_WINDOW_LEFT,
 KEYBOARD_EVENT_PRESSED_KEY, KEY_ENTER, 0, CLICK_ZONE_FILE_DIALOG_PREVIEW_WINDOW_ENTER,
 MOUSE_EVENT_CLICK_ON_ZONE, CLICK_ZONE_FILE_DIALOG_PREVIEW_WINDOW_ENTER, 0, CLICK_ZONE_FILE_DIALOG_PREVIEW_WINDOW_ENTER,
 KEYBOARD_EVENT_PRESSED_KEY, KEY_RIGHT, 0, CLICK_ZONE_FILE_DIALOG_PREVIEW_WINDOW_RIGHT,
 MOUSE_EVENT_CLICK_ON_ZONE, CLICK_ZONE_FILE_DIALOG_PREVIEW_WINDOW_RIGHT, 0, CLICK_ZONE_FILE_DIALOG_PREVIEW_WINDOW_RIGHT,

 END_OF_EVENTS,
};

dword_t file_dialog_name_input_event_interface[] = { 0,
 KEYBOARD_EVENT_PRESSED_KEY, KEY_ESC, 0, CLICK_ZONE_FILE_DIALOG_NAME_INPUT_CANCEL,
 MOUSE_EVENT_CLICK_ON_ZONE, CLICK_ZONE_FILE_DIALOG_NAME_INPUT_CANCEL, 0, CLICK_ZONE_FILE_DIALOG_NAME_INPUT_CANCEL,
 KEYBOARD_EVENT_PRESSED_KEY, KEY_ENTER, 0, CLICK_ZONE_FILE_DIALOG_NAME_INPUT_APPROVE,
 MOUSE_EVENT_CLICK_ON_ZONE, CLICK_ZONE_FILE_DIALOG_NAME_INPUT_APPROVE, 0, CLICK_ZONE_FILE_DIALOG_NAME_INPUT_APPROVE,

 TEXT_AREA_WITH_PERMANENT_FOCUS_EVENT, 0,

 END_OF_EVENTS,
};