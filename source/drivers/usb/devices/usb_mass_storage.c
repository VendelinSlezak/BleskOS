//BleskOS

/*
* MIT License
* Copyright (c) 2023-2025 Vendelín Slezák
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

void usb_msd_save_informations(byte_t device_address, struct usb_full_interface_info_t interface) {
 //return means that there will be no information saved about USB msd on this device

 //check if this device do not already have msd interface
 if(usb_devices[device_address].msd.is_present == STATUS_TRUE) {
  l("\nUSB ERROR: more MSD interfaces at one device");
  return;
 }

 //check if this interface has all needed informations
 if(interface.bulk_in_endpoint == 0 || interface.bulk_out_endpoint == 0) {
  l("\nUSB ERROR: USB MSD do not have bulk IN and OUT endpoint");
  return;
 }
 if(interface.bulk_in_endpoint->wMaxPacketSize == 0 || interface.bulk_out_endpoint->wMaxPacketSize == 0) {
  l("\nUSB ERROR: USB MSD wrong endpoint size");
  return;
 }

 //save informations
 usb_devices[device_address].msd.is_present = STATUS_TRUE;
 usb_devices[device_address].msd.is_initalized = STATUS_FALSE;
 usb_devices[device_address].msd.initalize = usb_msd_initalize;

 usb_devices[device_address].msd.interface = interface.interface_descriptor->bInterfaceNumber;

 usb_devices[device_address].msd.bulk_only.in.endpoint = interface.bulk_in_endpoint->endpoint_number;
 usb_devices[device_address].msd.bulk_only.in.endpoint_size = interface.bulk_in_endpoint->wMaxPacketSize;
 usb_devices[device_address].msd.bulk_only.in.endpoint_toggle = USB_TOGGLE_0;

 usb_devices[device_address].msd.bulk_only.out.endpoint = interface.bulk_out_endpoint->endpoint_number;
 usb_devices[device_address].msd.bulk_only.out.endpoint_size = interface.bulk_out_endpoint->wMaxPacketSize;
 usb_devices[device_address].msd.bulk_only.out.endpoint_toggle = USB_TOGGLE_0;

 usb_devices[device_address].msd.unit_state = USB_MSD_WITHOUT_UNIT;

 //log
 l("\nUSB Mass Storage with Bulk Only protocol\n Endpoint IN: "); lvw(usb_devices[device_address].msd.bulk_only.in.endpoint);
 l("\n Endpoint OUT: "); lvw(usb_devices[device_address].msd.bulk_only.out.endpoint);
}

void usb_msd_initalize(byte_t device_address) {
 //set device into initalization phase
 usb_devices[device_address].is_interface_in_initalization = STATUS_TRUE;

 l("\nstart of initalization of MSD");

 //sent INQUIRY command
 usb_bulk_only_prepare_transfer((struct usb_bbb_t *)&usb_devices[device_address].msd.bulk_only,
                                usb_msd_inquiry_success, usb_msd_inquiry_error,
                                USB_CBW_DIRECTION_READ,
                                6, //INQUIRY command has 6 bytes
                                sizeof(struct usb_msd_inquiry_t), //INQUIRY transfer size
                                (void *) malloc(sizeof(struct usb_msd_inquiry_t)));
 usb_devices[device_address].msd.bulk_only.usb_command_block_wrapper.inquiry.command = 0x12;
 usb_devices[device_address].msd.bulk_only.usb_command_block_wrapper.inquiry.big_endian_transfer_length = BIG_ENDIAN_WORD(sizeof(struct usb_msd_inquiry_t));
 usb_bulk_only_send_command(device_address, (struct usb_bbb_t *)&usb_devices[device_address].msd.bulk_only);
}

void usb_msd_inquiry_success(byte_t device_address) {
 l("\nUSB MSD: INQUIRY success");

 //log
 struct usb_msd_inquiry_t *inquiry = (struct usb_msd_inquiry_t *) usb_devices[device_address].msd.bulk_only.buffer;
 l("\n Device type: ");
 if(inquiry->peripheral_device_type==0x0) {
  l("Direct Access Block Device");
 }
 else if(inquiry->peripheral_device_type==0x5) {
  l("CD-ROM/DVD");
 }
 else {
  lhs(inquiry->peripheral_device_type, 1);
 }

 byte_t string[9];
 string[8] = 0;
 copy_memory((dword_t)(&inquiry->vendor_id), (dword_t)(&string), 8);
 l("\n Vendor ID string: ");
 l(string);
 
 copy_memory((dword_t)(&inquiry->product_id), (dword_t)(&string), 8);
 l("\n Product ID string: ");
 l(string);

 //free memory of INQUIRY
 free((dword_t)usb_devices[device_address].msd.bulk_only.buffer);

 //initalization is successfully done
 usb_devices[device_address].msd.is_initalized = STATUS_TRUE;
 usb_devices[device_address].is_interface_in_initalization = STATUS_FALSE;

 //check presence of unit
 usb_devices[device_address].msd.test_unit_ready_retries = 3;
 usb_devices[device_address].msd.retry_after_successfull_request_sense = usb_msd_send_tur;
 usb_msd_send_tur(device_address);
}

/* check presence of unit */

