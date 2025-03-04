//BleskOS

/*
* MIT License
* Copyright (c) 2023-2025 BleskOS developers
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

void usb_bulk_only_prepare_transfer(struct usb_bbb_t *bulk_only,
                                    void (*command_successfull)(byte_t device_address),
                                    void (*command_error)(byte_t device_address),
                                    byte_t transfer_direction,
                                    dword_t command_length,
                                    dword_t transfer_length,
                                    void *buffer) {
 //set variables
 bulk_only->command_successfull = command_successfull;
 bulk_only->command_error = command_error;
 bulk_only->buffer = buffer;

 //clear Command Block Wrapper
 clear_memory((dword_t)&bulk_only->usb_command_block_wrapper, sizeof(struct usb_command_block_wrapper_t));

 //set signature
 bulk_only->usb_command_block_wrapper.signature = 0x43425355; //'USBC'

 //set transfer ID
 bulk_only->usb_command_block_wrapper.transfer_id = 0x12345678; //any value

 //set direction of transfer
 bulk_only->usb_command_block_wrapper.direction = transfer_direction;

 //LUN is zero
 //TODO: support more LUNs

 //set command length
 bulk_only->usb_command_block_wrapper.command_length = command_length;

 //set transfer length
 bulk_only->usb_command_block_wrapper.transfer_length = transfer_length;

 //clear Command Status Wrapper
 clear_memory((dword_t)&bulk_only->usb_command_status_wrapper, sizeof(struct usb_command_status_wrapper_t));
}

void usb_bulk_only_send_command(byte_t device_address, struct usb_bbb_t *bulk_only) {
 //check if there is already some transfer
 if(bulk_only->is_running == STATUS_TRUE) {
  l("\nUSB ERROR: Request for BBB during transfer");
  return;
 }

 //set this transfer
 bulk_only->is_running = STATUS_TRUE;

 //send Command Block Wrapper
 if(bulk_only->usb_command_block_wrapper.transfer_length == 0) {
  usb_bulk_transfer_out(device_address,
                       (struct usb_bulk_transfer_info_t *)&bulk_only->out,
                       usb_bulk_only_transfer_data_success, //skip transfer stage and go directly to CSW transfer
                       usb_bulk_only_transfer_data_error, //skip transfer stage and go directly to CSW transfer
                       (void *)bulk_only,
                       1000,
                       (byte_t *)&bulk_only->usb_command_block_wrapper,
                       sizeof(struct usb_command_block_wrapper_t));
 }
 else {
  usb_bulk_transfer_out(device_address,
                       (struct usb_bulk_transfer_info_t *)&bulk_only->out,
                       usb_bulk_only_send_cbw_success,
                       usb_bulk_only_send_cbw_error,
                       (void *)bulk_only,
                       1000,
                       (byte_t *)&bulk_only->usb_command_block_wrapper,
                       sizeof(struct usb_command_block_wrapper_t));
 }
}

void usb_bulk_only_send_cbw_success(byte_t device_address, void *transfer_info) {
 //set pointer
 struct usb_bbb_t *bulk_only = (struct usb_bbb_t *) transfer_info;

 //close transfer
 usb_bulk_only_close_transfer(device_address, bulk_only);

 //read or write data
 if(bulk_only->usb_command_block_wrapper.direction == USB_CBW_DIRECTION_READ) {
  usb_bulk_transfer_in(device_address,
                       (struct usb_bulk_transfer_info_t *)&bulk_only->in,
                       usb_bulk_only_transfer_data_success,
                       usb_bulk_only_transfer_data_error,
                       (void *)bulk_only,
                       4000,
                       (byte_t *)bulk_only->buffer,
                       bulk_only->usb_command_block_wrapper.transfer_length);
 }
 else { //USB_CBW_DIRECTION_WRITE
  usb_bulk_transfer_out(device_address,
                       (struct usb_bulk_transfer_info_t *)&bulk_only->out,
                       usb_bulk_only_transfer_data_success,
                       usb_bulk_only_transfer_data_error,
                       (void *)bulk_only,
                       10000,
                       (byte_t *)bulk_only->buffer,
                       bulk_only->usb_command_block_wrapper.transfer_length);
 }
}

void usb_bulk_only_transfer_data_success(byte_t device_address, void *transfer_info) {
 //set pointer
 struct usb_bbb_t *bulk_only = (struct usb_bbb_t *) transfer_info;

 //close transfer
 usb_bulk_only_close_transfer(device_address, bulk_only);

 //read Command Status Wrapper
 usb_bulk_transfer_in(device_address,
                       (struct usb_bulk_transfer_info_t *)&bulk_only->in,
                       usb_bulk_only_read_csw_success,
                       usb_bulk_only_read_csw_error,
                       (void *)bulk_only,
                       1000,
                       (byte_t *)&bulk_only->usb_command_status_wrapper,
                       sizeof(struct usb_command_status_wrapper_t));
}

void usb_bulk_only_read_csw_success(byte_t device_address, void *transfer_info) {
 //set pointer
 struct usb_bbb_t *bulk_only = (struct usb_bbb_t *) transfer_info;

 //close transfer
 usb_bulk_only_close_transfer(device_address, bulk_only);
 bulk_only->is_running = STATUS_FALSE;

 //check Command Status Wrapper to see if command was successfull
 struct usb_command_status_wrapper_t *csw = (struct usb_command_status_wrapper_t *)&bulk_only->usb_command_status_wrapper;
 if(csw->signature==0x53425355 && csw->transfer_id==0x12345678 && csw->data_residue==0 && csw->status==0x00) {
  bulk_only->command_successfull(device_address);
 }
 else {
  bulk_only->command_error(device_address);
 }
}

/* errors */

void usb_bulk_only_close_transfer(byte_t device_address, struct usb_bbb_t *bulk_only) {
 if(bulk_only->in.is_running) {
  usb_close_bulk_transfer(device_address, (struct usb_bulk_transfer_info_t *)&bulk_only->in);
 }
 if(bulk_only->out.is_running) {
  usb_close_bulk_transfer(device_address, (struct usb_bulk_transfer_info_t *)&bulk_only->out);
 }
}

void usb_bulk_only_send_cbw_error(byte_t device_address, void *transfer_info) {
 l("\nUSB BBB: Command Block Wrapper not transferred");

 //set pointer
 struct usb_bbb_t *bulk_only = (struct usb_bbb_t *) transfer_info;

 //close transfer
 usb_bulk_only_close_transfer(device_address, bulk_only);
 bulk_only->is_running = STATUS_FALSE;

 //call upper layer
 bulk_only->command_error(device_address);
}

void usb_bulk_only_transfer_data_error(byte_t device_address, void *transfer_info) {
 l("\nUSB BBB: Data were not transferred");

 //set pointer
 struct usb_bbb_t *bulk_only = (struct usb_bbb_t *) transfer_info;

 //close transfer
 usb_bulk_only_close_transfer(device_address, bulk_only);
 bulk_only->is_running = STATUS_FALSE;

 //call upper layer
 bulk_only->command_error(device_address);
}

void usb_bulk_only_read_csw_error(byte_t device_address, void *transfer_info) {
 l("\nUSB BBB: Command Status Wrapper not received");

 //set pointer
 struct usb_bbb_t *bulk_only = (struct usb_bbb_t *) transfer_info;

 //close transfer
 usb_bulk_only_close_transfer(device_address, bulk_only);
 bulk_only->is_running = STATUS_FALSE;

 //call upper layer
 bulk_only->command_error(device_address);
}