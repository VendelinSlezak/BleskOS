/* 
* BleskOS
*
* MIT License
* Copyright (c) 2023-2025 BleskOS developers
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

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