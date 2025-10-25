/* 
* BleskOS
*
* MIT License
* Copyright (c) 2023-2025 BleskOS developers
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

/* includes */
#include <kernel/x86/kernel.h>
#include <hardware/groups/logging/logging.h>
#include <kernel/x86/hardware/id.h>
#include <kernel/x86/entities/entities.h>
#include <kernel/x86/scheduler/lock.h>

#include <hardware/groups/human_input/human_input.h>

/* local variables */
human_input_device_functions_t ps2_keyboard_group_functions[] = {
    { ps2_keyboard_is_key_pressed, ps2_keyboard_set_leds }
};

enum {
    PARSING_STATE_START = 0x01,
    PARSING_STATE_IF,
    PARSING_ACTION_NEW_STATE,
    PARSING_ACTION_PRESSED_KEY_CONVERT_BY_ARRAY,
    PARSING_ACTION_RELEASED_KEY_CONVERT_BY_ARRAY,
    PARSING_ACTION_PRESSED_OR_RELEASED_KEY_CONVERT_BY_ARRAY,
    PARSING_ACTION_PRINT_SCREEN_PRESSED,
    PARSING_ACTION_PRINT_SCREEN_RELEASED,
    PARSING_ACTION_PAUSE_PRESSED,
};

human_input_keys_t ps2_keyboard_scancode_set1_map[] = {
    [0x01] = KEY_ESCAPE,
    [0x02] = KEY_1,
    [0x03] = KEY_2,
    [0x04] = KEY_3,
    [0x05] = KEY_4,
    [0x06] = KEY_5,
    [0x07] = KEY_6,
    [0x08] = KEY_7,
    [0x09] = KEY_8,
    [0x0A] = KEY_9,
    [0x0B] = KEY_0,
    [0x0C] = KEY_MINUS,
    [0x0D] = KEY_EQUAL,
    [0x0E] = KEY_BACKSPACE,
    [0x0F] = KEY_TAB,
    [0x10] = KEY_Q,
    [0x11] = KEY_W,
    [0x12] = KEY_E,
    [0x13] = KEY_R,
    [0x14] = KEY_T,
    [0x15] = KEY_Y,
    [0x16] = KEY_U,
    [0x17] = KEY_I,
    [0x18] = KEY_O,
    [0x19] = KEY_P,
    [0x1A] = KEY_LEFT_BRACKET,
    [0x1B] = KEY_RIGHT_BRACKET,
    [0x1C] = KEY_ENTER,
    [0x1D] = KEY_LEFT_CTRL,
    [0x1E] = KEY_A,
    [0x1F] = KEY_S,
    [0x20] = KEY_D,
    [0x21] = KEY_F,
    [0x22] = KEY_G,
    [0x23] = KEY_H,
    [0x24] = KEY_J,
    [0x25] = KEY_K,
    [0x26] = KEY_L,
    [0x27] = KEY_SEMICOLON,
    [0x28] = KEY_APOSTROPHE,
    [0x29] = KEY_GRAVE,
    [0x2A] = KEY_LEFT_SHIFT,
    [0x2B] = KEY_BACKSLASH,
    [0x2C] = KEY_Z,
    [0x2D] = KEY_X,
    [0x2E] = KEY_C,
    [0x2F] = KEY_V,
    [0x30] = KEY_B,
    [0x31] = KEY_N,
    [0x32] = KEY_M,
    [0x33] = KEY_COMMA,
    [0x34] = KEY_PERIOD,
    [0x35] = KEY_SLASH,
    [0x36] = KEY_RIGHT_SHIFT,
    [0x37] = KEY_NUMPAD_ASTERISK,
    [0x38] = KEY_LEFT_ALT,
    [0x39] = KEY_SPACE,
    [0x3A] = KEY_CAPS_LOCK,
    [0x3B] = KEY_F1,
    [0x3C] = KEY_F2,
    [0x3D] = KEY_F3,
    [0x3E] = KEY_F4,
    [0x3F] = KEY_F5,
    [0x40] = KEY_F6,
    [0x41] = KEY_F7,
    [0x42] = KEY_F8,
    [0x43] = KEY_F9,
    [0x44] = KEY_F10,
    [0x45] = KEY_NUM_LOCK,
    [0x46] = KEY_SCROLL_LOCK,
    [0x47] = KEY_NUMPAD_7,
    [0x48] = KEY_NUMPAD_8,
    [0x49] = KEY_NUMPAD_9,
    [0x4A] = KEY_NUMPAD_MINUS,
    [0x4B] = KEY_NUMPAD_4,
    [0x4C] = KEY_NUMPAD_5,
    [0x4D] = KEY_NUMPAD_6,
    [0x4E] = KEY_NUMPAD_PLUS,
    [0x4F] = KEY_NUMPAD_1,
    [0x50] = KEY_NUMPAD_2,
    [0x51] = KEY_NUMPAD_3,
    [0x52] = KEY_NUMPAD_0,
    [0x53] = KEY_NUMPAD_DOT,
    [0x57] = KEY_F11,
    [0x58] = KEY_F12,
};

