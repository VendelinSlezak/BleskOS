//BleskOS

/*
* MIT License
* Copyright (c) 2023-2024 Vendelín Slezák
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

void pata_select_sector_lba28(word_t base_port, dword_t sector, byte_t number_of_sectors) {
 outb(base_port + 2, number_of_sectors);
 outb(base_port + 3, (sector & 0xFF));
 outb(base_port + 4, ((sector>>8) & 0xFF));
 outb(base_port + 5, ((sector>>16) & 0xFF));
}

void pata_select_sector_lba48(word_t base_port, dword_t sector, byte_t number_of_sectors) {
 outb(base_port + 2, 0); //number of sectors>>8 & 0xFF
 outb(base_port + 3, ((sector>>24) & 0xFF));
 outb(base_port + 4, 0); //sector>>32 & 0xFF
 outb(base_port + 5, 0); //sector>>36 & 0xFF
 pata_select_sector_lba28(base_port, (dword_t)sector, number_of_sectors);
}

byte_t pata_read(word_t base_port, dword_t sector, byte_t number_of_sectors, dword_t memory) {
 word_t *mem = (word_t *) memory;
 
 //drive must be already selected
 
 //select sector
 if(sector<0x01000000) {
  pata_select_sector_lba28(base_port, sector, number_of_sectors);
  outb(base_port + 7, 0x20); //LBA28 READ command
 }
 else {
  pata_select_sector_lba48(base_port, sector, number_of_sectors);
  outb(base_port + 7, 0x24); //LBA48 READ command
 }
 
 //transfer data
 for(int i=0; i<number_of_sectors; i++) {
  //wait for drive to be ready
  if(ide_wait_for_data(base_port, 500)==STATUS_ERROR) {
   return STATUS_ERROR;
  }

  //read data of sector
  for(dword_t j=0; j<256; j++) {
   *mem = inw(base_port + 0);
   mem++;
  }
 }
 
 return STATUS_GOOD;
}

byte_t pata_write(word_t base_port, dword_t sector, byte_t number_of_sectors, dword_t memory) {
 word_t *mem = (word_t *) memory;
 
 //drive must be already selected
 
 //select sector
 if(sector<0x01000000) {
  pata_select_sector_lba28(base_port, sector, number_of_sectors);
  outb(base_port + 7, 0x30); //LBA28 WRITE command
 }
 else {
  pata_select_sector_lba48(base_port, sector, number_of_sectors);
  outb(base_port + 7, 0x34); //LBA48 WRITE command
 }

 //transfer data
 for(int i=0; i<number_of_sectors; i++) {
  //wait for drive to be ready
  if(ide_wait_for_data(base_port, 500)==STATUS_ERROR) {
   return STATUS_ERROR;
  }

  //write data of sector
  for(dword_t j=0; j<256; j++) {
   outw(base_port + 0, *mem);
   mem++;
  }
 }
 
 
 //flush cache memory for immediate writing
 outb(base_port + 7, 0xE7);
 return ide_wait_drive_not_busy_with_error_status(base_port, 500);
}