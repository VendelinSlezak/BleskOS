#ifndef BUILD_HARDWARE_CONTROLLERS_PCI_PCI_H
#define BUILD_HARDWARE_CONTROLLERS_PCI_PCI_H

/* 
* BleskOS
*
* MIT License
* Copyright (c) 2023-2025 BleskOS developers
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include <hardware/controllers/pci/device.h>

typedef enum {
    io,
    mmio
} pci_access_type_t;

#define PCI_BAR0 0x10
#define PCI_BAR1 0x14
#define PCI_BAR2 0x18
#define PCI_BAR3 0x1C
#define PCI_BAR4 0x20
#define PCI_BAR5 0x24

#define PCI_MMIO_BAR 0x0
#define PCI_IO_BAR 0x1
byte_t pci_inb(pci_device_info_t device, dword_t offset);
word_t pci_inw(pci_device_info_t device, dword_t offset);
dword_t pci_ind(pci_device_info_t device, dword_t offset);
void pci_outb(pci_device_info_t device, dword_t offset, byte_t value);
void pci_outw(pci_device_info_t device, dword_t offset, word_t value);
void pci_outd(pci_device_info_t device, dword_t offset, dword_t value);
void pci_set_bits(pci_device_info_t device, dword_t offset, dword_t bits);
void pci_clear_bits(pci_device_info_t device, dword_t offset, dword_t bits);
byte_t *pci_get_vendor_name(word_t vendor_id);
byte_t *pci_get_device_type_string(dword_t type);
dword_t pci_get_bar_type(pci_device_info_t device, dword_t bar);
word_t pci_get_io(pci_device_info_t device, dword_t bar);
dword_t pci_get_mmio(pci_device_info_t device, dword_t bar);
dword_t pci_get_64_bit_mmio(pci_device_info_t device, dword_t bar);
void initialize_pci_controller(void);
void scan_pci_device(pci_device_info_t device);

#endif /* BUILD_HARDWARE_CONTROLLERS_PCI_PCI_H */
