//BleskOS

/*
* MIT License
* Copyright (c) 2023-2025 BleskOS developers
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

/* detect presence of serial ports */

// TODO: add serial ports on PCI bus

void check_presence_of_serial_ports(void) {
    // check presence of ports on fixed addresses
    check_presence_of_serial_port(0x3F8, 4);
    check_presence_of_serial_port(0x2F8, 3);
    check_presence_of_serial_port(0x3E8, 4);
    check_presence_of_serial_port(0x2E8, 3);
}

void check_presence_of_serial_port(word_t base, dword_t irq) {
    // check number of already connected serial ports
    if(components->n_serial_port >= MAX_NUMBER_OF_SERIAL_PORTS) {
        return;
    }

    // TODO: what all is needed to detect serial port?
    if(inb(base + 5) == 0xFF || inb(base + 5) == 0x00) {
        return;
    }

    // add serial port
    components->serial_port[components->n_serial_port].base = base;
    components->serial_port[components->n_serial_port].irq = irq;
    components->n_serial_port++;
}

/* initalizing of serial ports */

void initalize_serial_ports(void) {
    for(dword_t i = 0; i < components->n_serial_port; i++) {
        initalize_serial_port(i);
    }
}

void initalize_serial_port(dword_t number_of_port) {
    word_t base = components->serial_port[number_of_port].base;

    // log
    logf("\n\nDriver: Serial Port\nDevice: IO 0x%04x", base);

    // disable interrupts
    outb(base + 1, 0x00);

    // TODO: detect PnP devices

    // try to estabilish connection
    serial_port_estabilish_connection(number_of_port, 0, 8, 0, 0, 0);

    // enable interrupts
    set_irq_handler(components->serial_port[number_of_port].irq, (dword_t)serial_port_irq);
    outb(base + 1, 0x01);

    // wait if device on port sends identification, otherwise assume that this is used as debugging port
    wait(20);
    if(components->serial_port[number_of_port].device_type == SERIAL_PORT_NO_DEVICE) {
        components->serial_port[number_of_port].device_type = SERIAL_PORT_DEBUG_DEVICE;
        
        // send all log data up to this point
        for(dword_t i = 0; i < logging_memory_chars; i++) {
            if(logging_memory[i] < 0x80) {
                serial_port_send_byte(number_of_port, logging_memory[i]);
            }
        }
    }
}

/* setting connection on serial port */

void serial_port_estabilish_connection(dword_t number_of_port, word_t baud_rate, byte_t number_of_bits_per_transfer, byte_t stop_bit_setting, byte_t parity_bit_setting, byte_t break_enable_bit) {
    word_t base = components->serial_port[number_of_port].base;

    // find fastest working baud rate
    if(baud_rate == 0) {
        // set 8 bit mode
        outb(base + 3, 0x03);

        // set loopback mode
        outb(base + 4, 0x1E);

        // try all baud rates from 115200 to 300
        for(word_t divisor = 1; divisor <= 384; divisor++) {
            // set baud rate
            serial_port_set_baud_rate(number_of_port, divisor);

            // send data
            outb(base + 0, 0xAE);

            // test if same data was returned
            if(inb(base + 0) == 0xAE) {
                baud_rate = (115200/divisor);
                break;
            }
        }
    }
    // select predefined baud rate
    else {
        // find divisor for entered baud rate
        for(word_t divisor = 1; divisor <= 384; divisor++) {
            if(115200/divisor == baud_rate) {
                // set baud rate
                serial_port_set_baud_rate(number_of_port, divisor);
                break;
            }
        }
    }
    components->serial_port[number_of_port].baud_rate = baud_rate;

    // set transmission type
    outb(base + 3, (number_of_bits_per_transfer-5) | (stop_bit_setting << 2) | (parity_bit_setting << 3) | (break_enable_bit << 6));

    // enable FIFO, clear receive and transmit FIFO, enable interrupts for port
    outb(base + 2, 0xC7);

    // start normal operation of port
    outb(base + 4, 0x0F);

    // log
    logf("\nSerial port %d: Established connection with baud rate %d", number_of_port, baud_rate);
}

