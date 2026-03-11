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
#include <kernel/hardware/groups/logging/logging.h>

/* functions */
void search_for_pir_table(void) {
    pir_table_t *pir = NULL;

    // search for PIR table
    for(uint32_t offset = 0x10000; offset < (0x20000 - sizeof(pir_table_t)); offset += 16) {
        pir_table_t *candidate = (pir_table_t *) (firmware_info.bios_code + offset);
        if(    candidate->signature != 0x52495024  // "$PIR"
            || (offset + candidate->table_size) > 0x20000
            || candidate->table_size < sizeof(pir_table_t)
            || !firmware_table_checksum_validation((uint8_t *) candidate, candidate->table_size)) {
            continue;
        }
        log("\nPIR table found at 0x%05x", 0xE0000 + offset);
        pir = candidate;
        break;
    }
    if(pir == NULL) {
        return;
    }

    // log content of PIR table
    int number_of_pir_entries = (pir->table_size - sizeof(pir_table_t)) / sizeof(pir_table_entry_t);
    log("\n Version: %d.%d", (pir->version >> 8) & 0xFF, pir->version & 0xFF);
    log("\n Router PCI location: 0:%d:%d:%d", pir->router_bus, pir->router_dev, pir->router_func);
    for(int i = 0; i < number_of_pir_entries; i++) {
        for(int pin = 0; pin < 4; pin++) {
            log("\n PCI location 0:%d:%d:* pin %c has link 0x%02x with available ISA IRQs ", pir->entries[i].bus, pir->entries[i].slot, 'A' + pin, pir->entries[i].link[pin]);
            for(int j = 0; j < 16; j++) {
                if((pir->entries[i].bitmap[pin] >> j) & 0x1) {
                    log("%d ", j);
                }
            }

            // TODO: save useful info from PIR table for later use
        }
    }
}