//BleskOS

/*
* MIT License
* Copyright (c) 2023-2025 BleskOS developers
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#define PCI_BAR0 0x10
#define PCI_BAR1 0x14
#define PCI_BAR2 0x18
#define PCI_BAR3 0x1C
#define PCI_BAR4 0x20
#define PCI_BAR5 0x24

#define PCI_MMIO_BAR 0x0
#define PCI_IO_BAR 0x1

#define DEVICE_PRESENCE_IS_NOT_KNOWN 0xFF
#define DEVICE_NOT_PRESENT 0
#define DEVICE_PRESENT 1
#define DEVICE_PRESENT_BUT_ERROR_STATE 2

#define VENDOR_INTEL 0x8086
#define VENDOR_AMD_1 0x1022
#define VENDOR_AMD_2 0x1002
#define VENDOR_BROADCOM 0x14E4
#define VENDOR_REALTEK 0x10EC
#define VENDOR_QUALCOMM_ATHEROS_1 0x168C
#define VENDOR_QUALCOMM_ATHEROS_2 0x1969
#define VENDOR_NVIDIA 0x10DE
#define VENDOR_TEXAS_INSTUMENTS 0x104C
#define VENDOR_CONEXANT_SYSTEMS 0x14F1
#define VENDOR_SIGMATEL 0x8384
#define VENDOR_RED_HAT 0x1AF4

dword_t pci_devices_array_mem = 0, pci_num_of_devices = 0;

dword_t pci_read(dword_t bus, dword_t device, dword_t function, dword_t offset);
void pci_write(dword_t bus, dword_t device, dword_t function, dword_t offset, dword_t value);
void pci_writeb(dword_t bus, dword_t device, dword_t function, dword_t offset, dword_t value);
dword_t pci_read_bar_type(dword_t bus, dword_t device, dword_t function, dword_t bar);
word_t pci_read_io_bar(dword_t bus, dword_t device, dword_t function, dword_t bar);
dword_t pci_read_mmio_bar(dword_t bus, dword_t device, dword_t function, dword_t bar);
void pci_enable_io_busmastering(dword_t bus, dword_t device, dword_t function);
void pci_enable_mmio_busmastering(dword_t bus, dword_t device, dword_t function);
void pci_disable_interrupts(dword_t bus, dword_t device, dword_t function);
void scan_pci(void);
void scan_pci_device(dword_t bus, dword_t device, dword_t function);
byte_t *get_pci_vendor_string(dword_t vendor_id);