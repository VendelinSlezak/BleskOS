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
#include <kernel/firmware/main.h>
#include <kernel/memory/virtual_memory.h>
#include <kernel/memory/memory_allocators.h>
#include <kernel/hardware/groups/logging/logging.h>

/* functions */
void read_bda_data(void) {
    bios_data_area_t *bda = (bios_data_area_t *) firmware_info.bda;
    log("\nBIOS data area");
    int number_of_com_ports = (bda->number_of_com_ports > 4) ? 4 : bda->number_of_com_ports;
    for(int i = 0; i < number_of_com_ports; i++) {
        log("\n COM%d I/O Port: 0x%04x", i + 1, bda->com_io_ports[i]);
    }
    for(int i = 0; i < bda->number_of_lpt_ports; i++) {
        log("\n LPT%d I/O Port: 0x%04x", i + 1, bda->lpt_io_ports[i]);
    }
    if(bda->is_floppy_present == 1) {
        log("\n Floppy Drive Present");
    }
    if(bda->is_math_coprocessor_present == 1) {
        log("\n Math Coprocessor Present");
    }
    if(bda->is_ps2_mouse_present == 1) {
        log("\n PS/2 Mouse Present");
    }
    log("\n Graphic Output Type: ", bda->graphic_output_type);
    switch(bda->graphic_output_type) {
        case 0b00: log("EGA (or newer)"); break;
        case 0b01: log("CGA 40x25"); break;
        case 0b10: log("CGA 80x25"); break;
        case 0b11: log("MDA"); break;
    }
    log("\n Number of Floppy Drives: %d", bda->number_of_floppy_drives + 1);
    if(bda->is_dma_controller_present == 1) {
        log("\n DMA Controller Present");
    }
    if(bda->is_gameport_present == 1) {
        log("\n Gameport Present");
    }
    if(bda->is_internal_modem_present == 1) {
        log("\n Internal Modem Present");
    }
    uint32_t conventional_memory_size = bda->size_of_usable_memory_in_bytes * 1024;
    if(conventional_memory_size > 640 * 1024) {
        log("\n Warning: Conventional memory size reported by BDA is larger than 640 KB, adjusting to 640 KB");
        conventional_memory_size = 640 * 1024;
    }
    if(conventional_memory_size < 4 * 1024) {
        log("\n Warning: Conventional memory size reported by BDA is smaller than 4 KB, adjusting to 640 KB");
        conventional_memory_size = 4 * 1024;
    }
    firmware_info.last_kb_of_conventional_memory = temp_phy_alloc(conventional_memory_size - 0x400, 0x400, VM_KERNEL | VM_UNCACHEABLE);
    log("\n Size of Usable Memory: %d KB", bda->size_of_usable_memory_in_bytes);
    uint32_t ebda_base = bda->ebda_base * 0x10;
    if(ebda_base < 0xA0000) {
        log("\n EBDA Base: 0x%x", bda->ebda_base * 0x10);
        if(conventional_memory_size == ebda_base) {
            uint32_t ebda_size = 0xA0000 - conventional_memory_size;
            if(ebda_size <= 128 * 1024) {
                log("\n EBDA Size: %d B", ebda_size);
                firmware_info.ebda = temp_phy_alloc(ebda_base, ebda_size, VM_KERNEL | VM_UNCACHEABLE);
            }
            else {
                log("\n Warning: EBDA size calculated from BDA is larger than 128 KB, ignoring EBDA");
            }
        }
        else {
            log("\n Warning: EBDA base reported by BDA does not match conventional memory size, ignoring EBDA");
        }
    }
    else {
        log("\n Warning: EBDA base reported by BDA is invalid, ignoring EBDA");
    }
}