human_input_keys_t ps2_keyboard_scancode_set1_e0_map[] = {
    [0x10] = KEY_MEDIA_PREV,
    [0x19] = KEY_MEDIA_NEXT,
    [0x1C] = KEY_NUMPAD_ENTER,
    [0x1D] = KEY_RIGHT_CTRL,
    [0x20] = KEY_VOLUME_MUTE,
    [0x21] = KEY_LAUNCH_CALCULATOR,
    [0x22] = KEY_MEDIA_PLAY_PAUSE,
    [0x24] = KEY_MEDIA_STOP,
    [0x2E] = KEY_VOLUME_DOWN,
    [0x30] = KEY_VOLUME_UP,
    [0x32] = KEY_WWW_HOME,
    [0x35] = KEY_NUMPAD_SLASH,
    [0x38] = KEY_RIGHT_ALT,
    [0x47] = KEY_HOME,
    [0x48] = KEY_UP_ARROW,
    [0x49] = KEY_PAGE_UP,
    [0x4B] = KEY_LEFT_ARROW,
    [0x4D] = KEY_RIGHT_ARROW,
    [0x4F] = KEY_END,
    [0x50] = KEY_DOWN_ARROW,
    [0x51] = KEY_PAGE_DOWN,
    [0x52] = KEY_INSERT,
    [0x53] = KEY_DELETE,
    [0x5B] = KEY_LEFT_GUI,
    [0x5C] = KEY_RIGHT_GUI,
    [0x5D] = KEY_LAUNCH_APPS,
    [0x5E] = KEY_POWER,
    [0x5F] = KEY_SLEEP,
    [0x63] = KEY_WAKE,
    [0x65] = KEY_WWW_SEARCH,
    [0x66] = KEY_WWW_FAVOURITES,
    [0x67] = KEY_WWW_REFRESH,
    [0x68] = KEY_WWW_STOP,
    [0x69] = KEY_WWW_FORWARD,
    [0x6A] = KEY_WWW_BACK,
    [0x6B] = KEY_LAUNCH_MY_COMPUTER,
    [0x6C] = KEY_LAUNCH_MAIL,
    [0x6D] = KEY_MEDIA_SELECT
};

