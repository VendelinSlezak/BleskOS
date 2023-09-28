//BleskOS

/*
* MIT License
* Copyright (c) 2023 Vendelín Slezák
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

void wait_for_usb_keyboard(void) {
 byte_t *keyboard_data_mem = (byte_t *) usb_keyboard_data_memory;
 
 for(int i=0; i<8; i++) {
  keyboard_data_mem[i]=0;
 }

 usb_keyboard_wait = 1;
 ps2_keyboard_wait = 1;
 usb_new_device_detected = 0;
 ethernet_link_state_change = 0;
 keyboard_value = 0;
 keyboard_unicode = 0;
 
 while(usb_keyboard_wait==1 && ps2_keyboard_wait==1 && usb_new_device_detected==0 && ethernet_link_state_change==0) {
  asm("hlt");
  if(is_ethernet_cable_connected==STATUS_TRUE && connected_to_network==STATUS_FALSE) {
   connect_to_network_with_message();
   break;
  }
 }
 
 if(usb_new_device_detected==1) {
  detect_usb_devices();
 }
 
 if(ps2_keyboard_wait==0) {
  return;
 }
 else if(usb_keyboard_wait==0) {
  keyboard_process_code(usb_keyboard_value);
 }
}

void usb_keyboard_set_led(byte_t led_state) {
 if(usb_controllers[usb_keyboard_controller].type==USB_CONTROLLER_UHCI) {
  uhci_set_keyboard_led(usb_keyboard_controller, usb_keyboard_port, usb_keyboard_interface, usb_keyboard_endpoint_out, led_state);
 }
}