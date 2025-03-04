//BleskOS

/*
* MIT License
* Copyright (c) 2023-2025 BleskOS developers
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

void initalize_ohci_controller(byte_t controller_number) {
 //log
 l("\n\nOHCI controller ");
 lv((mmio_ind(ohci_controllers[controller_number].base+0x00) >> 4) & 0xF);
 l(".");
 lv(mmio_ind(ohci_controllers[controller_number].base+0x00) & 0xF);

 //Host Controller Reset
 mmio_outd(ohci_controllers[controller_number].base+0x08, (1 << 0));
 wait(10);
 if((mmio_ind(ohci_controllers[controller_number].base+0x08) & (1 << 0)) != 0) {
  l("Host Controller Reset error");
  return;
 }

 //Global Reset
 mmio_outd(ohci_controllers[controller_number].base+0x04, 0x00);
 wait(50);
 mmio_outd(ohci_controllers[controller_number].base+0x04, (0b11 << 6)); //suspended state
 
 //set FM interval
 mmio_outd(ohci_controllers[controller_number].base+0x34, 0xA7782EDF);
 
 //set periodic start
 mmio_outd(ohci_controllers[controller_number].base+0x40, 0x00002A2F);
 
 //power all ports
 mmio_outd(ohci_controllers[controller_number].base+0x48, ((mmio_ind(ohci_controllers[controller_number].base+0x48) & 0xFFFFE000) | (1 << 9)));
 mmio_outd(ohci_controllers[controller_number].base+0x4C, 0);
 mmio_outd(ohci_controllers[controller_number].base+0x50, 0x10000); //set global power
 
 //read number of ports
 ohci_controllers[controller_number].number_of_ports = (mmio_ind(ohci_controllers[controller_number].base+0x48) & 0xF);
 
 //allocate memory for HCCA
 ohci_controllers[controller_number].hcca = (struct ohci_hcca_t *) aligned_calloc(256, 0xFF);
 mmio_outd(ohci_controllers[controller_number].base+0x18, (dword_t)ohci_controllers[controller_number].hcca);
 
 //set interrupt Endpoint Descriptors
 ohci_controllers[controller_number].interrupt_endpoint_descriptor = (struct ohci_endpoint_descriptor_t *) aligned_calloc(sizeof(struct ohci_endpoint_descriptor_t)*6, 0xF);
 for(dword_t i=0; i<6; i++) {
  ohci_controllers[controller_number].interrupt_endpoint_descriptor[i].next_ed_pointer = (dword_t)&ohci_controllers[controller_number].interrupt_endpoint_descriptor[i+1];
  ohci_controllers[controller_number].interrupt_endpoint_descriptor[i].skip = 1;
 }
 ohci_controllers[controller_number].interrupt_endpoint_descriptor[5].next_ed_pointer = 0;
 for(dword_t i = 0; i < OHCI_NUMBER_OF_POINTERS_IN_INTERRUPT_TABLE; i++) {
  if((i % 32) == 0) {
   ohci_controllers[controller_number].hcca->interrupt_table[i] = (dword_t)&ohci_controllers[controller_number].interrupt_endpoint_descriptor[0];
  }
  else if((i % 16) == 0) {
   ohci_controllers[controller_number].hcca->interrupt_table[i] = (dword_t)&ohci_controllers[controller_number].interrupt_endpoint_descriptor[1];
  }
  else if((i % 8) == 0) {
   ohci_controllers[controller_number].hcca->interrupt_table[i] = (dword_t)&ohci_controllers[controller_number].interrupt_endpoint_descriptor[2];
  }
  else if((i % 4) == 0) {
   ohci_controllers[controller_number].hcca->interrupt_table[i] = (dword_t)&ohci_controllers[controller_number].interrupt_endpoint_descriptor[3];
  }
  else if((i % 2) == 0) {
   ohci_controllers[controller_number].hcca->interrupt_table[i] = (dword_t)&ohci_controllers[controller_number].interrupt_endpoint_descriptor[4];
  }
  else {
   ohci_controllers[controller_number].hcca->interrupt_table[i] = (dword_t)&ohci_controllers[controller_number].interrupt_endpoint_descriptor[5];
  }
 }

 //set Control ED
 mmio_outd(ohci_controllers[controller_number].base+0x24, 0);
 mmio_outd(ohci_controllers[controller_number].base+0x20, 0);

 //set Bulk ED
 mmio_outd(ohci_controllers[controller_number].base+0x2C, 0);
 mmio_outd(ohci_controllers[controller_number].base+0x28, 0);

 //enable interrupts
 set_irq_handler(ohci_controllers[controller_number].irq, (dword_t)usb_irq);
 mmio_outd(ohci_controllers[controller_number].base+0x10, (1 << 1) | (1 << 31));

 //start controller
 mmio_outd(ohci_controllers[controller_number].base+0x04, (1 << 2) | (0b10 << 6));
 wait(50);
}

/* irq handler */

