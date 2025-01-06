//BleskOS

/*
* MIT License
* Copyright (c) 2023-2025 Vendelín Slezák
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

byte_t sata_read(dword_t port_base_address, dword_t command_list_memory, dword_t sector, byte_t number_of_sectors, dword_t memory) {
 return ahci_send_ata_command(port_base_address, command_list_memory, AHCI_READ, 0x25, number_of_sectors, sector, memory, 512*number_of_sectors);
}

byte_t sata_write(dword_t port_base_address, dword_t command_list_memory, dword_t sector, byte_t number_of_sectors, dword_t memory) {
 return ahci_send_ata_command(port_base_address, command_list_memory, AHCI_WRITE, 0x35, number_of_sectors, sector, memory, 512*number_of_sectors);
}