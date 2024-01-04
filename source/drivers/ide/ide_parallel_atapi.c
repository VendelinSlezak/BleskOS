//BleskOS

/*
* MIT License
* Copyright (c) 2023-2024 Vendelín Slezák
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

byte_t patapi_send_packet_command(word_t base_port, word_t transfer_length) {
 //drive must be already selected
 
 //send command
 outb(base_port + 1, 0);
 outb(base_port + 2, 0);
 outb(base_port + 3, 0);
 outb(base_port + 4, (byte_t)transfer_length);
 outb(base_port + 5, (byte_t)(transfer_length>>8));
 outb(base_port + 7, 0xA0);
 
 //wait
 if(ide_wait_for_data(base_port, 50)==STATUS_ERROR) {
  wait(1);
  inb(base_port+7); //clear interrupt state
  return STATUS_ERROR;
 }
 
 return STATUS_GOOD;
}

byte_t patapi_detect_disk(word_t base_port, word_t alt_base_port) {
 //reset controller
 ide_reset_controller(base_port, alt_base_port);

 //send packet command
 if(patapi_send_packet_command(base_port, 0)==STATUS_ERROR) {
  return STATUS_ERROR;
 }
 
 //send packet
 outw(base_port + 0, 0x00); //TEST UNIT READY
 outw(base_port + 0, 0);
 outw(base_port + 0, 0);
 outw(base_port + 0, 0);
 outw(base_port + 0, 0);
 outw(base_port + 0, 0);

 //wait for processing command
 if(ide_wait_drive_not_busy(base_port, 100)==STATUS_ERROR) {
  return STATUS_ERROR;
 }
 
 //read response
 if((inb(base_port + 7) & 0x01)==0x01) { //error
  return STATUS_ERROR;
 }
 else {
  return STATUS_GOOD;
 }
}

byte_t patapi_read_capabilites(word_t base_port, word_t alt_base_port) {
 word_t response = 0;
 optical_disk_size = 0;

 //reset controller
 ide_reset_controller(base_port, alt_base_port);
 
 //send packet command
 if(patapi_send_packet_command(base_port, 8)==STATUS_ERROR) {
  return STATUS_ERROR;
 }
 
 //send packet
 outw(base_port + 0, 0x25); //READ CAPABILITES
 outw(base_port + 0, 0x1);
 outw(base_port + 0, 0);
 outw(base_port + 0, 0);
 outw(base_port + 0, 0);
 outw(base_port + 0, 0);
 
 //wait
 if(ide_wait_for_data(base_port, 200)==STATUS_ERROR) {
  return STATUS_ERROR;
 }
 
 //read response
 response = inw(base_port + 0);
 optical_disk_size = (BIG_ENDIAN(response)<<16);
 response = inw(base_port + 0);
 optical_disk_size |= BIG_ENDIAN(response);
 inw(base_port + 0);
 inw(base_port + 0);
 
 return STATUS_GOOD;
}

byte_t patapi_eject_drive(word_t base_port, word_t alt_base_port) {
 //reset controller
 ide_reset_controller(base_port, alt_base_port);

 //send packet command
 if(patapi_send_packet_command(base_port, 0)==STATUS_ERROR) {
  return STATUS_ERROR;
 }
 
 //send packet
 outw(base_port + 0, 0x1B); //eject command
 outw(base_port + 0, 0);
 outw(base_port + 0, 0x0002);
 outw(base_port + 0, 0);
 outw(base_port + 0, 0);
 outw(base_port + 0, 0);
 
 return STATUS_GOOD;
}

byte_t patapi_read(word_t base_port, word_t alt_base_port, dword_t sector, byte_t number_of_sectors, dword_t memory) {
 word_t *mem = (word_t *) memory;
 word_t value;

 //clear any device output
 ide_clear_device_output(base_port);

 //reset controller
 ide_reset_controller(base_port, alt_base_port);
 
 //send packet command
 if(patapi_send_packet_command(base_port, 2048)==STATUS_ERROR) {
  return STATUS_ERROR;
 }
 
 //send packet
 outw(base_port + 0, 0xA8); //read command
 value = (word_t)(sector>>16);
 outw(base_port + 0, BIG_ENDIAN(value));
 value = (word_t)sector;
 outw(base_port + 0, BIG_ENDIAN(value));
 outw(base_port + 0, 0);
 outw(base_port + 0, (number_of_sectors<<8)); //number of sectors
 outw(base_port + 0, 0);

 //transfer data
 for(int i=0; i<number_of_sectors; i++) {
  //wait for drive to be ready
  if(ide_wait_for_data(base_port, 2000)==STATUS_ERROR) { //max 4 seconds
   return STATUS_ERROR;
  }

  //read data of sector
  for(dword_t j=0; j<1024; j++) {
   *mem = inw(base_port + 0);
   mem++;
  }
 }
 
 return STATUS_GOOD;
}

byte_t patapi_read_audio_cd_toc(word_t base_port, word_t alt_base_port, dword_t memory) {
 word_t *mem = (word_t *) memory;
 word_t value;

 //clear any device output
 ide_clear_device_output(base_port);

 //reset controller
 ide_reset_controller(base_port, alt_base_port);
 
 //send packet command
 if(patapi_send_packet_command(base_port, 252)==STATUS_ERROR) {
  return STATUS_ERROR;
 }
 
 //send packet
 outw(base_port + 0, 0x43); //read TOC command
 outw(base_port + 0, 0);
 outw(base_port + 0, 0);
 outw(base_port + 0, 0);
 outw(base_port + 0, 252); //read info max about 31 tracks TODO: what is right value to read more?
 outw(base_port + 0, 0);
 
 //wait max 4 seconds
 if(ide_wait_for_data(base_port, 200)==STATUS_ERROR) {
  return STATUS_ERROR;
 }
 
 //read data until device sends data
 for(dword_t i=0; i<126; i++) {
  if((inb(base_port+7) & 0x08)==0x08) { //Data Ready bit is set
   *mem = inw(base_port + 0);
   mem++;
  }
  else {
   break;
  }
 }

 //clear any device output
 ide_clear_device_output(base_port);
 
 return STATUS_GOOD;
}

byte_t patapi_read_audio_cd_sector(word_t base_port, dword_t sector, dword_t memory) {
 word_t *mem = (word_t *) memory;
 word_t value;

 //clear any device output
 ide_clear_device_output(base_port);
 
 //send packet command
 if(patapi_send_packet_command(base_port, 2352)==STATUS_ERROR) {
  return STATUS_ERROR;
 }
 
 //send packet
 outw(base_port + 0, 0xBE); //read CD command
 value = (word_t)(sector>>16);
 outw(base_port + 0, BIG_ENDIAN(value));
 value = (word_t)sector;
 outw(base_port + 0, BIG_ENDIAN(value));
 outw(base_port + 0, 0);
 outw(base_port + 0, 0x1001); //one sector and we are reading User Data - 0x10
 outw(base_port + 0, 0);
 
 //wait max 6 seconds
 if(ide_wait_for_data(base_port, 3000)==STATUS_ERROR) {
  return STATUS_ERROR;
 }
 
 //read data
 for(int i=0; i<1176; i++) {
  *mem = inw(base_port + 0);
  mem++;
 }
 
 return STATUS_GOOD;
}