byte_t ohci_acknowledge_interrupt(dword_t number_of_controller) {
 //read interrupt status
 volatile dword_t irq_status = mmio_ind(ohci_controllers[number_of_controller].base+0x0C);

 //return if nothing happend
 if(irq_status == 0) {
  return STATUS_FALSE;
 }

 //check if there was Unrecoverable Error
 if((irq_status & (1 << 4)) == (1 << 4)) {
  l("\nOHCI: Serious Unrecoverable Error detected");
 }

 //clear interrupt status
 mmio_outd(ohci_controllers[number_of_controller].base+0x0C, irq_status);
 return STATUS_TRUE;
}

/* initalizing of ports */

byte_t ohci_check_port(dword_t number_of_controller, dword_t number_of_port) {
 //calculate register of port
 dword_t ohci_port = (ohci_controllers[number_of_controller].base+0x54+(number_of_port*4));
 dword_t ohci_port_value = mmio_ind(ohci_port);

 //is status change bit clear?
 if((ohci_port_value & (1 << 16))==0x0) {
  //check if this is zero address device that is not initalized
  if(usb_devices[0].is_used == STATUS_TRUE
     && usb_devices[0].controller_type == USB_OHCI
     && usb_devices[0].controller_number == number_of_controller
     && usb_devices[0].port_number == number_of_port
     && usb_devices[0].hub_address == 0
     && usb_devices[0].hub_port_number == 0
     && usb_devices[0].is_port_in_initalization == STATUS_FALSE) {
   return CONTROLLER_PORT_ZERO_ADDRESS_DEVICE;
  }
  //there was no change of device
  else {
   return CONTROLLER_PORT_NO_CHANGE;
  }
 }

 //clear status change bit
 mmio_outd(ohci_port, (1 << 16));

 //was device removed?
 if((ohci_port_value & (1 << 0))==0x0) {
  //device was removed from this port
  return CONTROLLER_PORT_REMOVED_DEVICE;
 }
 //else: status change + device = to this port was connected new device

 //check conditions that would prevent initalizing of new USB device - we need zero device to be free and not too much USB devices to be connected
 if(usb_devices[0].is_used == STATUS_TRUE || number_of_usb_devices >= MAX_NUMBER_OF_USB_DEVICES) {
  return CONTROLLER_PORT_NO_CHANGE; //we can not initalize two devices at same time, but condition on this port is not cleared, so when needed condition(s) changes, we will initalize this device
 }

 //clear zero device entry
 clear_memory((dword_t)(&usb_devices[0]), sizeof(struct usb_device_t));

 //save basic informations about this device
 usb_devices[0].is_used = STATUS_TRUE;
 usb_devices[0].controller_type = USB_OHCI;
 usb_devices[0].controller_number = number_of_controller;
 usb_devices[0].port_number = number_of_port;
 usb_devices[0].is_port_in_initalization = STATUS_TRUE;

 //start reset of port
 mmio_outd(ohci_port, (1 << 4));

 //wait 100 ms to stop reset
 create_task(ohci_enable_port, TASK_TYPE_PERIODIC_INTERRUPT, 50);

 return CONTROLLER_PORT_NO_CHANGE;
}

void ohci_enable_port(void) {
 //calculate register of port
 dword_t ohci_port = (ohci_controllers[usb_devices[0].controller_number].base+0x54+(usb_devices[0].port_number*4));

 //start enabling of port
 mmio_outd(ohci_port, (1 << 1));

 //remove this task
 destroy_task(ohci_enable_port);

 //wait until is port enabled or not with 50ms timeout
 usb_devices[0].timeout = (time_of_system_running+50);
 create_task(ohci_check_if_port_is_enabled, TASK_TYPE_PERIODIC_INTERRUPT, 1);
}

