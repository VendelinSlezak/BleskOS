//BleskOS

/*
* MIT License
* Copyright (c) 2023-2025 BleskOS developers
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

/* detect presence of parallel ports */

void parallel_port_add_new_pci_device(struct pci_device_info_t device) {
    // check number of already connected ports
    if(components->n_parallel_port >= MAX_NUMBER_OF_PARALLEL_PORTS) {
        return;
    }

    // log driver
    logf("%s", __FILE__);

    // save basic device informations
    copy_memory((dword_t)&device, (dword_t)&components->parallel_port[components->n_parallel_port].pci, sizeof(struct pci_device_info_t));

    // read other device informations
    components->parallel_port[components->n_parallel_port].base = pci_get_io(device, PCI_BAR0);
    components->parallel_port[components->n_parallel_port].is_pci_device = STATUS_TRUE;

    // configure PCI
    pci_set_bits(device, 0x04, PCI_STATUS_BUSMASTERING | PCI_STATUS_IO);

    // update number of devices
    components->n_parallel_port++;
}

void check_presence_of_parallel_ports(void) {
    // check presence of ports on fixed addresses
    check_presence_of_parallel_port(0x3BC, 7);
    check_presence_of_parallel_port(0x378, 7);
    check_presence_of_parallel_port(0x278, 6);
}

void check_presence_of_parallel_port(word_t base, dword_t irq) {
    // check number of already connected parallel ports
    if(components->n_parallel_port >= MAX_NUMBER_OF_PARALLEL_PORTS) {
        return;
    }

    // TODO: is there better detection method?
    if(inb(base + 2) == 0xFF) {
        return;
    }
    if(inb(base + 2) == 0x00) {
        outb(base + 2, 0x08);
        if(inb(base + 2) != 0x08) {
            return;
        }
        outb(base + 2, 0x00);
    }

    // add parallel port
    components->parallel_port[components->n_parallel_port].base = base;
    components->parallel_port[components->n_parallel_port].irq = irq;
    components->n_parallel_port++;
}

/* initalizing of parallel ports */

void initalize_parallel_ports(void) {
    for(dword_t i = 0; i < components->n_parallel_port; i++) {
        initalize_parallel_port(i);
    }
}

void initalize_parallel_port(dword_t number_of_port) {
    word_t base = components->parallel_port[number_of_port].base;

    // log
    logf("\n\nDriver: Parallel Port\nDevice: ");
    if(components->parallel_port[number_of_port].is_pci_device == STATUS_TRUE) {
        logf("PCI bus %d:%d:%d:%d",
            components->parallel_port[number_of_port].pci.segment,
            components->parallel_port[number_of_port].pci.bus,
            components->parallel_port[number_of_port].pci.device,
            components->parallel_port[number_of_port].pci.function);
    }
    else {
        logf("IO 0x%04x", base);
    }

    // disable data port
    outb(base + 0, 0x00);

    // disable control port
    outb(base + 2, PARALLEL_PORT_INIT_BIT);
    wait(10);

    // reset device
    outb(base + 2, 0x00);
    wait(10);
    outb(base + 2, PARALLEL_PORT_INIT_BIT);
    wait(10);
    
    // enable port
    outb(base + 2, PARALLEL_PORT_INIT_BIT);
}

/* sending data */

dword_t parallel_port_send_data(dword_t number_of_port, byte_t *data, dword_t length_of_data) {
    for(dword_t i = 0; i < length_of_data; i++) {
        if(parallel_port_send_byte(number_of_port, *data) == STATUS_ERROR) {
            return STATUS_ERROR;
        }
        data++;
    }

    return STATUS_GOOD;
}

dword_t parallel_port_send_byte(dword_t number_of_port, byte_t data) {
    word_t base = components->parallel_port[number_of_port].base;

    // wait for device to be receptive
    timeout = (time_of_system_running + 200);
    while(time_of_system_running < timeout) {
        if((inb(base + 1) & 0x80) == 0x80) {
            break;
        }
    }
    if(time_of_system_running >= timeout) {
        return STATUS_ERROR;
    }
    wait(1);

    // set data on pins
    outb(base + 0, data);

    // tell device that we are sending data
    outb(base + 2, inb(base + 2) | PARALLEL_PORT_SELECT_BIT | PARALLEL_PORT_STROBE_BIT);
    wait(10);
    outb(base + 2, (inb(base + 2) & ~PARALLEL_PORT_STROBE_BIT) | PARALLEL_PORT_SELECT_BIT);
    inb(base + 1);
    wait(10);

    // wait for device to be receptive
    timeout = (time_of_system_running + 200);
    while(time_of_system_running < timeout) {
        if((inb(base + 1) & 0x80) == 0x80) {
            break;
        }
    }
    if(time_of_system_running >= timeout) {
        return STATUS_ERROR;
    }
    wait(1);

    // check if there was error
    if((inb(base + 1) & 0x08) == 0x00) {
        logf("\nParralel port: Error after sending byte 0x%02x 0x%02x", inb(base + 1), inb(base + 2));
        return STATUS_ERROR;
    }

    // data is successfully sended
    return STATUS_GOOD;
}