void usb_msd_monitor_unit_state(void) {
 for(dword_t i=1; i<MAX_NUMBER_OF_USB_DEVICES; i++) {
  if(usb_devices[i].is_used == STATUS_TRUE && usb_devices[i].msd.is_present == STATUS_TRUE) {
   //check if there is reset sequence waiting
   if(usb_devices[i].msd.reset_sequence_waiting == STATUS_TRUE) {
    usb_msd_call_reset_sequence(i);
   }
   else if(usb_devices[i].msd.reset_sequence_running == STATUS_FALSE && usb_devices[i].msd.bulk_only.is_running == STATUS_FALSE && usb_devices[i].msd.is_initalized == STATUS_TRUE) {
    //check if there is new unit without paritions connected
    if(usb_devices[i].msd.unit_state == USB_MSD_WITH_UNIT && usb_devices[i].msd.partitions_connected == STATUS_FALSE) {
     connect_partitions_of_medium(MEDIUM_USB_MSD, i);
     usb_devices[i].msd.partitions_connected = STATUS_TRUE;
     usb_device_change_event = STATUS_TRUE;
    }
    //check if is unit still present, or if new unit was connected
    else {
     usb_devices[i].msd.test_unit_ready_retries = 1;
     usb_devices[i].msd.retry_after_successfull_request_sense = 0;
     usb_msd_send_tur(i);
    }
   }
   
  }
 }
}

void usb_msd_send_tur(byte_t device_address) {
 //check if there are possible tries
 if(usb_devices[device_address].msd.test_unit_ready_retries == 0) {
  l("\nUSB MSD: TUR retries ended");
  //TODO: call error
  return;
 }
 usb_devices[device_address].msd.test_unit_ready_retries--;

 //send TEST_UNIT_READY command
 usb_bulk_only_prepare_transfer((struct usb_bbb_t *)&usb_devices[device_address].msd.bulk_only,
                                usb_msd_tur_success, usb_msd_tur_error,
                                0,
                                6, //TEST_UNIT_READY command has 6 bytes
                                0, 0); //TEST_UNIT_READY will not transfer anything
 usb_bulk_only_send_command(device_address, (struct usb_bbb_t *)&usb_devices[device_address].msd.bulk_only);
}

void usb_msd_tur_success(byte_t device_address) {
 //if there already was unit, TEST_UNIT_READY only checked that it is still here
 if(usb_devices[device_address].msd.unit_state == USB_MSD_WITH_UNIT) {
  return;
 }
 else {
  l("\nUSB MSD: unit is ready");
 }

 //send READ_CAPACITY(10) command
 usb_devices[device_address].msd.retry_after_successfull_request_sense = 0; //TODO: retry
 usb_bulk_only_prepare_transfer((struct usb_bbb_t *)&usb_devices[device_address].msd.bulk_only,
                                usb_msd_read_capacity_success, usb_msd_read_capacity_error,
                                USB_CBW_DIRECTION_READ,
                                10, //READ_CAPACITY(10) command has 10 bytes
                                sizeof(struct usb_read_capacity_t), //READ_CAPACITY transfer size
                                (void *) malloc(sizeof(struct usb_read_capacity_t)));
 usb_devices[device_address].msd.bulk_only.usb_command_block_wrapper.read_capacity.command = 0x25;
 usb_bulk_only_send_command(device_address, (struct usb_bbb_t *)&usb_devices[device_address].msd.bulk_only);
}

