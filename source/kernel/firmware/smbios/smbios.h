/* 
* BleskOS
*
* MIT License
* Copyright (c) 2023-2026 BleskOS developers
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

typedef struct {
    uint32_t anchor_string;
    uint8_t checksum;
    uint8_t length;
    uint8_t major_version;
    uint8_t minor_version;
    uint16_t max_structure_size;
    uint8_t entry_point_revision;
    uint8_t formatted_area[5];
    uint8_t intermediate_anchor_string[5];
    uint8_t intermediate_checksum;
    uint16_t table_length;
    uint32_t table_address;
    uint16_t number_of_structures;
    uint8_t bcd_revision;
}__attribute__((packed)) smbios_32_bit_entry_point_t;

typedef struct {
    uint8_t type;
    uint8_t length;
    uint16_t handle;
}__attribute__((packed)) smbios_structure_header_t;

typedef struct {
    smbios_structure_header_t header;
    uint8_t vendor_string;
    uint8_t firmware_version_string;
    uint16_t bios_starting_address_segment;
    uint8_t firmware_release_date_string;
    uint8_t firmware_rom_size;
    uint64_t : 3;
    uint64_t firmware_characteristics_supported : 1;
    uint64_t isa_supported : 1;
    uint64_t mca_supported : 1;
    uint64_t eisa_supported : 1;
    uint64_t pci_supported : 1;
    uint64_t pcmcia_supported : 1;
    uint64_t plug_and_play_supported : 1;
    uint64_t apm_supported : 1;
    uint64_t upgradeable_firmware : 1;
    uint64_t firmware_shadowing_allowed : 1;
    uint64_t vl_vesa_supported : 1;
    uint64_t escd_supported : 1;
    uint64_t cd_boot_supported : 1;
    uint64_t selectable_boot_supported : 1;
    uint64_t firmware_rom_is_socketed : 1;
    uint64_t pcmcia_boot_supported : 1;
    uint64_t edd_supported : 1;
    uint64_t int13_floppy_nec_9800_supported : 1;
    uint64_t int13_floppy_toshiba_supported : 1;
    uint64_t int13_floppy_360kb_supported : 1;
    uint64_t int13_floppy_1_2mb_supported : 1;
    uint64_t int13_floppy_720kb_supported : 1;
    uint64_t int13_floppy_2_88mb_supported : 1;
    uint64_t int5_print_screen_service_supported : 1;
    uint64_t int9_8042_keyboard_services_supported : 1;
    uint64_t int14_serial_services_supported : 1;
    uint64_t int17_printer_services_supported : 1;
    uint64_t int10_cga_mono_video_services_supported : 1;
    uint64_t nec_pc98 : 1;
    uint64_t : 32;

    uint8_t acpi_supported : 1;
    uint8_t usb_legacy_supported : 1;
    uint8_t agp_supported : 1;
    uint8_t i2o_boot_supported : 1;
    uint8_t ls120_superdisk_boot_supported : 1;
    uint8_t atapi_zip_drive_boot_supported : 1;
    uint8_t ieee_1394_boot_supported : 1;
    uint8_t smart_battery_supported : 1;

    uint8_t bios_boot_specification_supported : 1;
    uint8_t network_service_boot_supported : 1;
    uint8_t enable_targeted_content_distribution : 1;
    uint8_t uefi_supported : 1;
    uint8_t smbios_of_virtual_machine : 1;
    uint8_t manufacturing_mode_supported : 1;
    uint8_t manufacturing_mode_enabled : 1;
    uint8_t : 1;

    uint8_t platform_firmware_major;
    uint8_t platform_firmware_minor;
    uint8_t embedded_controller_firmware_major;
    uint8_t embedded_controller_firmware_minor;

    uint16_t extended_firmware_rom_size : 14;
    uint16_t extended_firmware_rom_size_unit : 2;
}__attribute__((packed)) smbios_platform_firmware_info_t;

typedef struct {
    smbios_structure_header_t header;
    uint8_t manufacturer_string;
    uint8_t product_name_string;
    uint8_t version_string;
    uint8_t serial_number_string;

    uint8_t UUID[16];
    uint8_t wake_up_type;

    uint8_t sku_number_string;
    uint8_t family_string;
}__attribute__((packed)) smbios_system_info_t;

typedef struct {
    smbios_structure_header_t header;
    uint8_t manufacturer_string;
    uint8_t product_string;
    uint8_t version_string;
    uint8_t serial_number_string;
    // TODO: add more fields
}__attribute__((packed)) smbios_baseboard_info_t;

