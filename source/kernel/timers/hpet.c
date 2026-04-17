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
#include <kernel/hardware/groups/logging/logging.h>
#include <kernel/firmware/main.h>
#include <kernel/cpu/interrupt.h>
#include <kernel/timers/main.h>

/* local variables */
acpi_gas_t hpet_gas;
uint32_t hpet_counter_clk_period_fs;
uint32_t ticks_per_microsecond;

/* functions */
int does_hpet_exist(void) {
    if(firmware_info.hpet != NULL) {
        return true;
    }
    else {
        return false;
    }
}

// TODO: implement also 32 bit main counter
int initialize_hpet(void) {
    // read HPET ACPI table
    hpet_table_t *hpet_table = (hpet_table_t *) firmware_info.hpet;
    int comparator_count = (hpet_table->comparator_count + 1);
    hpet_gas = hpet_table->gas_address;
    if(transform_acpi_gas_to_virtual_memory(&hpet_gas, 0x100 + (comparator_count * 0x20)) == false) {
        log("\n[HPET] Error: cannot map HPET MMIO registers");
        return ERROR;
    }

    // read HPET capabilities
    uint32_t capabilities = acpi_gas_read(hpet_gas, 0x00);
    if((((capabilities >> 8) & 0x1F) + 1) != comparator_count) {
        log("\n[HPET] ERROR: HPET comparator count (%d) does not match HPET table comparator count (%d)", (((capabilities >> 8) & 0x1F) + 1), comparator_count);
        return ERROR;
    }
    if(comparator_count < 3) {
        log("\n[HPET] ERROR: HPET has less than 3 comparators (%d)", comparator_count);
        return ERROR;
    }
    if(capabilities & (1 << 13)) {
        log("\n[HPET] HPET main counter is 64-bit");
    }
    else {
        log("\n[HPET] HPET main counter is 32-bit");
        return ERROR;
    }
    if((capabilities >> 16) != hpet_table->pci_vendor_id) {
        log("\n[HPET] Warning: HPET PCI Vendor ID (0x%04x) does not match HPET table PCI Vendor ID (0x%04x)", (capabilities >> 16), hpet_table->pci_vendor_id);
    }
    log("\n[HPET] Vendor ID: 0x%04x", capabilities >> 16);
    log("\n[HPET] Revision ID: 0x%02x", capabilities & 0xFF);
    log("\n[HPET] Number of comparators: %d", comparator_count);
    if(capabilities & (1 << 15)) {
        log("\n[HPET] Legacy replacement routing supported");
    }
    hpet_counter_clk_period_fs = acpi_gas_read(hpet_gas, 0x04);
    if(hpet_counter_clk_period_fs == 0 || hpet_counter_clk_period_fs > 0x05F5E100) {
        log("\n[HPET] Error: HPET counter clock period is invalid (%d femtoseconds)", hpet_counter_clk_period_fs);
        return ERROR;
    }
    log("\n[HPET] Counter clock period: %d femtoseconds", hpet_counter_clk_period_fs);
    ticks_per_microsecond = 1000000000 / hpet_counter_clk_period_fs;

    // disable all comparators
    for(int i = 0; i < comparator_count; i++) {
        acpi_gas_write(hpet_gas, 0x100 + (i * 0x20) + 0x00, 0x0);
    }

    // reset main counter
    acpi_gas_write(hpet_gas, 0x10, 0x0);
    acpi_gas_write(hpet_gas, 0xF0, 0x0);
    acpi_gas_write(hpet_gas, 0xF4, 0x0);
    acpi_gas_write(hpet_gas, 0x10, 0x1);

    // set time function
    get_time_in_microseconds = get_hpet_time_in_microseconds;

    return SUCCESS;
}

uint64_t get_hpet_time_in_microseconds(void) {
    uint32_t high, low;
    do {
        high = acpi_gas_read(hpet_gas, 0xF4);
        low = acpi_gas_read(hpet_gas, 0xF0);
    } while(high != acpi_gas_read(hpet_gas, 0xF4));

    return (((uint64_t)high << 32) | low) / ticks_per_microsecond;
}