dword_t ps2_keyboard_scancode_set1_parsing_map[] = {
    PARSING_STATE_START, PARSING_STATE_IF, 0xE1, PARSING_ACTION_NEW_STATE, 0xE1,
    0xE1, PARSING_STATE_IF, 0x1D, PARSING_ACTION_NEW_STATE, 0xE1+1,
    0xE1+1, PARSING_STATE_IF, 0x45, PARSING_ACTION_NEW_STATE, 0xE1+2,
    0xE1+2, PARSING_STATE_IF, 0xE1, PARSING_ACTION_NEW_STATE, 0xE1+3,
    0xE1+3, PARSING_STATE_IF, 0x9D, PARSING_ACTION_NEW_STATE, 0xE1+4,
    0xE1+4, PARSING_STATE_IF, 0xC5, PARSING_ACTION_PAUSE_PRESSED, 0,

    PARSING_STATE_START, PARSING_STATE_IF, 0xE0, PARSING_ACTION_NEW_STATE, 0xE0,
    0xE0, PARSING_STATE_IF, 0x2A, PARSING_ACTION_NEW_STATE, 0xE02A,
    0xE02A, PARSING_STATE_IF, 0xE0, PARSING_ACTION_NEW_STATE, 0xE02AE0,
    0xE02AE0, PARSING_STATE_IF, 0x37, PARSING_ACTION_PRINT_SCREEN_PRESSED, 0,

    0xE0, PARSING_STATE_IF, 0xB7, PARSING_ACTION_NEW_STATE, 0xE0B7,
    0xE0B7, PARSING_STATE_IF, 0xE0, PARSING_ACTION_NEW_STATE, 0xE0B7E0,
    0xE0B7E0, PARSING_STATE_IF, 0xAA, PARSING_ACTION_PRINT_SCREEN_RELEASED, 0,

    0xE0, PARSING_ACTION_PRESSED_OR_RELEASED_KEY_CONVERT_BY_ARRAY, (dword_t)ps2_keyboard_scancode_set1_e0_map, 0, 0,

    PARSING_STATE_START, PARSING_ACTION_PRESSED_OR_RELEASED_KEY_CONVERT_BY_ARRAY, (dword_t)ps2_keyboard_scancode_set1_map, 0, 0,

    0
};

human_input_keys_t ps2_keyboard_scancode_set2_map[] = {
    [0x01] = KEY_F9,
    [0x03] = KEY_F5,
    [0x04] = KEY_F3,
    [0x05] = KEY_F1,
    [0x06] = KEY_F2,
    [0x07] = KEY_F12,
    [0x09] = KEY_F10,
    [0x0A] = KEY_F8,
    [0x0B] = KEY_F6,
    [0x0C] = KEY_F4,
    [0x0D] = KEY_TAB,
    [0x0E] = KEY_GRAVE,
    [0x11] = KEY_LEFT_ALT,
    [0x12] = KEY_LEFT_SHIFT,
    [0x14] = KEY_LEFT_CTRL,
    [0x15] = KEY_Q,
    [0x16] = KEY_1,
    [0x1A] = KEY_Z,
    [0x1B] = KEY_S,
    [0x1C] = KEY_A,
    [0x1D] = KEY_W,
    [0x1E] = KEY_2,
    [0x21] = KEY_C,
    [0x22] = KEY_X,
    [0x23] = KEY_D,
    [0x24] = KEY_E,
    [0x25] = KEY_4,
    [0x26] = KEY_3,
    [0x29] = KEY_SPACE,
    [0x2A] = KEY_V,
    [0x2B] = KEY_F,
    [0x2C] = KEY_T,
    [0x2D] = KEY_R,
    [0x2E] = KEY_5,
    [0x31] = KEY_N,
    [0x32] = KEY_B,
    [0x33] = KEY_H,
    [0x34] = KEY_G,
    [0x35] = KEY_Y,
    [0x36] = KEY_6,
    [0x3A] = KEY_M,
    [0x3B] = KEY_J,
    [0x3C] = KEY_U,
    [0x3D] = KEY_7,
    [0x3E] = KEY_8,
    [0x41] = KEY_COMMA,
    [0x42] = KEY_K,
    [0x43] = KEY_I,
    [0x44] = KEY_O,
    [0x45] = KEY_0,
    [0x46] = KEY_9,
    [0x49] = KEY_PERIOD,
    [0x4A] = KEY_SLASH,
    [0x4B] = KEY_L,
    [0x4C] = KEY_SEMICOLON,
    [0x4D] = KEY_P,
    [0x4E] = KEY_MINUS,
    [0x52] = KEY_APOSTROPHE,
    [0x54] = KEY_LEFT_BRACKET,
    [0x55] = KEY_EQUAL,
    [0x58] = KEY_CAPS_LOCK,
    [0x59] = KEY_RIGHT_SHIFT,
    [0x5A] = KEY_ENTER,
    [0x5B] = KEY_RIGHT_BRACKET,
    [0x5D] = KEY_BACKSLASH,
    [0x66] = KEY_BACKSPACE,
    [0x69] = KEY_NUMPAD_1,
    [0x6B] = KEY_NUMPAD_4,
    [0x6C] = KEY_NUMPAD_7,
    [0x70] = KEY_NUMPAD_0,
    [0x71] = KEY_NUMPAD_DOT,
    [0x72] = KEY_NUMPAD_2,
    [0x73] = KEY_NUMPAD_5,
    [0x74] = KEY_NUMPAD_6,
    [0x75] = KEY_NUMPAD_8,
    [0x76] = KEY_ESCAPE,
    [0x77] = KEY_NUM_LOCK,
    [0x78] = KEY_F11,
    [0x79] = KEY_NUMPAD_PLUS,
    [0x7A] = KEY_NUMPAD_3,
    [0x7B] = KEY_NUMPAD_MINUS,
    [0x7C] = KEY_NUMPAD_ASTERISK,
    [0x7D] = KEY_NUMPAD_9,
    [0x7E] = KEY_SCROLL_LOCK,
    [0x83] = KEY_F7,
};