void ohci_check_if_port_is_enabled(void) {
 //calculate register of port
 dword_t ohci_port = (ohci_controllers[usb_devices[0].controller_number].base+0x54+(usb_devices[0].port_number*4));
 dword_t ohci_port_value = mmio_ind(ohci_port);

 //check if port is enabled
 if((ohci_port_value & (1 << 1))==(1 << 1)) {
  l("\nOHCI port is enabled in "); lvw(50-(usb_devices[0].timeout-time_of_system_running));
  lh(ohci_port_value);
  
  //when port is enabled, it mean, that device have address 0 and is ready for transfers, so fill all needed entries
  usb_devices[0].disable_device_on_port = ohci_disable_device_on_port;
  usb_devices[0].get_state_of_transfer = get_state_of_ohci_transfer;

  usb_devices[0].control_transfer_without_data = ohci_control_transfer_without_data;
  usb_devices[0].control_transfer_with_data = ohci_control_transfer_with_data;
  usb_devices[0].close_control_transfer = ohci_close_control_transfer;

  usb_devices[0].interrupt_transfer = ohci_interrupt_transfer;
  usb_devices[0].restart_interrupt_transfer = ohci_restart_interrupt_transfer;
  usb_devices[0].close_interrupt_transfer = ohci_close_interrupt_transfer;

  usb_devices[0].bulk_transfer = ohci_bulk_transfer;
  usb_devices[0].close_bulk_transfer = ohci_close_bulk_transfer;

  //get speed of connected device
  if((ohci_port_value & (1 << 9))==(1 << 9)) {
   usb_devices[0].device_speed = USB_LOW_SPEED;
  }
  else {
   usb_devices[0].device_speed = USB_FULL_SPEED;
  }

  //port is now initalized
  usb_devices[0].is_port_in_initalization = STATUS_FALSE;

  //remove this task
  destroy_task(ohci_check_if_port_is_enabled);
 }
 else if(time_of_system_running >= usb_devices[0].timeout) {
  //timeout error
  l("\nERROR: OHCI device was not enabled");

  //disable device
  ohci_disable_device_on_port(usb_devices[0].controller_number, usb_devices[0].port_number);

  //initalization of port ended, and device is not in zero address state
  usb_devices[0].is_used = STATUS_FALSE;

  //remove this task
  destroy_task(ohci_check_if_port_is_enabled);
 }
}

void ohci_disable_device_on_port(dword_t number_of_controller, dword_t number_of_port) {
 //calculate register of port
 dword_t ohci_port = (ohci_controllers[number_of_controller].base+0x54+(usb_devices[0].port_number*4));

 //disable device
 mmio_outd(ohci_port, (1 << 0));
}

/* transfers */

void ohci_set_ed(struct ohci_endpoint_descriptor_t *ed,
                 byte_t address,
                 byte_t endpoint,
                 word_t endpoint_size,
                 dword_t number_of_tds) {
 //clear Endpoint Descriptor
 clear_memory((dword_t)ed, sizeof(struct ohci_endpoint_descriptor_t));

 //set address
 ed->address = address;

 //set endpoint
 ed->endpoint = endpoint;
 ed->maximum_packet_length = endpoint_size;
 
 //set speed of transfer
 if(usb_devices[address].device_speed == USB_LOW_SPEED) {
  ed->low_speed_transfer = 1;
 }

 //set pointers to Transfer Descriptors
 ed->head_of_td = (((dword_t)ed + sizeof(struct ohci_endpoint_descriptor_t))>>4);
 ed->tail_of_td = ((dword_t)ed + sizeof(struct ohci_endpoint_descriptor_t) + (sizeof(struct ohci_transfer_descriptor_t)*number_of_tds));
}

void ohci_set_td(byte_t *tms,
                 dword_t td_number, dword_t next_td_number,
                 byte_t type,
                 byte_t interrupt_on_complete,
                 byte_t toggle,
                 byte_t *buffer,
                 word_t length_of_buffer) {
 struct ohci_transfer_descriptor_t *td = (struct ohci_transfer_descriptor_t *) ((dword_t)tms + sizeof(struct ohci_endpoint_descriptor_t) + (sizeof(struct ohci_transfer_descriptor_t)*td_number));
 clear_memory((dword_t)td, sizeof(struct ohci_transfer_descriptor_t));

 //clear fields
 td->reserved = 0;
 td->error_counter = 0;

 //short packets are OK
 td->buffer_rounding = 1;

 //set type of packet
 td->packet_identification = type;

 //set interrupt
 if(interrupt_on_complete == STATUS_FALSE) {
  td->delay_interrupt = 0b111;
 }
 else {
  td->delay_interrupt = 0b000;
 }

 //set toggle bit
 td->toggle = (0b10 | toggle);

 //set active status
 td->status = 0b1110;

 //set buffer
 if(length_of_buffer != 0) {
  td->current_buffer_pointer = (dword_t) buffer;
  td->buffer_end = ((dword_t)buffer + length_of_buffer - 1);
 }
 else {
  td->current_buffer_pointer = 0;
  td->buffer_end = 0;
 }
 
 //set pointer to next Transfer Descriptor
 td->next_td_pointer = ((dword_t)tms + sizeof(struct ohci_endpoint_descriptor_t) + (sizeof(struct ohci_transfer_descriptor_t)*(td_number+1)));
}