void serial_port_set_baud_rate(dword_t number_of_port, word_t divisor) {
    word_t base = components->serial_port[number_of_port].base;

    // start setting of baud rate
    outb(base + 3, inb(base + 3) | 0x80);

    // send baud rate divisor
    outb(base + 0, (divisor & 0xFF));
    outb(base + 1, (divisor >> 8));

    // end setting of baud rate
    outb(base + 3, inb(base + 3) & ~0x80);
}

void serial_port_detect_device(dword_t number_of_port) {
    word_t base = components->serial_port[number_of_port].base;

    // TODO: what is proper method to detect device on serial port?
    // if( ??? ) {
    //     components->serial_port[number_of_port].is_connected = STATUS_TRUE;
    // }
    // else {
    //     components->serial_port[number_of_port].is_connected = STATUS_FALSE;
    // }
}

/* interrupt handler */

void serial_port_irq(void) {
    // check all serial ports
    for(dword_t i = 0; i < components->n_serial_port; i++) {
        // check if there are some data on this port
        if((inb(components->serial_port[i].base + 5) & 0x1) == 0x1) {
            // read all data from port to buffer
            byte_t buffer[256];
            dword_t buffer_p = 0;
            for(dword_t j = 0; j < 256; j++) {
                if((inb(components->serial_port[i].base + 5) & 0x1) == 0x1) {
                    // read data to buffer
                    buffer[buffer_p] = inb(components->serial_port[i].base + 0);
                    buffer_p++;
                }
                else {
                    break;
                }
            }

            // process data
            if(buffer_p == 0) {
                continue;
            }
            if(components->serial_port[i].device_type == SERIAL_PORT_NO_DEVICE) {
                // mouse detected
                if(buffer[0] == 'M') {
                    logf("\nSerial port %d device detected\n Type: Mouse\n Protocol: ", i);
                    if(buffer_p == 1) {
                        logf("Microsoft protocol, 2 buttons");
                        components->serial_port[i].device_type = SERIAL_PORT_MOUSE_2_BUTTONS;
                    }
                    else if(buffer[1] == '3') {
                        logf("Extended Microsoft protocol, 3 buttons");
                        components->serial_port[i].device_type = SERIAL_PORT_MOUSE_3_BUTTONS;
                    }
                    else if(buffer[1] == 'Z') {
                        logf("Extended Microsoft protocol, mouse wheel mode");
                        components->serial_port[i].device_type = SERIAL_PORT_MOUSE_WHEEL;
                    }

                    // TODO: move Plug'n'Play identification string parsing to special file and parse it all
                    if(buffer_p >= 12) {
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
                }
            }
            else if(components->serial_port[i].device_type == SERIAL_PORT_MOUSE_2_BUTTONS
                    || components->serial_port[i].device_type == SERIAL_PORT_MOUSE_3_BUTTONS
                    || components->serial_port[i].device_type == SERIAL_PORT_MOUSE_WHEEL) {
                // check if whole packet was transferred
                if(buffer_p >= 3) {
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
                    if(buffer_p >= 4) {
                        if(components->serial_port[i].device_type != SERIAL_PORT_MOUSE_2_BUTTONS) {
                            mouse_buttons |= ((buffer[3] >> 3) & 0x4);
                        }

                        if(components->serial_port[i].device_type == SERIAL_PORT_MOUSE_WHEEL && (buffer[3] & 0xF) != 0) {
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
        }
    }
    
}

/* sending data */

dword_t serial_port_send_data(dword_t number_of_port, byte_t *data, dword_t length_of_data) {
    for(dword_t i = 0; i < length_of_data; i++) {
        if(serial_port_send_byte(number_of_port, *data) == STATUS_ERROR) {
            return STATUS_ERROR;
        }
        data++;
    }

    return STATUS_GOOD;
}

dword_t serial_port_send_byte(dword_t number_of_port, byte_t data) {
    // wait for line
    for(dword_t i = 0; i < 1000; i++) {
        // is line ready to send data?
        if((inb(components->serial_port[number_of_port].base + 5) & (1 << 5)) == (1 << 5)) {
            outb(components->serial_port[number_of_port].base, data);
            return STATUS_GOOD;
        }
    }

    return STATUS_ERROR;
}