human_input_keys_t ps2_keyboard_scancode_set2_e0_map[] = {
    [0x10] = KEY_LAUNCH_BROWSER,
    [0x11] = KEY_RIGHT_ALT,
    [0x14] = KEY_RIGHT_CTRL,
    [0x15] = KEY_MEDIA_PREV,
    [0x18] = KEY_WWW_FAVOURITES,
    [0x1F] = KEY_LEFT_GUI,
    [0x20] = KEY_WWW_REFRESH,
    [0x21] = KEY_VOLUME_DOWN,
    [0x23] = KEY_VOLUME_MUTE,
    [0x27] = KEY_RIGHT_GUI,
    [0x28] = KEY_WWW_STOP,
    [0x2B] = KEY_LAUNCH_CALCULATOR,
    [0x2F] = KEY_LAUNCH_APPS,
    [0x30] = KEY_WWW_FORWARD,
    [0x32] = KEY_VOLUME_UP,
    [0x34] = KEY_MEDIA_PLAY_PAUSE,
    [0x37] = KEY_POWER,
    [0x38] = KEY_WWW_BACK,
    [0x3A] = KEY_WWW_HOME,
    [0x3B] = KEY_MEDIA_STOP,
    [0x3F] = KEY_SLEEP,
    [0x40] = KEY_LAUNCH_MY_COMPUTER,
    [0x48] = KEY_LAUNCH_MAIL,
    [0x4A] = KEY_NUMPAD_SLASH,
    [0x4D] = KEY_MEDIA_NEXT,
    [0x50] = KEY_MEDIA_SELECT,
    [0x5A] = KEY_NUMPAD_ENTER,
    [0x5E] = KEY_WAKE,
    [0x69] = KEY_END,
    [0x6B] = KEY_LEFT_ARROW,
    [0x6C] = KEY_HOME,
    [0x70] = KEY_INSERT,
    [0x71] = KEY_DELETE,
    [0x72] = KEY_DOWN_ARROW,
    [0x74] = KEY_RIGHT_ARROW,
    [0x75] = KEY_UP_ARROW,
    [0x7A] = KEY_PAGE_DOWN,
    [0x7D] = KEY_PAGE_UP,
};