void ohci_insert_endpoint_descriptor(dword_t number_of_controller, dword_t type_of_transfer, struct ohci_endpoint_descriptor_t *ed) {
 cli();
 
 if(type_of_transfer == USB_CONTROL_TRANSFER) {
  ed->next_ed_pointer = mmio_ind(ohci_controllers[number_of_controller].base+0x20);
  mmio_outd(ohci_controllers[number_of_controller].base+0x04, mmio_ind(ohci_controllers[number_of_controller].base+0x04) & ~(1 << 4));
  mmio_outd(ohci_controllers[number_of_controller].base+0x20, (dword_t)ed);
  mmio_outd(ohci_controllers[number_of_controller].base+0x04, mmio_ind(ohci_controllers[number_of_controller].base+0x04) | (1 << 4));
  mmio_outd(ohci_controllers[number_of_controller].base+0x08, (1 << 1));
 }
 else if(type_of_transfer == USB_BULK_TRANSFER) {
  ed->next_ed_pointer = mmio_ind(ohci_controllers[number_of_controller].base+0x28);
  mmio_outd(ohci_controllers[number_of_controller].base+0x04, mmio_ind(ohci_controllers[number_of_controller].base+0x04) & ~(1 << 5));
  mmio_outd(ohci_controllers[number_of_controller].base+0x28, (dword_t)ed);
  mmio_outd(ohci_controllers[number_of_controller].base+0x04, mmio_ind(ohci_controllers[number_of_controller].base+0x04) | (1 << 5));
  mmio_outd(ohci_controllers[number_of_controller].base+0x08, (1 << 2));
 }

 sti();
}

void ohci_remove_endpoint_descriptor(dword_t number_of_controller, dword_t type_of_transfer, struct ohci_endpoint_descriptor_t *ed) {
 cli();

 //stop transfer
 dword_t first_endpoint_descriptor = 0;
 if(type_of_transfer == USB_CONTROL_TRANSFER) {
  mmio_outd(ohci_controllers[number_of_controller].base+0x04, mmio_ind(ohci_controllers[number_of_controller].base+0x04) & ~(1 << 4));
  first_endpoint_descriptor = mmio_ind(ohci_controllers[number_of_controller].base+0x20);
 }
 else if(type_of_transfer == USB_BULK_TRANSFER) {
  mmio_outd(ohci_controllers[number_of_controller].base+0x04, mmio_ind(ohci_controllers[number_of_controller].base+0x04) & ~(1 << 5));
  first_endpoint_descriptor = mmio_ind(ohci_controllers[number_of_controller].base+0x28);
 }

 //check if Endpoint Descriptor is first in list
 if(first_endpoint_descriptor == (dword_t)ed) {
  //check if there is only one Endpoint Descriptor in list
  if(ed->next_ed_pointer == 0) {
   //clear pointer
   if(type_of_transfer == USB_CONTROL_TRANSFER) {
    mmio_outd(ohci_controllers[number_of_controller].base+0x20, 0);
   }
   else if(type_of_transfer == USB_BULK_TRANSFER) {
    mmio_outd(ohci_controllers[number_of_controller].base+0x28, 0);
   }

   //list is already stopped, so there is nothing more to do
   sti();
   return;
  }
  else {
   //set pointer to next Endpoint Descriptor
   if(type_of_transfer == USB_CONTROL_TRANSFER) {
    mmio_outd(ohci_controllers[number_of_controller].base+0x20, ed->next_ed_pointer);
   }
   else if(type_of_transfer == USB_BULK_TRANSFER) {
    mmio_outd(ohci_controllers[number_of_controller].base+0x28, ed->next_ed_pointer);
   }
  }
 }
 else {
  //go through Endpoint Descriptors
  struct ohci_endpoint_descriptor_t *previous_ed = (struct ohci_endpoint_descriptor_t *) first_endpoint_descriptor;
  struct ohci_endpoint_descriptor_t *actual_ed = (struct ohci_endpoint_descriptor_t *) (previous_ed->next_ed_pointer);

  while(1) {
   //our Endpoint Descriptor is here
   if((dword_t)actual_ed == (dword_t)ed) {
    previous_ed->next_ed_pointer = ed->next_ed_pointer; //remove Endpoint Descriptor
    break;
   }
   //end of list
   else if(actual_ed->next_ed_pointer == 0) {
    break;
   }

   //go to next Endpoint Descriptor
   previous_ed = actual_ed;
   actual_ed = (struct ohci_endpoint_descriptor_t *) (actual_ed->next_ed_pointer);
  }
 }

 //start transfer
 if(type_of_transfer == USB_CONTROL_TRANSFER) {
  mmio_outd(ohci_controllers[number_of_controller].base+0x04, mmio_ind(ohci_controllers[number_of_controller].base+0x04) | (1 << 4));
 }
 else if(type_of_transfer == USB_BULK_TRANSFER) {
  mmio_outd(ohci_controllers[number_of_controller].base+0x04, mmio_ind(ohci_controllers[number_of_controller].base+0x04) | (1 << 5));
 }

 sti();
}

