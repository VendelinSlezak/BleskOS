//BleskOS

/*
* MIT License
* Copyright (c) 2023-2024 Vendelín Slezák
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

void usb_mass_storage_initalize(byte_t device_number) {
 //if there is no device attached, exit
 if(usb_mass_storage_devices[device_number].entry!=USB_MSD_ENTRY_DEVICE_ATTACHED) {
  return;
 }

 //as first thing we need to read Inquiry
 if(usb_msd_read_inquiry(device_number)==STATUS_ERROR) {
  //try again
  if(usb_msd_read_inquiry(device_number)==STATUS_ERROR) {
   log("\nUSB MSD: Unable to read Inquiry");
   return;
  }
 }

 //then we need to wait for device to be ready and then read capacity
 for(dword_t i=0; i<3; i++) {
  if(usb_msd_test_unit_ready(device_number)==STATUS_GOOD) {
   if(usb_msd_read_capacity(device_number)==STATUS_GOOD) {
    //log size of USB Mass Storage Drive
    log("\n Sector size: ");
    log_var(usb_mass_storage_devices[device_number].size_of_sector);
    log("\n Number of sectors: ");
    log_var(usb_mass_storage_devices[device_number].size_in_sectors);
    log("\n Size in MB: ");
    log_var((usb_mass_storage_devices[device_number].size_in_sectors/2000));

    //USB Mass Storage Drive was successfully initalized
    usb_mass_storage_devices[device_number].entry = USB_MSD_ENTRY_DEVICE_INITALIZED;
   }
   else {
    log("\nUSB MSD: Read Capacity (10) failed");
   }

   return;
  }
  else {
   //after Test Unit Ready reports that device is not ready, device expects that we will try to find out why by data from Request sense
   usb_msd_send_request_sense(device_number);
  }
 }

 log("\nUSB MSD: device is not ready");
}

void usb_msd_recover_from_error(byte_t device_number) {
 //reset device
 usb_control_transfer_without_data(usb_mass_storage_devices[device_number].controller_number, usb_mass_storage_devices[device_number].address, usb_mass_storage_devices[device_number].device_speed, 0x21, 0xFF, 0x0000, usb_mass_storage_devices[device_number].interface);
 
 //reset both endpoints
 usb_reset_endpoint(usb_mass_storage_devices[device_number].controller_number, usb_mass_storage_devices[device_number].address, usb_mass_storage_devices[device_number].device_speed, usb_mass_storage_devices[device_number].endpoint_in);
 usb_mass_storage_devices[device_number].toggle_in = 0;
 usb_reset_endpoint(usb_mass_storage_devices[device_number].controller_number, usb_mass_storage_devices[device_number].address, usb_mass_storage_devices[device_number].device_speed, usb_mass_storage_devices[device_number].endpoint_out);
 usb_mass_storage_devices[device_number].toggle_out = 0;
 
 log("\nUSB MSD: recover complete");
}

void usb_msd_prepare_cbw_for_command(byte_t command_length, dword_t transfer_length) {
 //clear CBW
 clear_memory((dword_t)usb_command_block_wrapper, sizeof(struct usb_command_block_wrapper_t));

 //set values
 usb_command_block_wrapper->signature = 0x43425355;
 usb_command_block_wrapper->transfer_id = 0x12345678; //this can be any value
 usb_command_block_wrapper->transfer_length = transfer_length;
 //usb_command_block_wrapper->lun = 0
 usb_command_block_wrapper->command_length = command_length;
}

byte_t usb_msd_transfer_command(byte_t device_number, byte_t direction, dword_t memory, dword_t length_of_transfer) {
 byte_t status;

 //set location of this device for EHCI transfers
 if(usb_mass_storage_devices[device_number].controller_type==USB_CONTROLLER_EHCI) {
  ehci_hub_address = usb_mass_storage_devices[device_number].ehci_hub_address;
  ehci_hub_port_number = usb_mass_storage_devices[device_number].ehci_hub_port_number;
 }

 //clear Command Status Wrapper
 clear_memory((dword_t)usb_command_status_wrapper, sizeof(struct usb_command_status_wrapper_t));

 //send Command Block Wrapper
 status = usb_bulk_out(usb_mass_storage_devices[device_number].controller_number, usb_mass_storage_devices[device_number].address, usb_mass_storage_devices[device_number].device_speed, usb_mass_storage_devices[device_number].endpoint_out, usb_mass_storage_devices[device_number].toggle_out, (dword_t)usb_command_block_wrapper, 31, 500);
 usb_mass_storage_devices[device_number].toggle_out = usb_bulk_toggle;
 if(status==STATUS_ERROR) {
  log("\nUSB MSD: CBW transfer error");

  usb_msd_recover_from_error(device_number);
  return STATUS_ERROR;
 }

 //transfer data of command
 if(length_of_transfer!=0) {
  if(direction==USB_CBW_DIRECTION_READ) {
   status = usb_bulk_in(usb_mass_storage_devices[device_number].controller_number, usb_mass_storage_devices[device_number].address, usb_mass_storage_devices[device_number].device_speed, usb_mass_storage_devices[device_number].endpoint_in, usb_mass_storage_devices[device_number].toggle_in, memory, length_of_transfer, 2000);
   usb_mass_storage_devices[device_number].toggle_in = usb_bulk_toggle;
   if(status==STATUS_ERROR) {
    log("\nUSB MSD: bulk in transfer error");

    usb_msd_recover_from_error(device_number);
    return STATUS_ERROR;
   }
  }
  else { //USB_CBW_DIRECTION_WRITE
   status = usb_bulk_out(usb_mass_storage_devices[device_number].controller_number, usb_mass_storage_devices[device_number].address, usb_mass_storage_devices[device_number].device_speed, usb_mass_storage_devices[device_number].endpoint_out, usb_mass_storage_devices[device_number].toggle_out, memory, length_of_transfer, 5000);
   usb_mass_storage_devices[device_number].toggle_out = usb_bulk_toggle;
   if(status==STATUS_ERROR) {
    log("\nUSB MSD: bulk out transfer error");

    usb_msd_recover_from_error(device_number);
    return STATUS_ERROR;
   }
  }
 }
 
 //read Command Status Wrapper
 status = usb_bulk_in(usb_mass_storage_devices[device_number].controller_number, usb_mass_storage_devices[device_number].address, usb_mass_storage_devices[device_number].device_speed, usb_mass_storage_devices[device_number].endpoint_in, usb_mass_storage_devices[device_number].toggle_in, (dword_t)usb_command_status_wrapper, 13, 500);
 usb_mass_storage_devices[device_number].toggle_in = usb_bulk_toggle;
 if(status==STATUS_ERROR) {
  log("\nUSB MSD: CSW transfer error");

  usb_msd_recover_from_error(device_number);
  return STATUS_ERROR;
 }
 
 //test if this command was successful
 if(usb_command_status_wrapper->signature==0x53425355 && usb_command_status_wrapper->transfer_id==0x12345678 && usb_command_status_wrapper->data_residue==0 && usb_command_status_wrapper->status==0x00) {
  return STATUS_GOOD;
 }
 else {
  return STATUS_ERROR;
 }
}

byte_t usb_msd_read_inquiry(byte_t device_number) {
 //create CBW with INQUIRY command
 usb_msd_prepare_cbw_for_command(6, 36);
 usb_command_block_wrapper->direction = USB_CBW_DIRECTION_READ;
 usb_command_block_wrapper->inquiry.command = 0x12;
 usb_command_block_wrapper->inquiry.big_endian_transfer_length = (36<<8);
 
 //transfer INQUIRY command
 struct usb_msd_inquiry_t inquiry_response;
 if(usb_msd_transfer_command(device_number, USB_CBW_DIRECTION_READ, (dword_t)(&inquiry_response), 36)==STATUS_GOOD) {
  log("\n Device type: ");
  if(inquiry_response.peripheral_device_type==0x0) {
   log("Direct Access Block Device");
  }
  else if(inquiry_response.peripheral_device_type==0x5) {
   log("CD-ROM/DVD");
  }
  else {
   log_hex_specific_size(inquiry_response.peripheral_device_type, 1);
  }

  byte_t string[9];
  string[8] = 0;
  copy_memory((dword_t)(&inquiry_response.vendor_id), (dword_t)(&string), 8);
  log("\n Vendor ID string: ");
  log(string);
  
  copy_memory((dword_t)(&inquiry_response.product_id), (dword_t)(&string), 8);
  log("\n Product ID string: ");
  log(string);
  
  return STATUS_GOOD;
 }
 else {
  return STATUS_ERROR;
 }
}

byte_t usb_msd_test_unit_ready(byte_t device_number) {
 //create CBW with TEST UNIT READY command
 usb_msd_prepare_cbw_for_command(6, 0);
 
 //return response
 return usb_msd_transfer_command(device_number, 0, 0, 0);
}

byte_t usb_msd_send_request_sense(byte_t device_number) {
 //create CBW with REQUEST SENSE command
 usb_msd_prepare_cbw_for_command(6, 18);
 usb_command_block_wrapper->direction = USB_CBW_DIRECTION_READ;
 usb_command_block_wrapper->request_sense.command = 0x03;
 usb_command_block_wrapper->request_sense.transfer_length = 18;
 
 //transfer REQUEST SENSE command
 struct usb_msd_request_sense_t request_sense_response;
 if(usb_msd_transfer_command(device_number, USB_CBW_DIRECTION_READ, (dword_t)(&request_sense_response), 18)==STATUS_GOOD) {
  log("\nUSB MSD: ");
  if(request_sense_response.sense_key==0x00 && request_sense_response.additional_sense_code==0x00 && request_sense_response.additional_sense_qualifier==0x00) {
   log("No Sense");
  }
  else if(request_sense_response.sense_key==0x02 && request_sense_response.additional_sense_code==0x3A && request_sense_response.additional_sense_qualifier==0x00) {
   log("Medium not present");
  }
  else if(request_sense_response.sense_key==0x06 && request_sense_response.additional_sense_code==0x28 && request_sense_response.additional_sense_qualifier==0x00) {
   log("Not ready to ready transition - Media changed");
  }
  else if(request_sense_response.sense_key==0x07 && request_sense_response.additional_sense_code==0x27 && request_sense_response.additional_sense_qualifier==0x00) {
   log("Write protected media");
  }
  else {
   log_hex_specific_size_with_space(request_sense_response.sense_key, 2);
   log_hex_specific_size_with_space(request_sense_response.additional_sense_code, 2);
   log_hex_specific_size(request_sense_response.additional_sense_qualifier, 2);
  }

  return STATUS_GOOD;
 }
 else {
  return STATUS_ERROR;
 }
}

byte_t usb_msd_read_capacity(byte_t device_number) {
 //create CBW with READ CAPACITY command
 usb_msd_prepare_cbw_for_command(10, 8);
 usb_command_block_wrapper->direction = USB_CBW_DIRECTION_READ;
 usb_command_block_wrapper->read_capacity.command = 0x25;

 //transfer READ CAPACITY command
 dword_t read_capacity_response[2];
 if(usb_msd_transfer_command(device_number, USB_CBW_DIRECTION_READ, (dword_t)(&read_capacity_response), 8)==STATUS_GOOD) {
  //this command is sended only during initalization of device, so we can parse and save values right here and do not need to return them to upper method
  usb_mass_storage_devices[device_number].size_in_sectors = reverse_endian_in_dword(read_capacity_response[0]);
  usb_mass_storage_devices[device_number].size_in_sectors--; //some devices report one more sector than real value
  usb_mass_storage_devices[device_number].size_of_sector = reverse_endian_in_dword(read_capacity_response[1]);

  return STATUS_GOOD;
 }
 else {
  return STATUS_ERROR;
 }
}

byte_t usb_msd_read(byte_t device_number, dword_t sector, byte_t number_of_sectors, dword_t memory) { 
 //create CBW with READ (10) command
 usb_msd_prepare_cbw_for_command(10, (number_of_sectors*usb_mass_storage_devices[device_number].size_of_sector));
 usb_command_block_wrapper->direction = USB_CBW_DIRECTION_READ;
 usb_command_block_wrapper->read_write_command.command = 0x28;
 usb_command_block_wrapper->read_write_command.big_endian_lba = reverse_endian_in_dword(sector);
 usb_command_block_wrapper->read_write_command.big_endian_number_of_sectors = reverse_endian_in_word(number_of_sectors);

 //read sectors
 return usb_msd_transfer_command(device_number, USB_CBW_DIRECTION_READ, memory, (number_of_sectors*usb_mass_storage_devices[device_number].size_of_sector));
}

byte_t usb_msd_write(byte_t device_number, dword_t sector, byte_t number_of_sectors, dword_t memory) {
 //create CBW with WRITE (10) command
 usb_msd_prepare_cbw_for_command(10, (number_of_sectors*usb_mass_storage_devices[device_number].size_of_sector));
 usb_command_block_wrapper->direction = USB_CBW_DIRECTION_WRITE;
 usb_command_block_wrapper->read_write_command.command = 0x2A;
 usb_command_block_wrapper->read_write_command.big_endian_lba = reverse_endian_in_dword(sector);
 usb_command_block_wrapper->read_write_command.big_endian_number_of_sectors = reverse_endian_in_word(number_of_sectors);

 //read sectors
 return usb_msd_transfer_command(device_number, USB_CBW_DIRECTION_WRITE, memory, (number_of_sectors*usb_mass_storage_devices[device_number].size_of_sector));
}