dword_t ps2_keyboard_scancode_set2_parsing_map[] = {
    PARSING_STATE_START, PARSING_STATE_IF, 0xE1, PARSING_ACTION_NEW_STATE, 0xE1,
    0xE1, PARSING_STATE_IF, 0x14, PARSING_ACTION_NEW_STATE, 0xE1+1,
    0xE1+1, PARSING_STATE_IF, 0x77, PARSING_ACTION_NEW_STATE, 0xE1+2,
    0xE1+2, PARSING_STATE_IF, 0xE1, PARSING_ACTION_NEW_STATE, 0xE1+3,
    0xE1+3, PARSING_STATE_IF, 0xF0, PARSING_ACTION_NEW_STATE, 0xE1+4,
    0xE1+4, PARSING_STATE_IF, 0x14, PARSING_ACTION_NEW_STATE, 0xE1+5,
    0xE1+5, PARSING_STATE_IF, 0xF0, PARSING_ACTION_NEW_STATE, 0xE1+6,
    0xE1+6, PARSING_STATE_IF, 0x77, PARSING_ACTION_PAUSE_PRESSED, 0,

    PARSING_STATE_START, PARSING_STATE_IF, 0xE0, PARSING_ACTION_NEW_STATE, 0xE0,
    0xE0, PARSING_STATE_IF, 0x12, PARSING_ACTION_NEW_STATE, 0xE012,
    0xE012, PARSING_STATE_IF, 0xE0, PARSING_ACTION_NEW_STATE, 0xE012E0,
    0xE012E0, PARSING_STATE_IF, 0x7C, PARSING_ACTION_PRINT_SCREEN_PRESSED, 0,

    0xE0, PARSING_STATE_IF, 0xF0, PARSING_ACTION_NEW_STATE, 0xE0F0,
    0xE0F0, PARSING_STATE_IF, 0x7C, PARSING_ACTION_NEW_STATE, 0xE0F07C,
    0xE0F07C, PARSING_STATE_IF, 0xE0, PARSING_ACTION_NEW_STATE, 0xE0F07CE0,
    0xE0F07CE0, PARSING_STATE_IF, 0xF0, PARSING_ACTION_NEW_STATE, 0xE0F07CE1,
    0xE0F07CE1, PARSING_STATE_IF, 0x12, PARSING_ACTION_PRINT_SCREEN_RELEASED, 0,

    0xE0F0, PARSING_ACTION_RELEASED_KEY_CONVERT_BY_ARRAY, (dword_t)ps2_keyboard_scancode_set2_e0_map, 0, 0,

    0xE0, PARSING_ACTION_PRESSED_KEY_CONVERT_BY_ARRAY, (dword_t)ps2_keyboard_scancode_set2_e0_map, 0, 0,

    PARSING_STATE_START, PARSING_STATE_IF, 0xF0, PARSING_ACTION_NEW_STATE, 0xF0,
    0xF0, PARSING_ACTION_RELEASED_KEY_CONVERT_BY_ARRAY, (dword_t)ps2_keyboard_scancode_set2_map, 0, 0,

    PARSING_STATE_START, PARSING_ACTION_PRESSED_KEY_CONVERT_BY_ARRAY, (dword_t)ps2_keyboard_scancode_set2_map, 0, 0,

    0
};