byte_t get_state_of_ohci_transfer(byte_t device_address, void *transfer_descriptor_pointer, dword_t number_of_tds) {
 //set pointer to first Transfer Descriptor
 struct ohci_transfer_descriptor_t *transfer_descriptor = (struct ohci_transfer_descriptor_t *) transfer_descriptor_pointer;

 //check state of all transfer descriptors
 for(dword_t i=0; i<number_of_tds; i++) {
  //check if last descriptor is successfully transferred
  if(transfer_descriptor[i].next_td_pointer == 0 && transfer_descriptor[i].status == 0b0000) {
   return USB_TRANSFER_DONE;
  }
  //check if there is error
  else if(transfer_descriptor[i].status != 0b1110 && transfer_descriptor[i].status != 0b0000) {
   //log error
   l("\nOHCI transfer error: ");
   if(transfer_descriptor[i].status == 0b0001) {
    l("CRC error");
   }
   else if(transfer_descriptor[i].status == 0b0010) {
    l("Bitstuff error");
   }
   else if(transfer_descriptor[i].status == 0b0011) {
    l("Data Toggle mismatch");
   }
   else if(transfer_descriptor[i].status == 0b0100) {
    l("Stalled");
   }
   else if(transfer_descriptor[i].status == 0b0101) {
    l("Device not responding");
   }
   else if(transfer_descriptor[i].status == 0b0110) {
    l("PID check failure");
   }
   else if(transfer_descriptor[i].status == 0b0111) {
    l("Unexpected PID");
   }
   else if(transfer_descriptor[i].status == 0b1000) {
    l("Data Overrun");
   }
   else if(transfer_descriptor[i].status == 0b1001) {
    l("Data Underrun");
   }
   else if(transfer_descriptor[i].status == 0b1100) {
    l("Buffer Overrun");
   }
   else if(transfer_descriptor[i].status == 0b1101) {
    l("Buffer Underrun");
   }
   else {
    lhs(transfer_descriptor[i].status, 2);
   }

   return USB_TRANSFER_ERROR;
  }
 }

 //last descriptor is not transferred and no error was detected
 return USB_TRANSFER_NOT_TRANSFERRED;
}

void ohci_control_transfer_without_data(byte_t device_address) {
 //allocate memory for Endpoint Descriptor + SETUP td + ACK td
 byte_t *tms = (byte_t *) (aligned_calloc(sizeof(struct ohci_endpoint_descriptor_t)+sizeof(struct ohci_transfer_descriptor_t)*(1+1), 0xF));

 //set SETUP transfer descriptor
 ohci_set_td(tms,
             0, 1, //TD on position 0, pointing to TD at position 1
             OHCI_PACKET_SETUP,
             STATUS_FALSE, //no interrupt
             USB_TOGGLE_0,
             (byte_t *)(&usb_devices[device_address].setup_buffer),
             8); //setup buffer is always 8 bytes long

 //set ACK transfer descriptor
 ohci_set_td(tms,
             1, 0, //TD on position 1, not pointing to anything else
             OHCI_PACKET_IN,
             STATUS_TRUE,  //interrupt when transferred
             USB_TOGGLE_1,
             0, //nothing is transferred
             0);

 //set Endpoint Descriptor
 ohci_set_ed((struct ohci_endpoint_descriptor_t *)tms,
             device_address,
             0, //endpoint 0
             usb_devices[device_address].control_transfer.endpoint_size,
             1+1); //two TDs

 //set transfer info, this will start checking of this transfer when IRQ will arrive
 usb_devices[device_address].control_transfer.is_running = STATUS_TRUE;
 usb_devices[device_address].control_transfer.tms_pointer = tms;
 usb_devices[device_address].control_transfer.first_td_pointer = (void *) ((dword_t)tms + sizeof(struct ohci_endpoint_descriptor_t));
 usb_devices[device_address].control_transfer.number_of_tds = (1+1);

 //start transfer
 ohci_insert_endpoint_descriptor(usb_devices[device_address].controller_number, USB_CONTROL_TRANSFER, (struct ohci_endpoint_descriptor_t *)tms);
}

