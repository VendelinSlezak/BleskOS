//BleskOS

/*
* MIT License
* Copyright (c) 2023-2025 BleskOS developers
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

void read_acpi_tables(void) {
    logf("\n\n");

    // if ACPI table do not exist, we can assume that 8042 controller exist
    components->p_8042_controller = STATUS_TRUE;

    // search for RSDP table
    struct rsdp_table_t *rsdp = (struct rsdp_table_t *) 0xE0000;
    for(dword_t i = 0; i < 0x1FFF; i++, rsdp = (struct rsdp_table_t *) ((dword_t)rsdp + 0x10)) {
        if(rsdp->signature == 0x2052545020445352 && acpi_table_is_checksum_valid(rsdp, 20) == STATUS_TRUE) { //signature = 'RSD PTR '
            if(rsdp->revision >= 2 && acpi_table_is_checksum_valid((byte_t *)rsdp+20, 20) == STATUS_FALSE) {
                continue;
            }
            components->p_acpi = STATUS_TRUE;
            break;
        }
    }
    if(components->p_acpi == STATUS_FALSE && components->p_ebda == STATUS_TRUE) {
        rsdp = (struct rsdp_table_t *) components->ebda.memory;
        for(dword_t i = 0; i < 0x3F; i++, rsdp = (struct rsdp_table_t *) ((dword_t)rsdp + 0x10)) {
            if(rsdp->signature == 0x2052545020445352 && acpi_table_is_checksum_valid(rsdp, 20) == STATUS_TRUE) { //signature = 'RSD PTR '
                if(rsdp->revision >= 2 && acpi_table_is_checksum_valid((byte_t *)rsdp+20, 20) == STATUS_FALSE) {
                    continue;
                }
                components->p_acpi = STATUS_TRUE;
                break;
            }
        }
    }
    if(components->p_acpi == STATUS_FALSE) {
        logf("ACPI tables do not exist");
        return;
    }
    else {
        logf("ACPI TABLES INFO\nRSDP table at 0x%x", (dword_t)rsdp);
        logf("\n Revision: %d", rsdp->revision);
        logf("\n OEM ID: %06s", rsdp->oem_id);
    }

    // move to RSDT or XSDT table
    if(rsdp->revision == 1) { // unknown revision number
        return;
    }
    // RSDT table
    else if(rsdp->revision == 0) {
        struct rsdt_table_t *rsdt = (struct rsdt_table_t *) rsdp->rsdt_address;

        // check checksum and signature
        if(acpi_table_is_checksum_valid(rsdt, rsdt->header.length) == STATUS_FALSE) {
            logf("\nInvalid RSDT checksum");
            return;
        }
        if(rsdt->header.signature != 0x54445352) {
            logf("\nInvalid RSDT signature");
            return;
        }

        // log table info
        logf("\nRSDT table at 0x%x", (dword_t)rsdt);
        logf("\n Revision: %d", rsdt->header.revision);
        logf("\n OEM ID: %06s", rsdt->header.oem_id);
        logf("\n OEM table ID: %08s", rsdt->header.oem_table_id);

        // go through all ACPI tables
        for(dword_t i = 0; i < ((rsdt->header.length - sizeof(struct acpi_table_header_t))/4); i++) {
            read_acpi_table(rsdt->tables[i]);
        }
    }
    // XSDT table
    else {
        // BleskOS as 32-bit system can read memory only below 4 GB
        if((rsdp->xsdt_address >> 32) == 0) {
            struct xsdt_table_t *xsdt = (struct xsdt_table_t *) (dword_t)rsdp->xsdt_address;

            // check checksum and signature
            if(acpi_table_is_checksum_valid(xsdt, xsdt->header.length) == STATUS_FALSE) {
                logf("\nInvalid XSDT checksum");
                return;
            }
            if(xsdt->header.signature != 0x54445358) {
                logf("\nInvalid XSDT signature");
                return;
            }

            // log table info
            logf("\nXSDT table at 0x%x", (dword_t)xsdt);
            logf("\n Revision: %d", xsdt->header.revision);
            logf("\n OEM ID: %06s", xsdt->header.oem_id);
            logf("\n OEM table ID: %08s", xsdt->header.oem_table_id);

            // go through all ACPI tables
            for(dword_t i = 0; i < ((xsdt->header.length - sizeof(struct acpi_table_header_t))/8); i++) {
                // BleskOS as 32-bit system can read memory only below 4 GB
                if((xsdt->tables[i] >> 32) == 0) {
                    read_acpi_table((void *)(dword_t)xsdt->tables[i]);
                }
                else {
                    logf("\nCan not access table at 0x%x%x", xsdt->tables[i] >> 32, (dword_t)xsdt->tables[i]);
                }
            }
        }
        else {
            logf("\nCan not access XSDT table at 0x%x%x", rsdp->xsdt_address >> 32, (dword_t)rsdp->xsdt_address);
            return;
        }
    }
}

dword_t acpi_table_is_checksum_valid(void *table, dword_t length_of_table) {
    byte_t *pointer = (byte_t *) table;
    byte_t checksum = 0;

    for(dword_t i = 0; i < length_of_table; i++) {
        checksum += *pointer;
        pointer++;
    }

    if(checksum == 0) {
        return STATUS_TRUE;
    }
    else {
        return STATUS_FALSE;
    }
}

void read_acpi_table(void *table) {
    struct acpi_table_header_t *header = (struct acpi_table_header_t *) table;

    logf("\n%04s table revision %d at 0x%x ", &header->signature, header->revision, (dword_t)table);

    if(header->signature == 0x50434146) { // 'FACP'
        // check if this is duplicate table
        if(components->acpi.p_fadp == STATUS_TRUE) {
            return;
        }

        // save existence of table
        struct fadp_table_t *fadp = (struct fadp_table_t *) table;
        components->acpi.p_fadp = STATUS_TRUE;

        // read registers
        components->acpi.pm1a_control_reg = fadp->pm1a_cnt_blk;
        components->acpi.pm1b_control_reg = fadp->pm1b_cnt_blk;

        // TODO: this do not always work
        // // detect 8042 controller
        // if(header->revision >= 3 && (fadp->iapc_boot_arch & (1 << 1)) == 0) {
        //     components->p_8042_controller = STATUS_FALSE;
        // }

        logf("\n SCI interrupt: %d", fadp->sci_int);
        logf("\n ACPI control reg: 0x%04x", fadp->smi_cmd);
        logf("\n PM1a ctrl reg: 0x%04x", fadp->pm1a_cnt_blk);
        logf("\n PM1b ctrl reg: 0x%04x", fadp->pm1b_cnt_blk);

        // get DSDT pointer
        if(fadp->x_dsdt >> 32 == 0 && (dword_t)fadp->x_dsdt != 0) {
            components->acpi.dsdt = (struct acpi_table_header_t *)(dword_t)fadp->x_dsdt;
        }
        else if(fadp->dsdt != 0) {
            components->acpi.dsdt = (struct acpi_table_header_t *) fadp->dsdt;
        }
        if(components->acpi.dsdt != 0) {
            components->acpi.dsdt_aml_data = (byte_t *) ((dword_t)fadp->dsdt + sizeof(struct acpi_table_header_t));

            logf("\n DSDT: 0x%x", (dword_t)components->acpi.dsdt);
            parse_s5_shutdown_values();

            // dump_aml_objects(components->acpi.dsdt_aml_data, components->acpi.dsdt->length);
        }
    }
    else if(header->signature == 0x54455048) { // 'HPET'
        // check if this is duplicate table
        if(components->p_hpet == STATUS_TRUE) {
            return;
        }

        // check if we can use this timer
        struct hpet_table_t *hpet = (struct hpet_table_t *) table;
        if((hpet->base_address.address >> 32) != 0) {
            logf("\n Unsupported 64-bit address");
            return;
        }
        if(hpet->base_address.address == 0) {
            logf("\n Invalid base address");
            return;
        }

        // save existence of table
        components->p_hpet = STATUS_TRUE;
        components->hpet.port_type = hpet->base_address.address_type;
        components->hpet.base = (dword_t)hpet->base_address.address;

        // log
        logf("\n Vendor name: %s", pci_get_vendor_name(hpet->id >> 16));
    }
    else if(header->signature == 0x4746434D) { // 'MCFG'
        // check if this is duplicate table
        if(components->pci.is_memory_access_supported == STATUS_TRUE) {
            return;
        }

        // save data about table
        components->pci.is_memory_access_supported = STATUS_TRUE;
        components->pci.mcfg = table;
    }
}