human_input_keys_t ps2_keyboard_scancode_set3_map[] = {
    [0x1C] = KEY_A,
    [0x32] = KEY_B,
    [0x21] = KEY_C,
    [0x23] = KEY_D,
    [0x24] = KEY_E,
    [0x2B] = KEY_F,
    [0x34] = KEY_G,
    [0x33] = KEY_H,
    [0x43] = KEY_I,
    [0x3B] = KEY_J,
    [0x42] = KEY_K,
    [0x4B] = KEY_L,
    [0x3A] = KEY_M,
    [0x31] = KEY_N,
    [0x44] = KEY_O,
    [0x4D] = KEY_P,
    [0x15] = KEY_Q,
    [0x2D] = KEY_R,
    [0x1B] = KEY_S,
    [0x2C] = KEY_T,
    [0x3C] = KEY_U,
    [0x2A] = KEY_V,
    [0x1D] = KEY_W,
    [0x22] = KEY_X,
    [0x35] = KEY_Y,
    [0x1A] = KEY_Z,
    [0x45] = KEY_0,
    [0x16] = KEY_1,
    [0x1E] = KEY_2,
    [0x26] = KEY_3,
    [0x25] = KEY_4,
    [0x2E] = KEY_5,
    [0x36] = KEY_6,
    [0x3D] = KEY_7, // ?
    [0x3E] = KEY_8,
    [0x46] = KEY_9,
    [0x0E] = KEY_GRAVE,
    [0x4E] = KEY_MINUS,
    [0x55] = KEY_EQUAL,
    [0x5C] = KEY_BACKSLASH,
    [0x66] = KEY_BACKSPACE,
    [0x29] = KEY_SPACE,
    [0x0D] = KEY_TAB,
    [0x14] = KEY_CAPS_LOCK,
    [0x12] = KEY_LEFT_SHIFT,
    [0x11] = KEY_LEFT_CTRL,
    [0x8B] = KEY_LEFT_GUI,
    [0x19] = KEY_LEFT_ALT,
    [0x59] = KEY_RIGHT_SHIFT,
    [0x58] = KEY_RIGHT_CTRL,
    [0x8C] = KEY_RIGHT_GUI,
    [0x39] = KEY_RIGHT_ALT,
    [0x8D] = KEY_LAUNCH_APPS,
    [0x5A] = KEY_ENTER,
    [0x08] = KEY_ESCAPE,
    [0x07] = KEY_F1,
    [0x0F] = KEY_F2,
    [0x17] = KEY_F3,
    [0x1F] = KEY_F4,
    [0x27] = KEY_F5,
    [0x2F] = KEY_F6,
    [0x37] = KEY_F7,
    [0x3F] = KEY_F8,
    [0x47] = KEY_F9,
    [0x4F] = KEY_F10,
    [0x56] = KEY_F11,
    [0x5E] = KEY_F12,
    [0x57] = KEY_PRINT_SCREEN,
    [0x5F] = KEY_SCROLL_LOCK,
    [0x62] = KEY_PAUSE,
    [0x54] = KEY_LEFT_BRACKET,
    [0x67] = KEY_INSERT,
    [0x6E] = KEY_HOME,
    [0x6F] = KEY_PAGE_UP,
    [0x64] = KEY_DELETE,
    [0x65] = KEY_END,
    [0x6D] = KEY_PAGE_DOWN,
    [0x63] = KEY_UP_ARROW,
    [0x61] = KEY_LEFT_ARROW,
    [0x60] = KEY_DOWN_ARROW,
    [0x6A] = KEY_RIGHT_ARROW,
    [0x76] = KEY_NUM_LOCK,
    [0x4A] = KEY_NUMPAD_SLASH,
    [0x7E] = KEY_NUMPAD_ASTERISK,
    [0x4E] = KEY_NUMPAD_MINUS,
    [0x7C] = KEY_NUMPAD_PLUS,
    [0x79] = KEY_NUMPAD_ENTER,
    [0x71] = KEY_NUMPAD_DOT,
    [0x70] = KEY_NUMPAD_0,
    [0x69] = KEY_NUMPAD_1,
    [0x72] = KEY_NUMPAD_2,
    [0x7A] = KEY_NUMPAD_3,
    [0x6B] = KEY_NUMPAD_4,
    [0x73] = KEY_NUMPAD_5,
    [0x74] = KEY_NUMPAD_6,
    [0x6C] = KEY_NUMPAD_7,
    [0x75] = KEY_NUMPAD_8,
    [0x7D] = KEY_NUMPAD_9,
    [0x5B] = KEY_RIGHT_BRACKET,
    [0x4C] = KEY_SEMICOLON,
    [0x52] = KEY_APOSTROPHE,
    [0x41] = KEY_COMMA,
    [0x49] = KEY_PERIOD,
    [0x4A] = KEY_SLASH
};

dword_t ps2_keyboard_scancode_set3_parsing_map[] = {
    PARSING_STATE_START, PARSING_STATE_IF, 0xF0, PARSING_ACTION_NEW_STATE, 0xF0,
    0xF0, PARSING_ACTION_RELEASED_KEY_CONVERT_BY_ARRAY, (dword_t)ps2_keyboard_scancode_set3_map, 0, 0,

    PARSING_STATE_START, PARSING_ACTION_PRESSED_KEY_CONVERT_BY_ARRAY, (dword_t)ps2_keyboard_scancode_set3_map, 0, 0,

    0
};