void ohci_control_transfer_with_data(byte_t device_address, byte_t transfer_direction, word_t buffer_size) {
 //load data of device to variables
 dword_t control_endpoint_size = usb_devices[device_address].control_transfer.endpoint_size;
 byte_t *buffer = usb_devices[device_address].control_transfer.transfer_buffer;

 //calculate how many transfer descriptors do we need
 dword_t number_of_data_td = (buffer_size / control_endpoint_size);
 if((buffer_size % control_endpoint_size) != 0) {
  number_of_data_td++;
 }

 //allocate memory for Endpoint Descriptor + SETUP td + DATA tds (=number_of_data_td) + ACK td
 byte_t *tms = (byte_t *) (aligned_calloc(sizeof(struct ohci_endpoint_descriptor_t)+sizeof(struct ohci_transfer_descriptor_t)*(1+number_of_data_td+1), 0xF));

 //set SETUP transfer descriptor
 ohci_set_td(tms,
             0, 1, //TD on position 0, pointing to TD at position 1
             OHCI_PACKET_SETUP,
             STATUS_FALSE, //no interrupt
             USB_TOGGLE_0,
             (byte_t *)(&usb_devices[device_address].setup_buffer),
             8); //setup buffer is always 8 bytes long

 //set DATA transfer descriptors
 for(dword_t i = 1, toggle = 1, transfer_size = control_endpoint_size; i<(number_of_data_td+1); i++) {
  //set size of transfer descriptor, always control_endpoint_size except last transfer if it is smaller than control_endpoint_size
  if(buffer_size < control_endpoint_size) {
   transfer_size = buffer_size;
  }

  //set transfer descriptor
  if(transfer_direction == USB_TRANSFER_IN) {
   ohci_set_td(tms,
               i, (i+1), //TD on position i, pointing to next TD
               OHCI_PACKET_IN,
               STATUS_FALSE, //no interrupt
               toggle,
               buffer,
               transfer_size);
  }
  else { //USB_TRANSFER_OUT
   ohci_set_td(tms,
               i, (i+1), //TD on position i, pointing to next TD
               OHCI_PACKET_OUT,
               STATUS_FALSE, //no interrupt
               toggle,
               buffer,
               transfer_size);
  }

  //move buffer pointer
  buffer += control_endpoint_size;

  //change size of rest of transfer
  buffer_size -= transfer_size;

  //change toggle
  toggle = ((toggle + 0x1) & 0x1);
 }

 //set ACK transfer descriptor
 if(transfer_direction == USB_TRANSFER_IN) {
  ohci_set_td(tms,
              (number_of_data_td+1), 0, //last TD not pointing to anything else
              OHCI_PACKET_OUT,
              STATUS_TRUE,  //interrupt when transferred
              USB_TOGGLE_1,
              0, //nothing is transferred
              0);
 }
 else { //USB_TRANSFER_OUT
  ohci_set_td(tms,
              (number_of_data_td+1), 0, //last TD not pointing to anything else
              OHCI_PACKET_IN,
              STATUS_TRUE,  //interrupt when transferred
              USB_TOGGLE_1,
              0, //nothing is transferred
              0);
 }

 //set Endpoint Descriptor
 ohci_set_ed((struct ohci_endpoint_descriptor_t *)tms,
             device_address,
             0, //endpoint 0
             usb_devices[device_address].control_transfer.endpoint_size,
             1+number_of_data_td+1); //number of TDs

 //set transfer info, this will start checking of this transfer when IRQ will arrive
 usb_devices[device_address].control_transfer.is_running = STATUS_TRUE;
 usb_devices[device_address].control_transfer.tms_pointer = tms;
 usb_devices[device_address].control_transfer.first_td_pointer = (void *) ((dword_t)tms + sizeof(struct ohci_endpoint_descriptor_t));
 usb_devices[device_address].control_transfer.number_of_tds = (1+number_of_data_td+1);

 //start transfer
 ohci_insert_endpoint_descriptor(usb_devices[device_address].controller_number, USB_CONTROL_TRANSFER, (struct ohci_endpoint_descriptor_t *)tms);
}

void ohci_close_control_transfer(byte_t device_address) {
 //remove Endpoint Descriptor from transfer chain
 ohci_remove_endpoint_descriptor(usb_devices[device_address].controller_number, USB_CONTROL_TRANSFER, (struct ohci_endpoint_descriptor_t *)usb_devices[device_address].control_transfer.tms_pointer);
}

