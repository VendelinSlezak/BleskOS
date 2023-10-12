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
  return;
 }
 if(usb_msd_send_request_sense_command(device_number)==STATUS_ERROR) {
  return;
 }
 if(usb_msd_send_capacity_command(device_number)==STATUS_ERROR) {
  return;
 }
 
 usb_mass_storage_devices[device_number].type = USB_MSD_INITALIZED;
 
 //read info about partitions
 byte_t medium = storage_medium;
 byte_t medium_number = storage_medium_number;
 select_storage_medium(MEDIUM_USB_MSD, device_number);
 read_partition_info();
 for(int i=0; i<4; i++) {
  usb_mass_storage_devices[device_number].partitions_type[i]=partitions[i].type;
  usb_mass_storage_devices[device_number].partitions_first_sector[i]=partitions[i].first_sector;
  usb_mass_storage_devices[device_number].partitions_num_of_sectors[i]=partitions[i].num_of_sectors;
 }
 select_storage_medium(medium, medium_number);
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
 dword_t *csw = (dword_t *) usb_mass_storage_csw_memory;
 byte_t status=0;
 
 csw[0]=0;
 csw[1]=0;

 //send Command Block Wrapper
 status = usb_bulk_out(usb_mass_storage_devices[device_number].controller_number, usb_mass_storage_devices[device_number].port, usb_mass_storage_devices[device_number].address, usb_mass_storage_devices[device_number].endpoint_out, usb_mass_storage_devices[device_number].toggle_out, usb_mass_storage_cbw_memory, 31, 500);
 usb_mass_storage_devices[device_number].toggle_out = get_usb_bulk_toggle(usb_mass_storage_devices[device_number].controller_number);

 //transfer data of command
 if(direction==USB_CBW_READ) {
  status = usb_bulk_in(usb_mass_storage_devices[device_number].controller_number, usb_mass_storage_devices[device_number].port, usb_mass_storage_devices[device_number].address, usb_mass_storage_devices[device_number].endpoint_in, usb_mass_storage_devices[device_number].toggle_in, memory, length_of_transfer, 2000);
  usb_mass_storage_devices[device_number].toggle_in = get_usb_bulk_toggle(usb_mass_storage_devices[device_number].controller_number);
 }
 else {
  status = usb_bulk_out(usb_mass_storage_devices[device_number].controller_number, usb_mass_storage_devices[device_number].port, usb_mass_storage_devices[device_number].address, usb_mass_storage_devices[device_number].endpoint_out, usb_mass_storage_devices[device_number].toggle_out, memory, length_of_transfer, 2000);
  usb_mass_storage_devices[device_number].toggle_out = get_usb_bulk_toggle(usb_mass_storage_devices[device_number].controller_number);
 }
  
 //read Command Status Wrapper
 status = usb_bulk_in(usb_mass_storage_devices[device_number].controller_number, usb_mass_storage_devices[device_number].port, usb_mass_storage_devices[device_number].address, usb_mass_storage_devices[device_number].endpoint_in, usb_mass_storage_devices[device_number].toggle_in, usb_mass_storage_csw_memory, 13, 500);
 usb_mass_storage_devices[device_number].toggle_in = get_usb_bulk_toggle(usb_mass_storage_devices[device_number].controller_number);
 
 //test if this command was successful
  if(csw[0]==0x53425355 && csw[1]==0x12345678) {
   return STATUS_GOOD;
  }
  else {
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
 
 usb_msd_create_cbw(USB_CBW_READ, 10, (number_of_sectors*usb_mass_storage_devices[device_number].size_of_sector));
 cbw8[15] = USB_MSD_READ_10; //READ 10 command
 cbw8[17] = ((sector>>24) & 0xFF); //sector
 cbw8[18] = ((sector>>16) & 0xFF); //sector
 cbw8[19] = ((sector>>8) & 0xFF); //sector
 cbw8[20] = (sector & 0xFF); //sector
 cbw8[23] = number_of_sectors; //number of sectors
 
 if(usb_msd_transfer_command(device_number, USB_CBW_READ, memory, (number_of_sectors*usb_mass_storage_devices[device_number].size_of_sector))==STATUS_ERROR) {
  usb_msd_send_request_sense_command(device_number);
  return STATUS_ERROR;
 }
 
 return STATUS_GOOD;
}

byte_t usb_msd_write(byte_t device_number, dword_t sector, byte_t number_of_sectors, dword_t memory) {
 byte_t *cbw8 = (byte_t *) usb_mass_storage_cbw_memory;
 
 usb_msd_create_cbw(USB_CBW_WRITE, 10, (number_of_sectors*usb_mass_storage_devices[device_number].size_of_sector));
 cbw8[15] = USB_MSD_WRITE_10; //WRITE 10 command
 cbw8[17] = ((sector>>24) & 0xFF); //sector
 cbw8[18] = ((sector>>16) & 0xFF); //sector
 cbw8[19] = ((sector>>8) & 0xFF); //sector
 cbw8[20] = (sector & 0xFF); //sector
 cbw8[23] = number_of_sectors; //number of sectors
 
 if(usb_msd_transfer_command(device_number, USB_CBW_WRITE, memory, (number_of_sectors*usb_mass_storage_devices[device_number].size_of_sector))==STATUS_ERROR) {
  usb_msd_send_request_sense_command(device_number);
  return STATUS_ERROR;
 }
 
 return STATUS_GOOD;
}
