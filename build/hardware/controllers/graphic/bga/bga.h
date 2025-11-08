#ifndef BUILD_HARDWARE_CONTROLLERS_GRAPHIC_BGA_BGA_H
#define BUILD_HARDWARE_CONTROLLERS_GRAPHIC_BGA_BGA_H

/* 
* BleskOS
*
* MIT License
* Copyright (c) 2023-2025 BleskOS developers
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#ifndef BGA_H
#define BGA_H

#include <hardware/controllers/pci/device.h>
#include <hardware/devices/monitor/monitor.h>

#define BGA_REGISTER_INDEX 0x01CE
#define BGA_REGISTER_DATA 0x01CF

#define BGA_INDEX_ID 0
#define BGA_INDEX_WIDTH 1
#define BGA_INDEX_HEIGHT 2
#define BGA_INDEX_BPP 3
#define BGA_INDEX_ENABLE 4

typedef struct {
    /* internal device part */
    pci_device_info_t pci_device;
    dword_t version;

    /* controller part */
    dword_t number_of_devices;
    dword_t devices[1];
} e_bga_attr_t;

extern pci_supported_classic_devices_by_driver_t pci_bga_devices[];

#endif
word_t bga_read(byte_t index);
void bga_write(byte_t index, word_t value);
dword_t is_bga_controller_present(void);
void initialize_bga_controller_entity(void);
void initialize_bga_controller(void);
void initialize_pci_bga_controller_entity(pci_device_info_t device);
void initialize_pci_bga_controller(void);
void initialize_bga_controller_with_lfb(void *linear_frame_buffer);
dword_t bga_controller_change_monitor_resolution(monitor_mode_t mode);

#endif /* BUILD_HARDWARE_CONTROLLERS_GRAPHIC_BGA_BGA_H */
