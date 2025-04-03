//BleskOS

/*
* MIT License
* Copyright (c) 2023-2025 BleskOS developers
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

/* detect presence of serial ports */

void serial_port_add_new_pci_device(struct pci_device_info_t device) {
    // check number of already connected ports
    if(components->n_serial_port >= MAX_NUMBER_OF_SERIAL_PORTS) {
        return;
    }

    // log driver
    logf("%s", __FILE__);

    // save basic device informations
    copy_memory((dword_t)&device, (dword_t)&components->serial_port[components->n_serial_port].pci, sizeof(struct pci_device_info_t));

    // read other device informations
    components->serial_port[components->n_serial_port].base = pci_get_io(device, PCI_BAR0);
    components->serial_port[components->n_serial_port].is_pci_device = STATUS_TRUE;

    // configure PCI
    pci_set_bits(device, 0x04, PCI_STATUS_BUSMASTERING | PCI_STATUS_IO);

    // update number of devices
    components->n_serial_port++;
}

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

    // TODO: is there better detection method?
    if(inb(base + 1) == 0xFF) {
        return;
    }
    if(inb(base + 1) == 0x00) {
        outb(base + 1, 0x04);
        if(inb(base + 1) != 0x04) {
            return;
        }
        outb(base + 1, 0x00);
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
    logf("\n\nDriver: Serial Port\nDevice: ");
    if(components->serial_port[number_of_port].is_pci_device == STATUS_TRUE) {
        logf("PCI bus %d:%d:%d:%d",
            components->serial_port[number_of_port].pci.segment,
            components->serial_port[number_of_port].pci.bus,
            components->serial_port[number_of_port].pci.device,
            components->serial_port[number_of_port].pci.function);
    }
    else {
        logf("IO 0x%04x", base);
    }

    // disable interrupts
    outb(base + 1, 0x00);

    // disable line
    outb(base + 3, 0x00);

    // TODO: detect PnP devices in proper way

    // estabilish connection on port
    serial_port_estabilish_connection(number_of_port, 0, 8, 0, 0, 0);

    // set method to detect device type
    components->serial_port[number_of_port].process_data = detect_serial_device;

    // enable interrupts
    if(components->serial_port[number_of_port].is_pci_device == STATUS_TRUE) {
        pci_device_install_interrupt_handler(components->serial_port[number_of_port].pci, serial_port_irq);
    }
    else {
        set_irq_handler(components->serial_port[number_of_port].irq, (dword_t)serial_port_irq);
    }
    outb(base + 1, 0x01);

    // if there is device, it should now send data catched by detect_serial_device(), wait 20ms for it to happen
    volatile dword_t timeout = (time_of_system_running+20);
    while(time_of_system_running < timeout) {
        if(components->serial_port[number_of_port].is_device_connected == STATUS_TRUE) {
            return;
        }
    }

    // otherwise assume that this is used as debugging port
    components->serial_port[number_of_port].device_type = SERIAL_PORT_DEBUG_DEVICE;
    components->serial_port[number_of_port].process_data = 0; // TODO: write debug commands interface
    
    // send all log data up to this point
    for(dword_t i = 0; i < logging_memory_chars; i++) {
        if(logging_memory[i] < 0x80) {
            if(logging_memory[i] == 0xA) {
                serial_port_send_byte(number_of_port, 0xD);
            }
            serial_port_send_byte(number_of_port, logging_memory[i]);
        }
    }
}

void detect_serial_device(dword_t number_of_port, void *data, byte_t data_length) {
    byte_t *buffer = data;

    // log
    logf("\nSerial port %d device detected");

    // there is some device
    components->serial_port[number_of_port].is_device_connected = STATUS_TRUE;

    // detect mouse
    if(detect_and_initalize_serial_mouse(number_of_port, data, data_length) == STATUS_TRUE) {
        return;
    }
    // unknown device
    else {
        components->serial_port[number_of_port].device_type = SERIAL_PORT_UNKNOWN_DEVICE;
    }
}

/* setting connection on serial port */

void serial_port_estabilish_connection(dword_t number_of_port, word_t baud_rate, byte_t number_of_bits_per_transfer, byte_t stop_bit_setting, byte_t parity_bit_setting, byte_t break_enable_bit) {
    word_t base = components->serial_port[number_of_port].base;

    // find fastest working baud rate
    if(baud_rate == 0) {
        // set loopback mode
        outb(base + 4, 0x1E);

        // set 8 bit transfer mode, 1 stop bit, no parity bit, no break enable bit
        outb(base + 3, 0x03);

        // try all baud rates from 115200 to 300
        for(word_t divisor = 1; divisor <= 384; divisor++) {
            // set baud rate
            serial_port_set_baud_rate(number_of_port, divisor);

            // send data
            outb(base + 0, 0xAA);

            // test if same data was returned
            if(inb(base + 0) == 0xAA) {
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
    components->serial_port[number_of_port].stop_bit = stop_bit_setting;
    components->serial_port[number_of_port].parity_bit = parity_bit_setting;
    components->serial_port[number_of_port].break_enable_bit = break_enable_bit;

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
    //     components->serial_port[number_of_port].is_device_connected = STATUS_TRUE;
    // }
    // else {
    //     components->serial_port[number_of_port].is_device_connected = STATUS_FALSE;
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
                    buffer[buffer_p++] = inb(components->serial_port[i].base + 0);
                }
                else {
                    break;
                }
            }

            // process data
            if(buffer_p == 0) {
                continue;
            }
            else if(components->serial_port[i].process_data != 0) {
                components->serial_port[i].process_data(i, buffer, buffer_p);
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