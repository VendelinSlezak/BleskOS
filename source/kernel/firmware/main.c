/* 
* BleskOS
*
* MIT License
* Copyright (c) 2023-2026 BleskOS developers
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

/* includes */
#include <libc/string.h>
#include <kernel/firmware/bios_data_area/bda.h>
#include <kernel/firmware/pir/pir.h>
#include <kernel/firmware/mp_tables/mp_tables.h>
#include <kernel/firmware/acpi/acpi.h>
#include <kernel/firmware/smbios/smbios.h>
#include <kernel/memory/virtual_memory.h>
#include <kernel/memory/memory_allocators.h>
#include <kernel/hardware/groups/logging/logging.h>

/* global variables */
firmware_info_t firmware_info;

/* functions */
void read_firmware_data(void) {
    memset(&firmware_info, 0, sizeof(firmware_info_t));

    // load firmware data to virtual memory
    firmware_info.bda = temp_phy_alloc(0x400, 0x100, VM_KERNEL | VM_UNCACHEABLE);
    firmware_info.bios_code = temp_phy_alloc(0xE0000, 0x20000, VM_KERNEL | VM_UNCACHEABLE);

    // read BIOS data area
    read_bda_data();

    // search for tables
    search_for_pir_table();
    search_for_mp_floating_pointer_table();
    search_for_smbios_tables();
    search_for_acpi_tables();
}

int firmware_table_checksum_validation(void *ptr, uint32_t size) {
    uint8_t *ptr8 = ptr;
    uint8_t sum = 0;
    for(uint32_t i = 0; i < size; i++) {
        sum += ptr8[i];
    }
    return (sum == 0);
}