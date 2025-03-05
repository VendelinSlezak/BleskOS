//BleskOS

/*
* MIT License
* Copyright (c) 2023-2025 BleskOS developers
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

void usb_keyboard_save_informations(byte_t device_address, struct usb_full_interface_info_t interface) {
 //return means that there will be no information saved about USB keyboard on this device

 //check if this device do not already have keyboard interface
 if(usb_devices[device_address].keyboard.is_present == STATUS_TRUE) {
  logf("\nUSB ERROR: more keyboard interfaces at one device");
  return;
 }

 //check if this interface has all needed informations
 if(interface.interrupt_in_endpoint == 0) {
  logf("\nUSB ERROR: USB keyboard do not have interrupt in endpoint");
  return;
 }
 if(interface.interrupt_in_endpoint->bInterval == 0) {
  logf("\nUSB ERROR: USB keyboard invalid endpoint interval");
  return;
 }
 if(interface.interrupt_in_endpoint->wMaxPacketSize < 8) {
  logf("\nUSB ERROR: USB keyboard invalid endpoint size");
  return;
 }

 //save informations
 usb_devices[device_address].keyboard.is_present = STATUS_TRUE;
 usb_devices[device_address].keyboard.is_initalized = STATUS_FALSE;
 usb_devices[device_address].keyboard.initalize = usb_keyboard_initalize;

 usb_devices[device_address].keyboard.interface = interface.interface_descriptor->bInterfaceNumber;
 usb_devices[device_address].keyboard.alternative_interface = interface.interface_descriptor->bAlternateSetting;

 usb_devices[device_address].keyboard.interrupt_transfer.endpoint = interface.interrupt_in_endpoint->endpoint_number;
 usb_devices[device_address].keyboard.interrupt_transfer.endpoint_size = 8; //we use boot interface
 usb_devices[device_address].keyboard.interrupt_transfer.interval = interface.interrupt_in_endpoint->bInterval;

 //check if there is not interrupt endpoint for setting LEDs
 if(interface.interrupt_out_endpoint != 0) {
  usb_devices[device_address].keyboard.interrupt_led_transfer.endpoint = interface.interrupt_out_endpoint->endpoint_number;
  usb_devices[device_address].keyboard.interrupt_led_transfer.endpoint_size = 1;
  usb_devices[device_address].keyboard.interrupt_led_transfer.interval = interface.interrupt_out_endpoint->bInterval;
 }

 //log
 logf("\nUSB keyboard\n Endpoint: %d", usb_devices[device_address].keyboard.interrupt_transfer.endpoint);
 logf("\n Size: %d", usb_devices[device_address].keyboard.interrupt_transfer.endpoint_size);
 logf("\n Interval: %d", usb_devices[device_address].keyboard.interrupt_transfer.interval); logf("ms");
}

void usb_keyboard_initalize(byte_t device_address) {
 //check if we can use control endpoint
 if(usb_devices[device_address].control_transfer.is_running == STATUS_TRUE) {
  return;
 }

 //set device into initalization phase
 usb_devices[device_address].is_interface_in_initalization = STATUS_TRUE;

 //set use of HID protocol = 1
 usb_control_transfer_without_data(device_address, usb_keyboard_set_boot_protocol_success, usb_keyboard_set_boot_protocol_error, 100, 0x21, 0x0B, 1, usb_devices[device_address].keyboard.interface);
}

void usb_keyboard_set_boot_protocol_success(byte_t device_address) {
 //close transfer
 usb_close_control_transfer(device_address);

 //disable idle - not all devices supports this, but some expect it
 usb_control_transfer_without_data(device_address, usb_keyboard_set_idle_success, usb_keyboard_set_idle_error, 100, 0x21, 0x0A, 0, usb_devices[device_address].keyboard.interface);
}

void usb_keyboard_set_idle_success(byte_t device_address) {
 //close transfer
 usb_close_control_transfer(device_address);

 //finalize initalization
 usb_keyboard_initalization_success(device_address);
}

void usb_keyboard_initalization_success(byte_t device_address) {
//allocate memory for transfers
 usb_devices[device_address].keyboard.interrupt_transfer.transfer_buffer = (void *) calloc(usb_devices[device_address].keyboard.interrupt_transfer.endpoint_size);

 //start interrupt transfers
 usb_interrupt_transfer_in(device_address, (struct usb_interrupt_transfer_info_t *)&usb_devices[device_address].keyboard.interrupt_transfer, usb_keyboard_interrupt_transfer_successfull);

 //initalization is successfully done
 usb_devices[device_address].keyboard.is_initalized = STATUS_TRUE;
 usb_devices[device_address].is_interface_in_initalization = STATUS_FALSE;
}

void usb_keyboard_interrupt_transfer_successfull(byte_t device_address) {
 //parse key value
 struct usb_keyboard_buffer_t *keyboard_buffer = (struct usb_keyboard_buffer_t *) (usb_devices[device_address].keyboard.interrupt_transfer.transfer_buffer);

 //process pressed keys
 if(keyboard_buffer->keys[0] != 0) { //some key is pressed
  //update status if some control keys are pressed
  usb_devices[device_address].keyboard.keys.shift = STATUS_FALSE;
  usb_devices[device_address].keyboard.keys.ctrl = STATUS_FALSE;
  usb_devices[device_address].keyboard.keys.alt = STATUS_FALSE;
  if(keyboard_buffer->left_shift == 1 || keyboard_buffer->right_shift == 1) {
   usb_devices[device_address].keyboard.keys.shift = STATUS_TRUE;
  }
  if(keyboard_buffer->left_ctrl == 1 || keyboard_buffer->right_ctrl == 1) {
   usb_devices[device_address].keyboard.keys.ctrl = STATUS_TRUE;
  }
  if(keyboard_buffer->left_alt == 1 || keyboard_buffer->right_alt == 1) {
   usb_devices[device_address].keyboard.keys.alt = STATUS_TRUE;
  }
  keyboard_update_keys_state();

  //process newly pressed key
  for(dword_t i=0; i<6; i++) {
   if(usb_keyboard_layout[keyboard_buffer->keys[i]] != usb_devices[device_address].keyboard.keys.pressed_keys[i]) {
    if(usb_keyboard_layout[keyboard_buffer->keys[i]] == KEY_CAPSLOCK) {
     keyboard_led_state.capslock = ~keyboard_led_state.capslock; //reverse state
    }
    else {
     keyboard_process_code(usb_keyboard_layout[keyboard_buffer->keys[i]]); //process normal key

     //USB keyboard will not resent pressed key, so set time when key will be "pressed again", if user holds it
     usb_devices[device_address].keyboard.pressed_key_timeout = (time_of_system_running+500);
    }
   }
  }
 }
 else {
  //clear timeout
  usb_devices[device_address].keyboard.pressed_key_timeout = 0;
 }

 //copy all pressed keys
 for(dword_t i=0; i<6; i++) {
  usb_devices[device_address].keyboard.keys.pressed_keys[i] = usb_keyboard_layout[keyboard_buffer->keys[i]];
 }
}

void usb_keyboards_check_pressed_keys(void) {
 for(dword_t i=1; i<MAX_NUMBER_OF_USB_DEVICES; i++) {
  if(usb_devices[i].is_used == STATUS_TRUE && usb_devices[i].keyboard.is_present == STATUS_TRUE && usb_devices[i].keyboard.is_initalized == STATUS_TRUE) {
   //check if user is holding key
   if(usb_devices[i].keyboard.pressed_key_timeout != 0 && time_of_system_running >= usb_devices[i].keyboard.pressed_key_timeout) {
    //simulate pressing key again, because user is holding it
    keyboard_process_code(usb_devices[i].keyboard.keys.pressed_keys[0]);

    //move timeout to next pressing if user will hold it
    usb_devices[i].keyboard.pressed_key_timeout += 50;
   }
   
   //check led status
   if(usb_devices[i].keyboard.leds.capslock != keyboard_led_state.capslock
      || usb_devices[i].keyboard.leds.numlock != keyboard_led_state.numlock
      || usb_devices[i].keyboard.leds.scrollock != keyboard_led_state.scrollock) {
    //create packet data
    usb_devices[i].keyboard.led_buffer[0] = 0x00;
    if(keyboard_led_state.numlock == STATUS_TRUE) {
     usb_devices[i].keyboard.led_buffer[0] |= (1 << 0);
    }
    if(keyboard_led_state.capslock == STATUS_TRUE) {
     usb_devices[i].keyboard.led_buffer[0] |= (1 << 1);
    }
    if(keyboard_led_state.scrollock == STATUS_TRUE) {
     usb_devices[i].keyboard.led_buffer[0] |= (1 << 2);
    }

    //check if we will use interrupt endpoint or control endpoint
    if(usb_devices[i].keyboard.interrupt_led_transfer.endpoint != 0) {
     //TODO: check if this work on real hardware

     //interrupt endpoint
     if(usb_devices[i].keyboard.interrupt_led_transfer.is_running == STATUS_TRUE) {
      usb_close_interrupt_transfer(i, (struct usb_interrupt_transfer_info_t *)&usb_devices[i].keyboard.interrupt_led_transfer);
     }

     //start interrupt transfer
     usb_interrupt_transfer_out(i, (struct usb_interrupt_transfer_info_t *)&usb_devices[i].keyboard.interrupt_led_transfer, usb_devices[i].keyboard.led_buffer, usb_keyboard_led_interrupt_transfer_successfull);
    }
    else {
     //control endpoint
     if(usb_devices[i].control_transfer.is_running == STATUS_FALSE) {
      usb_control_transfer_out(i, usb_keyboard_set_led_success, usb_keyboard_set_led_error, 50, 0x21, 0x09, 0x0200, usb_devices[i].keyboard.interface, 1, usb_devices[i].keyboard.led_buffer);
     }
    }
   }

  }
 }
}

void usb_keyboard_update_led_status_with_transfer_data(byte_t device_address) {
 usb_devices[device_address].keyboard.leds.numlock = (usb_devices[device_address].keyboard.led_buffer[0] & 0x1);
 usb_devices[device_address].keyboard.leds.capslock = ((usb_devices[device_address].keyboard.led_buffer[0]>>1) & 0x1);
 usb_devices[device_address].keyboard.leds.scrollock = ((usb_devices[device_address].keyboard.led_buffer[0]>>2) & 0x1);
}

void usb_keyboard_set_led_success(byte_t device_address) {
 //close transfer
 usb_close_control_transfer(device_address);

 //update LED status
 usb_keyboard_update_led_status_with_transfer_data(device_address);
}

void usb_keyboard_led_interrupt_transfer_successfull(byte_t device_address) {
 //close interrupt transfer
 usb_devices[device_address].close_interrupt_transfer(device_address, (struct usb_interrupt_transfer_info_t *)&usb_devices[device_address].keyboard.interrupt_led_transfer);

 //update LED status
 usb_keyboard_update_led_status_with_transfer_data(device_address);
}

/* errors */

void usb_keyboard_stop_initalization(byte_t device_address, byte_t *err_string) {
 logf("\nUSB keyboard error: %s", err_string);
 usb_devices[device_address].keyboard.is_present = STATUS_FALSE;
 usb_devices[device_address].is_interface_in_initalization = STATUS_FALSE;
 usb_close_control_transfer(device_address);
}

void usb_keyboard_set_boot_protocol_error(byte_t device_address) {
 usb_keyboard_stop_initalization(device_address, "set BOOT protocol error");
}

void usb_keyboard_set_idle_error(byte_t device_address) {
 //some deviced do not support this feature, so it do not matter if it fails, we will finalize initalization
 logf("\nUSB keyboard: set idle not supported");
 
 //close transfer
 usb_close_control_transfer(device_address);

 //read first 8 bytes of HID descriptor
 usb_keyboard_interrupt_transfer_successfull(device_address);
}

void usb_keyboard_set_led_error(byte_t device_address) {
 logf("\nUSB keyboard: error with setting LEDs");

 //close transfer
 usb_close_control_transfer(device_address);

 //update LED status to prevent infinite tries
 usb_keyboard_update_led_status_with_transfer_data(device_address);
}