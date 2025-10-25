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
#include <hardware/groups/logging/logging.h>
#include <kernel/x86/cpu/commands.h>
#include <kernel/x86/entities/entities.h>
#include <kernel/x86/interrupt_controllers/main.h>
#include <kernel/x86/scheduler/lock.h>
#include <kernel/x86/libc/string.h>

#include <hardware/devices/human_input/ps2_keyboard.h>

/* local variables */
dword_t human_input_entity;
e_human_input_group_t *human_input_attr;

const word_t key_to_unicode[INPUT_KEY_COUNT] = {
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

/* functions */
void initialize_human_input_group(void) {
    human_input_entity = create_entity("Human Input group", E_TYPE_KERNEL_RING);
    human_input_attr = entity_get_attr_ptr(human_input_entity);
}

void human_input_group_add_device(dword_t device_entity, dword_t device_id, human_input_device_functions_t *functions) {
    // check if there is free space for device
    if(human_input_attr->number_of_devices >= HUMAN_INPUT_MAX_NUMBER_OF_DEVICES) {
        log("\n[ERROR] Too many Human Input devices");
        return;
    }

    // save number of new device
    dword_t arr_num = human_input_attr->number_of_devices;
    human_input_attr->number_of_devices++;

    // save functions
    human_input_attr->devices[arr_num] = device_entity;
    human_input_attr->ids[arr_num] = device_id;
    human_input_attr->functions[arr_num] = functions;
}

void human_input_insert_value_to_packet(dword_t key, dword_t value) {
    // check if stack is full
    LOCK_MUTEX(&human_input_attr->event_stack_mutex);
    dword_t new_event_stack_producer = ((human_input_attr->event_stack_producer + 1) % HUMAN_INPUT_SIZE_OF_EVENT_STACK);
    if(new_event_stack_producer == human_input_attr->event_stack_consumer) {
        UNLOCK_MUTEX(&human_input_attr->event_stack_mutex);
        return;
    }

    // write value into stack
    human_input_attr->event_stack[human_input_attr->event_stack_producer].packet_toggle = human_input_attr->event_stack_toggle;
    human_input_attr->event_stack[human_input_attr->event_stack_producer].key = key;
    human_input_attr->event_stack[human_input_attr->event_stack_producer].value = value;
    human_input_attr->event_stack_producer = new_event_stack_producer;
    UNLOCK_MUTEX(&human_input_attr->event_stack_mutex);
}

void human_input_end_of_packet(void) {
    human_input_attr->event_stack_toggle = ((human_input_attr->event_stack_toggle + 1) & 0x1);
}

dword_t human_input_has_value(void) {
    if(human_input_attr->event_stack_producer == human_input_attr->event_stack_consumer) {
        return FALSE;
    }
    else {
        return TRUE;
    }
}

void human_input_pop_event(void) {
    // reset actual event state
    human_input_attr->new_pressed_key = 0;
    human_input_attr->new_pressed_key_unicode = 0;
    memset(&human_input_attr->keys_of_last_event, 0, sizeof(human_input_attr->keys_of_last_event));

    // pop event
    dword_t toggle = human_input_attr->event_stack[human_input_attr->event_stack_consumer].packet_toggle;
    while(  toggle == human_input_attr->event_stack[human_input_attr->event_stack_consumer].packet_toggle
            && human_input_attr->event_stack_producer != human_input_attr->event_stack_consumer) {
        // update state
        dword_t changed_key = human_input_attr->event_stack[human_input_attr->event_stack_consumer].key;
        dword_t new_value = human_input_attr->event_stack[human_input_attr->event_stack_consumer].value;
        human_input_attr->keys_of_last_event[changed_key] = new_value;

        // keyboard key
        if(changed_key < MOUSE_BUTTON_LEFT && new_value == 1) {
            human_input_attr->new_pressed_key = changed_key;
            human_input_attr->new_pressed_key_unicode = human_input_get_unicode_of_key(changed_key);
        }

        // move consumer
        human_input_attr->event_stack_consumer = ((human_input_attr->event_stack_consumer + 1) % HUMAN_INPUT_SIZE_OF_EVENT_STACK);
    }

    // TODO: update diacritic keys state

    // TODO: change leds
}

dword_t human_input_get_unicode_of_key(dword_t key) {
    return key_to_unicode[key];
}

dword_t human_input_is_key_pressed(dword_t key) {
    // TODO: trace pressed keys by events, not by last state

    for(int i = 0; i < human_input_attr->number_of_devices; i++) {
        if(human_input_attr->functions[i]->is_key_pressed(human_input_attr->devices[i], key) == TRUE) {
            return TRUE;
        }
    }

    return FALSE;
}

dword_t human_input_last_event_get_pressed_key(void) {
    return human_input_attr->new_pressed_key;
}

dword_t human_input_last_event_get_pressed_key_unicode(void) {
    return human_input_attr->new_pressed_key_unicode;
}