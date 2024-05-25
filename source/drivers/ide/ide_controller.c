//BleskOS

/*
* MIT License
* Copyright (c) 2023-2024 Vendelín Slezák
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

void initalize_ide_controller(byte_t number_of_controller) {
 //reset controller
 ide_reset_controller(storage_controllers[number_of_controller].base_1, storage_controllers[number_of_controller].base_2);

 //initalize master drive
 initalize_ide_controller_drive(number_of_controller, PATA_MASTER);

 //initalize slave drive
 initalize_ide_controller_drive(number_of_controller, PATA_SLAVE);
}

void initalize_ide_controller_drive(byte_t number_of_controller, byte_t number_of_drive) {
 //select drive
 ide_select_drive(storage_controllers[number_of_controller].base_1, number_of_drive);

 //if there is floating bus, it mean that this drive do not exist
 if(ide_is_bus_floating(storage_controllers[number_of_controller].base_1)==STATUS_TRUE) {
  return;
 }

 //send IDENTIFY command that will detect what type of device is connected
 ide_send_identify_drive(storage_controllers[number_of_controller].base_1);

 //save info about connected device
 if(hard_disk_info.controller_type==NO_CONTROLLER && ide_drive_type==PATA_HARD_DISK_SIGNATURE && ide_drive_size!=0) {
  hard_disk_info.controller_type = IDE_CONTROLLER;
  hard_disk_info.device_port = number_of_drive;
  hard_disk_info.base_1 = storage_controllers[number_of_controller].base_1;
  hard_disk_info.base_2 = storage_controllers[number_of_controller].base_2;
  hard_disk_info.number_of_sectors = ide_drive_size;
 }
 else if(optical_drive_info.controller_type==NO_CONTROLLER && ide_drive_type==PATA_OPTICAL_DRIVE_SIGNATURE) {
  optical_drive_info.controller_type = IDE_CONTROLLER;
  optical_drive_info.device_port = number_of_drive;
  optical_drive_info.base_1 = storage_controllers[number_of_controller].base_1;
  optical_drive_info.base_2 = storage_controllers[number_of_controller].base_2;
  optical_drive_info.number_of_sectors = 0;
 }

 //log
 log("\nIDE controller ");
 log_hex_specific_size_with_space(storage_controllers[number_of_controller].base_1, 4);
 if(number_of_drive==PATA_MASTER) {
  log("MASTER ");
 }
 else {
  log("SLAVE ");
 }
 log_hex_specific_size_with_space(ide_drive_type, 4);
 log_var(ide_drive_size);
}

byte_t ide_send_identify_drive(word_t base_port) {
 //reset variables
 ide_drive_type = 0xFFFF;
 ide_drive_size = 0;
 
 //send IDENTIFY command
 outb(base_port + 2, 0);
 outb(base_port + 3, 0);
 outb(base_port + 4, 0);
 outb(base_port + 5, 0);
 outb(base_port + 7, 0xEC);

 //wait
 if(ide_wait_for_data(base_port, 50)==STATUS_ERROR) {
  //read drive type
  ide_drive_type = (inb(base_port + 4) | (inb(base_port + 5) << 8));

  return STATUS_ERROR;
 }

 //read drive type
 ide_drive_type = (inb(base_port + 4) | (inb(base_port + 5) << 8));

 //read device info
 clear_memory((dword_t)device_info, 512);
 word_t *device_info_pointer = (word_t *) (device_info);
 for(int i=0; i<256; i++) {
  device_info_pointer[i] = inw(base_port + 0);
 }

 //read useful values
 if(device_info->lba48_total_number_of_sectors!=0) {
  if(device_info->lba48_total_number_of_sectors>0xFFFFFFFF) {
   ide_drive_size = 0xFFFFFFFF;
  }
  else {
   ide_drive_size = device_info->lba48_total_number_of_sectors;
  }
 }
 else {
  ide_drive_size = device_info->lba28_total_number_of_sectors;
 }

 return STATUS_GOOD;
}

byte_t ide_is_bus_floating(word_t base_port) {
 if(inb(base_port + 7)==0xFF || inb(base_port + 7)==0x7F) { //invalid state of status register
  return STATUS_TRUE;
 }
 else {
  return STATUS_FALSE;
 }
}

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
  byte_t status_register = inb(base_port + 7);
  if((status_register & 0x01)==0x01) { //error
   return STATUS_ERROR;
  }
  else if((status_register & 0x80)==0x00) { //drive is not busy
   return STATUS_GOOD;
  }
 }
 
 return STATUS_ERROR;
}

byte_t ide_wait_for_data(word_t base_port, dword_t wait_ticks) {
 ticks=0;

 while(ticks<wait_ticks) {
  asm("nop");
  byte_t status_register = inb(base_port + 7);
  if((status_register & 0x88)==0x08) { //data are ready
   return STATUS_GOOD;
  }
  else if((status_register & 0x81)==0x01) { //error
   return STATUS_ERROR;
  }
 }
 
 return STATUS_ERROR;
}

void ide_clear_device_output(word_t base_port) {
 for(dword_t i=0; i<2048*0xFFFF; i++) {
  if((inb(base_port+7) & 0x08)==0x08) { //Data Ready bit is set
   inw(base_port+0);
  }
  else {
   return;
  }
 }
}

void ide_reset_controller(word_t base_port, word_t alt_base_port) {
 //get actual selected drive
 byte_t selected_drive = inb(base_port+6);

 //reset controller
 outb(alt_base_port+2, (1<<2) | (1<<1));
 wait(1);
 outb(alt_base_port+2, (1<<1));
 wait(1);
 if(inb(base_port + 7)!=0xFF) {
  ide_wait_drive_not_busy(base_port, 100);
 }

 //restore previously selected drive
 ide_select_drive(base_port, selected_drive);
}

void ide_select_drive(word_t base_port, byte_t drive) {
 ide_wait_drive_not_busy(base_port, 100);
 outb(base_port + 6, drive);
 wait(5);
}