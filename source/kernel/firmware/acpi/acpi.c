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
#include <kernel/cpu/commands.h>
#include <kernel/firmware/main.h>
#include <kernel/firmware/acpi/madt.h>
#include <kernel/memory/virtual_memory.h>
#include <kernel/memory/memory_allocators.h>
#include <kernel/hardware/groups/logging/logging.h>

/* functions */
void search_for_acpi_tables(void) {
    // search in 0xE0000 - 0xFFFFF
    acpi_sdp_table_ptr_t sdp = search_for_sdb_table_in_memory(firmware_info.bios_code + 0x00000, 0x20000);
    if(sdp.rsdt_pm == NULL && sdp.xsdt_pm == NULL) {
        // search in first KB of EBDA
        if(firmware_info.ebda != NULL) {
            search_for_sdb_table_in_memory(firmware_info.ebda, 0x400);
        }
        if(sdp.rsdt_pm == NULL && sdp.xsdt_pm == NULL) {
            return;
        }
    }

    // go through RSDT / XSDT
    if(sdp.rsdt_pm != NULL) {
        acpi_table_header_t *header = temp_phy_alloc(sdp.rsdt_pm, sizeof(acpi_table_header_t), VM_KERNEL);
        if(header->signature != ACPI_SIGNATURE_32("RSDT")) {
            log("\n Error: RSDT table has invalid signature");
            return;
        }
        rsdt_table_t *rsdt = temp_phy_alloc(sdp.rsdt_pm, header->length, VM_KERNEL);
        if(!firmware_table_checksum_validation((uint8_t *) rsdt, header->length)) {
            log("\n Error: RSDT table has invalid checksum");
            return;
        }
        log("\nRSDT table | Revision: %d | OEM: %06s | OEM table ID: %08s", header->revision, header->oem_id, header->oem_table_id);
        
        // check each table listed in RSDT
        uint32_t number_of_tables = (header->length - sizeof(acpi_table_header_t)) / 4;
        for(int i = 0; i < number_of_tables; i++) {
            check_acpi_table(rsdt->tables[i]);
        }
    }
    else if(sdp.xsdt_pm != NULL) {
        acpi_table_header_t *header = temp_phy_alloc(sdp.xsdt_pm, sizeof(acpi_table_header_t), VM_KERNEL);
        if(header->signature != ACPI_SIGNATURE_32("XSDT")) {
            log("\n Error: XSDT table has invalid signature");
            return;
        }
        xsdt_table_t *xsdt = temp_phy_alloc(sdp.xsdt_pm, header->length, VM_KERNEL);
        if(!firmware_table_checksum_validation((uint8_t *) xsdt, header->length)) {
            log("\n Error: XSDT table has invalid checksum");
            return;
        }
        log("\nXSDT table | Revision: %d | OEM: %06s | OEM table ID: %08s", header->revision, header->oem_id, header->oem_table_id);
        
        // check each table listed in XSDT
        uint32_t number_of_tables = (header->length - sizeof(acpi_table_header_t)) / 8;
        for(int i = 0; i < number_of_tables; i++) {
            if(xsdt->tables[i] >= 0x100000000) {
                continue;
            }
            check_acpi_table((uint32_t) xsdt->tables[i]);
        }
    }
}

acpi_sdp_table_ptr_t search_for_sdb_table_in_memory(void *start, uint32_t size) {
    acpi_sdp_table_ptr_t sdp_ptr = {0};
    
    for(uint32_t offset = 0; offset < (size - sizeof(rsdp_table_t)); offset += 16) {
        rsdp_table_t *rsdp = (rsdp_table_t *) (start + offset);
        if(    rsdp->signature != ACPI_SIGNATURE_64("RSD PTR ")
            || (offset + sizeof(rsdp_table_t)) > size
            || !firmware_table_checksum_validation((uint8_t *) rsdp, sizeof(rsdp_table_t))) {
            continue;
        }

        if(rsdp->revision < 2) {
            log("\nRSDP table | Revision: %d | OEM: %06s", rsdp->revision, rsdp->oem_id);
            sdp_ptr.rsdt_pm = rsdp->rsdt_address;
        }
        else {
            xsdp_table_t *xsdp = (xsdp_table_t *) (start + offset);
            if(    (offset + sizeof(xsdp_table_t)) > size
                || !firmware_table_checksum_validation((uint8_t *) xsdp, sizeof(xsdp_table_t))) {
                continue;
            }
            log("\nXSDP table | Revision: %d | OEM: %06s", xsdp->revision, xsdp->oem_id);
            if(xsdp->xsdt_address < 0x100000000) {
                sdp_ptr.xsdt_pm = xsdp->xsdt_address;
            }
            else {
                log("\n Warning: XSDT physical address is above 4 GB, ignoring");
            }
        }

        break;
    }

    return sdp_ptr;
}

void check_acpi_table(uint32_t table_pm) {
    acpi_table_header_t *header = temp_phy_alloc(table_pm, sizeof(acpi_table_header_t), VM_KERNEL);
    log("\nACPI Table %04s | Revision: %d | OEM: %06s | OEM table ID: %08s", &header->signature, header->revision, header->oem_id, header->oem_table_id);

    // check if we will further process this table
    if(    header->signature != ACPI_SIGNATURE_32("FACP")
        && header->signature != ACPI_SIGNATURE_32("APIC")
        && header->signature != ACPI_SIGNATURE_32("MCFG")
        && header->signature != ACPI_SIGNATURE_32("HPET")) {
        return;
    }

    // load table to virtual memory and check checksum
    void *table = temp_phy_alloc(table_pm, header->length, VM_KERNEL);
    if(!firmware_table_checksum_validation((uint8_t *) table, header->length)) {
        log(" | Error: invalid checksum");
        return;
    }

    // save table pointer
    if(header->signature == ACPI_SIGNATURE_32("FACP")) {
        firmware_info.fadt = table;
    }
    else if(header->signature == ACPI_SIGNATURE_32("APIC")) {
        firmware_info.madt = table;
        dump_madt_table();
    }
    else if(header->signature == ACPI_SIGNATURE_32("MCFG")) {
        firmware_info.mcfg = table;
        firmware_info.size_of_mcfg = header->length;
    }
    else if(header->signature == ACPI_SIGNATURE_32("HPET")) {
        firmware_info.hpet = table;
    }
}

int transform_acpi_gas_to_virtual_memory(acpi_gas_t *gas, uint32_t size) {
    if(gas->address_space_id == 0) { // system memory
        if(gas->address >= 0x100000000) {
            log("\n[ACPI GAS] Error: ACPI GAS address is above 4GB boundary");
            return ERROR;
        }
        gas->address = (uint32_t) perm_phy_alloc((uint32_t)gas->address, size, VM_KERNEL | VM_UNCACHEABLE);
    }
    return SUCCESS;
}

uint32_t acpi_gas_read(acpi_gas_t gas, uint32_t offset) {
    if(gas.address_space_id == 0) { // System Memory (MMIO)
        return *((volatile uint32_t *)((uint32_t)gas.address + offset));
    } 
    else if(gas.address_space_id == 1) { // System I/O
        return ind((uint16_t)(gas.address + offset));
    }
    else {
        return 0;
    }
}

void acpi_gas_write(acpi_gas_t gas, uint32_t offset, uint64_t value) {
    if(gas.address_space_id == 0) { // System Memory (MMIO)
        *((volatile uint32_t *)((uint32_t)gas.address + offset)) = (uint32_t)value;
    } 
    else if(gas.address_space_id == 1) { // System I/O
        outd((uint16_t)(gas.address + offset), (uint32_t)value);
    }
}