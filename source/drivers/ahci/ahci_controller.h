//BleskOS

/*
* MIT License
* Copyright (c) 2023 Vendelín Slezák
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

dword_t ahci_controllers[10];
dword_t ahci_ports[32][2];

dword_t ahci_controllers_pointer = 0, ahci_ports_pointer = 0;
dword_t ahci_base, ahci_drive_size, ahci_hdd_base, ahci_hdd_size, ahci_cdrom_base, ahci_cdrom_size;
dword_t ahci_hdd_cmd_mem, ahci_hdd_receive_fis_mem, ahci_hdd_fis_mem, ahci_cdrom_cmd_mem, ahci_cdrom_receive_fis_mem, ahci_cdrom_fis_mem;

void initalize_ahci_controllers(void);
void initalize_ahci_port(dword_t base_port, dword_t commands_memory, dword_t receive_fis_memory);