void usb_msd_read_capacity_success(byte_t device_address) {
 l("\nUSB MSD: READ_CAPACITY(10) success");

 //read returned data
 struct usb_read_capacity_t *read_capacity = (struct usb_read_capacity_t *) usb_devices[device_address].msd.bulk_only.buffer;
 usb_devices[device_address].msd.number_of_sectors = BIG_ENDIAN_DWORD(read_capacity->big_endian_number_of_sectors_on_disk)-1; //some devices report one more sector than real value
 usb_devices[device_address].msd.size_of_sector = BIG_ENDIAN_DWORD(read_capacity->big_endian_size_of_sector_on_disk);

 //log
 log("\n Sector size: ");
 log_var(usb_devices[device_address].msd.size_of_sector);
 log("\n Number of sectors: ");
 log_var(usb_devices[device_address].msd.number_of_sectors);
 log("\n Size in MB: ");
 log_var((usb_devices[device_address].msd.number_of_sectors/2000));

 //free memory of READ_CAPACITY(10)
 free((dword_t)usb_devices[device_address].msd.bulk_only.buffer);

 //update unit status
 usb_devices[device_address].msd.unit_state = USB_MSD_WITH_UNIT;
 usb_devices[device_address].msd.partitions_connected = STATUS_FALSE;

 //TODO: get real device limit of sectors per transfer request
 usb_devices[device_address].msd.max_sectors_per_transfer_request = 128;
}

/* operations with unit */

byte_t usb_msd_read(byte_t device_address, dword_t sector, dword_t number_of_sectors, byte_t *memory) {
 usb_msd_transfer(device_address, USB_TRANSFER_IN, sector, number_of_sectors, memory);
}

byte_t usb_msd_write(byte_t device_address, dword_t sector, dword_t number_of_sectors, byte_t *memory) {
 usb_msd_transfer(device_address, USB_TRANSFER_OUT, sector, number_of_sectors, memory);
}

