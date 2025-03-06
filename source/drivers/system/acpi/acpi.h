//BleskOS

/*
* MIT License
* Copyright (c) 2023-2025 BleskOS developers
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

struct acpi_info_t {
    dword_t p_fadp;
    word_t command_reg;
    word_t pm1a_control_reg;
    word_t pm1a_control_reg_shutdown_s5_value;
    word_t pm1b_control_reg;
    word_t pm1b_control_reg_shutdown_s5_value;
    struct acpi_table_header_t *dsdt;
    byte_t *dsdt_aml_data;
};

#define ACPI_GAS_ADDRESS_ID_MEMORY 0
#define ACPI_GAS_ADDRESS_ID_IO 1

struct generic_address_structure_t {
    byte_t address_type;
    byte_t register_bit_width;
    byte_t register_bit_offset;
    byte_t reserved;
    qword_t address;
}__attribute__((packed));

struct rsdp_table_t {
    qword_t signature;
    byte_t checksum;
    byte_t oem_id[6];
    byte_t revision;
    dword_t rsdt_address;

    dword_t length;
    qword_t xsdt_address;
    byte_t extended_checksum;
    byte_t reserved[3];
}__attribute__((packed));

struct acpi_table_header_t {
    dword_t signature;
    dword_t length;
    byte_t revision;
    byte_t checksum;
    byte_t oem_id[6];
    byte_t oem_table_id[8];
    dword_t oem_revision;
    dword_t creator_id;
    dword_t creator_revision;
}__attribute__((packed));

struct rsdt_table_t {
    struct acpi_table_header_t header;
    void *tables[];
}__attribute__((packed));

struct xsdt_table_t {
    struct acpi_table_header_t header;
    qword_t tables[];
}__attribute__((packed));

struct fadp_table_t {
    struct acpi_table_header_t header;
    dword_t firmware_ctrl;
    dword_t dsdt;
    byte_t reserved;
    byte_t preferred_pm_profile;
    word_t sci_int;
    dword_t smi_cmd;
    byte_t acpi_enable;
    byte_t acpi_disable;
    byte_t s4bios_req;
    byte_t pstate_cnt;
    dword_t pm1a_evt_blk;
    dword_t pm1b_evt_blk;
    dword_t pm1a_cnt_blk;
    dword_t pm1b_cnt_blk;
    dword_t pm2_cnt_blk;
    dword_t pm_tmr_blk;
    dword_t gpe0_blk;
    dword_t gpe1_blk;
    byte_t pm1_evt_len;
    byte_t pm1_cnt_len;
    byte_t pm2_cnt_len;
    byte_t pm_tmr_len;
    byte_t gpe0_blk_len;
    byte_t gpe1_blk_len;
    byte_t gpe1_base;
    byte_t cst_cnt;
    word_t p_lvl2_lat;
    word_t p_lvl3_lat;
    word_t flush_size;
    word_t flush_stride;
    byte_t duty_offset;
    byte_t duty_width;
    byte_t day_alrm;
    byte_t mon_alrm;
    byte_t century;
    word_t iapc_boot_arch;
    byte_t reserved2;
    dword_t flags;
    byte_t reset_reg[12];
    byte_t reset_value;
    byte_t reserved3[3];
    qword_t x_firmware_ctrl;
    qword_t x_dsdt;
    byte_t x_pm1a_evt_blk[12];
    byte_t x_pm1b_evt_blk[12];
    byte_t x_pm1a_cnt_blk[12];
    byte_t x_pm1b_cnt_blk[12];
    byte_t x_pm2_cnt_blk[12];
    byte_t x_pm_tmr_blk[12];
    byte_t x_gpe0_blk[12];
    byte_t x_gpe1_blk[12];
}__attribute__((packed));

struct hpet_table_t {
    struct acpi_table_header_t header;
    dword_t id;
    struct generic_address_structure_t base_address;
    byte_t sequence_number;
    word_t minimum_ticks_in_periodic_mode;
    byte_t page_protection;
}__attribute__((packed));

void read_acpi_tables(void);
dword_t acpi_table_is_checksum_valid(void *table, dword_t length_of_table);
void read_acpi_table(void *table);