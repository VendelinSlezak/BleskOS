/* 
* BleskOS
*
* MIT License
* Copyright (c) 2023-2026 BleskOS developers
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include <kernel/hardware/controllers/pci/pci.h>

typedef struct {
    uint16_t vendor_id;
    uint8_t *vendor_name;
} pci_vendor_id_list_t;

typedef struct {
    uint16_t vendor_id;
    uint16_t device_id;
} pci_supported_classic_devices_by_driver_t;

typedef struct {
    uint16_t vendor_id;
    uint16_t device_id;
    uint16_t subsystem_vendor_id;
    uint16_t subsystem_id;
} pci_supported_subsystem_devices_by_driver_t;

typedef struct { 
    pci_supported_classic_devices_by_driver_t *driver_classic_devices;
    pci_supported_subsystem_devices_by_driver_t *driver_subsystem_devices;
    void (*initialize)(pci_device_t device);
} pci_drivers_for_type_t;

typedef struct { 
    uint32_t type;
    uint8_t *description;
    pci_drivers_for_type_t *drivers;
} pci_device_type_list_t;

extern pci_vendor_id_list_t pci_vendor_list[];
extern pci_device_type_list_t pci_device_type_list[];