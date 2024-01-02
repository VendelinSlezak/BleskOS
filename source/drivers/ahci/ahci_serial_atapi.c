//BleskOS

/*
* MIT License
* Copyright (c) 2023-2024 Vendelín Slezák
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

byte_t satapi_send_command(dword_t base_port, dword_t commands_memory, dword_t fis_memory, dword_t transfer_length, dword_t cmd1, dword_t cmd2, dword_t cmd3, dword_t memory) {
 dword_t *commands_mem = (dword_t *) commands_memory;
 dword_t *fis_mem = (dword_t *) fis_memory;
 byte_t status = 0;
 
 //set first command
 if(transfer_length!=0) {
  commands_mem[0]=0x00010025;
 }
 else {
  commands_mem[0]=0;
 }
 commands_mem[1]=0;
 commands_mem[2]=fis_memory;
 commands_mem[3]=0;
 
 //set fis command
 mmio_outb(fis_memory + 0x00, 0x27); //host to device
 mmio_outb(fis_memory + 0x01, 0x80); //we send command
 mmio_outb(fis_memory + 0x02, 0xA0); //packet command
 mmio_outb(fis_memory + 0x03, 0x00);
 mmio_outb(fis_memory + 0x04, 0x00);
 mmio_outb(fis_memory + 0x05, ((transfer_length) & 0xFF)); //first byte of transferred length
 mmio_outb(fis_memory + 0x06, ((transfer_length>>8) & 0xFF)); //second byte of transferred length
 mmio_outb(fis_memory + 0x07, 0xA0); //device head
 mmio_outb(fis_memory + 0x08, 0x00);
 mmio_outb(fis_memory + 0x09, 0x00);
 mmio_outb(fis_memory + 0x0A, 0x00);
 mmio_outb(fis_memory + 0x0B, 0x00);
 mmio_outb(fis_memory + 0x0C, 0x00);
 mmio_outb(fis_memory + 0x0D, 0x00);
 mmio_outb(fis_memory + 0x0E, 0x00);
 mmio_outb(fis_memory + 0x0F, 0x08); //control
 mmio_outd(fis_memory + 0x10, 0x00);
 
 //set ATAPI command
 mmio_outd(fis_memory + 0x40, cmd1);
 mmio_outd(fis_memory + 0x44, cmd2);
 mmio_outd(fis_memory + 0x48, cmd3);
 
 //set prdt
 if(transfer_length!=0) {
  mmio_outd(fis_memory + 0x80, memory);
  mmio_outd(fis_memory + 0x84, 0);
  mmio_outd(fis_memory + 0x88, 0);
  mmio_outd(fis_memory + 0x8C, (transfer_length-1));
 }
 else {
  mmio_outd(fis_memory + 0x80, 0);
  mmio_outd(fis_memory + 0x84, 0);
  mmio_outd(fis_memory + 0x88, 0);
  mmio_outd(fis_memory + 0x8C, 0);
 }

 //wait for drive to be ready
 ticks = 0;
 status = STATUS_ERROR;
 while(ticks<200) {
  if((mmio_ind(base_port + 0x20) & 0x88)==0x00) {
   status = STATUS_GOOD;
   break;
  }
 }
 if(status==STATUS_ERROR) {
  return STATUS_ERROR; //ERROR: drive is not ready after timeout
 }
 
 //send command
 mmio_outd(base_port + 0x10, 0xFFFFFFFF); //clear status
 mmio_outd(base_port + 0x38, 0x1); //send first command

 //wait
 ticks = 0;
 status = STATUS_ERROR;
 while(ticks<500) {
  if((mmio_ind(base_port + 0x38) & 0x1)==0x0) {
   status = STATUS_GOOD;
   break;
  }
  if((mmio_ind(base_port + 0x10) & 0x40000000)==0x40000000) {
   //some error occured
   break;
  }
 }
 if(status==STATUS_ERROR) {
  return STATUS_ERROR; //command was not processed
 }

 return STATUS_GOOD;
}

byte_t satapi_eject_drive(dword_t base_port, dword_t commands_memory, dword_t fis_memory) {
 return satapi_send_command(base_port, commands_memory, fis_memory, 0, 0x0000001B, 0x00000002, 0, 0);
}

byte_t satapi_detect_disk(dword_t base_port, dword_t commands_memory, dword_t fis_memory) { //TODO:
 return satapi_send_command(base_port, commands_memory, fis_memory, 0, 0, 0, 0, 0); //TEST UNIT READY
}

byte_t satapi_read_drive_capabilites(dword_t base_port, dword_t commands_memory, dword_t fis_memory) {
 word_t *ahci_drive_info_mem = (word_t *) ahci_drive_info;
 dword_t response;
 optical_disk_size = 0;
 
 if(satapi_send_command(base_port, commands_memory, fis_memory, 8, 0x00010025, 0, 0, ahci_drive_info)==STATUS_ERROR) {
  return STATUS_ERROR;
 }
 
 //read disk size
 response = ahci_drive_info_mem[0];
 optical_disk_size = (BIG_ENDIAN(response)<<16);
 response = ahci_drive_info_mem[1];
 optical_disk_size |= BIG_ENDIAN(response);
 
 return STATUS_GOOD;
}

byte_t satapi_read_sector(dword_t base_port, dword_t commands_memory, dword_t fis_memory, dword_t sector, dword_t memory) {
 word_t upper_sector = (sector >> 16);
 sector &= 0xFFFF;
 
 return satapi_send_command(base_port, commands_memory, fis_memory, 2048, (0x00A8 | BIG_ENDIAN(upper_sector)), (BIG_ENDIAN(sector)), 0x00000100, memory);
}
