//BleskOS

/*
* MIT License
* Copyright (c) 2023-2025 BleskOS developers
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

void initalize_storage_controllers(void) {
 //now we support only one hard disk and one optical drive
 hard_disk_info.controller_type = NO_CONTROLLER;
 optical_drive_info.controller_type = NO_CONTROLLER;

 //allocate memory for IDENTIFY command data
 device_info = (struct ata_identify_command_device_info *) (malloc(512));

    // run IDE driver
    for(dword_t i = 0; i < components->n_ide; i++) {
        initalize_ide_controller(i);
    }

    // run AHCI driver
    for(dword_t i = 0; i < components->n_ahci; i++) {
        initalize_ahci_controller(i);
    }

 //free memory
 free((void *)device_info);

 //log
 logf("\n");
 if(hard_disk_info.controller_type!=NO_CONTROLLER) {
  logf("\nHard disk: ");
  if(hard_disk_info.controller_type==IDE_CONTROLLER) {
   logf("IDE 0x%04x 0x%02x ", hard_disk_info.base_1, hard_disk_info.device_port);
  }
  else if(hard_disk_info.controller_type==AHCI_CONTROLLER) {
   logf("AHCI 0x%x port %d ", hard_disk_info.base_1, hard_disk_info.device_port);
  }
  logf("Number of sectors: %d", hard_disk_info.number_of_sectors);
 }

 if(optical_drive_info.controller_type!=NO_CONTROLLER) {
  logf("\nOptical drive: ");
  if(optical_drive_info.controller_type==IDE_CONTROLLER) {
   logf("IDE 0x%04x 0x%02x", optical_drive_info.base_1, optical_drive_info.device_port);
  }
  else if(optical_drive_info.controller_type==AHCI_CONTROLLER) {
   logf("AHCI 0x%x port %d", optical_drive_info.base_1, optical_drive_info.device_port);
  }
 }

 //connect partitions of devices
 if(hard_disk_info.controller_type!=NO_CONTROLLER) {
  connect_partitions_of_medium(MEDIUM_HARD_DISK, DEFAULT_MEDIUM_NUMBER);
 }
 if(optical_drive_info.controller_type!=NO_CONTROLLER && detect_optical_disk()==STATUS_TRUE) {
  connect_partitions_of_medium(MEDIUM_OPTICAL_DRIVE, DEFAULT_MEDIUM_NUMBER);
 }
}

byte_t does_storage_medium_exist(byte_t medium_type, byte_t medium_number) {
 if(medium_type==MEDIUM_HARD_DISK) { //hard disk
  if(medium_number==DEFAULT_MEDIUM_NUMBER && hard_disk_info.controller_type!=NO_CONTROLLER) {
   return STATUS_TRUE;
  }
 }
 else if(medium_type==MEDIUM_OPTICAL_DRIVE) { //optical drive
  if(medium_number==DEFAULT_MEDIUM_NUMBER && optical_drive_info.controller_type!=NO_CONTROLLER) {
   return STATUS_TRUE;
  }
 }
 else if(medium_type==MEDIUM_USB_MSD) { //USB mass storage device
  if(usb_devices[medium_number].is_used == STATUS_TRUE && usb_devices[medium_number].msd.is_present == STATUS_TRUE) {
   return STATUS_TRUE;
  }
 }
 
 return STATUS_FALSE;
}

void select_storage_medium(byte_t medium_type, byte_t medium_number) {
 //test if this medium is not already selected
 if(storage_medium==medium_type && storage_medium_number==medium_number) {
  return;
 }

 //set variables
 storage_medium = medium_type;
 storage_medium_number = medium_number;

 //select medium
 if(medium_type==MEDIUM_HARD_DISK && medium_number==DEFAULT_MEDIUM_NUMBER) { //hard disk
  //if hard disk is connected to IDE port, select it
  if(hard_disk_info.controller_type==IDE_CONTROLLER) {
   ide_select_drive(hard_disk_info.base_1, hard_disk_info.device_port);
  }
 }
 else if(medium_type==MEDIUM_OPTICAL_DRIVE && medium_number==DEFAULT_MEDIUM_NUMBER) { //optical drive
  //if optical drive is connected to IDE port, select it
  if(optical_drive_info.controller_type==IDE_CONTROLLER) {
   ide_select_drive(optical_drive_info.base_1, optical_drive_info.device_port);
  }
 }
}

byte_t read_storage_medium(dword_t sector, byte_t num_of_sectors, dword_t memory) {
 if(storage_medium==MEDIUM_HARD_DISK) { //hard disk
  if(hard_disk_info.controller_type==AHCI_CONTROLLER) { //hard disk is connected to AHCI port
   return sata_read(hard_disk_info.base_1, hard_disk_info.base_2, sector, num_of_sectors, memory);
  }
  if(hard_disk_info.controller_type==IDE_CONTROLLER) { //hard disk is connected to IDE port
   return pata_read(hard_disk_info.base_1, sector, num_of_sectors, (byte_t *)memory);
  }
 }
 else if(storage_medium==MEDIUM_OPTICAL_DRIVE) { //optical drive
  if(optical_drive_info.controller_type==AHCI_CONTROLLER) { //CDROM is connected to AHCI port
   return satapi_read(optical_drive_info.base_1, optical_drive_info.base_2, sector, num_of_sectors, memory);
  }
  if(optical_drive_info.controller_type==IDE_CONTROLLER) { //CDROM is connected to IDE port
   return patapi_read(optical_drive_info.base_1, optical_drive_info.base_2, sector, num_of_sectors, (byte_t *)memory);
  }
 }
 else if(storage_medium==MEDIUM_USB_MSD) { //USB mass storage device
  if(usb_devices[storage_medium_number].is_used == STATUS_FALSE || usb_devices[storage_medium_number].msd.is_present == STATUS_FALSE) {
   return STATUS_ERROR; //there is no USB mass storage device connected
  }
  return usb_msd_read(storage_medium_number, sector, num_of_sectors, (byte_t *)memory);
 }
 
 return STATUS_ERROR;
}

byte_t write_storage_medium(dword_t sector, byte_t num_of_sectors, dword_t memory) {
 if(storage_medium==MEDIUM_HARD_DISK) { //hard disk
  if(hard_disk_info.controller_type==AHCI_CONTROLLER) { //hard disk is connected to AHCI port
   return sata_write(hard_disk_info.base_1, hard_disk_info.base_2, sector, num_of_sectors, memory);
  }
  if(hard_disk_info.controller_type==IDE_CONTROLLER) { //hard disk is connected to IDE port
   return pata_write(hard_disk_info.base_1, sector, num_of_sectors, (byte_t *)memory);
  }
 }
 else if(storage_medium==MEDIUM_OPTICAL_DRIVE) {
  return STATUS_ERROR; //we do not support writing to optical disks
 }
 else if(storage_medium==MEDIUM_USB_MSD) { //USB mass storage device
  if(usb_devices[storage_medium_number].is_used == STATUS_FALSE || usb_devices[storage_medium_number].msd.is_present == STATUS_FALSE) {
   return STATUS_ERROR; //there is no USB mass storage device connected
  }
  return usb_msd_write(storage_medium_number, sector, num_of_sectors, (byte_t *)memory);
 }
 
 return STATUS_ERROR;
}

byte_t read_audio_cd(dword_t sector, dword_t num_of_sectors, dword_t memory) {
 if(storage_medium==MEDIUM_OPTICAL_DRIVE) {
  if(optical_drive_info.controller_type==AHCI_CONTROLLER) { //CDROM is connected to AHCI port
   return satapi_read_audio_cd_sector(optical_drive_info.base_1, optical_drive_info.base_2, sector, num_of_sectors, memory);
  }
  if(optical_drive_info.controller_type==IDE_CONTROLLER) { //CDROM is connected to IDE port
   return patapi_read_audio_cd(optical_drive_info.base_1, optical_drive_info.base_2, sector, num_of_sectors, (byte_t *)memory);
  }
 }

 return STATUS_ERROR;
}

byte_t detect_optical_disk(void) {
 if(does_storage_medium_exist(MEDIUM_OPTICAL_DRIVE, DEFAULT_MEDIUM_NUMBER)==STATUS_FALSE) {
  return STATUS_FALSE;
 }

 //set variables
 byte_t is_disk_inserted = STATUS_FALSE;
 optical_disk_size = 0;
 optical_disk_sector_size = 0;
 
 //select optical drive
 byte_t previously_selected_storage_medium = storage_medium;
 byte_t previously_selected_storage_medium_number = storage_medium_number;
 if(storage_medium!=MEDIUM_OPTICAL_DRIVE) {
  select_storage_medium(MEDIUM_OPTICAL_DRIVE, DEFAULT_MEDIUM_NUMBER);
 }
 
 //detect disk
 if(optical_drive_info.controller_type==AHCI_CONTROLLER) { //CDROM is connected to AHCI port
  is_disk_inserted = satapi_detect_disk(optical_drive_info.base_1, optical_drive_info.base_2);
 }
 else if(optical_drive_info.controller_type==IDE_CONTROLLER) { //CDROM is connected to IDE port
  is_disk_inserted = patapi_detect_disk(optical_drive_info.base_1, optical_drive_info.base_2);
 }

 //if there is disk inserted, read capabilities
 if(is_disk_inserted==STATUS_TRUE) {
  if(optical_drive_info.controller_type==AHCI_CONTROLLER) { //CDROM is connected to AHCI port
   satapi_read_capabilities(optical_drive_info.base_1, optical_drive_info.base_2);
  }
  else if(optical_drive_info.controller_type==IDE_CONTROLLER) { //CDROM is connected to IDE port
   patapi_read_capabilities(optical_drive_info.base_1, optical_drive_info.base_2);
  }
 }
 
 //select previous medium
 if(previously_selected_storage_medium!=MEDIUM_OPTICAL_DRIVE || previously_selected_storage_medium_number!=DEFAULT_MEDIUM_NUMBER) {
  select_storage_medium(previously_selected_storage_medium, previously_selected_storage_medium_number);
 }
 
 //return status
 return is_disk_inserted;
}

void eject_optical_disk(void) {
 if(does_storage_medium_exist(MEDIUM_OPTICAL_DRIVE, DEFAULT_MEDIUM_NUMBER)==STATUS_FALSE) {
  return;
 }
 
 //select optical drive
 byte_t previously_selected_storage_medium = storage_medium;
 byte_t previously_selected_storage_medium_number = storage_medium_number;
 if(storage_medium!=MEDIUM_OPTICAL_DRIVE) {
  select_storage_medium(MEDIUM_OPTICAL_DRIVE, DEFAULT_MEDIUM_NUMBER);
 }
 
 //eject drive
 if(optical_drive_info.controller_type==AHCI_CONTROLLER) { //CDROM is connected to AHCI port
  satapi_start_stop_command(optical_drive_info.base_1, optical_drive_info.base_2, PATA_EJECT);
 }
 else if(optical_drive_info.controller_type==IDE_CONTROLLER) { //CDROM is connected to IDE port
  patapi_start_stop_command(optical_drive_info.base_1, optical_drive_info.base_2, PATA_EJECT);
 }
 
 //select previous medium
 if(previously_selected_storage_medium!=MEDIUM_OPTICAL_DRIVE || previously_selected_storage_medium_number!=DEFAULT_MEDIUM_NUMBER) {
  select_storage_medium(previously_selected_storage_medium, previously_selected_storage_medium_number);
 }

 //update device list
 remove_partitions_of_medium_from_list(MEDIUM_OPTICAL_DRIVE, DEFAULT_MEDIUM_NUMBER);
}

void spin_down_optical_drive(void) {
 if(optical_drive_info.controller_type==AHCI_CONTROLLER) { //CDROM is connected to AHCI port
  satapi_start_stop_command(optical_drive_info.base_1, optical_drive_info.base_2, PATA_SPIN_DOWN);
 }
 else if(optical_drive_info.controller_type==IDE_CONTROLLER) { //CDROM is connected to IDE port
  patapi_start_stop_command(optical_drive_info.base_1, optical_drive_info.base_2, PATA_SPIN_DOWN);
 }
}

byte_t read_optical_disk_toc(void) {
 if(does_storage_medium_exist(MEDIUM_OPTICAL_DRIVE, DEFAULT_MEDIUM_NUMBER)==STATUS_FALSE) {
  return STATUS_ERROR;
 }

 //clear TOC structure
 clear_memory((dword_t)(&optical_disk_table_of_content), sizeof(optical_disk_table_of_content));

 //read TOC
 if(optical_drive_info.controller_type==AHCI_CONTROLLER) { //CDROM is connected to AHCI port
  if(satapi_read_cd_toc(optical_drive_info.base_1, optical_drive_info.base_2, (dword_t)(&optical_disk_table_of_content))==STATUS_ERROR) {
   return STATUS_ERROR;
  }
 }
 else if(optical_drive_info.controller_type==IDE_CONTROLLER) { //CDROM is connected to IDE port
  if(patapi_read_cd_toc(optical_drive_info.base_1, optical_drive_info.base_2, (dword_t)(&optical_disk_table_of_content))==STATUS_ERROR) {
   return STATUS_ERROR;
  }
 }

 //convert big endian values to little endian
 optical_disk_table_of_content.length = BIG_ENDIAN_WORD(optical_disk_table_of_content.length);
 for(dword_t i=0; i<(optical_disk_table_of_content.last_track+1); i++) {
  byte_t byte_1 = (byte_t)(optical_disk_table_of_content.track[i].first_sector);
  byte_t byte_2 = (byte_t)(optical_disk_table_of_content.track[i].first_sector>>8);
  byte_t byte_3 = (byte_t)(optical_disk_table_of_content.track[i].first_sector>>16);
  byte_t byte_4 = (byte_t)(optical_disk_table_of_content.track[i].first_sector>>24);
  optical_disk_table_of_content.track[i].first_sector = (byte_1<<24 | byte_2<<16 | byte_3<<8 | byte_4);
 }
 
 return STATUS_GOOD;
}

void refresh_devices(void) {
 //detect state of optical disk
 if(detect_optical_disk()==STATUS_FALSE) {
  remove_partitions_of_medium_from_list(MEDIUM_OPTICAL_DRIVE, DEFAULT_MEDIUM_NUMBER);
 }
 else {
  byte_t is_optical_disk_connected = STATUS_FALSE;
  for(dword_t i=0; i<MAX_NUMBER_OF_CONNECTED_PARTITIONS; i++) {
   if(connected_partitions[i].medium_type==MEDIUM_OPTICAL_DRIVE && connected_partitions[i].medium_number==DEFAULT_MEDIUM_NUMBER) {
    is_optical_disk_connected = STATUS_TRUE;
    break;
   }
  }

  if(is_optical_disk_connected==STATUS_FALSE) {
   connect_partitions_of_medium(MEDIUM_OPTICAL_DRIVE, DEFAULT_MEDIUM_NUMBER);
  }
 }
}