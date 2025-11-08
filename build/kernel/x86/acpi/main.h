#ifndef BUILD_KERNEL_X86_ACPI_MAIN_H
#define BUILD_KERNEL_X86_ACPI_MAIN_H

/* 
* BleskOS
*
* MIT License
* Copyright (c) 2023-2025 BleskOS developers
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#define ACPI_GAS_ADDRESS_ID_MEMORY 0
#define ACPI_GAS_ADDRESS_ID_IO 1

#define ACPI_SIGNATURE_64(signature) *((qword_t *)&signature)
#define ACPI_SIGNATURE_32(signature) *((dword_t *)&signature)

typedef struct {
    byte_t address_type;
    byte_t register_bit_width;
    byte_t register_bit_offset;
    byte_t reserved;
    qword_t address;
}__attribute__((packed)) generic_address_structure_t;

typedef struct {
    qword_t signature;
    byte_t checksum;
    byte_t oem_id[6];
    byte_t revision;
    dword_t rsdt_address;

    dword_t length;
    qword_t xsdt_address;
    byte_t extended_checksum;
    byte_t reserved[3];
}__attribute__((packed)) xsdp_table_t;

typedef struct {
    dword_t signature;
    dword_t length;
    byte_t revision;
    byte_t checksum;
    byte_t oem_id[6];
    byte_t oem_table_id[8];
    dword_t oem_revision;
    dword_t creator_id;
    dword_t creator_revision;
}__attribute__((packed)) acpi_table_header_t;

typedef struct {
    acpi_table_header_t header;
    void *tables[];
}__attribute__((packed)) rsdt_table_t;

typedef struct {
    acpi_table_header_t header;
    qword_t tables[];
}__attribute__((packed)) xsdt_table_t;
void read_acpi(void);
dword_t acpi_table_is_checksum_valid(void *table, dword_t length_of_table);
void acpi_check_table(dword_t table_start, acpi_table_header_t *acpi_table);

#endif /* BUILD_KERNEL_X86_ACPI_MAIN_H */