void ohci_interrupt_transfer(byte_t device_address, byte_t transfer_direction, struct usb_interrupt_transfer_info_t *interrupt_transfer) {
 //recalculate interval
 dword_t interval = interrupt_transfer->interval;
 if(interrupt_transfer->interval == 0) {
  l("\nOHCI ERROR: invalid interrupt transfer interval");
  return;
 }
 else if(interrupt_transfer->interval > 2) { //interval 1ms / 2ms do not need to be recalculated
  if(interrupt_transfer->interval == 3) {
   interval = 2;
  }
  else if(interrupt_transfer->interval < 8) {
   interval = 4;
  }
  else if(interrupt_transfer->interval < 16) {
   interval = 8;
  }
  else if(interrupt_transfer->interval < 32) {
   interval = 16;
  }
  else {
   interval = 32;
  }
 }

 //allocate memory for Endpoint Descriptor + INTERRUPT td
 byte_t *tms = (byte_t *) (aligned_calloc(sizeof(struct ohci_endpoint_descriptor_t)+sizeof(struct ohci_transfer_descriptor_t), 0xF));

 //set INTERRUPT transfer descriptor
 if(transfer_direction == USB_TRANSFER_IN) {
  ohci_set_td(tms,
              0, 0, //TD on position 0, not pointing to anything
              OHCI_PACKET_IN,
              STATUS_TRUE, //interrupt when transferred
              USB_TOGGLE_0,
              (byte_t *)(interrupt_transfer->transfer_buffer),
              interrupt_transfer->endpoint_size);
 }
 else { // USB_TRANSFER_OUT
  ohci_set_td(tms,
              0, 0, //TD on position 0, not pointing to anything
              OHCI_PACKET_OUT,
              STATUS_TRUE, //interrupt when transferred
              USB_TOGGLE_0,
              (byte_t *)(interrupt_transfer->transfer_buffer),
              interrupt_transfer->endpoint_size);
 }

 //set Endpoint Descriptor
 ohci_set_ed((struct ohci_endpoint_descriptor_t *)tms,
             device_address,
             interrupt_transfer->endpoint,
             interrupt_transfer->endpoint_size,
             1); //number of TDs
 
 //set transfer info, this will start checking of this transfer when IRQ will arrive
 interrupt_transfer->is_running = STATUS_TRUE;
 interrupt_transfer->tms_pointer = tms;
 interrupt_transfer->td_pointer = (void *) ((dword_t)tms + sizeof(struct ohci_endpoint_descriptor_t));

 //insert Endpoint Descriptor to HCCA
 struct ohci_endpoint_descriptor_t *ed = (struct ohci_endpoint_descriptor_t *) tms;
 cli();

 struct ohci_endpoint_descriptor_t *interrupt_endpoint_descriptor;
 if(interval == 32) {
  interrupt_endpoint_descriptor = &ohci_controllers[usb_devices[device_address].controller_number].interrupt_endpoint_descriptor[0];
 }
 else if(interval == 16) {
  interrupt_endpoint_descriptor = &ohci_controllers[usb_devices[device_address].controller_number].interrupt_endpoint_descriptor[1];
 }
 else if(interval == 8) {
  interrupt_endpoint_descriptor = &ohci_controllers[usb_devices[device_address].controller_number].interrupt_endpoint_descriptor[2];
 }
 else if(interval == 4) {
  interrupt_endpoint_descriptor = &ohci_controllers[usb_devices[device_address].controller_number].interrupt_endpoint_descriptor[3];
 }
 else if(interval == 2) {
  interrupt_endpoint_descriptor = &ohci_controllers[usb_devices[device_address].controller_number].interrupt_endpoint_descriptor[4];
 }
 else {
  interrupt_endpoint_descriptor = &ohci_controllers[usb_devices[device_address].controller_number].interrupt_endpoint_descriptor[5];
 }

 //insert our Queue Head
 ed->next_ed_pointer = interrupt_endpoint_descriptor->next_ed_pointer;
 interrupt_endpoint_descriptor->next_ed_pointer = (dword_t)ed;

 sti();
}

void ohci_restart_interrupt_transfer(byte_t device_address, struct usb_interrupt_transfer_info_t *interrupt_transfer) {
 struct ohci_endpoint_descriptor_t *ed = (struct ohci_endpoint_descriptor_t *) interrupt_transfer->tms_pointer;
 struct ohci_transfer_descriptor_t *td = (struct ohci_transfer_descriptor_t *) ((dword_t)interrupt_transfer->td_pointer);

 //set buffer
 td->current_buffer_pointer = (dword_t)interrupt_transfer->transfer_buffer;
 td->buffer_end = (td->current_buffer_pointer + interrupt_transfer->endpoint_size - 1);

 //set Transfer Descriptor pointer
 td->next_td_pointer = ((dword_t)td + sizeof(struct ohci_transfer_descriptor_t));

 //activate Transfer Descriptor
 td->status = 0b1110;

 //clear halt
 ed->halted = 0;

 //activate Endpoint Descriptor
 ed->head_of_td = ((dword_t)td >> 4);
}

void ohci_close_interrupt_transfer(byte_t device_address, struct usb_interrupt_transfer_info_t *interrupt_transfer) {
 //remove Endpoint Descriptor from Interrupt Table
 struct ohci_endpoint_descriptor_t *ed = (struct ohci_endpoint_descriptor_t *) interrupt_transfer->tms_pointer;
 ed->skip = 1;
 cli();

 //go through all Endpoint Descriptors to find our Endpoint Descriptor
 struct ohci_endpoint_descriptor_t *interrupt_endpoint_descriptor = &ohci_controllers[usb_devices[device_address].controller_number].interrupt_endpoint_descriptor[0];
 while(1) {
  //end of list, our Endpoint Descriptor was not inserted here
  if(interrupt_endpoint_descriptor->next_ed_pointer == 0) {
   break;
  }
  //our Endpoint Descriptor is inserted here
  else if(interrupt_endpoint_descriptor->next_ed_pointer == (dword_t)ed) {
   interrupt_endpoint_descriptor->next_ed_pointer = ed->next_ed_pointer;
   break;
  }

  //go to next Endpoint Descriptor
  interrupt_endpoint_descriptor = (struct ohci_endpoint_descriptor_t *) interrupt_endpoint_descriptor->next_ed_pointer;
 }

 sti();
}

