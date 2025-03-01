//BleskOS

/*
* MIT License
* Copyright (c) 2023-2025 Vendelín Slezák
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

void parse_usb_device_descriptor(dword_t descriptor_mem) {
 struct usb_device_descriptor_t *usb_device_descriptor = (struct usb_device_descriptor_t *) (descriptor_mem);
 
 log("\nUSB descriptor");
 log("\n USB version of this device: ");
 if(usb_device_descriptor->usb_specification_release_number==0x0100) {
  log("1.0");
 }
 else if(usb_device_descriptor->usb_specification_release_number==0x0110) {
  log("1.1");
 }
 else if(usb_device_descriptor->usb_specification_release_number==0x0200) {
  log("2.0");
 }
 else {
  log_hex_specific_size(usb_device_descriptor->usb_specification_release_number, 4);
 }
 log("\n Device type: ");
 if(usb_device_descriptor->device_class==0x00 && usb_device_descriptor->device_subclass==0x00 && usb_device_descriptor->device_protocol==0x00) {
  log("in configuration descriptor");
 }
 else {
  log_hex_specific_size_with_space(usb_device_descriptor->device_class, 2);
  log_hex_specific_size_with_space(usb_device_descriptor->device_subclass, 2);
  log_hex_specific_size(usb_device_descriptor->device_protocol, 2);
 }
 log("\n Control endpoint: ");
 dword_t usb_control_endpoint_size = usb_device_descriptor->size_of_control_endpoint;
 if(usb_control_endpoint_size==8) {
  log("low speed");
 }
 else if(usb_control_endpoint_size==64) {
  log("full speed");
 }
 else {
  log_var(usb_control_endpoint_size);
 }
 log("\n Vendor ID: ");
 log_hex(usb_device_descriptor->vendor_id);
 log("\n Product ID: ");
 log_hex(usb_device_descriptor->product_id);
 log("\n Device version: ");
 log_hex(usb_device_descriptor->device_release_number);
 log("\n Number of configurations: ");
 log_var(usb_device_descriptor->number_of_configurations);
}