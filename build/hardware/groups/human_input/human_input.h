#ifndef BUILD_HARDWARE_GROUPS_HUMAN_INPUT_HUMAN_INPUT_H
#define BUILD_HARDWARE_GROUPS_HUMAN_INPUT_HUMAN_INPUT_H

/* 
* BleskOS
*
* MIT License
* Copyright (c) 2023-2025 BleskOS developers
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include <kernel/x86/scheduler/lock.h>

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

    AXIS_MOUSE_X,
    AXIS_MOUSE_Y,
    AXIS_MOUSE_WHEEL,
    AXIS_MOUSE_HWHEEL,

    AXIS_GAMEPAD_LEFT_X,     // -1.0 .. +1.0
    AXIS_GAMEPAD_LEFT_Y,     // -1.0 .. +1.0
    AXIS_GAMEPAD_RIGHT_X,    // -1.0 .. +1.0
    AXIS_GAMEPAD_RIGHT_Y,    // -1.0 .. +1.0
    AXIS_GAMEPAD_LT,         // 0.0 .. 1.0
    AXIS_GAMEPAD_RT,         // 0.0 .. 1.0

    AXIS_TOUCH_X,            // 0 .. screen width
    AXIS_TOUCH_Y,            // 0 .. screen height
    AXIS_TOUCH_PRESSURE,     // 0.0 .. 1.0

    INPUT_KEY_COUNT
} human_input_keys_t;

typedef struct {
    dword_t capslock : 1;
    dword_t scrollock : 1;
    dword_t numlock : 1;
} leds_t;

typedef struct {
    dword_t (*is_key_pressed)(dword_t entity, dword_t key);
    dword_t (*set_led)(dword_t entity, leds_t leds);
} human_input_device_functions_t;

typedef struct {
    dword_t packet_toggle;
    dword_t key;
    dword_t value;
} human_input_event_stack_entry_t;

#define HUMAN_INPUT_MAX_NUMBER_OF_DEVICES 16
#define HUMAN_INPUT_SIZE_OF_EVENT_STACK 128
typedef struct {
    dword_t number_of_devices;
    dword_t devices[HUMAN_INPUT_MAX_NUMBER_OF_DEVICES];
    dword_t ids[HUMAN_INPUT_MAX_NUMBER_OF_DEVICES];
    human_input_device_functions_t *functions[HUMAN_INPUT_MAX_NUMBER_OF_DEVICES];

    mutex_t event_stack_mutex;
    dword_t event_stack_producer;
    dword_t event_stack_consumer;
    dword_t event_stack_toggle;
    human_input_event_stack_entry_t event_stack[HUMAN_INPUT_SIZE_OF_EVENT_STACK];

    dword_t new_pressed_key;
    dword_t new_pressed_key_unicode;
    dword_t keys_of_last_event[INPUT_KEY_COUNT];
} e_human_input_group_t;
void initialize_human_input_group(void);
void human_input_group_add_device(dword_t device_entity, dword_t device_id, human_input_device_functions_t *functions);
void human_input_insert_value_to_packet(dword_t key, dword_t value);
void human_input_end_of_packet(void);
dword_t human_input_has_value(void);
void human_input_pop_event(void);
dword_t human_input_get_unicode_of_key(dword_t key);
dword_t human_input_is_key_pressed(dword_t key);
dword_t human_input_last_event_get_pressed_key(void);
dword_t human_input_last_event_get_pressed_key_unicode(void);

#endif /* BUILD_HARDWARE_GROUPS_HUMAN_INPUT_HUMAN_INPUT_H */