byte_t usb_msd_transfer(byte_t device_address, byte_t direction, dword_t sector, dword_t number_of_sectors, byte_t *memory) {
 //check if we can transfer data from this device
 if(usb_devices[device_address].msd.bulk_only.is_running == STATUS_TRUE
    || usb_devices[device_address].msd.reset_sequence_waiting == STATUS_TRUE
    || usb_devices[device_address].msd.reset_sequence_running == STATUS_TRUE) {
  //wait
  volatile dword_t timeout = (time_of_system_running+500);
  while(usb_devices[device_address].msd.bulk_only.is_running == STATUS_TRUE
        || usb_devices[device_address].msd.reset_sequence_waiting == STATUS_TRUE
        || usb_devices[device_address].msd.reset_sequence_running == STATUS_TRUE) {
   if(time_of_system_running > timeout) {
    l("\nUSB MSD: can not do transfer ");
    if(usb_devices[device_address].msd.bulk_only.is_running == STATUS_TRUE) {
     l("Bulk running ");
    }
    if(usb_devices[device_address].msd.reset_sequence_waiting == STATUS_TRUE) {
     l("Reset waiting ");
    }
    if(usb_devices[device_address].msd.reset_sequence_running == STATUS_TRUE) {
     l("Reset running ");
    }
    return STATUS_ERROR;
   }
  }

  //check if is device ready to transfer data
  if(usb_devices[device_address].msd.is_present = STATUS_FALSE || usb_devices[device_address].msd.unit_state == USB_MSD_WITHOUT_UNIT) {
   return STATUS_ERROR;
  }

  //device is ready to transfer data
 }

 //now we support only 512 bytes sectors TODO: add support for bigger sizes
 if(usb_devices[device_address].msd.size_of_sector!=512) {
  l("\nUSB MSD: Bad size of sector");
  return STATUS_ERROR;
 }

 //start transfer
 usb_devices[device_address].msd.sector = sector;
 usb_devices[device_address].msd.number_of_rest_of_sectors = number_of_sectors;
 usb_devices[device_address].msd.memory_of_transfer = memory;
 usb_devices[device_address].msd.transfer_status = USB_TRANSFER_NOT_TRANSFERRED;
 if(direction == USB_TRANSFER_IN) {
  if(usb_devices[device_address].msd.number_of_rest_of_sectors > usb_devices[device_address].msd.max_sectors_per_transfer_request) {
   usb_msd_read_10(device_address, sector, usb_devices[device_address].msd.max_sectors_per_transfer_request, memory);
  }
  else {
   usb_msd_read_10(device_address, sector, number_of_sectors, memory);
  }
 }
 else {
  if(usb_devices[device_address].msd.number_of_rest_of_sectors > usb_devices[device_address].msd.max_sectors_per_transfer_request) {
   usb_msd_write_10(device_address, sector, usb_devices[device_address].msd.max_sectors_per_transfer_request, memory);
  }
  else {
   usb_msd_write_10(device_address, sector, number_of_sectors, memory);
  }
 }

 //wait for tranfer to complete
 // TODO: later rewrite to complete transfers by calling methods
 while(usb_devices[device_address].msd.transfer_status == USB_TRANSFER_NOT_TRANSFERRED) {
  asm("hlt");
 }

 if(usb_devices[device_address].msd.transfer_status == USB_TRANSFER_ERROR) {
  l("\nUSB MSD: transfer error");
  return STATUS_ERROR;
 }
 else {
  return STATUS_GOOD;
 }
}

void usb_msd_read_10(byte_t device_address, dword_t sector, word_t number_of_sectors, byte_t *memory) {
 //send READ(10) command
 usb_devices[device_address].msd.retry_after_successfull_request_sense = usb_msd_transfer_not_successfull;
 usb_bulk_only_prepare_transfer((struct usb_bbb_t *)&usb_devices[device_address].msd.bulk_only,
                                usb_msd_read_10_success, usb_msd_read_10_error,
                                USB_CBW_DIRECTION_READ,
                                10, //READ(10) command has 10 bytes
                                (number_of_sectors*usb_devices[device_address].msd.size_of_sector), //READ(10) transfer size
                                (void *) memory);
 usb_devices[device_address].msd.bulk_only.usb_command_block_wrapper.read_write_command.command = 0x28;
 usb_devices[device_address].msd.bulk_only.usb_command_block_wrapper.read_write_command.big_endian_lba = BIG_ENDIAN_DWORD(sector);
 usb_devices[device_address].msd.bulk_only.usb_command_block_wrapper.read_write_command.big_endian_number_of_sectors = BIG_ENDIAN_WORD(number_of_sectors);
 usb_bulk_only_send_command(device_address, (struct usb_bbb_t *)&usb_devices[device_address].msd.bulk_only);
}

void usb_msd_read_10_success(byte_t device_address) {
 //change values
 usb_devices[device_address].msd.sector += BIG_ENDIAN_WORD(usb_devices[device_address].msd.bulk_only.usb_command_block_wrapper.read_write_command.big_endian_number_of_sectors);
 usb_devices[device_address].msd.number_of_rest_of_sectors -= BIG_ENDIAN_WORD(usb_devices[device_address].msd.bulk_only.usb_command_block_wrapper.read_write_command.big_endian_number_of_sectors);
 usb_devices[device_address].msd.memory_of_transfer += (BIG_ENDIAN_WORD(usb_devices[device_address].msd.bulk_only.usb_command_block_wrapper.read_write_command.big_endian_number_of_sectors)*usb_devices[device_address].msd.size_of_sector);
 
 //check if this was whole transfer
 if(usb_devices[device_address].msd.number_of_rest_of_sectors == 0) {
  usb_devices[device_address].msd.transfer_status = USB_TRANSFER_DONE;
 }
 //otherwise continue in transfer
 else if(usb_devices[device_address].msd.number_of_rest_of_sectors > usb_devices[device_address].msd.max_sectors_per_transfer_request) {
  usb_msd_read_10(device_address, usb_devices[device_address].msd.sector, usb_devices[device_address].msd.max_sectors_per_transfer_request, usb_devices[device_address].msd.memory_of_transfer);
 }
 else {
  usb_msd_read_10(device_address, usb_devices[device_address].msd.sector, usb_devices[device_address].msd.number_of_rest_of_sectors, usb_devices[device_address].msd.memory_of_transfer);
 }
}

