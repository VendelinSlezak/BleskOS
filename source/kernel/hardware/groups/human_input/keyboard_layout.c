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
#include <kernel/hardware/groups/human_input/human_input.h>
#include <kernel/libc/string.h>

/* global variables */
uint8_t *keyboard_layout;

/* local variables */
uint32_t *keyboard_no_shift_to_unicode;
uint32_t *keyboard_with_shift_to_unicode;

uint32_t en_keyboard_no_shift_to_unicode[INPUT_KEY_COUNT] = {
    [KEY_A] = L'a', [KEY_B] = L'b', [KEY_C] = L'c', [KEY_D] = L'd', [KEY_E] = L'e',
    [KEY_F] = L'f', [KEY_G] = L'g', [KEY_H] = L'h', [KEY_I] = L'i', [KEY_J] = L'j',
    [KEY_K] = L'k', [KEY_L] = L'l', [KEY_M] = L'm', [KEY_N] = L'n', [KEY_O] = L'o',
    [KEY_P] = L'p', [KEY_Q] = L'q', [KEY_R] = L'r', [KEY_S] = L's', [KEY_T] = L't',
    [KEY_U] = L'u', [KEY_V] = L'v', [KEY_W] = L'w', [KEY_X] = L'x', [KEY_Y] = L'y',
    [KEY_Z] = L'z',

    [KEY_0] = L'0', [KEY_1] = L'1', [KEY_2] = L'2', [KEY_3] = L'3', [KEY_4] = L'4',
    [KEY_5] = L'5', [KEY_6] = L'6', [KEY_7] = L'7', [KEY_8] = L'8', [KEY_9] = L'9',

    [KEY_GRAVE] = L'`',
    [KEY_MINUS] = L'-',
    [KEY_EQUAL] = L'=',
    [KEY_LEFT_BRACKET] = L'[',
    [KEY_RIGHT_BRACKET] = L']',
    [KEY_BACKSLASH] = L'\\',
    [KEY_SEMICOLON] = L';',
    [KEY_APOSTROPHE] = L'\'',
    [KEY_COMMA] = L',',
    [KEY_PERIOD] = L'.',
    [KEY_SLASH] = L'/',
    [KEY_SPACE] = L' ',
    [KEY_TAB] = L'\t',
    [KEY_ENTER] = L'\n',

    [KEY_NUMPAD_0] = L'0', [KEY_NUMPAD_1] = L'1', [KEY_NUMPAD_2] = L'2',
    [KEY_NUMPAD_3] = L'3', [KEY_NUMPAD_4] = L'4', [KEY_NUMPAD_5] = L'5',
    [KEY_NUMPAD_6] = L'6', [KEY_NUMPAD_7] = L'7', [KEY_NUMPAD_8] = L'8',
    [KEY_NUMPAD_9] = L'9', [KEY_NUMPAD_DOT] = L'.',
    [KEY_NUMPAD_PLUS] = L'+',
    [KEY_NUMPAD_MINUS] = L'-',
    [KEY_NUMPAD_ASTERISK] = L'*',
    [KEY_NUMPAD_SLASH] = L'/',
};
uint32_t en_keyboard_with_shift_to_unicode[INPUT_KEY_COUNT] = {
    [KEY_A] = L'A', [KEY_B] = L'B', [KEY_C] = L'C', [KEY_D] = L'D', [KEY_E] = L'E',
    [KEY_F] = L'F', [KEY_G] = L'G', [KEY_H] = L'H', [KEY_I] = L'I', [KEY_J] = L'J',
    [KEY_K] = L'K', [KEY_L] = L'L', [KEY_M] = L'M', [KEY_N] = L'N', [KEY_O] = L'O',
    [KEY_P] = L'P', [KEY_Q] = L'Q', [KEY_R] = L'R', [KEY_S] = L'S', [KEY_T] = L'T',
    [KEY_U] = L'U', [KEY_V] = L'V', [KEY_W] = L'W', [KEY_X] = L'X', [KEY_Y] = L'Y',
    [KEY_Z] = L'Z',

    [KEY_0] = L')', [KEY_1] = L'!', [KEY_2] = L'@', [KEY_3] = L'#', [KEY_4] = L'$',
    [KEY_5] = L'%', [KEY_6] = L'^', [KEY_7] = L'&', [KEY_8] = L'*', [KEY_9] = L'(',

    [KEY_GRAVE] = L'~',
    [KEY_MINUS] = L'_',
    [KEY_EQUAL] = L'+',
    [KEY_LEFT_BRACKET] = L'{',
    [KEY_RIGHT_BRACKET] = L'}',
    [KEY_BACKSLASH] = L'|',
    [KEY_SEMICOLON] = L':',
    [KEY_APOSTROPHE] = L'"',
    [KEY_COMMA] = L'<',
    [KEY_PERIOD] = L'>',
    [KEY_SLASH] = L'?',
    [KEY_SPACE] = L' ',
    [KEY_TAB] = L'\t',
    [KEY_ENTER] = L'\n',

    [KEY_NUMPAD_0] = L'0', [KEY_NUMPAD_1] = L'1', [KEY_NUMPAD_2] = L'2',
    [KEY_NUMPAD_3] = L'3', [KEY_NUMPAD_4] = L'4', [KEY_NUMPAD_5] = L'5',
    [KEY_NUMPAD_6] = L'6', [KEY_NUMPAD_7] = L'7', [KEY_NUMPAD_8] = L'8',
    [KEY_NUMPAD_9] = L'9', [KEY_NUMPAD_DOT] = L'.',
    [KEY_NUMPAD_PLUS] = L'+',
    [KEY_NUMPAD_MINUS] = L'-',
    [KEY_NUMPAD_ASTERISK] = L'*',
    [KEY_NUMPAD_SLASH] = L'/',
};

