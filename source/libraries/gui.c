/* 
* BleskOS
*
* MIT License
* Copyright (c) 2023-2026 BleskOS developers
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

/* includes */
#include <kernel/hardware/subsystems/windows/windows.h>
#include <kernel/hardware/groups/human_input/human_input.h>
#include <kernel/hardware/main.h>
#include <kernel/software/syscall.h>
#include <libraries/main.h>
#include <libraries/screen.h>

/* global variables */
screen_buffer_t *window_buffer = NULL;
hardware_list_entry_t *window_subsystem = NULL;
hardware_list_entry_t *human_input = NULL;

/* local variables */
human_input_event_stack_t human_input_event_stack;
uint8_t listener_end_signal;
void(*pressed_key_event_handlers[INPUT_KEY_COUNT])(void) = {0};
uint32_t number_of_areas = 0;
gui_area_t *gui_areas = NULL;

/* functions */
screen_buffer_t *syslib_initialize_gui(void) {
    window_buffer = syslib->create_window();
    gui_areas = (gui_area_t *) syslib->calloc(sizeof(gui_area_t), 1);
    number_of_areas = 1;

    if(human_input != NULL) {
        syscall_create_thread(gui_event_listener, (uint32_t)syslib->malloc(4096) + 4096, &listener_end_signal);
    }

    return window_buffer;
}

screen_buffer_t *syslib_create_window(void) {
    if(window_subsystem == NULL) {
        return NULL;
    }
    static windows_subsystem_command_t command = {
        .type = WINDOWS_SUBSYSTEM_CREATE_WINDOW,
        .argument = {
            0, 0
        }
    };
    syscall_send_command_to_device(window_subsystem, &command);
    syslib->window_width = command.argument[0];
    syslib->window_height = command.argument[1];
    window_buffer = syslib_create_screen_buffer(syslib->window_width, syslib->window_height);
    return window_buffer;
}

void syslib_redraw_window(void) {
    if(window_subsystem == NULL) {
        return;
    }
    static windows_subsystem_command_t command = {
        .type = WINDOWS_SUBSYSTEM_REDRAW_WINDOW,
        .argument = {
            0
        }
    };
    command.argument[0] = (uint32_t) &window_buffer->buffer;
    syscall_send_command_to_device(window_subsystem, &command);
}

void syslib_add_canvas_component(uint32_t area_number, void (*redraw)(uint32_t screen_x, uint32_t screen_y, uint32_t screen_width, uint32_t screen_height, uint32_t x_offset, uint32_t y_offset)) {
    if(area_number >= number_of_areas) {
        return;
    }

    // TODO: calculate exact position in area and initialize variables for next components
    canvas_component_t *canvas = (canvas_component_t *) syslib->calloc(sizeof(canvas_component_t), 1);
    canvas->x = 0;
    canvas->y = 0;
    canvas->width = window_buffer->width;
    canvas->height = window_buffer->height;
    canvas->x_offset = 0;
    canvas->y_offset = 0;
    canvas->redraw = redraw;

    gui_area_t *area = &gui_areas[area_number];
    area->number_of_components++;
    area->components = (component_t *) syslib->realloc(area->components, sizeof(component_t) * area->number_of_components);
    area->components[area->number_of_components - 1].type = CANVAS_COMPONENT;
    area->components[area->number_of_components - 1].structure = canvas;
}

void syslib_register_pressed_key_event_handler(uint32_t key, void (*handler)(void)) {
    pressed_key_event_handlers[key] = handler;
}

void syslib_redraw_gui(void) {
    for(uint32_t i = 0; i < number_of_areas; i++) {
        gui_area_t *area = &gui_areas[i];
        for(uint32_t j = 0; j < area->number_of_components; j++) {
            if(area->components[j].type == CANVAS_COMPONENT) {
                canvas_component_t *component = (canvas_component_t *) area->components[j].structure;
                component->redraw(component->x, component->y, component->width, component->height, component->x_offset, component->y_offset);
            }
        }
    }
    syslib->redraw_window();
}

void gui_event_listener(void) {
    static human_input_group_command_t command = {
        .type = HUMAN_INPUT_GROUP_LISTEN_TO_EVENTS,
        .argument = {
            0
        }
    };
    command.argument[0] = (uint32_t) &human_input_event_stack;
    syscall_send_command_to_device(human_input, &command);

    while(true) {
        asm volatile("pause");

        // check if there are any events
        if(human_input_event_stack.consumer == human_input_event_stack.producer) {
            syscall_switch();
        }

        // pop all events
        human_input_event_t *event = &human_input_event_stack.stack[human_input_event_stack.consumer];
        while(human_input_event_stack.consumer != human_input_event_stack.producer) {
            switch(event->type) {
                case HUMAN_INPUT_EVENT_KEY_PRESSED: {
                    if(pressed_key_event_handlers[event->value] != NULL) {
                        pressed_key_event_handlers[event->value]();
                    }
                    break;
                }
                case HUMAN_INPUT_EVENT_KEY_RELEASED: {
                    break;
                }
                default: {
                    syslib->logf("\nUnknown event type: %d", event->type);
                    break;
                }
            }
            human_input_event_stack.consumer = (human_input_event_stack.consumer + 1) % MAX_NUMBER_OF_HUMAN_INPUT_EVENTS;
            event = &human_input_event_stack.stack[human_input_event_stack.consumer];
        }
    }
}