void usb_msd_write_10(byte_t device_address, dword_t sector, word_t number_of_sectors, byte_t *memory) {
 //send WRITE(10) command
 usb_devices[device_address].msd.retry_after_successfull_request_sense = usb_msd_transfer_not_successfull;
 usb_bulk_only_prepare_transfer((struct usb_bbb_t *)&usb_devices[device_address].msd.bulk_only,
                                usb_msd_write_10_success, usb_msd_write_10_error,
                                USB_CBW_DIRECTION_WRITE,
                                10, //WRITE(10) command has 10 bytes
                                (number_of_sectors*usb_devices[device_address].msd.size_of_sector), //WRITE(10) transfer size
                                (void *) memory);
 usb_devices[device_address].msd.bulk_only.usb_command_block_wrapper.read_write_command.command = 0x2A;
 usb_devices[device_address].msd.bulk_only.usb_command_block_wrapper.read_write_command.big_endian_lba = BIG_ENDIAN_DWORD(sector);
 usb_devices[device_address].msd.bulk_only.usb_command_block_wrapper.read_write_command.big_endian_number_of_sectors = BIG_ENDIAN_WORD(number_of_sectors);
 usb_bulk_only_send_command(device_address, (struct usb_bbb_t *)&usb_devices[device_address].msd.bulk_only);
}

void usb_msd_write_10_success(byte_t device_address) {
 //change values
 usb_devices[device_address].msd.sector += BIG_ENDIAN_WORD(usb_devices[device_address].msd.bulk_only.usb_command_block_wrapper.read_write_command.big_endian_number_of_sectors);
 usb_devices[device_address].msd.number_of_rest_of_sectors -= BIG_ENDIAN_WORD(usb_devices[device_address].msd.bulk_only.usb_command_block_wrapper.read_write_command.big_endian_number_of_sectors);
 usb_devices[device_address].msd.memory_of_transfer += (BIG_ENDIAN_WORD(usb_devices[device_address].msd.bulk_only.usb_command_block_wrapper.read_write_command.big_endian_number_of_sectors)*usb_devices[device_address].msd.size_of_sector);
 
 //check if this was whole transfer
 if(usb_devices[device_address].msd.number_of_rest_of_sectors == 0) {
  usb_devices[device_address].msd.transfer_status = USB_TRANSFER_DONE;
 }
 //otherwise continue in transfer
 else if(usb_devices[device_address].msd.number_of_rest_of_sectors > usb_devices[device_address].msd.max_sectors_per_transfer_request) {
  usb_msd_write_10(device_address, usb_devices[device_address].msd.sector, usb_devices[device_address].msd.max_sectors_per_transfer_request, usb_devices[device_address].msd.memory_of_transfer);
 }
 else {
  usb_msd_write_10(device_address, usb_devices[device_address].msd.sector, usb_devices[device_address].msd.number_of_rest_of_sectors, usb_devices[device_address].msd.memory_of_transfer);
 }
}

void usb_msd_transfer_not_successfull(byte_t device_address) {
 usb_devices[device_address].msd.transfer_status = USB_TRANSFER_ERROR;
}

/* request sense */

