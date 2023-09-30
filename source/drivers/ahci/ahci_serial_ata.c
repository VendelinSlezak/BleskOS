//BleskOS

/*
* MIT License
* Copyright (c) 2023 Vendelín Slezák
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

byte_t sata_wait_for_drive(dword_t base_port, dword_t wait_ticks) {
 ticks = 0;
 while(ticks<wait_ticks) {
  if((mmio_ind(base_port + 0x20) & 0x88)==0x00) {
   return STATUS_GOOD;
  }
 }
 return STATUS_ERROR; //ERROR: drive is not ready after timeout
}

byte_t sata_send_command(dword_t base_port, dword_t commands_memory, dword_t fis_memory, byte_t command, dword_t sector, byte_t number_of_sectors, dword_t transfer_length, dword_t memory) {
 dword_t *commands_mem = (dword_t *) commands_memory;
 dword_t *fis_mem = (dword_t *) fis_memory;
 byte_t status = 0;
 
 //set first command
 commands_mem[0]=0x00010005;
 commands_mem[1]=0;
 commands_mem[2]=fis_memory;
 commands_mem[3]=0;
 
 //set fis command
 mmio_outb(fis_memory + 0x00, 0x27); //host to device
 mmio_outb(fis_memory + 0x01, 0x80); //we send command
 mmio_outb(fis_memory + 0x02, command); //command
 mmio_outb(fis_memory + 0x03, 0x00);
 mmio_outb(fis_memory + 0x04, (sector & 0xFF)); //first byte of sector
 mmio_outb(fis_memory + 0x05, ((sector>>8) & 0xFF)); //second byte of sector
 mmio_outb(fis_memory + 0x06, ((sector>>16) & 0xFF)); //third byte of sector
 mmio_outb(fis_memory + 0x07, 0xA0); //device head
 mmio_outb(fis_memory + 0x08, ((sector>>24) & 0xFF)); //fourth byte of sector
 mmio_outb(fis_memory + 0x09, 0x00); //fifth byte of sector
 mmio_outb(fis_memory + 0x0A, 0x00); //sixth byte of sector
 mmio_outb(fis_memory + 0x0B, 0x00);
 mmio_outb(fis_memory + 0x0C, number_of_sectors);
 mmio_outb(fis_memory + 0x0D, 0x00);
 mmio_outb(fis_memory + 0x0E, 0x00);
 mmio_outb(fis_memory + 0x0F, 0x08); //control
 mmio_outd(fis_memory + 0x10, 0x00);
 
 //set prdt
 mmio_outd(fis_memory + 0x80, memory);
 mmio_outd(fis_memory + 0x84, 0);
 mmio_outd(fis_memory + 0x88, 0);
 mmio_outd(fis_memory + 0x8C, transfer_length);

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

byte_t sata_read_drive_info(dword_t base_port, dword_t commands_memory, dword_t fis_memory) {
 dword_t *ahci_drive_info_mem = (dword_t *) ahci_drive_info;
 
 if(sata_send_command(base_port, commands_memory, fis_memory, 0xEC, 0, 0, 0x1FF, ahci_drive_info)==STATUS_ERROR) {
  return STATUS_ERROR;
 }
 
 //read useful values
 ahci_drive_info_mem = (dword_t *) ((dword_t)ahci_drive_info+120); //28 bits size
 ahci_drive_size = *ahci_drive_info_mem;
 ahci_drive_info_mem = (dword_t *) ((dword_t)ahci_drive_info+200); //48 bits size
 if(*ahci_drive_info_mem!=0) {
  ahci_drive_size = *ahci_drive_info_mem;
 }
 
 return STATUS_GOOD;
}

byte_t sata_read(dword_t base_port, dword_t commands_memory, dword_t fis_memory, dword_t sector, byte_t number_of_sectors, dword_t memory) {
 return sata_send_command(base_port, commands_memory, fis_memory, 0x25, sector, number_of_sectors, ((number_of_sectors*512)-1), memory);
}

byte_t sata_write(dword_t base_port, dword_t commands_memory, dword_t fis_memory, dword_t sector, byte_t number_of_sectors, dword_t memory) {
 return sata_send_command(base_port, commands_memory, fis_memory, 0x35, sector, number_of_sectors, ((number_of_sectors*512)-1), memory);
}
