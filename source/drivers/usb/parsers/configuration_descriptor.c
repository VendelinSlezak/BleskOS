//BleskOS

/*
* MIT License
* Copyright (c) 2023-2025 Vendelín Slezák
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

byte_t is_configuration_descriptor_valid(struct usb_configuration_descriptor_t *configuration_descriptor) {
 if(configuration_descriptor->bDescriptorType == USB_CONFIGURATION_DESCRIPTOR_TYPE && configuration_descriptor->bLength == sizeof(struct usb_configuration_descriptor_t)) {
  //log configuration
  l("\nUSB DEVICE CONFIGURATION DESCRIPTOR\nConfiguration ");
  lvw(configuration_descriptor->bConfigurationValue);
  l("Number of interfaces: "); lv(configuration_descriptor->bNumInterfaces);
  return sizeof(struct usb_configuration_descriptor_t);
 }
 else {
  return STATUS_FALSE;
 }
}

struct usb_full_interface_info_t configuration_descriptor_parse_interface(byte_t *configuration_descriptor_pointer, dword_t length_of_rest_of_descriptor) {
 struct usb_full_interface_info_t interface;
 clear_memory((dword_t)&interface, sizeof(struct usb_full_interface_info_t)); //interface.is_parsed = STATUS_FALSE;

 //find next interface descriptor
 while(1) {
  //set pointer to actual position
  struct usb_configuration_descriptor_header_t *descriptor_header = (struct usb_configuration_descriptor_header_t *) configuration_descriptor_pointer;

  //move to next descriptor
  if(descriptor_header->bLength > length_of_rest_of_descriptor) {
   l("\nERROR: descriptor length is above configuration descriptor");
   return interface;
  }
  else if(descriptor_header->bLength == 0) {
   l("\nERROR: descriptor zero length");
   return interface;
  }
  configuration_descriptor_pointer += descriptor_header->bLength;
  interface.length_to_skip += descriptor_header->bLength;
  length_of_rest_of_descriptor -= descriptor_header->bLength;
  if(length_of_rest_of_descriptor == 0) { //no interface descriptor was here
   return interface;
  }

  //check if that was interface descriptor
  if(descriptor_header->bDescriptorType == USB_INTERFACE_DESCRIPTOR_TYPE && descriptor_header->bLength == sizeof(struct usb_interface_descriptor_t)) {
   //save interface descriptor
   interface.interface_descriptor = (struct usb_interface_descriptor_t *) descriptor_header;

   //log interface descriptor
   l("\nInterface ");
   lvw(interface.interface_descriptor->bInterfaceNumber);
   lvw(interface.interface_descriptor->bAlternateSetting);
   lhsw(interface.interface_descriptor->bInterfaceClass, 2);
   lhsw(interface.interface_descriptor->bInterfaceSubClass, 2);
   lhsw(interface.interface_descriptor->bInterfaceProtocol, 2);
   break;
  }
 }

 //parse all descriptors under interface descriptor
 while(1) {
  //set pointer to actual position
  struct usb_configuration_descriptor_header_t *descriptor_header = (struct usb_configuration_descriptor_header_t *) configuration_descriptor_pointer;

  //check if this is endpoint descriptor
  if(descriptor_header->bDescriptorType == USB_ENDPOINT_DESCRIPTOR_TYPE && descriptor_header->bLength == sizeof(struct usb_endpoint_descriptor_t)) {
   struct usb_endpoint_descriptor_t *endpoint_descriptor = (struct usb_endpoint_descriptor_t *) configuration_descriptor_pointer;

   l("\n Endpoint ");

   //check if this is not invalid entry
   if(endpoint_descriptor->endpoint_number == 0) {
    l("error: 0");
   }
   else {
    //check direction of endpoint
    if(endpoint_descriptor->endpoint_direction == USB_ENDPOINT_DIRECTION_OUT) {
     l("OUT ");

     //check type of endpoint
     if(endpoint_descriptor->endpoint_type == USB_ENDPOINT_TYPE_ISOCHRONOUS) {
      l("isochronous ");
      if(interface.isochronous_out_endpoint == 0) {
       interface.isochronous_out_endpoint = endpoint_descriptor;
      }
      else {
       l("ERROR override ");
      }
     }
     else if(endpoint_descriptor->endpoint_type == USB_ENDPOINT_TYPE_BULK) {
      l("bulk ");
      if(interface.bulk_out_endpoint == 0) {
       interface.bulk_out_endpoint = endpoint_descriptor;
      }
      else {
       l("ERROR override ");
      }
     }
     else if(endpoint_descriptor->endpoint_type == USB_ENDPOINT_TYPE_INTERRUPT) {
      l("interrupt ");
      if(interface.interrupt_out_endpoint == 0) {
       interface.interrupt_out_endpoint = endpoint_descriptor;
      }
      else {
       l("ERROR override ");
      }
     }
     else {
      l("control ");
     }
    }
    else { //USB_ENDPOINT_DIRECTION_IN
     l("IN ");

     //check type of endpoint
     if(endpoint_descriptor->endpoint_type == USB_ENDPOINT_TYPE_ISOCHRONOUS) {
      l("isochronous ");
      if(interface.isochronous_in_endpoint == 0) {
       interface.isochronous_in_endpoint = endpoint_descriptor;
      }
      else {
       l("ERROR override ");
      }
     }
     else if(endpoint_descriptor->endpoint_type == USB_ENDPOINT_TYPE_BULK) {
      l("bulk ");
      if(interface.bulk_in_endpoint == 0) {
       interface.bulk_in_endpoint = endpoint_descriptor;
      }
      else {
       l("ERROR override ");
      }
     }
     else if(endpoint_descriptor->endpoint_type == USB_ENDPOINT_TYPE_INTERRUPT) {
      l("interrupt ");
      if(interface.interrupt_in_endpoint == 0) {
       interface.interrupt_in_endpoint = endpoint_descriptor;
      }
      else {
       l("ERROR override ");
      }
     }
     else {
      l("control ");
     }
    }

    //log number of endpoint
    lv(endpoint_descriptor->endpoint_number);
   }
  }
  //check if this is HID descriptor
  else if(descriptor_header->bDescriptorType == USB_HID_DESCRIPTOR_TYPE) {
   l("\n HID descriptor ");

   if(interface.hid_descriptor == 0) {
    interface.hid_descriptor = (struct usb_hid_descriptor_t *) descriptor_header;
   }
   else {
    l("ERROR override");
   }
  }
  //check if this is another interface descriptor
  else if(descriptor_header->bDescriptorType == USB_INTERFACE_DESCRIPTOR_TYPE) {
   interface.is_parsed = STATUS_TRUE;
   return interface; //we successfully parsed interface
  }
  else {
   l("\n Descriptor ");
   lhs(descriptor_header->bDescriptorType, 2);
  }

  //move to next descriptor
  if(descriptor_header->bLength > length_of_rest_of_descriptor) {
   l("\nERROR: descriptor in interface length is above configuration descriptor");
   return interface;
  }
  else if(descriptor_header->bLength == 0) {
   l("\nERROR: descriptor in interface zero length");
   return interface;
  }
  configuration_descriptor_pointer += descriptor_header->bLength;
  interface.length_to_skip += descriptor_header->bLength;
  length_of_rest_of_descriptor -= descriptor_header->bLength;
  if(length_of_rest_of_descriptor == 0) { //end of configuration, we successfully parsed interface
   interface.is_parsed = STATUS_TRUE;
   return interface;
  }
 }
}