void usb_msd_send_request_sense(byte_t device_address) {
 usb_bulk_only_prepare_transfer((struct usb_bbb_t *)&usb_devices[device_address].msd.bulk_only,
                                usb_msd_request_sense_success, usb_msd_request_sense_error,
                                USB_CBW_DIRECTION_READ,
                                6, //READ(10) command has 6 bytes
                                sizeof(struct usb_msd_request_sense_t), //REQUEST_SENSE transfer size
                                (void *) malloc(sizeof(struct usb_msd_request_sense_t)));
 usb_devices[device_address].msd.bulk_only.usb_command_block_wrapper.request_sense.command = 0x03;
 usb_devices[device_address].msd.bulk_only.usb_command_block_wrapper.request_sense.transfer_length = sizeof(struct usb_msd_request_sense_t);
 usb_bulk_only_send_command(device_address, (struct usb_bbb_t *)&usb_devices[device_address].msd.bulk_only);
}

void usb_msd_request_sense_success(byte_t device_address) {
 //check Media is not present
 struct usb_msd_request_sense_t *request_sense = (struct usb_msd_request_sense_t *) usb_devices[device_address].msd.bulk_only.buffer;
 if(request_sense->sense_key==0x02 && request_sense->additional_sense_code==0x3A && request_sense->additional_sense_qualifier==0x00) {
  //update unit status
  usb_devices[device_address].msd.unit_state = USB_MSD_WITHOUT_UNIT;

  //remove connected partitions
  if(usb_devices[device_address].msd.partitions_connected == STATUS_TRUE) {
   remove_partitions_of_medium_from_list(MEDIUM_USB_MSD, device_address);
   usb_devices[device_address].msd.partitions_connected = STATUS_FALSE;
   usb_device_change_event = STATUS_TRUE;
  }
 }
 //other reasons will be logged
 else {
  l("\nUSB MSD: REQUEST_SENSE ");

  if(request_sense->sense_key==0x00 && request_sense->additional_sense_code==0x00 && request_sense->additional_sense_qualifier==0x00) {
    log("No Sense");
  }
  else if(request_sense->sense_key==0x06 && request_sense->additional_sense_code==0x28 && request_sense->additional_sense_qualifier==0x00) {
    log("Not ready to ready transition - Media changed");
  }
  else if(request_sense->sense_key==0x07 && request_sense->additional_sense_code==0x27 && request_sense->additional_sense_qualifier==0x00) {
    log("Write protected media");
  }
  else {
    log_hex_specific_size_with_space(request_sense->sense_key, 2);
    log_hex_specific_size_with_space(request_sense->additional_sense_code, 2);
    log_hex_specific_size(request_sense->additional_sense_qualifier, 2);
  }
 }

 //call method that will try to send again command that lead to error and then REQUEST_SENSE
 if(usb_devices[device_address].msd.retry_after_successfull_request_sense != 0) {
  usb_devices[device_address].msd.retry_after_successfull_request_sense(device_address);
 }
}

/* errors */

void usb_msd_inquiry_error(byte_t device_address) {
 l("\nUSB MSD: INQUIRY was not transferred");

 //free memory of INQUIRY buffer for data
 free((dword_t)usb_devices[device_address].msd.bulk_only.buffer);

 //initalization was not successfull
 usb_devices[device_address].msd.is_present = STATUS_FALSE;
 usb_devices[device_address].is_interface_in_initalization = STATUS_FALSE;
}

void usb_msd_tur_error(byte_t device_address) {
 //send REQUEST_SENSE
 usb_msd_send_request_sense(device_address);
}

void usb_msd_read_capacity_error(byte_t device_address) {
 l("\nUSB MSD: READ_CAPACITY(10) error");

 //free memory of READ_CAPACITY(10) buffer for data
 free((dword_t)usb_devices[device_address].msd.bulk_only.buffer);

 //send REQUEST_SENSE
 usb_msd_send_request_sense(device_address);
}

void usb_msd_read_10_error(byte_t device_address) {
 l("\nUSB MSD: READ(10) error");

 //send REQUEST_SENSE
 usb_msd_send_request_sense(device_address);
}

void usb_msd_write_10_error(byte_t device_address) {
 l("\nUSB MSD: WRITE(10) error");

 //send REQUEST_SENSE
 usb_msd_send_request_sense(device_address);
}

