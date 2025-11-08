/* 
* BleskOS
*
* MIT License
* Copyright (c) 2023-2025 BleskOS developers
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

/* includes */
#include <kernel/x86/kernel.h>
#include <hardware/groups/logging/logging.h>
#include <kernel/x86/memory/vm_allocator.h>
#include <kernel/x86/acpi/fadp.h>
#include <kernel/x86/acpi/aml.h>

/* functions */
void read_acpi(void) {
    // clear all map pointers
    kernel_attr->p_acpi = FALSE;
    kernel_attr->pm_madt = INVALID;
    kernel_attr->pm_hpet = INVALID;
    kernel_attr->pm_mcfg = INVALID;

    // map first MB to virtual memory
    void *first_mb_of_phy_mem = kpalloc(0x00000000, 1024*1024*1, VM_FLAG_WRITE_BACK);

    // search for RSDP / XSDP table in BIOS memory
    xsdp_table_t *xsdp = (xsdp_table_t *) ((dword_t)first_mb_of_phy_mem + 0xE0000);
    for(int i = 0; i < 0x1FFF; i++, xsdp = (xsdp_table_t *) ((dword_t)xsdp + 0x10)) {
        if(xsdp->signature == ACPI_SIGNATURE_64("RSD PTR ") && acpi_table_is_checksum_valid(xsdp, 20) == TRUE) {
            if(xsdp->revision >= 2 && acpi_table_is_checksum_valid(xsdp, 36) == FALSE) {
                continue;
            }
            kernel_attr->p_acpi = TRUE;
            break;
        }
    }

    // try to search in Extended BIOS Data Area memory
    if(kernel_attr->p_acpi == FALSE) {
        // check presence of EBDA
        word_t *ebda_memory_start = (word_t *) ((dword_t)first_mb_of_phy_mem + 0x40E);
        byte_t *ebda_memory_size = (byte_t *) ((dword_t)first_mb_of_phy_mem + 0x413);
        if(*ebda_memory_start != 0 && *ebda_memory_size != 0) {
            xsdp = (xsdp_table_t *) ((dword_t)first_mb_of_phy_mem +  (*ebda_memory_start * 0x10));

            // check first KB od EBDA
            for(int i = 0; i < 0x3F; i++, xsdp = (xsdp_table_t *) ((dword_t)xsdp + 0x10)) {
                if(xsdp->signature == ACPI_SIGNATURE_64("RSD PTR ") && acpi_table_is_checksum_valid(xsdp, 20) == TRUE) {
                    if(xsdp->revision >= 2 && acpi_table_is_checksum_valid(xsdp, 36) == FALSE) {
                        continue;
                    }
                    kernel_attr->p_acpi = TRUE;
                    break;
                }
            }
        }
    }

    // quit function or log RSDP table
    if(kernel_attr->p_acpi == FALSE) {
        unmap(first_mb_of_phy_mem);
        log("\n[ERROR] ACPI tables do not exist");
        return;
    }
    else {
        log("\n[ACPI] ");
        if(xsdp->revision < 2) {
            log("RSDP");
        }
        else {
            log("XSDP");
        }
        log(" table found at 0x%x | Revision: %d | OEM ID: %06s", (dword_t)xsdp, xsdp->revision, xsdp->oem_id);
    }

    // move to RSDT or XSDT table
    if(xsdp->revision == 1) { // unknown revision number
        return;
    }
    // RSDT table
    else if(xsdp->revision == 0) {
        dword_t table_start = xsdp->rsdt_address;
        unmap(first_mb_of_phy_mem);

        void *vm_of_rsdt_table = kpalloc(table_start, sizeof(rsdt_table_t), VM_FLAG_WRITE_BACK);
        rsdt_table_t *rsdt = (rsdt_table_t *) (vm_of_rsdt_table + (table_start & 0xFFF));
        dword_t size_of_table = rsdt->header.length;
        unmap(vm_of_rsdt_table);
        vm_of_rsdt_table = kpalloc(table_start, size_of_table, VM_FLAG_WRITE_BACK);
        rsdt = (rsdt_table_t *) (vm_of_rsdt_table + (table_start & 0xFFF));

        // check checksum and signature
        if(rsdt->header.signature != ACPI_SIGNATURE_32("RSDT")) {
            unmap(vm_of_rsdt_table);
            log("\n[ERROR] Invalid RSDT signature");
            return;
        }
        if(acpi_table_is_checksum_valid(rsdt, rsdt->header.length) == FALSE) {
            unmap(vm_of_rsdt_table);
            log("\n[ERROR] Invalid RSDT checksum");
            return;
        }

        // log table info
        log("\n[ACPI] RSDT table found at 0x%x | Revision: %d | OEM ID: %06s | OEM table ID: %08s",
            table_start,
            rsdt->header.revision,
            rsdt->header.oem_id,
            rsdt->header.oem_table_id);

        // go through all ACPI tables
        for(dword_t i = 0; i < ((rsdt->header.length - sizeof(acpi_table_header_t))/4); i++) {
            table_start = (dword_t) rsdt->tables[i];
            void *vm_of_table = kpalloc(table_start, sizeof(acpi_table_header_t), VM_FLAG_WRITE_BACK);
            acpi_check_table(table_start, (acpi_table_header_t *) (vm_of_table + (table_start & 0xFFF)));
            unmap(vm_of_table);
        }
        unmap(vm_of_rsdt_table);
    }
    // XSDT table
    else {
        // BleskOS as 32-bit system can read memory only below 4 GB
        if((xsdp->xsdt_address >> 32) == 0) {
            dword_t table_start = xsdp->xsdt_address;
            unmap(first_mb_of_phy_mem);

            void *vm_of_xsdt_table = kpalloc(table_start, sizeof(rsdt_table_t), VM_FLAG_WRITE_BACK);
            xsdt_table_t *xsdt = (xsdt_table_t *) (vm_of_xsdt_table + (table_start & 0xFFF));
            dword_t size_of_table = xsdt->header.length;
            unmap(vm_of_xsdt_table);
            vm_of_xsdt_table = kpalloc(table_start, size_of_table, VM_FLAG_WRITE_BACK);
            xsdt = (xsdt_table_t *) (vm_of_xsdt_table + (table_start & 0xFFF));

            // check checksum and signature
            if(xsdt->header.signature != ACPI_SIGNATURE_32("XSDT")) {
                unmap(vm_of_xsdt_table);
                log("\n[ERROR] Invalid XSDT signature");
                return;
            }
            if(acpi_table_is_checksum_valid(xsdt, xsdt->header.length) == FALSE) {
                unmap(vm_of_xsdt_table);
                log("\n[ERROR] Invalid XSDT checksum");
                return;
            }

            // log table info
            log("\n[ACPI] XSDT table found at 0x%x | Revision: %d | OEM ID: %06s | OEM table ID: %08s",
                table_start,
                xsdt->header.revision,
                xsdt->header.oem_id,
                xsdt->header.oem_table_id);

            // go through all ACPI tables
            for(dword_t i = 0; i < ((xsdt->header.length - sizeof(acpi_table_header_t))/8); i++) {
                // BleskOS as 32-bit system can read memory only below 4 GB
                if((xsdt->tables[i] >> 32) != 0) {
                    log("\n[ACPI] Can not access table at 0x%x%x", xsdt->tables[i] >> 32, (dword_t)xsdt->tables[i]);
                    continue;
                }

                table_start = (dword_t) xsdt->tables[i];
                void *vm_of_table = kpalloc(table_start, sizeof(acpi_table_header_t), VM_FLAG_WRITE_BACK);
                acpi_check_table(table_start, (acpi_table_header_t *) (vm_of_table + (table_start & 0xFFF)));
                unmap(vm_of_table);
            }
            unmap(vm_of_xsdt_table);
        }
        else {
            log("\n[ACPI] Can not access XSDT table at 0x%x%x", (dword_t)(xsdp->xsdt_address >> 32), (dword_t)xsdp->xsdt_address);
            return;
        }
    }
}