/* functions */
dword_t create_ps2_keyboard_device_entity(dword_t controller_entity, size_t controller_functions[]) {
    // create entity
    dword_t entity_number = create_entity("PS/2 Keyboard", E_TYPE_KERNEL_RING);

    // save controller info
    e_ps2_keyboard_attr_t *attr = entity_get_attr_ptr(entity_number);
    attr->device_id = get_unique_device_id();
    attr->controller_entity = controller_entity;
    attr->set_receive_function = (void (*)(dword_t, void (*)(dword_t, byte_t *, dword_t))) controller_functions[0];
    attr->send_command = (dword_t (*)(byte_t)) controller_functions[1];
    attr->send_command_with_payload = (dword_t (*)(byte_t, byte_t)) controller_functions[2];
    attr->send_command_with_return = (dword_t (*)(byte_t)) controller_functions[3];
    attr->send_command_with_payload_and_return = (dword_t (*)(byte_t, byte_t)) controller_functions[4];

    // return new device entity number
    return entity_number;
}

void initialize_ps2_keyboard(void) {
    e_ps2_keyboard_attr_t *attr = command_info->entity_attributes;
    log("\n[PS2] Initializing keyboard...");

    // set typematic rate to 500ms delay + max keys per second
    if(attr->send_command_with_payload(0xF3, (0b01 << 5) | (0 << 0)) == ERROR) {
        log("\n[PS2] ERROR: Keyboard did not set typematic rate");
        return;
    }

    // get scancode
    dword_t scancode_set = attr->send_command_with_payload_and_return(0xF0, 0);
    if(scancode_set == INVALID) {
        log("\n[PS2] ERROR: Keyboard did not send scancode set");
        return;
    }
    else if(scancode_set < 1 || scancode_set > 3) {
        log("\n[PS2] Unknown keyboard scancode set: %d\n[PS2] Trying to set scancode set 1", scancode_set);
        if(attr->send_command_with_payload(0xF0, 1) == ERROR) {
            log("\n[PS2] ERROR: Keyboard did not set scancode set 1");
            return;
        }
        scancode_set = 1;
    }
    log("\n[PS2] Keyboard scancode set: %d", scancode_set);
    attr->scancode_set = scancode_set;
    if(attr->scancode_set == 1) {
        attr->parse_map = ps2_keyboard_scancode_set1_parsing_map;
    }
    else if(attr->scancode_set == 2) {
        attr->parse_map = ps2_keyboard_scancode_set2_parsing_map;
    }
    else {
        attr->parse_map = ps2_keyboard_scancode_set3_parsing_map;
    }
    attr->parsing_state = PARSING_STATE_START;

    // start streaming
    if(attr->send_command(0xF4) == ERROR) {
        log("\n[PS2] ERROR: Keyboard did not start streaming");
        return;
    }
    attr->set_receive_function(command_info->entity_number, ps2_keyboard_receive);

    // TODO: register LED changer

    // add to group
    human_input_group_add_device(command_info->entity_number, attr->device_id, ps2_keyboard_group_functions);
    log("\n[PS2] Keyboard initialized");
}

void reset_rules(dword_t entity) {
    e_ps2_keyboard_attr_t *attr = entity_get_attr_ptr(entity);
    dword_t *parse_map = attr->parse_map;
    while(parse_map[0] != 0) {
        parse_map[1] = 2; // points to start of rule
        parse_map += parse_map[0];
    }
}