uint32_t sk_keyboard_no_shift_to_unicode[INPUT_KEY_COUNT] = {
    [KEY_A] = L'a', [KEY_B] = L'b', [KEY_C] = L'c', [KEY_D] = L'd', [KEY_E] = L'e',
    [KEY_F] = L'f', [KEY_G] = L'g', [KEY_H] = L'h', [KEY_I] = L'i', [KEY_J] = L'j',
    [KEY_K] = L'k', [KEY_L] = L'l', [KEY_M] = L'm', [KEY_N] = L'n', [KEY_O] = L'o',
    [KEY_P] = L'p', [KEY_Q] = L'q', [KEY_R] = L'r', [KEY_S] = L's', [KEY_T] = L't',
    [KEY_U] = L'u', [KEY_V] = L'v', [KEY_W] = L'w', [KEY_X] = L'x', 
    [KEY_Y] = L'z',
    [KEY_Z] = L'y',

    [KEY_1] = L'+', [KEY_2] = L'ľ', [KEY_3] = L'š', [KEY_4] = L'č', [KEY_5] = L'ť',
    [KEY_6] = L'ž', [KEY_7] = L'ý', [KEY_8] = L'á', [KEY_9] = L'í', [KEY_0] = L'é',

    [KEY_GRAVE] = L';',
    [KEY_MINUS] = L'´',
    [KEY_EQUAL] = L'ˇ',
    [KEY_LEFT_BRACKET] = L'ú',
    [KEY_RIGHT_BRACKET] = L'ä',
    [KEY_BACKSLASH] = L'ň',
    [KEY_SEMICOLON] = L'ô',
    [KEY_APOSTROPHE] = L'§',
    [KEY_COMMA] = L',',
    [KEY_PERIOD] = L'.',
    [KEY_SLASH] = L'-',
    [KEY_SPACE] = L' ',
    [KEY_TAB] = L'\t',
    [KEY_ENTER] = L'\n',

    [KEY_NUMPAD_0] = L'0', [KEY_NUMPAD_1] = L'1', [KEY_NUMPAD_2] = L'2',
    [KEY_NUMPAD_3] = L'3', [KEY_NUMPAD_4] = L'4', [KEY_NUMPAD_5] = L'5',
    [KEY_NUMPAD_6] = L'6', [KEY_NUMPAD_7] = L'7', [KEY_NUMPAD_8] = L'8',
    [KEY_NUMPAD_9] = L'9', [KEY_NUMPAD_DOT] = L',',
    [KEY_NUMPAD_PLUS] = L'+',
    [KEY_NUMPAD_MINUS] = L'-',
    [KEY_NUMPAD_ASTERISK] = L'*',
    [KEY_NUMPAD_SLASH] = L'/',
};