void ohci_bulk_transfer(byte_t device_address, byte_t transfer_direction, struct usb_bulk_transfer_info_t *bulk_transfer) {
 //get size of bulk endpoint
 dword_t endpoint_size = bulk_transfer->endpoint_size;
 if(endpoint_size > 64) {
  endpoint_size = 64;
 }

 //calculate how many Tranfer Descriptors are needed for this transfer
 dword_t number_of_tds = (bulk_transfer->size_of_untransferred_data / endpoint_size);
 dword_t size_of_last_td = (bulk_transfer->size_of_untransferred_data % endpoint_size);
 if(size_of_last_td != 0) { //last TD will be shorter than others
  number_of_tds++;
 }
 else { //last TD is already in number_of_tds, because transfer is aligned to endpoint_size, so it will have equal size as all other TDs
  size_of_last_td = endpoint_size;
 }

 //transfer max 128 Transfer Descriptors at one Endpoint Descriptor, so if size of transfer is bigger, it will be divided to multiple transfers
 if(number_of_tds > 128) {
  number_of_tds = 128;
  size_of_last_td = endpoint_size; //original last TD will not be transferred, and actual last TD will have equal size as all other TDs
 }

 //allocate memory for Endpoint Descriptor + DATA tds
 byte_t *tms = (byte_t *) (aligned_calloc(sizeof(struct ohci_endpoint_descriptor_t)+sizeof(struct ohci_transfer_descriptor_t)*number_of_tds, 0xF));

 //set DATA transfer descriptors except for last one
 for(dword_t i=0; i<(number_of_tds-1); i++) {
  if(transfer_direction == USB_TRANSFER_IN) {
   ohci_set_td(tms,
               i, (i+1), //TD on position i, pointing to next TD
               OHCI_PACKET_IN,
               STATUS_FALSE, //no interrupt
               bulk_transfer->endpoint_toggle, //bulk endpoint toggle
               (byte_t *)(bulk_transfer->transfer_buffer_pointer),
               endpoint_size);
  }
  else { // USB_TRANSFER_OUT
   ohci_set_td(tms,
               i, (i+1), //TD on position i, pointing to next TD
               OHCI_PACKET_OUT,
               STATUS_FALSE, //no interrupt
               bulk_transfer->endpoint_toggle, //bulk endpoint toggle
               (byte_t *)(bulk_transfer->transfer_buffer_pointer),
               endpoint_size);
  }

  //change toggle
  bulk_transfer->endpoint_toggle = ((bulk_transfer->endpoint_toggle += 1) & 0x1);

  //move buffer pointer
  bulk_transfer->transfer_buffer_pointer += endpoint_size;
 }

 //set last DATA transfer descriptor
 if(transfer_direction == USB_TRANSFER_IN) {
  ohci_set_td(tms,
              (number_of_tds-1), 0, //TD on position i, not pointing to anything
              OHCI_PACKET_IN,
              STATUS_TRUE, //interrupt after transfer
              bulk_transfer->endpoint_toggle, //bulk endpoint toggle
              (byte_t *)(bulk_transfer->transfer_buffer_pointer),
              size_of_last_td);
 }
 else { // USB_TRANSFER_OUT
  ohci_set_td(tms,
              (number_of_tds-1), 0, //TD on position i, not pointing to anything
              OHCI_PACKET_OUT,
              STATUS_TRUE, //interrupt after transfer
              bulk_transfer->endpoint_toggle, //bulk endpoint toggle
              (byte_t *)(bulk_transfer->transfer_buffer_pointer),
              size_of_last_td);
 }

 //change toggle
 bulk_transfer->endpoint_toggle = ((bulk_transfer->endpoint_toggle += 1) & 0x1);

 //move buffer pointer
 bulk_transfer->transfer_buffer_pointer += endpoint_size;

 //change size of remaining untransferred data
 bulk_transfer->size_of_untransferred_data -= ((number_of_tds-1)*endpoint_size + size_of_last_td);

 //set Endpoint Descriptor
 ohci_set_ed((struct ohci_endpoint_descriptor_t *)tms,
             device_address,
             bulk_transfer->endpoint,
             bulk_transfer->endpoint_size,
             number_of_tds); //number of TDs
 
 //set transfer info, this will start checking of this transfer when IRQ will arrive
 bulk_transfer->is_running = STATUS_TRUE;
 bulk_transfer->tms_pointer = tms;
 bulk_transfer->td_pointer = (void *) ((dword_t)tms + sizeof(struct ohci_endpoint_descriptor_t));
 bulk_transfer->number_of_tds = number_of_tds;

 //insert Endpoint Descriptor to transfer chain
 ohci_insert_endpoint_descriptor(usb_devices[device_address].controller_number, USB_BULK_TRANSFER, (struct ohci_endpoint_descriptor_t *)tms);
}

void ohci_close_bulk_transfer(byte_t device_address, struct usb_bulk_transfer_info_t *bulk_transfer) {
 //remove Endpoint Descriptor from transfer chain
 ohci_remove_endpoint_descriptor(usb_devices[device_address].controller_number, USB_BULK_TRANSFER, (struct ohci_endpoint_descriptor_t *)bulk_transfer->tms_pointer);
}