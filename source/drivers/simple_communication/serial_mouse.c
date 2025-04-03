//BleskOS

/*
* MIT License
* Copyright (c) 2023-2025 BleskOS developers
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

dword_t detect_and_initalize_serial_mouse(dword_t number_of_port, void *data, byte_t data_length) {
    byte_t *buffer = data;

    if(buffer[0] == 'M') {
        logf("\n Type: Mouse\n Protocol: ", number_of_port);
        
        // read protocol type
        if(data_length == 1 || (buffer[1] != '3' && buffer[1] != 'Z')) {
            logf("Microsoft protocol, 2 buttons");
            components->serial_port[number_of_port].device_type = SERIAL_PORT_MOUSE_2_BUTTONS;
        }
        else if(buffer[1] == '3') {
            logf("Extended Microsoft protocol, 3 buttons");
            components->serial_port[number_of_port].device_type = SERIAL_PORT_MOUSE_3_BUTTONS;
        }
        else if(buffer[1] == 'Z') {
            logf("Extended Microsoft protocol, mouse wheel mode");
            components->serial_port[number_of_port].device_type = SERIAL_PORT_MOUSE_WHEEL;
        }

        // set method to receive data from mouse
        components->serial_port[number_of_port].process_data = serial_mouse_process_data;

        // TODO: move Plug'n'Play identification string parsing to special file and parse it all
        if(data_length >= 12) {
            logf("\n PnP Revision: %d.%d", (byte_t)buffer[3], buffer[4]-0x24);
            logf("\n EISA ID: %03s", &buffer[5]);
            logf("\n Product ID: 0x");
            for(dword_t j = 8; j < 12; j++) {
                if(buffer[j] < 0x10) {
                    logf("%c", buffer[j]+'0');
                }
                else {
                    logf("%c", buffer[j]+'A'-0x10);
                }
            }
        }

        return STATUS_TRUE;
    }
    else {
        return STATUS_FALSE;
    }
}

void serial_mouse_process_data(dword_t number_of_port, void *data, byte_t data_length) {
    byte_t *buffer = data;

    // check if whole packet was transferred
    if(data_length >= 3) {
        // parse first three bytes
        mouse_buttons = (((buffer[0] >> 5) & 0x1) | ((buffer[0] >> 3) & 0x2));
        mouse_movement_x = (buffer[1] | ((buffer[0] & 0x3) << 6));
        if((mouse_movement_x & 0x80) == 0x80) {
            mouse_movement_x = (0-(0x100-mouse_movement_x));
        }
        mouse_movement_y = (buffer[2] | ((buffer[0] & 0xC) << 4));
        if((mouse_movement_y & 0x80) == 0x80) {
            mouse_movement_y = (0-(0x100-mouse_movement_y));
        }

        // parse fourth byte
        if(data_length >= 4) {
            if(components->serial_port[number_of_port].device_type != SERIAL_PORT_MOUSE_2_BUTTONS) {
                mouse_buttons |= ((buffer[3] >> 3) & 0x4);
            }

            if(components->serial_port[number_of_port].device_type == SERIAL_PORT_MOUSE_WHEEL && (buffer[3] & 0xF) != 0) {
                mouse_wheel_movement = (buffer[3] & 0xF);
                if((mouse_wheel_movement & 0x8) == 0x8) {
                    mouse_wheel_movement = (0-(0x10-mouse_wheel_movement));
                }
                mouse_wheel_movement *= -1;
            }
        }

        // update button state
        mouse_update_click_button_state();

        // inform method wait_for_user_input() from source/drivers/system/user_wait.c that there was received packet from mouse
        mouse_event = STATUS_TRUE;
    }
}