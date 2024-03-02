//BleskOS

/*
* MIT License
* Copyright (c) 2023-2024 Vendelín Slezák
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

byte_t ide_wait_drive_not_busy(word_t base_port, dword_t wait_ticks) {
 ticks=0;
 
 while(ticks<wait_ticks) {
  asm("nop");
  if((inb(base_port + 7) & 0x80)==0x00) { //drive is not busy
   return STATUS_GOOD;
  }
 }
 
 return STATUS_ERROR;
}

byte_t ide_wait_drive_not_busy_with_error_status(word_t base_port, dword_t wait_ticks) {
 ticks=0;
 
 while(ticks<wait_ticks) {
  asm("nop");
  if((inb(base_port + 7) & 0x01)==0x01) { //error
   return STATUS_ERROR;
  }
  else if((inb(base_port + 7) & 0x80)==0x00) { //drive is not busy
   return STATUS_GOOD;
  }
 }
 
 return STATUS_ERROR;
}

byte_t ide_wait_for_data(word_t base_port, dword_t wait_ticks) {
 ticks=0;

 while(ticks<wait_ticks) {
  asm("nop");
  if((inb(base_port + 7) & 0x88)==0x08) { //data are ready
   return STATUS_GOOD;
  }
  else if((inb(base_port + 7) & 0x81)==0x01) { //error
   return STATUS_ERROR;
  }
 }
 
 return STATUS_ERROR;
}

void ide_clear_device_output(word_t base_port) {
 for(dword_t i=0; i<1024*0xFFFF; i++) {
  if((inb(base_port+7) & 0x08)==0x08) { //Data Ready bit is set
   inw(base_port+0);
  }
  else {
   break;
  }
 }
}

byte_t ide_reset_controller(word_t base_port, word_t alt_base_port) {
 byte_t selected_drive = inb(base_port+6);

 outb(alt_base_port+2, (1<<2));
 wait(1);
 outb(alt_base_port+2, (1<<1));
 wait(1);
 byte_t status = ide_wait_drive_not_busy(base_port, 100);

 if(inb(base_port+6)!=selected_drive) { //reset will set master drive, so restore previous drive if it was not master
  ide_select_drive(base_port, selected_drive);
 }

 return status;
}

void ide_select_drive(word_t base_port, byte_t drive) {
 ide_wait_drive_not_busy(base_port, 100);
 outb(base_port + 6, drive);
 wait(5);
}

byte_t ide_send_identify_drive(word_t base_port) {
 word_t *ide_info16 = (word_t *) ide_drive_info;
 dword_t *ide_info32;

 ide_drive_type = 0xFFFF;
 ide_drive_size = 0;
 
 //send command
 outb(base_port + 2, 0);
 outb(base_port + 3, 0);
 outb(base_port + 4, 0);
 outb(base_port + 5, 0);
 outb(base_port + 7, 0xEC);

 //test presence of drive
 ide_wait_for_data(base_port, 50); //wait for controller to process command
 if(inb(base_port + 7)==0x00 || inb(base_port + 7)==0xFF || inb(base_port + 7)==0x7F) { //invalid state of status register
  return STATUS_ERROR;
 }
 
 //wait
 if(ide_wait_for_data(base_port, 50)==STATUS_ERROR) {
  //read drive type
  inb(base_port + 1);
  inb(base_port + 2);
  inb(base_port + 3);
  ide_drive_type = (inb(base_port + 4) | (inb(base_port + 5) << 8));

  return STATUS_ERROR;
 }

 //read drive type
 inb(base_port + 1);
 inb(base_port + 2);
 inb(base_port + 3);
 ide_drive_type = (inb(base_port + 4) | (inb(base_port + 5) << 8));
 
 //read
 for(int i=0; i<256; i++) {
  *ide_info16 = inw(base_port + 0);
  ide_info16++;
 }
 
 //read useful values
 ide_info32 = (dword_t *) ((dword_t)ide_drive_info+120); //number of sectors in 28 bits
 ide_drive_size = *ide_info32;
 ide_info32 = (dword_t *) ((dword_t)ide_drive_info+200); //number of sectors in 48 bits
 if(*ide_info32!=0) {
  ide_drive_size = *ide_info32;
 }
 
 return STATUS_GOOD;
}

void initalize_ide_controllers(void) {
 //initalize variables
 ide_hdd_base = 0;
 ide_hdd_alt_base = 0;
 ide_hdd_drive = 0;
 ide_cdrom_base = 0;
 ide_cdrom_alt_base = 0;
 ide_cdrom_drive = 0;

 //detect drives
 word_t *ide_controllers_info = (word_t *) (ide_controllers_info_mem);
 dword_t *ide_controllers_info32 = (dword_t *) (ide_controllers_info_mem+8);
 for(dword_t i=0; i<ide_controllers_pointer; i++, ide_controllers_info += 8, ide_controllers_info32 += 4) {
  if(inb(ide_controllers_info[0] + 7)==0xFF) {
   continue; //there is invalid value in status register, this controller do not exist
  }

  ide_reset_controller(ide_controllers_info[0], ide_controllers_info[1]);
  ide_select_drive(ide_controllers_info[0], PATA_MASTER);
  ide_send_identify_drive(ide_controllers_info[0]);
  ide_controllers_info[2] = ide_drive_type;
  ide_controllers_info32[0] = ide_drive_size;

  ide_reset_controller(ide_controllers_info[0], ide_controllers_info[1]);
  ide_select_drive(ide_controllers_info[0], PATA_SLAVE);
  ide_send_identify_drive(ide_controllers_info[0]);
  ide_controllers_info[3] = ide_drive_type;
  ide_controllers_info32[1] = ide_drive_size;

  ide_reset_controller(ide_controllers_info[0], ide_controllers_info[1]);
 }

 //find hard disk
 ide_controllers_info = (word_t *) (ide_controllers_info_mem);
 ide_controllers_info32 = (dword_t *) (ide_controllers_info_mem+8);
 for(dword_t i=0; i<ide_controllers_pointer; i++, ide_controllers_info += 8, ide_controllers_info32 += 4) {
  if(ide_controllers_info[2]==0x0000 && ide_controllers_info32[0]!=0) {
   //we found hard disk on master
   ide_hdd_base = ide_controllers_info[0];
   ide_hdd_alt_base = ide_controllers_info[1];
   ide_hdd_drive = PATA_MASTER;
   ide_hdd_size = ide_controllers_info32[0];
   break;
  }
  if(ide_controllers_info[3]==0x0000 && ide_controllers_info32[1]!=0) {
   //we found hard disk on slave
   ide_hdd_base = ide_controllers_info[0];
   ide_hdd_alt_base = ide_controllers_info[1];
   ide_hdd_drive = PATA_SLAVE;
   ide_hdd_size = ide_controllers_info32[1];
   break;
  }
 }

 //find optical disk drive
 ide_controllers_info = (word_t *) (ide_controllers_info_mem);
 ide_controllers_info32 = (dword_t *) (ide_controllers_info_mem+8);
 for(dword_t i=0; i<ide_controllers_pointer; i++, ide_controllers_info += 8, ide_controllers_info32 += 4) {
  if(ide_controllers_info[2]==0xEB14) {
   //we found optical disk drive on master
   ide_cdrom_base = ide_controllers_info[0];
   ide_cdrom_alt_base = ide_controllers_info[1];
   ide_cdrom_drive = PATA_MASTER;
   break;
  }
  if(ide_controllers_info[3]==0xEB14) {
   //we found optical disk drive on slave
   ide_cdrom_base = ide_controllers_info[0];
   ide_cdrom_alt_base = ide_controllers_info[1];
   ide_cdrom_drive = PATA_SLAVE;
   break;
  }
 }
 
 //LOG
 log("\nIDE\n");
 ide_controllers_info = (word_t *) (ide_controllers_info_mem);
 ide_controllers_info32 = (dword_t *) (ide_controllers_info_mem+8);
 for(dword_t i=0; i<ide_controllers_pointer; i++, ide_controllers_info += 8, ide_controllers_info32 += 4) {
  log("\n");
  log_hex_specific_size_with_space(ide_controllers_info[0], 4);
  log_hex_specific_size_with_space(ide_controllers_info[1], 4);
  log("MASTER ");
  log_hex_specific_size_with_space(ide_controllers_info[2], 4);
  log_var_with_space(ide_controllers_info32[0]);

  log("\n");
  log_hex_specific_size_with_space(ide_controllers_info[0], 4);
  log_hex_specific_size_with_space(ide_controllers_info[1], 4);
  log("SLAVE ");
  log_hex_specific_size_with_space(ide_controllers_info[3], 4);
  log_var(ide_controllers_info32[1]);
 }
}