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
#include <libc/string.h>
#include <kernel/hardware/groups/logging/logging.h>
#include <kernel/hardware/subsystems/windows/windows.h>
#include <kernel/memory/memory_allocators.h>
#include <kernel/cpu/mutex.h>
#include <kernel/hardware/main.h>
#include <kernel/cpu/scheduler.h>
#include <kernel/cpu/commands.h>
#include <kernel/cpu/info.h>
#include <kernel/software/syscall.h>

/* local variables */
human_input_group_t *human_input_group;
const uint16_t key_to_unicode[INPUT_KEY_COUNT] = {
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
uint32_t human_input_group_id;

/* functions */
void initialize_human_input_group(void) {
    human_input_group = kalloc(sizeof(human_input_group_t));
    human_input_group_id = get_unique_hardware_id();
    add_device_to_hardware_list(human_input_group_id, HARDWARE_TYPE_HUMAN_INPUT_DEVICE);
}

void add_human_input_group_device(uint32_t type, void *structure) {
    if(human_input_group->number_of_devices >= MAX_NUMBER_OF_HUMAN_INPUT_DEVICES) {
        return;
    }
    human_input_group->devices[human_input_group->number_of_devices].type = type;
    human_input_group->devices[human_input_group->number_of_devices].structure = structure;
    human_input_group->number_of_devices++;
}

void human_input_event_pressed_key(uint32_t key) {
    if(human_input_group->number_of_pressed_keys >= INPUT_KEY_COUNT) {
        return;
    }

    LOCK_MUTEX(&human_input_group->spinlock);
    human_input_group->pressed_keys[human_input_group->number_of_pressed_keys] = key;
    human_input_group->number_of_pressed_keys++;
    UNLOCK_MUTEX(&human_input_group->spinlock);
}

void human_input_event_released_key(uint32_t key) {
    if(human_input_group->number_of_released_keys >= INPUT_KEY_COUNT) {
        return;
    }

    LOCK_MUTEX(&human_input_group->spinlock);
    human_input_group->released_keys[human_input_group->number_of_released_keys] = key;
    human_input_group->number_of_released_keys++;
    UNLOCK_MUTEX(&human_input_group->spinlock);
}

void human_input_end_of_event(void) {
    // load where is event stack of current window
    process_t *process = NULL;
    human_input_event_stack_t *event_stack = NULL;
    if(current_window == NULL) {
        human_input_group->number_of_pressed_keys = 0;
        human_input_group->number_of_released_keys = 0;
        return;
    }
    process = current_window->process;
    if(process == NULL) {
        human_input_group->number_of_pressed_keys = 0;
        human_input_group->number_of_released_keys = 0;
        return;
    }
    event_stack = process->human_input_event_stack;
    if(event_stack == NULL) {
        human_input_group->number_of_pressed_keys = 0;
        human_input_group->number_of_released_keys = 0;
        return;
    }

    // switch to process virtual space
    uint32_t actual_page_directory = read_cr3();
    if(process->page_directory_physical_address != actual_page_directory) {
        write_cr3(process->page_directory_physical_address);
    }

    // add events to stack
    LOCK_MUTEX(&event_stack->spinlock);
    uint32_t next_producer = (event_stack->producer + 1) % MAX_NUMBER_OF_HUMAN_INPUT_EVENTS;
    if(next_producer != event_stack->consumer) {
        // add all pressed keys
        for(uint32_t i = 0; i < human_input_group->number_of_pressed_keys; i++) {
            next_producer = (event_stack->producer + 1) % MAX_NUMBER_OF_HUMAN_INPUT_EVENTS;
            if(next_producer == event_stack->consumer) {
                break;
            }
            event_stack->stack[event_stack->producer].type = HUMAN_INPUT_EVENT_KEY_PRESSED;
            event_stack->stack[event_stack->producer].value = human_input_group->pressed_keys[i];
            event_stack->producer = next_producer;
        }

        // add all released keys
        for(uint32_t i = 0; i < human_input_group->number_of_released_keys; i++) {
            next_producer = (event_stack->producer + 1) % MAX_NUMBER_OF_HUMAN_INPUT_EVENTS;
            if(next_producer == event_stack->consumer) {
                break;
            }
            event_stack->stack[event_stack->producer].type = HUMAN_INPUT_EVENT_KEY_RELEASED;
            event_stack->stack[event_stack->producer].value = human_input_group->released_keys[i];
            event_stack->producer = next_producer;
        }
    }
    UNLOCK_MUTEX(&event_stack->spinlock);

    // switch back from process virtual space
    if(process->page_directory_physical_address != actual_page_directory) {
        write_cr3(actual_page_directory);
    }

    // reset counter
    human_input_group->number_of_pressed_keys = 0;
    human_input_group->number_of_released_keys = 0;
}

/* userspace functions */
void human_input_group_process_userspace_command(uint32_t device_id, human_input_group_command_t *command) {
    if(device_id != human_input_group_id) {
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
            get_current_logical_processor_struct()->current_process->human_input_event_stack = event_stack;
            break;
        }
        case HUMAN_INPUT_GROUP_STOP_LISTENING_TO_EVENTS: {
            get_current_logical_processor_struct()->current_process->human_input_event_stack = NULL;
            break;
        }
    }
}