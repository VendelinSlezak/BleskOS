//BleskOS

/*
* MIT License
* Copyright (c) 2023-2025 BleskOS developers
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

byte_t satapi_test_unit_ready(dword_t port_base_address, dword_t command_list_memory) {
 byte_t atapi_command[0x10] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
 return ahci_send_atapi_command(port_base_address, command_list_memory, 0, 0, atapi_command);
}

byte_t satapi_detect_disk(dword_t port_base_address, dword_t command_list_memory) {
 byte_t status = satapi_test_unit_ready(port_base_address, command_list_memory);
 if(status==STATUS_GOOD) {
  return STATUS_GOOD;
 }
 else {
  //when optical disk is loaded, first TEST UNIT READY that will be sended will fail, so after STATUS_ERROR we will chcek again if there is really no disk
  return satapi_test_unit_ready(port_base_address, command_list_memory);
 }
}

byte_t satapi_start_stop_command(dword_t port_base_address, dword_t command_list_memory, byte_t command) {
 byte_t atapi_command[0x10] = {0x1B, 0, 0, 0, command, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
 return ahci_send_atapi_command(port_base_address, command_list_memory, 0, 0, atapi_command);
}

byte_t satapi_read_capabilities(dword_t port_base_address, dword_t command_list_memory) {
 byte_t atapi_command[0x10] = {0x25, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
 byte_t response[8];
 optical_disk_size = 0;
 optical_disk_sector_size = 0;

 //read capabilities
 if(ahci_send_atapi_command(port_base_address, command_list_memory, (dword_t)(&response), 8, atapi_command)==STATUS_ERROR){
  logf("\nAHCI: READ CAPABILITES error");
  return STATUS_ERROR;
 }

 //parse response
 optical_disk_size = (response[0]<<24 | response[1]<<16 | response[2]<<8 | response[3]);
 optical_disk_sector_size = (response[4]<<24 | response[5]<<16 | response[6]<<8 | response[7]);

 return STATUS_GOOD;
}

byte_t satapi_read_cd_toc(dword_t port_base_address, dword_t command_list_memory, dword_t memory) {
 byte_t atapi_command[0x10] = {0x43, 0, 0, 0, 0, 0, 0, 0, 252, 0, 0, 0, 0, 0, 0, 0};
 return ahci_send_atapi_command(port_base_address, command_list_memory, memory, 252, atapi_command);
}

byte_t satapi_read(dword_t port_base_address, dword_t command_list_memory, dword_t sector, byte_t number_of_sectors, dword_t memory) {
 byte_t atapi_command[0x10] = {0xA8, 0, ((sector>>24) & 0xFF), ((sector>>16) & 0xFF), ((sector>>8) & 0xFF), (sector & 0xFF), 0, 0, 0, number_of_sectors, 0, 0, 0, 0, 0, 0};
 return ahci_send_atapi_command(port_base_address, command_list_memory, memory, number_of_sectors*2048, atapi_command);
}

byte_t satapi_read_audio_cd_sector(dword_t port_base_address, dword_t command_list_memory, dword_t sector, byte_t number_of_sectors, dword_t memory) {
 byte_t atapi_command[0x10] = {0xBE, 0, ((sector>>24) & 0xFF), ((sector>>16) & 0xFF), ((sector>>8) & 0xFF), (sector & 0xFF), 0, 0, number_of_sectors, 0x10, 0, 0, 0, 0, 0, 0};
 return ahci_send_atapi_command(port_base_address, command_list_memory, memory, number_of_sectors*2352, atapi_command);
}