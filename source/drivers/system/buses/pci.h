//BleskOS

/*
* MIT License
* Copyright (c) 2023-2025 BleskOS developers
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

struct pci_info_t {
    dword_t is_memory_access_supported;
    struct mcfg_table_t *mcfg;
};

struct pci_device_info_t {
    dword_t segment;
    dword_t segment_memory;
    dword_t bus;
    dword_t device;
    dword_t function;

    dword_t vendor_id;
    dword_t device_id;
    dword_t interrupt_pin;
    dword_t interrupt_line;
    dword_t msi_register;
    dword_t msi_x_register;

    dword_t bios_handoff_timeout;
};

struct pci_supported_devices_list_t {
    word_t vendor_id;
    word_t device_id;
};

struct pci_vendor_id_list_t {
    word_t vendor_id;
    byte_t *vendor_name;
};

struct pci_vendor_id_list_t pci_vendor_list[] = {
    {0x8086, "Intel Corporation"},
    {0x10DE, "NVIDIA Corporation"},
    {0x1002, "AMD"},
    {0x1022, "AMD"},
    {0x1234, "QEMU Virtual Device"},
    {0x1D6B, "Linux Foundation"},
    {0x046D, "Logitech, Inc."},
    {0x05AC, "Apple, Inc."},
    {0x106B, "Apple, Inc."},
    {0x090C, "Silicon Motion, Inc."},
    {0x0BDA, "Realtek Semiconductor Corp."},
    {0x0CF3, "Qualcomm Atheros"},
    {0x04B3, "IBM Corporation"},
    {0x1A86, "QinHeng Electronics"},
    {0x05E3, "Genesys Logic, Inc."},
    {0x0A5C, "Broadcom Inc."},
    {0x045E, "Microsoft Corporation"},
    {0x0461, "Primax Electronics, Ltd."},
    {0x1BCF, "Sunplus Innovation Technology"},
    {0x04D9, "Holtek Semiconductor, Inc."},
    {0x093A, "Pixart Imaging, Inc."},
    {0x22B8, "Motorola PCS"},
    {0x0FCE, "Sony Ericsson Mobile Communications"},
    {0x054C, "Sony Corporation"},
    {0x1D50, "OpenMoko, Inc."},
    {0x0E8D, "MediaTek Inc."},
    {0x0483, "STMicroelectronics"},
    {0x17EF, "Lenovo"},
    {0x18D1, "Google, Inc."},
    {0x058F, "Alcor Micro, Corp."},
    {0x04A9, "Canon Inc."},
    {0x03F0, "Hewlett-Packard"},
    {0x0BB4, "HTC Corporation"},
    {0x16C0, "Van Ooijen Technische Informatica"},
    {0x0763, "Midiman"},
    {0x0424, "Standard Microsystems Corp."},
    {0x07D1, "D-Link System"},
    {0x0B05, "ASUSTek Computer, Inc."},
    {0x0C45, "Microdia"},
    {0x0451, "Texas Instruments"},
    {0x0D8C, "C-Media Electronics, Inc."},
    {0x0A12, "Cambridge Silicon Radio, Ltd."},
    {0x0E0F, "VMware, Inc."},
    {0x1058, "Western Digital Technologies, Inc."},
    {0x0BC2, "Seagate Technology LLC"},
    {0x2109, "VIA Labs, Inc."},
    {0x2019, "Plantronics, Inc."},
    {0x2357, "TP-Link"},
    {0x26BD, "Gigabyte Technology Co., Ltd."},
    {0x27C6, "Goodix Technology"},
    {0x28DE, "Valve Corporation"},
    {0x32AC, "8BitDo"},
    {0x0BDA, "Realtek Semiconductor Corp."},  
    {0x02D0, "Lite-On Technology Corp."},
    {0x168C, "Atheros Communications, Inc."},  
    {0x14E4, "Broadcom Corporation"},  
    {0x13D3, "AzureWave Technologies, Inc."},  
    {0x0E79, "Xiaomi Communications Co., Ltd."},
    {0x04F2, "Chicony Electronics Co., Ltd."},
    {0x80EE, "InnoTek Systemberatung GmbH"},
    {0x11C1, "LSI Corporation"},
    {0x15AD, "VMware"},
    {0, 0},
};

struct pci_device_type_list_t {
    dword_t type;
    byte_t *description;
    void (*driver_add_new_pci_device)(struct pci_device_info_t device);
    struct pci_supported_devices_list_t *supported_devices;
};

extern void intel_graphic_card_add_new_pci_device(struct pci_device_info_t device);
extern struct pci_supported_devices_list_t intel_graphic_card_supported_pci_devices[];
extern void vmware_graphic_card_add_new_pci_device(struct pci_device_info_t device);
extern struct pci_supported_devices_list_t vmware_graphic_card_supported_pci_devices[];

struct pci_device_type_list_t pci_device_type_list[] = {
    {0x00000000, "Unclassified device", 0, 0},
    {0x00010000, "VGA-compatible unclassified device", 0, 0},

    // Mass Storage Controller
    {0x01000000, "SCSI Bus Controller", 0, 0},
    {0x01010000, "IDE Controller (ISA mode-only)", 0, 0},
    {0x01010500, "IDE Controller (PCI mode-only)", 0, 0},
    {0x01010A00, "IDE Controller (ISA mode, both channels switched to PCI mode)", 0, 0},
    {0x01010F00, "IDE Controller (PCI mode, both channels switched to ISA mode)", 0, 0},
    {0x01018000, "IDE Controller (ISA mode-only, bus mastering)", 0, 0},
    {0x01018500, "IDE Controller (PCI mode-only, bus mastering)", 0, 0},
    {0x01018A00, "IDE Controller (ISA mode, both channels switched to PCI mode, bus mastering)", 0, 0},
    {0x01018F00, "IDE Controller (PCI mode, both channels switched to ISA mode, bus mastering)", 0, 0},
    {0x01020000, "Floppy Disk Controller", 0, 0},
    {0x01030000, "IPI Bus Controller", 0, 0},
    {0x01040000, "RAID Controller", 0, 0},
    {0x01052000, "ATA Controller (single DMA)", 0, 0},
    {0x01058000, "ATA Controller (vendor specific)", 0, 0},
    {0x01060000, "Serial ATA Controller", 0, 0},
    {0x01060100, "Serial ATA Controller (AHCI 1.0)", 0, 0},
    {0x01070000, "Serial Attached SCSI (SAS) Controller", 0, 0},
    {0x01080000, "Non-volatile Memory Controller", 0, 0},

    // Network Controller
    {0x02000000, "Ethernet Controller", 0, 0},
    {0x02010000, "Token Ring Controller", 0, 0},
    {0x02020000, "FDDI Controller", 0, 0},
    {0x02030000, "ATM Controller", 0, 0},
    {0x02040000, "ISDN Controller", 0, 0},
    {0x02050000, "WorldFip Controller", 0, 0},
    {0x02060000, "PICMG 2.14 Multi Computing", 0, 0},
    {0x02070000, "Infiniband Controller", 0, 0},
    {0x02080000, "Fabric Controller", 0, 0},
    {0x02800000, "Wireless Controller", 0, 0},

    // Display Controller
    {0x03000000, "VGA Compatible Graphic Controller", intel_graphic_card_add_new_pci_device, intel_graphic_card_supported_pci_devices},
    {0x03000000, "", vmware_graphic_card_add_new_pci_device, vmware_graphic_card_supported_pci_devices},
    {0x03000100, "8514 Compatible Graphic Controller", 0, 0},
    {0x03010000, "XGA Graphic Controller", 0, 0},
    {0x03020000, "3D Graphic Controller", 0, 0},

    // Multimedia Controller
    {0x04000000, "Multimedia Video Controller", 0, 0},
    {0x04010000, "Multimedia Audio Controller", 0, 0},
    {0x04020000, "Computer Telephony Device", 0, 0},
    {0x04030000, "Audio Device", 0, 0},

    // Memory Controller
    {0x05000000, "RAM Controller", 0, 0},
    {0x05010000, "Flash Controller", 0, 0},

    // Bridge Devices
    {0x06000000, "Host Bridge", 0, 0},
    {0x06010000, "ISA Bridge", 0, 0},
    {0x06020000, "EISA Bridge", 0, 0},
    {0x06030000, "MCA Bridge", 0, 0},
    {0x06040000, "PCI-to-PCI Bridge", 0, 0},
    {0x06040100, "PCI-to-PCI Bridge (Subtractive Decode)", 0, 0},
    {0x06050000, "PCMCIA Bridge", 0, 0},
    {0x06060000, "NuBus Bridge", 0, 0},
    {0x06070000, "CardBus Bridge", 0, 0},
    {0x06080000, "RACEway Bridge", 0, 0},
    {0x06090000, "PCI-to-PCI Bridge (Semi-Transparent)", 0, 0},
    {0x060A0000, "InfiniBand-to-PCI Host Bridge", 0, 0},
    {0x06800000, "Other Bridge", 0, 0},

    // Simple Communication Controllers
    {0x07000000, "Serial Controller", 0, 0},
    {0x07010000, "Parallel Controller", 0, 0},
    {0x07020000, "Multiport Serial Controller", 0, 0},
    {0x07030000, "Modem", 0, 0},
    {0x07040000, "GPIB (IEEE 488.1/2) Controller", 0, 0},
    {0x07050000, "Smart Card Controller", 0, 0},

    // Base System Peripherals
    {0x08000000, "PIC (Generic 8259-compatible)", 0, 0},
    {0x08010000, "DMA Controller", 0, 0},
    {0x08020000, "System Timer", 0, 0},
    {0x08030000, "RTC Controller", 0, 0},
    {0x08040000, "PCI Hot-Plug Controller", 0, 0},
    {0x08050000, "SD Host Controller", 0, 0},
    {0x08060000, "IOMMU", 0, 0},

    // Input Devices
    {0x09000000, "Keyboard Controller", 0, 0},
    {0x09010000, "Digitizer Pen", 0, 0},
    {0x09020000, "Mouse Controller", 0, 0},

    // Docking Stations
    {0x0A000000, "Generic Docking Station", 0, 0},

    // Processors
    {0x0B000000, "386 Processor", 0, 0},
    {0x0B010000, "486 Processor", 0, 0},
    {0x0B020000, "Pentium Processor", 0, 0},

    // Serial Bus Controllers
    {0x0C000000, "FireWire (IEEE 1394) Controller", 0, 0},
    {0x0C010000, "ACCESS Bus Controller", 0, 0},

    // Wireless Controllers
    {0x0D000000, "iRDA Controller", 0, 0},

    // Intelligent I/O Controllers
    {0x0E000000, "I2O Controller", 0, 0},

    // Satellite Communication Controllers
    {0x0F000000, "Satellite TV Controller", 0, 0},

    // Encryption/Decryption Controllers
    {0x10000000, "Network and Computing Encryption/Decryption Controller", 0, 0},

    // Data Acquisition and Signal Processing Controllers
    {0x11000000, "DPIO Module", 0, 0},

    // Processing Accelerators
    {0x12000000, "Processing Accelerator", 0, 0},

    // Non-Essential Instrumentation
    {0x13000000, "Instrumentation Device", 0, 0},

    // Other
    {0xFF000000, "Unassigned class", 0, 0},

    // end of list
    {0, 0, 0, 0}
};

#define PCI_STATUS_IO (1 << 0)
#define PCI_STATUS_MMIO (1 << 1)
#define PCI_STATUS_BUSMASTERING (1 << 2)

dword_t pci_ind(struct pci_device_info_t device, dword_t offset);
word_t pci_inw(struct pci_device_info_t device, dword_t offset);
byte_t pci_inb(struct pci_device_info_t device, dword_t offset);
void pci_outd(struct pci_device_info_t device, dword_t offset, dword_t value);
void pci_set_bits(struct pci_device_info_t device, dword_t offset, dword_t bits);
void pci_clear_bits(struct pci_device_info_t device, dword_t offset, dword_t bits);
void pci_outw(struct pci_device_info_t device, dword_t offset, word_t value);
void pci_outb(struct pci_device_info_t device, dword_t offset, byte_t value);
byte_t *pci_get_vendor_name(word_t vendor_id);
byte_t *pci_get_device_type_string(dword_t type);
void pci_new_scan(void);
void pci_new_scan_device(struct pci_device_info_t device);
word_t pci_get_io(struct pci_device_info_t device, dword_t bar);
dword_t pci_get_mmio(struct pci_device_info_t device, dword_t bar);
void pci_device_install_interrupt_handler(struct pci_device_info_t device, void (*handler)(void));

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

dword_t pci_read(dword_t segment_memory, dword_t bus, dword_t device, dword_t function, dword_t offset);
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