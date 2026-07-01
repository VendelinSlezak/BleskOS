/* 
* BleskOS
*
* MIT License
* Copyright (c) 2023-2025 BleskOS developers
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include <kernel/hardware/devices/cpu/mutex.h>
#include <kernel/hardware/main.h>

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

    INPUT_KEY_COUNT
} human_input_keys_t;
enum {
    KEY_RELEASED = 0,
    KEY_PRESSED = 1,
};
typedef enum {
    BUTTON_LEFT = 1,
    BUTTON_MIDDLE,
    BUTTON_RIGHT,
    BUTTON_4,
    BUTTON_5,

    INPUT_BUTTONS_COUNT
} human_input_buttons_t;
enum {
    BUTTON_RELEASED = 0,
    BUTTON_PRESSED = 1,
    BUTTON_DRAGGED = 2,
};
typedef enum {
    X_MOVEMENT = 1,
    Y_MOVEMENT,
    WHEEL_VERTICAL_MOVEMENT,
    WHEEL_HORIZONTAL_MOVEMENT,

    INPUT_MOVEMENTS_COUNT
} human_input_movements_t;
enum {
    NO_MOVEMENT_CHANGE = 0,
    NEW_MOVEMENT_CHANGE
};
typedef struct {
    uint32_t capslock : 1;
    uint32_t scrollock : 1;
    uint32_t numlock : 1;
    uint32_t : 29;
} leds_t;

typedef struct {
    uint8_t key_state[INPUT_KEY_COUNT];
    leds_t leds;
    uint8_t button_state[INPUT_BUTTONS_COUNT];
    uint8_t movement_state;
    uint32_t movement_value[INPUT_MOVEMENTS_COUNT];
} human_input_device_state_t;

typedef struct {
    uint8_t key_state[INPUT_KEY_COUNT];
    uint32_t key_state_timestamp[INPUT_KEY_COUNT];
    leds_t leds;
    uint8_t button_state[INPUT_BUTTONS_COUNT];
    uint32_t button_state_timestamp[INPUT_BUTTONS_COUNT];
    uint8_t movement_state;
    uint32_t movement_value[INPUT_MOVEMENTS_COUNT];

    uint32_t last_pressed_key;
    uint32_t last_pressed_key_unicode_value;
    uint32_t last_pressed_key_event_count;
} human_input_global_state_t;

enum {
    HUMAN_INPUT_GROUP_LISTEN_TO_EVENTS = 1,
    HUMAN_INPUT_GROUP_STOP_LISTENING_TO_EVENTS,
};
typedef struct {
    uint32_t type;
    uint32_t argument[];
} human_input_group_command_t;
enum {
    HUMAN_INPUT_EVENT_KEY_PRESSED = 1,
    HUMAN_INPUT_EVENT_KEY_RELEASED
};
typedef struct {
    uint32_t type;
    uint32_t value;
} human_input_event_t;
#define MAX_NUMBER_OF_HUMAN_INPUT_EVENTS 16
typedef struct {
    mutex_t spinlock;
    uint32_t producer;
    uint32_t consumer;
    human_input_event_t stack[MAX_NUMBER_OF_HUMAN_INPUT_EVENTS];
} human_input_event_stack_t;

typedef struct {
    human_input_device_state_t *(*get_state)(hardware_t *device);
    void (*keyboard_set_leds)(hardware_t *device, leds_t leds);
} human_input_group_device_functions_t;
typedef struct {
    hardware_t *device;
    human_input_group_device_functions_t *functions;
} human_input_device_t;
#define MAX_NUMBER_OF_HUMAN_INPUT_DEVICES 8
typedef struct {
    uint32_t number_of_devices;
    human_input_device_t devices[MAX_NUMBER_OF_HUMAN_INPUT_DEVICES];
} human_input_group_t;

extern human_input_global_state_t human_input_global_state;