void usb_msd_request_sense_error(byte_t device_address) {
 l("\nUSB MSD: REQUEST_SENSE error");

 //free memory of REQUEST_SENSE buffer for data
 free((dword_t)usb_devices[device_address].msd.bulk_only.buffer);

 //reset MSD
 usb_msd_call_reset_sequence(device_address);
}

/* reset sequence */

void usb_msd_call_reset_sequence(byte_t device_address) {
 //set waiting for reset sequence
 usb_devices[device_address].msd.reset_sequence_waiting = STATUS_TRUE;

 //check if we can perform reset sequence
 if(usb_devices[device_address].control_transfer.is_running == STATUS_TRUE) {
  return;
 }

 //start reset sequence by resetting MSD
 l("\nUSB MSD: Start of Reset Sequence");
 usb_devices[device_address].msd.reset_sequence_waiting = STATUS_FALSE;
 usb_devices[device_address].msd.reset_sequence_running = STATUS_TRUE;
 usb_control_transfer_without_data(device_address, usb_msd_reset_sequence_device_reset_success, usb_msd_reset_sequence_error, 200, 0x21, 0xFF, 0x0000, usb_devices[device_address].msd.interface);
}

void usb_msd_reset_sequence_device_reset_success(byte_t device_address) {
 //close transfer
 usb_close_control_transfer(device_address);

 l("\nUSB MSD: Reset IN endpoint");

 //reset IN endpoint
 usb_control_transfer_without_data(device_address, usb_msd_reset_sequence_reset_endpoint_in_success, usb_msd_reset_sequence_error, 200, 0x02, 0x01, 0x0000, usb_devices[device_address].msd.bulk_only.in.endpoint);
}

void usb_msd_reset_sequence_reset_endpoint_in_success(byte_t device_address) {
 //close transfer
 usb_close_control_transfer(device_address);

 //toggle of IN endpoint was resetted
 usb_devices[device_address].msd.bulk_only.in.endpoint_toggle = USB_TOGGLE_0;

 l("\nUSB MSD: Reset OUT endpoint");

 //reset OUT endpoint
 usb_control_transfer_without_data(device_address, usb_msd_reset_sequence_reset_endpoint_out_success, usb_msd_reset_sequence_error, 200, 0x02, 0x01, 0x0000, usb_devices[device_address].msd.bulk_only.out.endpoint);
}

void usb_msd_reset_sequence_reset_endpoint_out_success(byte_t device_address) {
 //close transfer
 usb_close_control_transfer(device_address);

 //toggle of OUT endpoint was resetted
 usb_devices[device_address].msd.bulk_only.out.endpoint_toggle = USB_TOGGLE_0;

 //if transfer called reset sequence, it mean that it ended in error
 if(usb_devices[device_address].msd.transfer_status == USB_TRANSFER_NOT_TRANSFERRED) {
  usb_devices[device_address].msd.transfer_status = USB_TRANSFER_ERROR;
 }

 //reset sequence was successfull
 l("\nUSB MSD: Reset Sequence was successfull");
 usb_devices[device_address].msd.reset_sequence_running = STATUS_FALSE;
}

void usb_msd_reset_sequence_error(byte_t device_address) {
 //log
 l("\nUSB MSD "); lvw(device_address); l("FATAL ERROR: Reset Sequence failed, disabling MSD interface");

 //close transfer
 usb_close_control_transfer(device_address);
 usb_devices[device_address].msd.reset_sequence_running = STATUS_FALSE;

 //if transfer called reset sequence, it mean that it ended in error
 if(usb_devices[device_address].msd.transfer_status == USB_TRANSFER_NOT_TRANSFERRED) {
  usb_devices[device_address].msd.transfer_status = USB_TRANSFER_ERROR;
 }

 //disable Mass Storage Device interface
 usb_devices[device_address].msd.is_present = STATUS_FALSE;
 remove_partitions_of_medium_from_list(MEDIUM_USB_MSD, device_address);
 usb_device_change_event = STATUS_TRUE;
}