//BleskOS

/*
* MIT License
* Copyright (c) 2023-2024 Vendelín Slezák
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

dword_t ahci_drive_info;

byte_t sata_wait_for_drive(dword_t base_port, dword_t wait_ticks);
byte_t sata_send_command(dword_t base_port, dword_t commands_memory, dword_t fis_memory, byte_t command, dword_t sector, byte_t number_of_sectors, dword_t transfer_length, dword_t memory);
byte_t sata_read_drive_info(dword_t base_port, dword_t commands_memory, dword_t fis_memory);
byte_t sata_read(dword_t base_port, dword_t commands_memory, dword_t fis_memory, dword_t sector, byte_t number_of_sectors, dword_t memory);
byte_t sata_write(dword_t base_port, dword_t commands_memory, dword_t fis_memory, dword_t sector, byte_t number_of_sectors, dword_t memory);
