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
#include <kernel/hardware/main.h>
#include <kernel/hardware/groups/logging/logging.h>
#include <kernel/hardware/groups/human_input/human_input.h>
#include <kernel/hardware/devices/memory/memory_allocators.h>
#include <kernel/hardware/controllers/controller_8042/controller_8042.h>
#include <kernel/hardware/subsystems/screen/screen.h>

/* local variables */
static human_input_group_device_functions_t ps2_mouse_functions = {
    .get_state = ps2_mouse_get_state,
};

/* functions */
void initialize_ps2_mouse(hardware_t *ps2_mouse) {
    log("\n[PS2] Initializing mouse...");
    ps2_mouse->data = kalloc(sizeof(ps2_mouse_data_t));
    ps2_mouse_data_t *data = (ps2_mouse_data_t *) ps2_mouse->data;
    controller_8042_communication_functions_t *functions = (controller_8042_communication_functions_t *) ps2_mouse->communication_functions;

    // try to activate wheel
    if(functions->send_command_with_payload(0xF3, 200) == ERROR) {
        log("\n[PS2] ERROR: Mouse did not set sample rate");
        ps2_mouse->is_initialized = true;
        return;
    }
    if(functions->send_command_with_payload(0xF3, 100) == ERROR) {
        log("\n[PS2] ERROR: Mouse did not set sample rate");
        ps2_mouse->is_initialized = true;
        return;
    }
    if(functions->send_command_with_payload(0xF3, 80) == ERROR) {
        log("\n[PS2] ERROR: Mouse did not set sample rate");
        ps2_mouse->is_initialized = true;
        return;
    }

    // get mouse ID
    uint32_t mouse_id = functions->send_command_with_return(0xF2);
    if(mouse_id == INVALID) {
        log("\n[PS2] ERROR: Mouse did not send ID");
        ps2_mouse->is_initialized = true;
        return;
    }

    // try to activate additional buttons and bidirectional wheel
    if(mouse_id == 3) {
        if(functions->send_command_with_payload(0xF3, 200) == ERROR) {
            log("\n[PS2] ERROR: Mouse did not set sample rate");
            ps2_mouse->is_initialized = true;
            return;
        }
        if(functions->send_command_with_payload(0xF3, 200) == ERROR) {
            log("\n[PS2] ERROR: Mouse did not set sample rate");
            ps2_mouse->is_initialized = true;
            return;
        }
        if(functions->send_command_with_payload(0xF3, 80) == ERROR) {
            log("\n[PS2] ERROR: Mouse did not set sample rate");
            ps2_mouse->is_initialized = true;
            return;
        }
    }

    // get mouse ID
    mouse_id = functions->send_command_with_return(0xF2);
    if(mouse_id == INVALID) {
        log("\n[PS2] ERROR: Mouse did not send ID");
        ps2_mouse->is_initialized = true;
        return;
    }
    if(mouse_id == 0) {
        data->bytes_per_packet = 3;
        data->status = PS2_MOUSE_DEFAULT;
    }
    else if(mouse_id == 3) {
        data->bytes_per_packet = 4;
        data->status = PS2_MOUSE_WHEEL_ACTIVATED;
    }
    else if(mouse_id == 4) {
        data->bytes_per_packet = 4;
        data->status = PS2_MOUSE_EXTRA_BUTTONS_ACTIVATED;
    }
    else {
        log("\n[PS2] ERROR: Unknown mouse ID 0x%02x", mouse_id);
        ps2_mouse->is_initialized = true;
        return;
    }
    log("\n[PS2] Mouse ID: 0x%02x", mouse_id);

    // start streaming
    if(functions->send_command(0xF4) == ERROR) {
        log("\n[PS2] ERROR: Mouse did not start streaming");
        ps2_mouse->is_initialized = true;
        return;
    }
    functions->set_receive_function(ps2_mouse, ps2_mouse_receive);

    // add to group
    add_human_input_device(ps2_mouse, &ps2_mouse_functions);
    log("\n[PS2] Mouse initialized");

    ps2_mouse->is_initialized = true;
}

void ps2_mouse_receive(hardware_t *ps2_mouse, uint8_t *buffer, uint32_t size) {
    ps2_mouse_data_t *data = (ps2_mouse_data_t *) ps2_mouse->data;

    for(int i = 0; i < size; i++, buffer++) {
        data->packet[data->packet_ptr++] = *buffer;
        if(data->packet_ptr < data->bytes_per_packet) {
            continue;
        }
        data->packet_ptr = 0;

        if((data->packet[0] & 0xC0) != 0x00) {
            continue; // discard packet
        }

        uint32_t left_button = data->packet[0] & 0x01;
        uint32_t middle_button = (data->packet[0] >> 1) & 0x01;
        uint32_t right_button = (data->packet[0] >> 2) & 0x01;
        uint32_t x_movement = data->packet[1];
        if((data->packet[0] & 0x10) == 0x10) {
            x_movement |= 0xFFFFFF00;
        }
        uint32_t y_movement = 0;
        if(data->packet[2] != 0) {
            y_movement = ((data->packet[2] * -1) & 0xFF);
            if((data->packet[0] & 0x20) == 0x00) {
                y_movement |= 0xFFFFFF00;
            }
        }
        uint32_t wheel_vertical_movement = 0;
        uint32_t wheel_horizontal_movement = 0;
        uint32_t button_4 = 0;
        uint32_t button_5 = 0;
        if(data->status == PS2_MOUSE_WHEEL_ACTIVATED) {
            if(data->packet[3] != 0) {
                wheel_vertical_movement = data->packet[3];
                if((wheel_vertical_movement & 0x80) == 0x80) {
                    wheel_vertical_movement |= 0xFFFFFF00;
                }
            }
        }
        else if(data->status == PS2_MOUSE_EXTRA_BUTTONS_ACTIVATED) {
            button_4 = (data->packet[3] >> 4) & 0x01;
            button_5 = (data->packet[3] >> 5) & 0x01;
            switch(data->packet[3] & 0x0F) {
                case 0x01: wheel_vertical_movement = 0x00000001; break;
                case 0x0F: wheel_vertical_movement = 0xFFFFFFFF; break;
                case 0x02: wheel_horizontal_movement = 0x00000001; break;
                case 0x0E: wheel_horizontal_movement = 0xFFFFFFFF; break;
                default: break;
            }
        }

        // process packet in human input group
        hid_reset_state_before_new_events(&data->state);
        hid_local_event_button(&data->state, BUTTON_LEFT, left_button);
        hid_local_event_button(&data->state, BUTTON_MIDDLE, middle_button);
        hid_local_event_button(&data->state, BUTTON_RIGHT, right_button);
        hid_local_event_button(&data->state, BUTTON_4, button_4);
        hid_local_event_button(&data->state, BUTTON_5, button_5);
        if(x_movement != 0 || y_movement != 0 || wheel_vertical_movement != 0 || wheel_horizontal_movement != 0) {
            hid_local_event_movement(&data->state, X_MOVEMENT, x_movement);
            hid_local_event_movement(&data->state, Y_MOVEMENT, y_movement);
            hid_local_event_movement(&data->state, WHEEL_VERTICAL_MOVEMENT, wheel_vertical_movement);
            hid_local_event_movement(&data->state, WHEEL_HORIZONTAL_MOVEMENT, wheel_horizontal_movement);
        }
        hid_process_changes_of_local_state(&data->state);
    }
}

human_input_device_state_t *ps2_mouse_get_state(hardware_t *ps2_mouse) {
    ps2_mouse_data_t *data = ps2_mouse->data;
    return &data->state;
}