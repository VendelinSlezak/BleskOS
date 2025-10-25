/* 
* BleskOS
*
* MIT License
* Copyright (c) 2023-2025 BleskOS developers
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#ifndef PCI_SUPPORTED_DEVICES_H
#define PCI_SUPPORTED_DEVICES_H

#include <hardware/controllers/pci/device.h>
#include <hardware/controllers/graphic/bga/bga.h>

typedef struct { 
    pci_supported_classic_devices_by_driver_t *driver_classic_devices;
    pci_supported_subsystem_devices_by_driver_t *driver_subsystem_devices;
    void (*initialize)(pci_device_info_t device);
} pci_drivers_for_type_t;

typedef struct { 
    dword_t type;
    byte_t *description;
    pci_drivers_for_type_t *drivers;
} pci_device_type_list_t;

pci_drivers_for_type_t vga_compatibile_graphic_controller_drivers[] = { 
    {  pci_bga_devices, NULL, initialize_pci_bga_controller_entity },
    {  NULL, NULL, NULL }
};

pci_device_type_list_t pci_device_type_list[] = { 
    // Unclassified
    { 0x00000000, "Non-VGA-Compatible Unclassified Device", NULL},
    { 0x00010000, "VGA-compatible unclassified device", NULL},

    // Mass Storage Controller
    { 0x01000000, "SCSI Bus Controller", NULL},
    { 0x01010000, "IDE Controller (ISA mode-only)", NULL},
    { 0x01010500, "IDE Controller (PCI mode-only)", NULL},
    { 0x01010A00, "IDE Controller (ISA mode, both channels switched to PCI mode)", NULL},
    { 0x01010F00, "IDE Controller (PCI mode, both channels switched to ISA mode)", NULL},
    { 0x01018000, "IDE Controller (ISA mode-only, bus mastering)", NULL},
    { 0x01018500, "IDE Controller (PCI mode-only, bus mastering)", NULL},
    { 0x01018A00, "IDE Controller (ISA mode, both channels switched to PCI mode, bus mastering)", NULL},
    { 0x01018F00, "IDE Controller (PCI mode, both channels switched to ISA mode, bus mastering)", NULL},
    { 0x01020000, "Floppy Disk Controller", NULL},
    { 0x01030000, "IPI Bus Controller", NULL},
    { 0x01040000, "RAID Controller", NULL},
    { 0x01052000, "ATA Controller (single DMA)", NULL},
    { 0x01053000, "ATA Controller (chained DMA)", NULL},
    { 0x01060000, "Serial ATA Controller (Vendor Specific)", NULL},
    { 0x01060100, "Serial ATA Controller (AHCI 1.0)", NULL},
    { 0x01060200, "Serial ATA Controller (SSB)", NULL},
    { 0x01070000, "Serial Attached SCSI Controller (SAS)", NULL},
    { 0x01070100, "Serial Attached SCSI Controller (SSB)", NULL},
    { 0x01080100, "Non-volatile Memory Controller (NVMHCI)", NULL},
    { 0x01080200, "Non-volatile Memory Controller (NVM Express)", NULL},
    { 0x01800000, "Other Mass Storage Controller", NULL},

    // Network Controller
    { 0x02000000, "Ethernet Controller", NULL},
    { 0x02010000, "Token Ring Controller", NULL},
    { 0x02020000, "FDDI Controller", NULL},
    { 0x02030000, "ATM Controller", NULL},
    { 0x02040000, "ISDN Controller", NULL},
    { 0x02050000, "WorldFip Controller", NULL},
    { 0x02060000, "PICMG 2.14 Multi Computing", NULL},
    { 0x02070000, "Infiniband Controller", NULL},
    { 0x02080000, "Fabric Controller", NULL},
    { 0x02800000, "Wireless Controller", NULL},

    // Display Controller
    { 0x03000000, "VGA Compatible Graphic Controller", vga_compatibile_graphic_controller_drivers },
    { 0x03000100, "8514 Compatible Graphic Controller", NULL},
    { 0x03010000, "XGA Graphic Controller", NULL},
    { 0x03020000, "3D Graphic Controller", NULL},

    // Multimedia Controller
    { 0x04000000, "Multimedia Video Controller", NULL},
    { 0x04010000, "Multimedia Audio Controller", NULL},
    { 0x04020000, "Computer Telephony Device", NULL},
    { 0x04030000, "Audio Device", NULL},

    // Memory Controller
    { 0x05000000, "RAM Controller", NULL},
    { 0x05010000, "Flash Controller", NULL},

    // Bridge Devices
    { 0x06000000, "Host Bridge", NULL},
    { 0x06010000, "ISA Bridge", NULL},
    { 0x06020000, "EISA Bridge", NULL},
    { 0x06030000, "MCA Bridge", NULL},
    { 0x06040000, "PCI-to-PCI Bridge", NULL},
    { 0x06040100, "PCI-to-PCI Bridge (Subtractive Decode)", NULL},
    { 0x06050000, "PCMCIA Bridge", NULL},
    { 0x06060000, "NuBus Bridge", NULL},
    { 0x06070000, "CardBus Bridge", NULL},
    { 0x06080000, "RACEway Bridge", NULL},
    { 0x06090000, "PCI-to-PCI Bridge (Semi-Transparent)", NULL},
    { 0x060A0000, "InfiniBand-to-PCI Host Bridge", NULL},
    { 0x06800000, "Other Bridge", NULL},

    // Simple Communication Controllers
    { 0x07000000, "Serial Controller 8250-Compatible (Generic XT)", NULL},
    { 0x07000100, "Serial Controller 16450-Compatible", NULL},
    { 0x07000200, "Serial Controller 16550-Compatible", NULL},
    { 0x07000300, "Serial Controller 16650-Compatible", NULL},
    { 0x07000400, "Serial Controller 16750-Compatible", NULL},
    { 0x07000500, "Serial Controller 16850-Compatible", NULL},
    { 0x07000600, "Serial Controller 16950-Compatible", NULL},
    { 0x07010000, "Standard Parallel Controller", NULL},
    { 0x07010100, "Bi-Directional Parallel Controller", NULL},
    { 0x07010200, "ECP 1.X Compliant Parallel Controller", NULL},
    { 0x07010300, "IEEE 1284 Parallel Controller", NULL},
    { 0x0701FE00, "IEEE 1284 Target Device Parallel Controller", NULL},
    { 0x07020000, "Multiport Serial Controller", NULL},
    { 0x07030000, "Modem", NULL},
    { 0x07040000, "GPIB (IEEE 488.1/2) Controller", NULL},
    { 0x07050000, "Smart Card Controller", NULL},

    // Base System Peripherals
    { 0x08000000, "PIC (Generic 8259-compatible)", NULL},
    { 0x08010000, "DMA Controller", NULL},
    { 0x08020000, "System Timer", NULL},
    { 0x08030000, "RTC Controller", NULL},
    { 0x08040000, "PCI Hot-Plug Controller", NULL},
    { 0x08050000, "SD Host Controller", NULL},
    { 0x08060000, "IOMMU", NULL},
    { 0x08800000, "Other Base System Peripheral", NULL},

    // Input Devices
    { 0x09000000, "Keyboard Controller", NULL},
    { 0x09010000, "Digitizer Pen", NULL},
    { 0x09020000, "Mouse Controller", NULL},
    { 0x09030000, "Scanner Controller", NULL},
    { 0x09040000, "Gameport Controller (Legacy)", NULL},
    { 0x09040100, "Gameport Controller (Standard)", NULL},

    // Docking Stations
    { 0x0A000000, "Generic Docking Station", NULL},
    { 0x0A010000, "Docking Station with Expansion Bus", NULL},

    // Processors
    { 0x0B000000, "386 Processor", NULL},
    { 0x0B010000, "486 Processor", NULL},
    { 0x0B020000, "Pentium Processor", NULL},
    { 0x0B100000, "Alpha Processor", NULL},
    { 0x0B200000, "PowerPC Processor", NULL},
    { 0x0B300000, "MIPS Processor", NULL},
    { 0x0B400000, "Co-Processor", NULL},

    // Serial Bus Controllers
    { 0x0C000000, "FireWire (IEEE 1394) Controller", NULL},
    { 0x0C001000, "FireWire (IEEE 1394) Controller (OpenHCI)", NULL},
    { 0x0C010000, "ACCESS Bus Controller", NULL},
    { 0x0C020000, "Serial Storage Architecture Controller", NULL},
    { 0x0C030000, "UHCI USB Controller", NULL},
    { 0x0C031000, "OHCI USB Controller", NULL},
    { 0x0C032000, "EHCI USB Controller", NULL},
    { 0x0C033000, "xHCI USB Controller", NULL},
    { 0x0C038000, "USB Miscellaneous Controller", NULL},
    { 0x0C03FE00, "USB Device Controller", NULL},
    { 0x0C040000, "Fibre Channel Controller", NULL},
    { 0x0C050000, "SMBus Controller", NULL},
    { 0x0C060000, "InfiniBand Controller", NULL},
    { 0x0C070000, "IPMI Interface (SMIC)", NULL},
    { 0x0C070100, "IPMI Interface (Keyboard Controller Style)", NULL},
    { 0x0C070200, "IPMI Interface (Block Transfer)", NULL},
    { 0x0C080000, "SERCOS Interface (Serial Real-time Communication System)", NULL},
    { 0x0C090000, "CANbus Controller", NULL},
    { 0x0C090000, "Other Serial Bus Controller", NULL},

    // Wireless Controllers
    { 0x0D000000, "iRDA Controller", NULL},
    { 0x0D010000, "Consumer IR Controller", NULL},
    { 0x0D020000, "RF Controller", NULL},
    { 0x0D030000, "Bluetooth Controller", NULL},
    { 0x0D040000, "Broadband Controller", NULL},
    { 0x0D050000, "Ethernet Wireless Controller", NULL},
    { 0x0D060000, "Wireless Network Controller", NULL},

    // Intelligent I/O Controllers
    { 0x0E000000, "I2O Controller", NULL},

    // Satellite Communication Controllers
    { 0x0F000000, "Satellite TV Controller", NULL},
    { 0x0F010000, "Satellite Audio Communication Controller", NULL},
    { 0x0F020000, "Satellite Voice Communication Controller", NULL},
    { 0x0F030000, "Satellite Data Communication Controller", NULL},

    // Encryption/Decryption Controllers
    { 0x10000000, "Network and Computing Encryption/Decryption Controller", NULL},
    { 0x10010000, "Entertainment Encryption/Decryption Controller", NULL},

    // Data Acquisition and Signal Processing Controllers
    { 0x11000000, "DPIO Module", NULL},
    { 0x11010000, "Performance Counters", NULL},
    { 0x11020000, "Communications Synchronization Controller", NULL},
    { 0x11030000, "Signal Processing Controller", NULL},

    // Processing Accelerators
    { 0x12000000, "Processing Accelerator", NULL},
    { 0x12010000, "AI Accelerator", NULL},

    // Non-Essential Instrumentation
    { 0x13000000, "Instrumentation Device", NULL},

    // Other
    { 0xFF000000, "Unassigned class", NULL},

    // end of list
    { 0, NULL, NULL}
};

#endif