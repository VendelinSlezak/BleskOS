//BleskOS

/*
* MIT License
* Copyright (c) 2023 Vendelín Slezák
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

void usb_mass_storage_initalize(byte_t device_number) {
 if(usb_mass_storage_devices[device_number].type!=USB_MSD_ATTACHED) {
  return;
 }

 if(usb_msd_send_inquiry_command(device_number)==STATUS_ERROR) {
  log("\nUSB msd: Unable to read Inquiry command");
  return;
 }
 if(usb_msd_send_capacity_command(device_number)==STATUS_ERROR) {
  usb_msd_recover_from_error(device_number);

  //send Request sense
  if(usb_msd_send_request_sense_command(device_number)==STATUS_ERROR) {
   log("\nUSB msd: Unable to get Request sense after first try to read capacity");
   return;
  }

  //try again
  if(usb_msd_send_capacity_command(device_number)==STATUS_ERROR) {
   usb_msd_recover_from_error(device_number);

   //send Request sense
   if(usb_msd_send_request_sense_command(device_number)==STATUS_ERROR) {
    log("\nUSB msd: Unable to get Request sense after second try to read capacity");
    return;
   }

   //try last time
   if(usb_msd_send_capacity_command(device_number)==STATUS_ERROR) {
    log("\nUSB msd: Unable to read capacity");
    return;
   }
  }
 }
 
 usb_mass_storage_devices[device_number].type = USB_MSD_INITALIZED;
}

void usb_msd_create_cbw(byte_t transfer_type, byte_t command_length, dword_t transfer_length) {
 dword_t *cbw32 = (dword_t *) usb_mass_storage_cbw_memory;
 
 cbw32[0] = 0x43425355; //signature
 cbw32[1] = 0x12345678; //transfer ID
 cbw32[2] = transfer_length;
 cbw32[3] = (transfer_type | (command_length<<16));
 for(int i=4; i<8; i++) {
  cbw32[i] = 0;
 }
}

byte_t usb_msd_transfer_command(byte_t device_number, byte_t direction, dword_t memory, dword_t length_of_transfer) {
 byte_t status=0;
 clear_memory(usb_mass_storage_csw_memory, 8);
 ehci_hub_address = usb_mass_storage_devices[device_number].ehci_hub_address;
 ehci_hub_port_number = usb_mass_storage_devices[device_number].ehci_hub_port_number;

 //send Command Block Wrapper
 status = usb_bulk_out(usb_mass_storage_devices[device_number].controller_number, usb_mass_storage_devices[device_number].address, usb_mass_storage_devices[device_number].device_speed, usb_mass_storage_devices[device_number].endpoint_out, usb_mass_storage_devices[device_number].toggle_out, usb_mass_storage_cbw_memory, 31, 500);
 if(status==STATUS_ERROR) {
  log("\nUSB msd: CBW transfer error");
  usb_mass_storage_devices[device_number].toggle_out = usb_bulk_toggle;
  return STATUS_ERROR;
 }
 usb_mass_storage_devices[device_number].toggle_out = usb_bulk_toggle;

 //transfer data of command
 if(direction==USB_CBW_READ) {
  status = usb_bulk_in(usb_mass_storage_devices[device_number].controller_number, usb_mass_storage_devices[device_number].address, usb_mass_storage_devices[device_number].device_speed, usb_mass_storage_devices[device_number].endpoint_in, usb_mass_storage_devices[device_number].toggle_in, memory, length_of_transfer, 2000);
  if(status==STATUS_ERROR) {
   log("\nUSB msd: bulk in transfer error");
   usb_mass_storage_devices[device_number].toggle_in = usb_bulk_toggle;
   return STATUS_ERROR;
  }
  usb_mass_storage_devices[device_number].toggle_in = usb_bulk_toggle;
 }
 else {
  status = usb_bulk_out(usb_mass_storage_devices[device_number].controller_number, usb_mass_storage_devices[device_number].address, usb_mass_storage_devices[device_number].device_speed, usb_mass_storage_devices[device_number].endpoint_out, usb_mass_storage_devices[device_number].toggle_out, memory, length_of_transfer, 2000);
  if(status==STATUS_ERROR) {
   log("\nUSB msd: bulk out transfer error");
   usb_mass_storage_devices[device_number].toggle_out = usb_bulk_toggle;
   return STATUS_ERROR;
  }
  usb_mass_storage_devices[device_number].toggle_out = usb_bulk_toggle;
 }
 
 //read Command Status Wrapper
 status = usb_bulk_in(usb_mass_storage_devices[device_number].controller_number, usb_mass_storage_devices[device_number].address, usb_mass_storage_devices[device_number].device_speed, usb_mass_storage_devices[device_number].endpoint_in, usb_mass_storage_devices[device_number].toggle_in, usb_mass_storage_csw_memory, 13, 500);
 if(status==STATUS_ERROR) {
  log("\nUSB msd: CSW transfer error");
  usb_mass_storage_devices[device_number].toggle_in = usb_bulk_toggle;
  return STATUS_ERROR;
 }
 usb_mass_storage_devices[device_number].toggle_in = usb_bulk_toggle;
 
 //test if this command was successful
 dword_t *csw = (dword_t *) (usb_mass_storage_csw_memory);
 if(csw[0]==0x53425355 && csw[1]==0x12345678) {
  return STATUS_GOOD;
 }
 else {
  log("\nUSB msd: wrong CSW ");
  log_var_with_space(length_of_transfer);
  log_hex_with_space(csw[0]);
  log_hex_with_space(csw[1]);
  return STATUS_ERROR;
 }
}

byte_t usb_msd_send_inquiry_command(byte_t device_number) {
 byte_t *cbw8 = (byte_t *) usb_mass_storage_cbw_memory;
 
 usb_msd_create_cbw(USB_CBW_READ, 6, 36);
 cbw8[15] = USB_MSD_INQUIRY; //INQUIRY command
 cbw8[19] = 36; //length of data to transfer
 
 return usb_msd_transfer_command(device_number, USB_CBW_READ, usb_mass_storage_response_memory, 36);
}

byte_t usb_msd_send_request_sense_command(byte_t device_number) {
 byte_t *cbw8 = (byte_t *) usb_mass_storage_cbw_memory;
 
 usb_msd_create_cbw(USB_CBW_READ, 6, 18);
 cbw8[15] = USB_MSD_REQUEST_SENSE; //REQUEST SENSE command
 cbw8[19] = 18; //length of data to transfer
 
 return usb_msd_transfer_command(device_number, USB_CBW_READ, usb_mass_storage_response_memory, 18);
}

byte_t usb_msd_send_capacity_command(byte_t device_number) {
 byte_t *cbw8 = (byte_t *) usb_mass_storage_cbw_memory;
 byte_t *data8 = (byte_t *) usb_mass_storage_response_memory;
 
 usb_msd_create_cbw(USB_CBW_READ, 10, 8);
 cbw8[15] = USB_MSD_CAPACITY; //CAPACITY command
 
 if(usb_msd_transfer_command(device_number, USB_CBW_READ, usb_mass_storage_response_memory, 8)==STATUS_ERROR) {
  return STATUS_ERROR;
 }
 
 usb_mass_storage_devices[device_number].size_in_sectors = ((data8[0]<<24) | (data8[1]<<16) | (data8[2]<<8) | (data8[3]));
 usb_mass_storage_devices[device_number].size_in_sectors--; //some devices report one more sector than real value
 usb_mass_storage_devices[device_number].size_of_sector = ((data8[4]<<24) | (data8[5]<<16) | (data8[6]<<8) | (data8[7]));
 
 log("\nsector size: ");
 log_var(usb_mass_storage_devices[device_number].size_of_sector);
 log("\nnumber of sectors: ");
 log_var(usb_mass_storage_devices[device_number].size_in_sectors);
 log("\nsize in MB: ");
 log_var((usb_mass_storage_devices[device_number].size_in_sectors/2000));
 
 return STATUS_GOOD;
}

byte_t usb_msd_read(byte_t device_number, dword_t sector, byte_t number_of_sectors, dword_t memory) {
 byte_t *cbw8 = (byte_t *) usb_mass_storage_cbw_memory;

 //create packet
 usb_msd_create_cbw(USB_CBW_READ, 10, (number_of_sectors*usb_mass_storage_devices[device_number].size_of_sector));
 cbw8[15] = USB_MSD_READ_10; //READ 10 command
 cbw8[17] = ((sector>>24) & 0xFF); //sector
 cbw8[18] = ((sector>>16) & 0xFF); //sector
 cbw8[19] = ((sector>>8) & 0xFF); //sector
 cbw8[20] = (sector & 0xFF); //sector
 cbw8[23] = number_of_sectors; //number of sectors
 
 //send packet
 if(usb_msd_transfer_command(device_number, USB_CBW_READ, memory, (number_of_sectors*usb_mass_storage_devices[device_number].size_of_sector))==STATUS_ERROR) {
  usb_msd_recover_from_error(device_number);

  //try again
  if(usb_msd_transfer_command(device_number, USB_CBW_READ, memory, (number_of_sectors*usb_mass_storage_devices[device_number].size_of_sector))==STATUS_ERROR) {
   usb_msd_recover_from_error(device_number);
   return STATUS_ERROR;
  }
 }
 
 return STATUS_GOOD;
}

byte_t usb_msd_write(byte_t device_number, dword_t sector, byte_t number_of_sectors, dword_t memory) {
 byte_t *cbw8 = (byte_t *) usb_mass_storage_cbw_memory;
 
 //create packet
 usb_msd_create_cbw(USB_CBW_WRITE, 10, (number_of_sectors*usb_mass_storage_devices[device_number].size_of_sector));
 cbw8[15] = USB_MSD_WRITE_10; //WRITE 10 command
 cbw8[17] = ((sector>>24) & 0xFF); //sector
 cbw8[18] = ((sector>>16) & 0xFF); //sector
 cbw8[19] = ((sector>>8) & 0xFF); //sector
 cbw8[20] = (sector & 0xFF); //sector
 cbw8[23] = number_of_sectors; //number of sectors
 
 //send packet
 if(usb_msd_transfer_command(device_number, USB_CBW_WRITE, memory, (number_of_sectors*usb_mass_storage_devices[device_number].size_of_sector))==STATUS_ERROR) {
  usb_msd_recover_from_error(device_number);

  //try again
  if(usb_msd_transfer_command(device_number, USB_CBW_WRITE, memory, (number_of_sectors*usb_mass_storage_devices[device_number].size_of_sector))==STATUS_ERROR) {
   usb_msd_recover_from_error(device_number);
   return STATUS_ERROR;
  }
 }
 
 return STATUS_GOOD;
}

void usb_msd_recover_from_error(byte_t device_number) {
 usb_control_transfer_without_data(usb_mass_storage_devices[device_number].controller_number, usb_mass_storage_devices[device_number].address, usb_mass_storage_devices[device_number].device_speed, 0x21, 0xFF, 0x0000, usb_mass_storage_devices[device_number].interface);
 usb_reset_endpoint(usb_mass_storage_devices[device_number].controller_number, usb_mass_storage_devices[device_number].address, usb_mass_storage_devices[device_number].device_speed, usb_mass_storage_devices[device_number].endpoint_in);
 usb_mass_storage_devices[device_number].toggle_in = 0;
 usb_reset_endpoint(usb_mass_storage_devices[device_number].controller_number, usb_mass_storage_devices[device_number].address, usb_mass_storage_devices[device_number].device_speed, usb_mass_storage_devices[device_number].endpoint_out);
 usb_mass_storage_devices[device_number].toggle_out = 0;
 log("\nUSB msd: recover complete");
}