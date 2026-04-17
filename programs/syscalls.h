#ifndef SYSCALLS_H
#define SYSCALLS_H

/* 
* BleskOS
*
* MIT License
* Copyright (c) 2023-2026 BleskOS developers
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#define NULL 0
#define true 1
#define false 0

typedef unsigned int size_t;
typedef unsigned int uintptr_t;
typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int uint32_t;
typedef unsigned long long uint64_t;

typedef struct {
    uint32_t max_width;
    uint32_t max_height;
    uint32_t width;
    uint32_t height;
    uint32_t buffer[];
} screen_buffer_t;

typedef struct {
    uint32_t type;
    uint8_t *data;
    uint32_t number_of_characters;
    uint32_t bytes_per_character;
    uint32_t character_width;
    uint32_t character_height;
} bitmap_font_t;

typedef struct syslib {
    uint32_t version;
    uint32_t num_of_logical_processors;
    uint32_t userspace_start;
    uint32_t userspace_size;

    uint32_t window_width;
    uint32_t window_height;

    void (*initialize)(struct syslib *syslib_ptr);

    void (*log)(char *string);
    void (*logf)(char *string, ...);
    
    uint64_t (*get_time_in_microseconds)(void);

    void *(*malloc)(uint32_t size);
    void *(*calloc)(uint32_t number, uint32_t size);
    void *(*realloc)(void *ptr, uint32_t size);
    void (*free)(void *ptr);

    screen_buffer_t *(*create_window)(void);
    void (*redraw_window)(void);

    screen_buffer_t *(*create_screen_buffer)(uint32_t width, uint32_t height);
    void (*destroy_screen_buffer)(screen_buffer_t *buffer);

    bitmap_font_t *(*load_bitmap_font)(void *file, uint32_t size);
    void (*draw_bitmap_char)(screen_buffer_t *buffer, uint32_t x, uint32_t y, bitmap_font_t *font, uint32_t character, uint32_t color);
    void (*draw_bitmap_string)(screen_buffer_t *buffer, uint32_t x, uint32_t y, bitmap_font_t *font, char *string, uint32_t color);
    void (*destroy_bitmap_font)(bitmap_font_t *font);

    screen_buffer_t *(*initialize_gui)(void);
    void (*add_canvas_component)(uint32_t area_number, void (*redraw)(uint32_t screen_x, uint32_t screen_y, uint32_t screen_width, uint32_t screen_height, uint32_t x_offset, uint32_t y_offset));
    void (*register_pressed_key_event_handler)(uint32_t key, void (*handler)(void));
    void (*redraw_gui)(void);

    /*

    void (*copy_screen_buffer)(screen_buffer_t *dst, uint32_t dst_x, uint32_t dst_y, screen_buffer_t *src, uint32_t src_x, uint32_t src_y, uint32_t width, uint32_t height);

    void (*redraw_part_of_window)(uint32_t window_x, uint32_t window_y, screen_buffer_t *buffer, uint32_t buffer_x, uint32_t buffer_y, uint32_t width, uint32_t height);

    */
} syslib_t;

