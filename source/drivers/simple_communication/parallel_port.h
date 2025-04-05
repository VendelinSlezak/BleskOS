//BleskOS

/*
* MIT License
* Copyright (c) 2023-2025 BleskOS developers
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#define MAX_NUMBER_OF_PARALLEL_PORTS 3
struct parallel_port_info_t {
    dword_t is_pci_device;
    struct pci_device_info_t pci;

    word_t base;
    dword_t irq;
    dword_t is_device_connected;
    dword_t device_type;

    void (*process_data)(dword_t number_of_port, void *data, byte_t data_length);
};

#define PARALLEL_PORT_BIDIRECTIONAL_BIT (1 << 5)
#define PARALLEL_PORT_INTERRUPT_BIT (1 << 4)
#define PARALLEL_PORT_SELECT_BIT (1 << 3)
#define PARALLEL_PORT_INIT_BIT (1 << 2)
#define PARALLEL_PORT_STROBE_BIT (1 << 0)

void parallel_port_add_new_pci_device(struct pci_device_info_t device);
void check_presence_of_parallel_ports(void);
void check_presence_of_parallel_port(word_t base, dword_t irq);

void initalize_parallel_ports(void);
void initalize_parallel_port(dword_t number_of_port);

dword_t parallel_port_send_data(dword_t number_of_port, byte_t *data, dword_t length_of_data);
dword_t parallel_port_send_byte(dword_t number_of_port, byte_t data);