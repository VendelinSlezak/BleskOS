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
#include <kernel/libc/string.h>
#include <kernel/hardware/groups/logging/logging.h>
#include <kernel/hardware/subsystems/windows/windows.h>
#include <kernel/hardware/devices/memory/memory_allocators.h>
#include <kernel/hardware/devices/cpu/mutex.h>
#include <kernel/hardware/main.h>
#include <kernel/hardware/devices/cpu/scheduler.h>
#include <kernel/hardware/devices/cpu/commands.h>
#include <kernel/hardware/devices/cpu/info.h>
#include <kernel/software/syscall.h>
#include <kernel/hardware/subsystems/screen/screen.h>
#include <kernel/hardware/devices/timers/main.h>

/* global variables */
human_input_global_state_t human_input_global_state;

/* local variables */
human_input_group_t *human_input_group;
mutex_t human_input_group_global_event_mutex;
uint32_t microseconds_to_repeat_key_first_time = 300000;
uint32_t microseconds_to_repeat_key_next_time = 50000;
uint32_t *key_to_unicode = NULL;

uint32_t keyboard_no_shift_to_unicode[INPUT_KEY_COUNT] = {
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
uint32_t keyboard_with_shift_to_unicode[INPUT_KEY_COUNT] = {
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

/* functions */
void initialize_human_input_group(void) {
    human_input_group = kalloc(sizeof(human_input_group_t));
    add_virtual_device_to_hardware_list(VIRTUAL_HARDWARE_HUMAN_INPUT_DEVICE);
    create_kernel_thread((uint32_t)check_human_input_state, 0, 0);
}

void add_human_input_device(hardware_t *device, human_input_group_device_functions_t *functions) {
    if(human_input_group->number_of_devices >= MAX_NUMBER_OF_HUMAN_INPUT_DEVICES) {
        return;
    }
    human_input_group->devices[human_input_group->number_of_devices].device = device;
    human_input_group->devices[human_input_group->number_of_devices].functions = functions;
    human_input_group->number_of_devices++;
}

void remove_human_input_device(hardware_t *device) {
    for(int i = 0; i < human_input_group->number_of_devices; i++) {
        if(human_input_group->devices[i].device == device) {
            for(int j = i; j < human_input_group->number_of_devices - 1; j++) {
                human_input_group->devices[j] = human_input_group->devices[j + 1];
            }
            human_input_group->number_of_devices--;
            break;
        }
    }
}

/* userspace functions */
void human_input_group_process_userspace_command(human_input_group_command_t *command) {
    if(return_validated_pointer(command, sizeof(human_input_group_command_t)) == NULL) {
        return;
    }

    switch(command->type) {
        case HUMAN_INPUT_GROUP_LISTEN_TO_EVENTS: {
            if(return_validated_pointer(command, sizeof(human_input_group_command_t) + sizeof(uint32_t)) == NULL) {
                break;
            }
            human_input_event_stack_t *event_stack = (human_input_event_stack_t *) command->argument[0];
            if(return_validated_pointer(event_stack, sizeof(human_input_event_stack_t)) == NULL) {
                return;
            }
            program_t *program = get_current_logical_processor_struct()->current_program;
            program->page_directory_for_human_input_event_stack = read_cr3();
            program->human_input_event_stack = event_stack;
            break;
        }
        case HUMAN_INPUT_GROUP_STOP_LISTENING_TO_EVENTS: {
            program_t *program = get_current_logical_processor_struct()->current_program;
            program->page_directory_for_human_input_event_stack = NULL;
            program->human_input_event_stack = NULL;
            break;
        }
    }
}

void hid_reset_state_before_new_events(human_input_device_state_t *state) {
    state->movement_state = NO_MOVEMENT_CHANGE;
}

void hid_local_event_key(human_input_device_state_t *state, uint32_t key, uint32_t value) {
    state->key_state[key] = value;
}

void hid_local_event_button(human_input_device_state_t *state, uint32_t button, uint32_t value) {
    state->button_state[button] = value;
}

void hid_local_event_movement(human_input_device_state_t *state, uint32_t movement, uint32_t value) {
    state->movement_value[movement] = value;
    state->movement_state = NEW_MOVEMENT_CHANGE;
}

void hid_process_changes_of_local_state(human_input_device_state_t *state) {
    LOCK_MUTEX(&human_input_group_global_event_mutex);

    // get actual global state
    human_input_device_state_t actual_global_state = {0};
    for(int i = 0; i < human_input_group->number_of_devices; i++) {
        human_input_device_state_t *device_state = human_input_group->devices[i].functions->get_state(human_input_group->devices[i].device);
        for(int j = 0; j < INPUT_KEY_COUNT; j++) {
            actual_global_state.key_state[j] |= device_state->key_state[j];
        }
        for(int j = 0; j < INPUT_BUTTONS_COUNT; j++) {
            actual_global_state.button_state[j] |= device_state->button_state[j];
        }
    }
    if(state->movement_state == NEW_MOVEMENT_CHANGE) {
        actual_global_state.movement_state = NEW_MOVEMENT_CHANGE;
        for(int j = 0; j < INPUT_MOVEMENTS_COUNT; j++) {
            actual_global_state.movement_value[j] = state->movement_value[j];
        }
    }

    // update global state and fire all new global events
    for(int i = 0; i < INPUT_KEY_COUNT; i++) {
        if(human_input_global_state.key_state[i] != actual_global_state.key_state[i]) {
            human_input_global_state.key_state[i] = actual_global_state.key_state[i];
            human_input_global_state.key_state_timestamp[i] = (*get_time_in_microseconds)();

            key_to_unicode = keyboard_no_shift_to_unicode;
            if(    (human_input_global_state.leds.capslock == false && (human_input_global_state.key_state[KEY_LEFT_SHIFT] == KEY_PRESSED || human_input_global_state.key_state[KEY_RIGHT_SHIFT] == KEY_PRESSED))
                || (human_input_global_state.leds.capslock == true && (human_input_global_state.key_state[KEY_LEFT_SHIFT] == KEY_RELEASED && human_input_global_state.key_state[KEY_RIGHT_SHIFT] == KEY_RELEASED))) {
                key_to_unicode = keyboard_with_shift_to_unicode;
            }
            uint32_t unicode_value = key_to_unicode[i];

            if(human_input_global_state.last_pressed_key == i && human_input_global_state.key_state[i] == KEY_RELEASED) {
                human_input_global_state.last_pressed_key = 0;
                human_input_global_state.last_pressed_key_event_count = 0;
            }
            if(human_input_global_state.key_state[i] == KEY_PRESSED) {
                if(i == KEY_CAPS_LOCK) {
                    human_input_global_state.leds.capslock = !human_input_global_state.leds.capslock;
                }
                else {
                    human_input_global_state.last_pressed_key = i;
                    human_input_global_state.last_pressed_key_unicode_value = unicode_value;
                    human_input_global_state.last_pressed_key_event_count = 0;
                }
            }

            screen_subsystem_event_keyboard_key(i, human_input_global_state.key_state[i], unicode_value);
        }
    }
    if(state->movement_state == NEW_MOVEMENT_CHANGE) {
        for(int i = 0; i < INPUT_BUTTONS_COUNT; i++) {
            if(human_input_global_state.button_state[i] == BUTTON_PRESSED) {
                human_input_global_state.button_state[i] = BUTTON_DRAGGED;
            }
        }
        for(int i = 0; i < INPUT_MOVEMENTS_COUNT; i++) {
            human_input_global_state.movement_value[i] = state->movement_value[i];
        }
        screen_subsystem_event_mouse_movement();
    }
    for(int i = 0; i < INPUT_BUTTONS_COUNT; i++) {
        switch(human_input_global_state.button_state[i]) {
            case BUTTON_RELEASED: {
                if(actual_global_state.button_state[i] == BUTTON_RELEASED) {
                    break;
                }
                human_input_global_state.button_state[i] = BUTTON_PRESSED;
                human_input_global_state.button_state_timestamp[i] = (*get_time_in_microseconds)();
                screen_subsystem_event_mouse_button(i, BUTTON_PRESSED);
                break;
            }
            case BUTTON_PRESSED:
            case BUTTON_DRAGGED: {
                if(actual_global_state.button_state[i] == BUTTON_PRESSED) {
                    break;
                }
                human_input_global_state.button_state[i] = BUTTON_RELEASED;
                human_input_global_state.button_state_timestamp[i] = (*get_time_in_microseconds)();
                screen_subsystem_event_mouse_button(i, BUTTON_RELEASED);
                break;
            }
        }
    }

    UNLOCK_MUTEX(&human_input_group_global_event_mutex);
}

// TODO: thread that will repeat firing of key events
void check_human_input_state(void) {
    while(true) {
        for(int i = 0; i < human_input_group->number_of_devices; i++) {
            if(human_input_group->devices[i].functions->keyboard_set_leds == NULL) {
                continue;
            }

            uint32_t *global_leds = (uint32_t *) &human_input_global_state.leds;
            uint32_t *device_leds = (uint32_t *) &human_input_group->devices[i].functions->get_state(human_input_group->devices[i].device)->leds;
            if(*global_leds != *device_leds) {
                human_input_group->devices[i].functions->keyboard_set_leds(human_input_group->devices[i].device, human_input_global_state.leds);
            }
        }

        uint32_t key = human_input_global_state.last_pressed_key;
        uint32_t key_unicode_value = human_input_global_state.last_pressed_key_unicode_value;
        uint32_t key_state = human_input_global_state.key_state[key];
        uint32_t key_timestamp = human_input_global_state.key_state_timestamp[key];
        uint32_t key_event_count = human_input_global_state.last_pressed_key_event_count;

        if(key == 0) {
            switch_to_another_thread();
            continue;
        }
        if(key_state == KEY_RELEASED) {
            human_input_global_state.last_pressed_key = 0;
            human_input_global_state.last_pressed_key_event_count = 0;
            switch_to_another_thread();
            continue;
        }

        uint32_t actual_timestamp = (*get_time_in_microseconds)();
        uint32_t diff = (actual_timestamp - key_timestamp);
        if(diff < microseconds_to_repeat_key_first_time) {
            switch_to_another_thread();
            continue;
        }
        diff -= microseconds_to_repeat_key_first_time;
        diff /= microseconds_to_repeat_key_next_time;
        if(key_event_count < diff) {
            human_input_global_state.last_pressed_key_event_count = diff;
            screen_subsystem_event_keyboard_key(key, KEY_PRESSED, key_unicode_value);
        }

        if(human_input_global_state.key_state[key] == KEY_RELEASED) {
            human_input_global_state.last_pressed_key = 0;
            human_input_global_state.last_pressed_key_event_count = 0;
        }

        switch_to_another_thread();
    }
}

uint32_t is_key_pressed(uint32_t key) {
    return (human_input_global_state.key_state[key] == KEY_PRESSED);
}