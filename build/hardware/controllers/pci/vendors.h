#ifndef BUILD_HARDWARE_CONTROLLERS_PCI_VENDORS_H
#define BUILD_HARDWARE_CONTROLLERS_PCI_VENDORS_H

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
    word_t vendor_id;
    byte_t *vendor_name;
} pci_vendor_id_list_t;

pci_vendor_id_list_t pci_vendor_list[] = {
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
    {0x8384, "SigmaTel"},
    {0x1AF4, "Red Hat, Inc."},
    {0x1000, "Broadcom / LSI"},
    {0x10EC, "Realtek Semiconductor Co., Ltd."},
    {0x1B36, "Red Hat, Inc."},
    {0, 0},
};

#endif /* BUILD_HARDWARE_CONTROLLERS_PCI_VENDORS_H */