dword_t acpi_table_is_checksum_valid(void *table, dword_t length_of_table) {
    byte_t *pointer = (byte_t *) table;
    byte_t checksum = 0;

    for(dword_t i = 0; i < length_of_table; i++, pointer++) {
        checksum += *pointer;
    }

    if(checksum == 0) {
        return TRUE;
    }
    else {
        return FALSE;
    }
}

void acpi_check_table(dword_t table_start, acpi_table_header_t *acpi_table) {
    log("\n[ACPI] %04s table found at 0x%x | Revision: %d | OEM ID: %06s | OEM table ID: %08s",
        &acpi_table->signature,
        table_start,
        acpi_table->revision,
        &acpi_table->oem_id,
        &acpi_table->oem_table_id);
    
    if(acpi_table->signature == ACPI_SIGNATURE_32("APIC")) {
        kernel_attr->pm_madt = table_start;
        kernel_attr->size_of_madt = acpi_table->length;
    }
    else if(acpi_table->signature == ACPI_SIGNATURE_32("HPET")) {
        kernel_attr->pm_hpet = table_start;
        kernel_attr->size_of_hpet = acpi_table->length;
    }
    else if(acpi_table->signature == ACPI_SIGNATURE_32("MCFG")) {
        kernel_attr->pm_mcfg = table_start;
        kernel_attr->size_of_mcfg = acpi_table->length;
    }
    else if(acpi_table->signature == ACPI_SIGNATURE_32("FACP")) {
        fadp_table_t *fadp = (fadp_table_t *) acpi_table;
        void *vm_of_table = kpalloc(fadp->dsdt, sizeof(acpi_table_header_t), VM_FLAG_WRITE_BACK);
        acpi_table_header_t *dsdt_header = (acpi_table_header_t *) (vm_of_table + (fadp->dsdt & 0xFFF));
        dword_t dsdt_size = dsdt_header->length;
        log("\n[ACPI] DSDT table found at 0x%x | Revision: %d | OEM ID: %06s | OEM table ID: %08s",
            fadp->dsdt,
            dsdt_header->revision,
            &dsdt_header->oem_id,
            &dsdt_header->oem_table_id);
        unmap(vm_of_table);
        vm_of_table = kpalloc(fadp->dsdt, dsdt_size, VM_FLAG_WRITE_BACK);
        dsdt_header = (acpi_table_header_t *) (vm_of_table + (fadp->dsdt & 0xFFF));
        dump_aml_code((byte_t *)dsdt_header, dsdt_header->length);
        unmap(vm_of_table);
    }
}