#define BASE_SCREEN_AREA 0
typedef enum {
    KEY_A = 1, KEY_B, KEY_C, KEY_D, KEY_E, KEY_F, KEY_G,
    KEY_H, KEY_I, KEY_J, KEY_K, KEY_L, KEY_M, KEY_N,
    KEY_O, KEY_P, KEY_Q, KEY_R, KEY_S, KEY_T, KEY_U,
    KEY_V, KEY_W, KEY_X, KEY_Y, KEY_Z,

    KEY_0, KEY_1, KEY_2, KEY_3, KEY_4,
    KEY_5, KEY_6, KEY_7, KEY_8, KEY_9,

    KEY_LEFT_CTRL,
    KEY_LEFT_SHIFT,
    KEY_LEFT_ALT,
    KEY_LEFT_GUI,
    KEY_RIGHT_CTRL,
    KEY_RIGHT_SHIFT,
    KEY_RIGHT_ALT,
    KEY_RIGHT_GUI,
    KEY_CAPS_LOCK,
    KEY_NUM_LOCK,
    KEY_SCROLL_LOCK,

    KEY_ESCAPE,
    KEY_F1, KEY_F2, KEY_F3, KEY_F4,
    KEY_F5, KEY_F6, KEY_F7, KEY_F8,
    KEY_F9, KEY_F10, KEY_F11, KEY_F12,
    KEY_F13, KEY_F14, KEY_F15, KEY_F16,
    KEY_F17, KEY_F18, KEY_F19, KEY_F20,
    KEY_F21, KEY_F22, KEY_F23, KEY_F24,

    KEY_PRINT_SCREEN,
    KEY_PAUSE,
    KEY_INSERT,
    KEY_DELETE,
    KEY_HOME,
    KEY_END,
    KEY_PAGE_UP,
    KEY_PAGE_DOWN,
    KEY_UP_ARROW,
    KEY_DOWN_ARROW,
    KEY_LEFT_ARROW,
    KEY_RIGHT_ARROW,

    KEY_GRAVE,         // `
    KEY_MINUS,         // -
    KEY_EQUAL,         // =
    KEY_BACKSPACE,
    KEY_TAB,
    KEY_LEFT_BRACKET,  // [
    KEY_RIGHT_BRACKET, // ]
    KEY_BACKSLASH,     // backslash
    KEY_SEMICOLON,     // ;
    KEY_APOSTROPHE,    // '
    KEY_COMMA,         // ,
    KEY_PERIOD,        // .
    KEY_SLASH,         // /
    KEY_SPACE,
    KEY_ENTER,

    KEY_NUMPAD_0,
    KEY_NUMPAD_1,
    KEY_NUMPAD_2,
    KEY_NUMPAD_3,
    KEY_NUMPAD_4,
    KEY_NUMPAD_5,
    KEY_NUMPAD_6,
    KEY_NUMPAD_7,
    KEY_NUMPAD_8,
    KEY_NUMPAD_9,
    KEY_NUMPAD_DOT,
    KEY_NUMPAD_ENTER,
    KEY_NUMPAD_PLUS,
    KEY_NUMPAD_MINUS,
    KEY_NUMPAD_ASTERISK,
    KEY_NUMPAD_SLASH,

    KEY_VOLUME_MUTE,
    KEY_VOLUME_DOWN,
    KEY_VOLUME_UP,
    KEY_MEDIA_NEXT,
    KEY_MEDIA_PREV,
    KEY_MEDIA_STOP,
    KEY_MEDIA_PLAY_PAUSE,
    KEY_MEDIA_SELECT,
    KEY_LAUNCH_MAIL,
    KEY_LAUNCH_BROWSER,
    KEY_LAUNCH_CALCULATOR,
    KEY_LAUNCH_APPS,
    KEY_LAUNCH_MEDIA,
    KEY_LAUNCH_MY_COMPUTER,
    KEY_POWER,
    KEY_SLEEP,
    KEY_WAKE,
    KEY_WWW_FAVOURITES,
    KEY_WWW_REFRESH,
    KEY_WWW_STOP,
    KEY_WWW_FORWARD,
    KEY_WWW_BACK,
    KEY_WWW_HOME,
    KEY_WWW_SEARCH,

    KEY_INTL_BACKSLASH,     // ISO key
    KEY_INTL_RO,            // Japan "ろ"
    KEY_INTL_YEN,           // ¥ key
    KEY_INTL_HENKAN,        // 変換
    KEY_INTL_MUHENKAN,      // 無変換
    KEY_INTL_KATAKANAHIRAGANA,
    KEY_INTL_ZENKAKUHANKAKU,
    KEY_ABNT_C1,            // brazilian "/" numpad
    KEY_ABNT_C2,            // brazilian "." numpad

    MOUSE_BUTTON_LEFT,
    MOUSE_BUTTON_RIGHT,
    MOUSE_BUTTON_MIDDLE,
    MOUSE_BUTTON_X1,
    MOUSE_BUTTON_X2,

    INPUT_KEY_COUNT
} human_input_keys_t;

#endif