#ifndef BUILD_HARDWARE_CONTROLLERS_PCI_DEVICE_H
#define BUILD_HARDWARE_CONTROLLERS_PCI_DEVICE_H

/* 
* BleskOS
*
* MIT License
* Copyright (c) 2023-2025 BleskOS developers
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

typedef struct {
    dword_t pci_mmio_start;
    dword_t segment;
    dword_t bus;
    dword_t device;
    dword_t function;

    dword_t vendor_id;
    dword_t device_id;
    dword_t subsystem_vendor_id;
    dword_t subsystem_id;
    dword_t interrupt_line;
    dword_t interrupt_pin;
    dword_t msi_register;
    dword_t msi_x_register;
} pci_device_info_t;

typedef struct {
    word_t vendor_id;
    word_t device_id;
} pci_supported_classic_devices_by_driver_t;

typedef struct {
    word_t vendor_id;
    word_t device_id;
    word_t subsystem_vendor_id;
    word_t subsystem_id;
} pci_supported_subsystem_devices_by_driver_t;
#endif /* BUILD_HARDWARE_CONTROLLERS_PCI_DEVICE_H */
