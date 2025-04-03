//BleskOS

/*
* MIT License
* Copyright (c) 2023-2025 BleskOS developers
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#define MAX_NUMBER_OF_SERIAL_PORTS 4
struct serial_port_info_t {
    dword_t is_pci_device;
    struct pci_device_info_t pci;

    word_t base;
    dword_t irq;
    dword_t is_device_connected;
    dword_t device_type;

    word_t baud_rate;
    byte_t number_of_bits_per_transfer;
    byte_t stop_bit;
    byte_t parity_bit;
    byte_t break_enable_bit;

    void (*process_data)(dword_t number_of_port, void *data, byte_t data_length);
};

enum {
    SERIAL_PORT_NO_DEVICE = 0,
    SERIAL_PORT_DEBUG_DEVICE,
    SERIAL_PORT_MOUSE_2_BUTTONS,
    SERIAL_PORT_MOUSE_3_BUTTONS,
    SERIAL_PORT_MOUSE_WHEEL,
    SERIAL_PORT_UNKNOWN_DEVICE,
};


void serial_port_add_new_pci_device(struct pci_device_info_t device);
void check_presence_of_serial_ports(void);
void check_presence_of_serial_port(word_t base, dword_t irq);

void initalize_serial_ports(void);
void initalize_serial_port(dword_t number_of_port);
void detect_serial_device(dword_t number_of_port, void *data, byte_t data_length);

void serial_port_estabilish_connection(dword_t number_of_port, word_t baud_rate, byte_t number_of_bits_per_transfer, byte_t stop_bit_setting, byte_t parity_bit_setting, byte_t break_enable_bit);
void serial_port_set_baud_rate(dword_t number_of_port, word_t divisor);
void serial_port_detect_device(dword_t number_of_port);

void serial_port_irq(void);

dword_t serial_port_send_data(dword_t number_of_port, byte_t *data, dword_t length_of_data);
dword_t serial_port_send_byte(dword_t number_of_port, byte_t data);