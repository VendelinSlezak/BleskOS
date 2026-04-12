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
void search_for_smbios_tables(void) {
    smbios_32_bit_entry_point_t *entry_point = NULL;

    // search for SMBIOS entry point table
    for(uint32_t offset = 0x10000; offset < (0x20000 - sizeof(smbios_32_bit_entry_point_t)); offset += 16) {
        entry_point = (smbios_32_bit_entry_point_t *) (firmware_info.bios_code + offset);
        if(    entry_point->anchor_string != 0x5F4D535F // "_SM_"
            || !(entry_point->length == 0x1E || entry_point->length == 0x1F)
            || !firmware_table_checksum_validation((uint8_t *) entry_point, sizeof(smbios_32_bit_entry_point_t))) {
                // TODO: more checks
            continue;
        }
        log("\nSMBIOS entry point table found at 0x%05x", 0xE0000 + offset);
        break;
    }
    if(entry_point == NULL) {
        return;
    }

    // log everything about entry point
    log("\n SMBIOS Version: %d.%d", entry_point->major_version, entry_point->minor_version);
    log("\n SMBIOS Entry Point Revision: %d", entry_point->entry_point_revision);
    log("\n SMBIOS BCD Revision: 0x%02x", entry_point->bcd_revision);
    
    // read all structures
    uint8_t *structure_ptr = temp_phy_alloc(entry_point->table_address, entry_point->table_length, VM_KERNEL);
    uint8_t *structure_end = structure_ptr + entry_point->table_length;
    for(int i = 0; i < entry_point->number_of_structures; i++) {
        if(structure_ptr >= structure_end) {
            break;
        }

        smbios_structure_header_t *structure_header = (smbios_structure_header_t *) structure_ptr;
        if(structure_header->type == 0) {
            smbios_platform_firmware_info_t *structure = (smbios_platform_firmware_info_t *) structure_ptr;
            log("\n Platform Firmware Information");
            if(structure->header.length >= 0x12) {
                log("\n  Vendor: %s", get_smbios_structure_string(structure, structure_end, structure->vendor_string));
                log("\n  Firmware Version: %s", get_smbios_structure_string(structure, structure_end, structure->firmware_version_string));
                log("\n  Firmware Release Date: %s", get_smbios_structure_string(structure, structure_end, structure->firmware_release_date_string));
                log("\n  Firmware ROM Size: %d KB", (structure->firmware_rom_size + 1) * 64);
                log("\n  BIOS Starting Segment: 0x%04x", structure->bios_starting_address_segment);
                /*log("\n  Firmware Characteristics Supported: %s", (structure->firmware_characteristics_supported) ? "Yes" : "No");
                log("\n  ISA Supported: %s", (structure->isa_supported) ? "Yes" : "No");
                log("\n  MCA Supported: %s", (structure->mca_supported) ? "Yes" : "No");
                log("\n  EISA Supported: %s", (structure->eisa_supported) ? "Yes" : "No");
                log("\n  PCI Supported: %s", (structure->pci_supported) ? "Yes" : "No");
                log("\n  PCMCIA Supported: %s", (structure->pcmcia_supported) ? "Yes" : "No");
                log("\n  Plug and Play Supported: %s", (structure->plug_and_play_supported) ? "Yes" : "No");
                log("\n  APM Supported: %s", (structure->apm_supported) ? "Yes" : "No");
                log("\n  Upgradeable Firmware: %s", (structure->upgradeable_firmware) ? "Yes" : "No");
                log("\n  Firmware Shadowing Allowed: %s", (structure->firmware_shadowing_allowed) ? "Yes" : "No");
                log("\n  VL VESA Supported: %s", (structure->vl_vesa_supported) ? "Yes" : "No");
                log("\n  ESCD Supported: %s", (structure->escd_supported) ? "Yes" : "No");
                log("\n  CD Boot Supported: %s", (structure->cd_boot_supported) ? "Yes" : "No");
                log("\n  Selectable Boot Supported: %s", (structure->selectable_boot_supported) ? "Yes" : "No");
                log("\n  Firmware ROM is Socketed: %s", (structure->firmware_rom_is_socketed) ? "Yes" : "No");
                log("\n  PCMCIA Boot Supported: %s", (structure->pcmcia_boot_supported) ? "Yes" : "No");
                log("\n  EDD Supported: %s", (structure->edd_supported) ? "Yes" : "No");
                log("\n  INT 13h Floppy NEC 9800 Supported: %s", (structure->int13_floppy_nec_9800_supported) ? "Yes" : "No");
                log("\n  INT 13h Floppy Toshiba Supported: %s", (structure->int13_floppy_toshiba_supported) ? "Yes" : "No");
                log("\n  INT 13h Floppy 360KB Supported: %s", (structure->int13_floppy_360kb_supported) ? "Yes" : "No");
                log("\n  INT 13h Floppy 1.2MB Supported: %s", (structure->int13_floppy_1_2mb_supported) ? "Yes" : "No");
                log("\n  INT 13h Floppy 720KB Supported: %s", (structure->int13_floppy_720kb_supported) ? "Yes" : "No");
                log("\n  INT 13h Floppy 2.88MB Supported: %s", (structure->int13_floppy_2_88mb_supported) ? "Yes" : "No");
                log("\n  INT 5h Print Screen Service Supported: %s", (structure->int5_print_screen_service_supported) ? "Yes" : "No");
                log("\n  INT 9h 8042 Keyboard Services Supported: %s", (structure->int9_8042_keyboard_services_supported) ? "Yes" : "No");
                log("\n  INT 14h Serial Services Supported: %s", (structure->int14_serial_services_supported) ? "Yes" : "No");
                log("\n  INT 17h Printer Services Supported: %s", (structure->int17_printer_services_supported) ? "Yes" : "No");
                log("\n  INT 10h CGA/Mono Video Services Supported: %s", (structure->int10_cga_mono_video_services_supported) ? "Yes" : "No");
                log("\n  NEC PC-98: %s", (structure->nec_pc98) ? "Yes" : "No");
                if(entry_point->major_version >= 2) {
                    if((entry_point->major_version > 2 || entry_point->minor_version >= 1) && structure->header.length >= 0x13) {
                        log("\n  ACPI Supported: %s", (structure->acpi_supported) ? "Yes" : "No");
                        log("\n  USB Legacy BIOS Supported: %s", (structure->usb_legacy_supported) ? "Yes" : "No");
                        log("\n  AGP Supported: %s", (structure->agp_supported) ? "Yes" : "No");
                        log("\n  I2O Boot Supported: %s", (structure->i2o_boot_supported) ? "Yes" : "No");
                        log("\n  LS-120 Superdisk Boot Supported: %s", (structure->ls120_superdisk_boot_supported) ? "Yes" : "No");
                        log("\n  ATAPI ZIP Drive Boot Supported: %s", (structure->atapi_zip_drive_boot_supported) ? "Yes" : "No");
                        log("\n  IEEE 1394 Boot Supported: %s", (structure->ieee_1394_boot_supported) ? "Yes" : "No");
                        log("\n  Smart Battery Supported: %s", (structure->smart_battery_supported) ? "Yes" : "No");
                    }
                    if((entry_point->major_version > 2 || entry_point->minor_version >= 3) && structure->header.length >= 0x14) {
                        log("\n  BIOS Boot Specification Supported: %s", (structure->bios_boot_specification_supported) ? "Yes" : "No");
                        log("\n  Function Key-initiated Network Boot Supported: %s", (structure->network_service_boot_supported) ? "Yes" : "No");
                        log("\n  Enable Targeted Content Distribution: %s", (structure->enable_targeted_content_distribution) ? "Yes" : "No");
                        log("\n  UEFI Supported: %s", (structure->uefi_supported) ? "Yes" : "No");
                        log("\n  Virtual Machine Running: %s", (structure->smbios_of_virtual_machine) ? "Yes" : "No");
                        log("\n  Manufacturing Mode Supported: %s", (structure->manufacturing_mode_supported) ? "Yes" : "No");
                        log("\n  Manufacturing Mode Enabled: %s", (structure->manufacturing_mode_enabled) ? "Yes" : "No");
                    }
                    if((entry_point->major_version > 2 || entry_point->minor_version >= 4) && structure->header.length >= 0x17) {
                        log("\n  Platform Firmware Major Version: %d", structure->platform_firmware_major);
                        log("\n  Platform Firmware Minor Version: %d", structure->platform_firmware_minor);
                        log("\n  Embedded Controller Firmware Major Version: %d", structure->embedded_controller_firmware_major);
                        log("\n  Embedded Controller Firmware Minor Version: %d", structure->embedded_controller_firmware_minor);
                    }
                }*/
            }
            if(entry_point->major_version >= 3) {
                if((entry_point->major_version > 3 || entry_point->minor_version >= 1) && structure->header.length >= 0x18) {
                    if(structure->extended_firmware_rom_size_unit == 0b00) {
                        log("\n  Extended Firmware ROM Size: %d MiB", structure->extended_firmware_rom_size);
                    }
                    else if(structure->extended_firmware_rom_size_unit == 0b01) {
                        log("\n  Extended Firmware ROM Size: %d GiB", structure->extended_firmware_rom_size);
                    }
                    else {
                        log("\n  Extended Firmware ROM Size: Unknown unit");
                    }
                }
            }
        }
        else if(structure_header->type == 1) {
            smbios_system_info_t *structure = (smbios_system_info_t *) structure_ptr;
            log("\n System Information");
            if(structure->header.length >= 0x07) {
                log("\n  Manufacturer: %s", get_smbios_structure_string(structure, structure_end, structure->manufacturer_string));
                log("\n  Product Name: %s", get_smbios_structure_string(structure, structure_end, structure->product_name_string));
                log("\n  Version: %s", get_smbios_structure_string(structure, structure_end, structure->version_string));
                log("\n  Serial Number: %s", get_smbios_structure_string(structure, structure_end, structure->serial_number_string));
            }
            if(entry_point->major_version >= 2) {
                if((entry_point->major_version > 2 || entry_point->minor_version >= 1) && structure->header.length >= 0x18) {
                    log("\n  UUID: %02x%02x%02x%02x-%02x%02x-%02x%02x-%02x%02x-%02x%02x%02x%02x%02x%02x",
                        structure->UUID[3],
                        structure->UUID[2],
                        structure->UUID[1],
                        structure->UUID[0],
                        structure->UUID[5],
                        structure->UUID[4],
                        structure->UUID[7],
                        structure->UUID[6],
                        structure->UUID[8],
                        structure->UUID[9],
                        structure->UUID[10],
                        structure->UUID[11],
                        structure->UUID[12],
                        structure->UUID[13],
                        structure->UUID[14],
                        structure->UUID[15]);
                    log("\n  Wake-up Type: ");
                    switch(structure->wake_up_type) {
                        case 0: log("Reserved"); break;
                        case 1: log("Other"); break;
                        case 2: log("Unknown"); break;
                        case 3: log("APM Timer"); break;
                        case 4: log("Modem Ring"); break;
                        case 5: log("LAN Remote");  break;
                        case 6: log("Power Switch"); break;
                        case 7: log("PCI PME#"); break;
                        case 8: log("AC Power Restored"); break;
                        default: log("Out of Spec"); break;
                    }
                }
                if((entry_point->major_version > 2 || entry_point->minor_version >= 4) && structure->header.length >= 0x1A) {
                    log("\n  SKU Number: %s", get_smbios_structure_string(structure, structure_end, structure->sku_number_string));
                    log("\n  Family: %s", get_smbios_structure_string(structure, structure_end, structure->family_string));
                }
            }
        }
        else if(structure_header->type == 2) {
            smbios_baseboard_info_t *structure = (smbios_baseboard_info_t *) structure_ptr;
            log("\n Baseboard Information");
            if(structure->header.length >= 0x07) {
                log("\n  Manufacturer: %s", get_smbios_structure_string(structure, structure_end, structure->manufacturer_string));
                log("\n  Product: %s", get_smbios_structure_string(structure, structure_end, structure->product_string));
                log("\n  Version: %s", get_smbios_structure_string(structure, structure_end, structure->version_string));
                log("\n  Serial Number: %s", get_smbios_structure_string(structure, structure_end, structure->serial_number_string));
                // TODO: more fields
            }
        }
        // TODO: more structure types
        else if(structure_header->type == 127) { // end of table
            break;
        }
        else {
            log("\n SMBIOS Structure Type: %d Length: 0x%02x Handle: 0x%04x", structure_header->type, structure_header->length, structure_header->handle);
        }

        // skip formatted part
        structure_ptr += structure_header->length;

        // skip unformatted part (strings)
        while(structure_ptr < (structure_end - 1)) {
            if(structure_ptr[0] == 0 && structure_ptr[1] == 0) {
                structure_ptr += 2;
                break;
            }
            structure_ptr++;
        }
    }
}

uint8_t *get_smbios_structure_string(void *structure, uint8_t *end, int string_index) {
    if(string_index == 0) {
        return "(not specified)";
    }

    // find start of requested string index
    smbios_structure_header_t *header = structure;
    uint8_t *string_area = (uint8_t *) structure + header->length;
    uint8_t actual_string_index = 1;
    while(actual_string_index != string_index) {
        // move to next string
        while(string_area < end) {
            if(*string_area == 0) { // zero ending before end of string area
                string_area++; // move to next string
                break;
            }
            string_area++;
        }
        if(string_area >= end) { // we are at end of structure, no more data are present
            return "(parsing failed)";
        }
        if(*string_area == 0) { // end of string area reached
            return "(parsing failed)";
        }
        actual_string_index++;
    }

    // check if this string has zero ending
    uint8_t *string = string_area;
    while(string < end) {
        if(*string == 0) { // zero ending before end of string area
            return (char *) string_area;
        }
        string++;
    }

    return "(parsing failed)";
}