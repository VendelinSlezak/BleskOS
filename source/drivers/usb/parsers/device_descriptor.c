//BleskOS

/*
* MIT License
* Copyright (c) 2023-2025 BleskOS developers
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

void parse_usb_device_descriptor(dword_t descriptor_mem) {
 struct usb_device_descriptor_t *usb_device_descriptor = (struct usb_device_descriptor_t *) (descriptor_mem);
 
 logf("\nUSB descriptor");
 logf("\n USB version of this device: ");
 if(usb_device_descriptor->usb_specification_release_number==0x0100) {
  logf("1.0");
 }
 else if(usb_device_descriptor->usb_specification_release_number==0x0110) {
  logf("1.1");
 }
 else if(usb_device_descriptor->usb_specification_release_number==0x0200) {
  logf("2.0");
 }
 else {
  logf("%04x", usb_device_descriptor->usb_specification_release_number);
 }
 logf("\n Device type: ");
 if(usb_device_descriptor->device_class==0x00 && usb_device_descriptor->device_subclass==0x00 && usb_device_descriptor->device_protocol==0x00) {
  logf("in configuration descriptor");
 }
 else {
    logf("%02x %02x %02x", usb_device_descriptor->device_class, usb_device_descriptor->device_subclass, usb_device_descriptor->device_protocol);
 }
 logf("\n Control endpoint: ");
 dword_t usb_control_endpoint_size = usb_device_descriptor->size_of_control_endpoint;
 if(usb_control_endpoint_size==8) {
  logf("low speed");
 }
 else if(usb_control_endpoint_size==64) {
  logf("full speed");
 }
 else {
  logf("%d", usb_control_endpoint_size);
 }
 logf("\n Vendor ID: %04x", usb_device_descriptor->vendor_id);
 logf("\n Product ID: %04x", usb_device_descriptor->product_id);
 logf("\n Device version: %04x", usb_device_descriptor->device_release_number);
 logf("\n Number of configurations: %d", usb_device_descriptor->number_of_configurations);
}