void ps2_keyboard_receive(dword_t entity, byte_t *buffer, dword_t size) {
    e_ps2_keyboard_attr_t *attr = entity_get_attr_ptr(entity);

    // parse all received bytes
    for(int i = 0; i < size; i++, buffer++) {
        // parse one byte
        dword_t *parse_map = attr->parse_map; // start of automat states
        while(TRUE) {
            // this is rule for actual state
            if(attr->parsing_state == parse_map[0]) {
                // action will be taken when received byte will compare with rule byte
                if(parse_map[1] == PARSING_STATE_IF) {
                    // received byte equals with rule byte
                    if(*buffer == parse_map[2]) {
                        // do action of this rule
                        if(parse_map[3] == PARSING_ACTION_NEW_STATE) {
                            attr->parsing_state = parse_map[4];
                            break;
                        }
                        else if(parse_map[3] == PARSING_ACTION_PRINT_SCREEN_PRESSED) {
                            ps2_keyboard_put_event(entity, KEY_PRINT_SCREEN, 1);
                            attr->parsing_state = PARSING_STATE_START;
                            break;
                        }
                        else if(parse_map[3] == PARSING_ACTION_PRINT_SCREEN_RELEASED) {
                            ps2_keyboard_put_event(entity, KEY_PRINT_SCREEN, 0);
                            attr->parsing_state = PARSING_STATE_START;
                            break;
                        }
                        else if(parse_map[3] == PARSING_ACTION_PAUSE_PRESSED) {
                            ps2_keyboard_put_event(entity, KEY_PAUSE, 1);
                            ps2_keyboard_put_event(entity, KEY_PAUSE, 0);
                            attr->parsing_state = PARSING_STATE_START;
                            break;
                        }
                        else if(parse_map[3] == PARSING_ACTION_PRESSED_KEY_CONVERT_BY_ARRAY) {
                            dword_t *map = (dword_t *) (parse_map[4]);
                            ps2_keyboard_put_event(entity, map[*buffer], 1);
                            attr->parsing_state = PARSING_STATE_START;
                            break;
                        }
                        else if(parse_map[3] == PARSING_ACTION_RELEASED_KEY_CONVERT_BY_ARRAY) {
                            dword_t *map = (dword_t *) (parse_map[4]);
                            ps2_keyboard_put_event(entity, map[*buffer], 0);
                            attr->parsing_state = PARSING_STATE_START;
                            break;
                        }
                        else if(parse_map[3] == PARSING_ACTION_PRESSED_OR_RELEASED_KEY_CONVERT_BY_ARRAY) {
                            dword_t *map = (dword_t *) (parse_map[2]);
                            if(*buffer < 0x80) {
                                ps2_keyboard_put_event(entity, map[*buffer], 1);
                            }
                            else {
                                ps2_keyboard_put_event(entity, map[*buffer - 0x80], 0);
                            }
                            attr->parsing_state = PARSING_STATE_START;
                            break;
                        }
                    }
                }
                else if(parse_map[1] == PARSING_ACTION_PRESSED_KEY_CONVERT_BY_ARRAY) {
                    dword_t *map = (dword_t *) (parse_map[2]);
                    ps2_keyboard_put_event(entity, map[*buffer], 1);
                    attr->parsing_state = PARSING_STATE_START;
                    break;
                }
                else if(parse_map[1] == PARSING_ACTION_RELEASED_KEY_CONVERT_BY_ARRAY) {
                    dword_t *map = (dword_t *) (parse_map[2]);
                    ps2_keyboard_put_event(entity, map[*buffer], 0);
                    attr->parsing_state = PARSING_STATE_START;
                    break;
                }
                else if(parse_map[1] == PARSING_ACTION_PRESSED_OR_RELEASED_KEY_CONVERT_BY_ARRAY) {
                    dword_t *map = (dword_t *) (parse_map[2]);
                    if(*buffer < 0x80) {
                        ps2_keyboard_put_event(entity, map[*buffer], 1);
                    }
                    else {
                        ps2_keyboard_put_event(entity, map[*buffer - 0x80], 0);
                    }
                    attr->parsing_state = PARSING_STATE_START;
                    break;
                }
            }

            // go to next state description
            parse_map += 5;

            // we are at end of known states, so we are at unknown state, reset
            if(parse_map[0] == 0) {
                attr->parsing_state = PARSING_STATE_START;
                break;
            }
        }
    }
}

void ps2_keyboard_put_event(dword_t entity, dword_t key, dword_t value) {
    e_ps2_keyboard_attr_t *attr = entity_get_attr_ptr(entity);
    attr->key_state[key] = value;
    human_input_insert_value_to_packet(key, value);
    human_input_end_of_packet();
}

dword_t ps2_keyboard_is_key_pressed(dword_t entity, dword_t key) {
    e_ps2_keyboard_attr_t *attr = entity_get_attr_ptr(entity);
    return attr->key_state[key];
}

dword_t ps2_keyboard_set_leds(dword_t entity, leds_t leds) {
    // TODO:
    return FALSE;
}