uint32_t sk_keyboard_with_shift_to_unicode[INPUT_KEY_COUNT] = {
    [KEY_A] = L'A', [KEY_B] = L'B', [KEY_C] = L'C', [KEY_D] = L'D', [KEY_E] = L'E',
    [KEY_F] = L'F', [KEY_G] = L'G', [KEY_H] = L'H', [KEY_I] = L'I', [KEY_J] = L'J',
    [KEY_K] = L'K', [KEY_L] = L'L', [KEY_M] = L'M', [KEY_N] = L'N', [KEY_O] = L'O',
    [KEY_P] = L'P', [KEY_Q] = L'Q', [KEY_R] = L'R', [KEY_S] = L'S', [KEY_T] = L'T',
    [KEY_U] = L'U', [KEY_V] = L'V', [KEY_W] = L'W', [KEY_X] = L'X', 
    [KEY_Y] = L'Z',
    [KEY_Z] = L'Y',

    [KEY_1] = L'1', [KEY_2] = L'2', [KEY_3] = L'3', [KEY_4] = L'4', [KEY_5] = L'5',
    [KEY_6] = L'6', [KEY_7] = L'7', [KEY_8] = L'8', [KEY_9] = L'9', [KEY_0] = L'0',

    [KEY_GRAVE] = L'°',
    [KEY_MINUS] = L'v',
    [KEY_EQUAL] = L'°',
    [KEY_LEFT_BRACKET] = L'/',
    [KEY_RIGHT_BRACKET] = L'(',
    [KEY_BACKSLASH] = L')',
    [KEY_SEMICOLON] = L'"',
    [KEY_APOSTROPHE] = L'!',
    [KEY_COMMA] = L'?',
    [KEY_PERIOD] = L':',
    [KEY_SLASH] = L'_',
    [KEY_SPACE] = L' ',
    [KEY_TAB] = L'\t',
    [KEY_ENTER] = L'\n',

    [KEY_NUMPAD_0] = L'0', [KEY_NUMPAD_1] = L'1', [KEY_NUMPAD_2] = L'2',
    [KEY_NUMPAD_3] = L'3', [KEY_NUMPAD_4] = L'4', [KEY_NUMPAD_5] = L'5',
    [KEY_NUMPAD_6] = L'6', [KEY_NUMPAD_7] = L'7', [KEY_NUMPAD_8] = L'8',
    [KEY_NUMPAD_9] = L'9', [KEY_NUMPAD_DOT] = L',',
    [KEY_NUMPAD_PLUS] = L'+',
    [KEY_NUMPAD_MINUS] = L'-',
    [KEY_NUMPAD_ASTERISK] = L'*',
    [KEY_NUMPAD_SLASH] = L'/',
};

/* functions */
uint32_t get_key_unicode_value(uint32_t key) {
    uint32_t *key_to_unicode = keyboard_no_shift_to_unicode;
    if(    (human_input_global_state.leds.capslock == false && (human_input_global_state.key_state[KEY_LEFT_SHIFT] == KEY_PRESSED || human_input_global_state.key_state[KEY_RIGHT_SHIFT] == KEY_PRESSED))
        || (human_input_global_state.leds.capslock == true && (human_input_global_state.key_state[KEY_LEFT_SHIFT] == KEY_RELEASED && human_input_global_state.key_state[KEY_RIGHT_SHIFT] == KEY_RELEASED))) {
        key_to_unicode = keyboard_with_shift_to_unicode;
    }
    return key_to_unicode[key];
}

void set_keyboard_layout(uint8_t *layout) {
    if(strcmp(layout, "EN") == 0) {
        keyboard_no_shift_to_unicode = en_keyboard_no_shift_to_unicode;
        keyboard_with_shift_to_unicode = en_keyboard_with_shift_to_unicode;
    }
    else if(strcmp(layout, "SK") == 0) {
        keyboard_no_shift_to_unicode = sk_keyboard_no_shift_to_unicode;
        keyboard_with_shift_to_unicode = sk_keyboard_with_shift_to_unicode;
    }
    else {
        return;
    }
    keyboard_layout = layout;
}