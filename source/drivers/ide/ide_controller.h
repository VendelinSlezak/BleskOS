//BleskOS

/*
* MIT License
* Copyright (c) 2023-2025 BleskOS developers
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#define MAX_NUMBER_OF_IDE_CONTROLLERS 8
struct ide_info_t {
    struct pci_device_info_t pci;

    word_t io_base;
    word_t control_base;

    dword_t master_drive_present;
    word_t master_drive_type;
    dword_t master_drive_size;

    dword_t slave_drive_present;
    word_t slave_drive_type;
    dword_t slave_drive_size;
};

struct ide_drive_info_t {
    word_t type;
    dword_t size;
};

#define PATA_MASTER 0xE0
#define PATA_SLAVE 0xF0

#define PATA_HARD_DISK_SIGNATURE 0x0000
#define PATA_OPTICAL_DRIVE_SIGNATURE 0xEB14

void ide_add_new_pci_device(struct pci_device_info_t device);
void initalize_ide_controller(byte_t number_of_controller);
void initalize_ide_controller_drive(byte_t number_of_controller, byte_t number_of_drive);
struct ide_drive_info_t ide_send_identify_drive(word_t base_port);
byte_t ide_is_bus_floating(word_t base_port);
byte_t ide_wait_drive_not_busy(word_t base_port, dword_t wait_ticks);
byte_t ide_wait_drive_not_busy_with_error_status(word_t base_port, dword_t wait_ticks);
byte_t ide_wait_for_data(word_t base_port, dword_t wait_ticks);
void ide_clear_device_output(word_t base_port);
void ide_reset_controller(word_t base_port, word_t alt_base_port);
void ide_select_drive(word_t base_port, byte_t drive);