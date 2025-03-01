//BleskOS

/*
* MIT License
* Copyright (c) 2023-2025 Vendelín Slezák
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

/* initalization of EHCI controller at boot */

void initalize_ehci_controller(dword_t number_of_controller) {
 //log
 l("\n\nEHCI controller ");

 //disable BIOS ownership
 dword_t pci_ehci_bios_register_offset = ((mmio_ind(ehci_controllers[number_of_controller].base+0x08)>>8) & 0xFF);
 if(pci_ehci_bios_register_offset >= 0x40 && (pci_read(ehci_controllers[number_of_controller].bus, ehci_controllers[number_of_controller].device, ehci_controllers[number_of_controller].function, pci_ehci_bios_register_offset) & 0xFF)==0x01) {
  //check if BIOS released ownership
  if((pci_read(ehci_controllers[number_of_controller].bus, ehci_controllers[number_of_controller].device, ehci_controllers[number_of_controller].function, pci_ehci_bios_register_offset) & 0x01010000)!=0x01000000) {
   //BIOS did not released ownership
   log("\nERROR: EHCI controller is still in BIOS ownership ");
   log_hex(pci_read(ehci_controllers[number_of_controller].bus, ehci_controllers[number_of_controller].device, ehci_controllers[number_of_controller].function, pci_ehci_bios_register_offset));
   ehci_controllers[number_of_controller].number_of_ports = 0;
   return;
  }
  else {
   l("\nBIOS ownership released");
  }
 }

 //calculate base of operational registers
 ehci_controllers[number_of_controller].operational_registers_base = (ehci_controllers[number_of_controller].base + mmio_inb(ehci_controllers[number_of_controller].base+0x00));
 dword_t oper_base = ehci_controllers[number_of_controller].operational_registers_base;
 
 //read number of ports
 ehci_controllers[number_of_controller].number_of_ports = (mmio_ind(ehci_controllers[number_of_controller].base+0x04) & 0xF);

 //stop controller
 l("\n");
 mmio_outd(oper_base+0x00, (8 << 16));
 volatile dword_t timeout = (time_of_system_running+10);
 while((mmio_ind(oper_base+0x04) & (1 << 12)) != (1 << 12)) {
  asm("nop");

  if(time_of_system_running >= timeout) {
   l("unable to halt");
   return;
  }
 }
 
 //reset controller
 mmio_outd(oper_base+0x00, (1 << 1) | (8 << 16));
 timeout = (time_of_system_running+50);
 while((mmio_ind(oper_base+0x00) & (1 << 1))==(1 << 1)) {
  asm("nop");

  if(time_of_system_running >= timeout) {
   l("Host Controller Reset error");
   return;
  }
 }
 l("reset in "); lv(50-(timeout-time_of_system_running));
 
 //set upper 32 bits of segment
 mmio_outd(oper_base+0x10, 0x00000000);
 
 //set frame
 mmio_outd(oper_base+0x0C, 0);

 //create main Queue Heads
 ehci_controllers[number_of_controller].periodic_qh = (struct ehci_empty_queue_head_t *) aligned_calloc(sizeof(struct ehci_empty_queue_head_t)*6, 0x1F);
 for(dword_t i=0; i<6; i++) {
  ehci_controllers[number_of_controller].periodic_qh[i].horizontal_pointer = ((dword_t)&ehci_controllers[number_of_controller].periodic_qh[i+1] | EHCI_QH_POINTS_TO_QH);
  ehci_controllers[number_of_controller].periodic_qh[i].current_qtd_pointer = ((dword_t)&ehci_controllers[number_of_controller].periodic_qh[i] + 128);
  ehci_controllers[number_of_controller].periodic_qh[i].maximum_packet_length = 8;
  ehci_controllers[number_of_controller].periodic_qh[i].high_bandwidth_pipe_multiplier = 0b01;
  ehci_controllers[number_of_controller].periodic_qh[i].td.next_qtd_pointer = EHCI_INVALID_QTD_POINTER;
  ehci_controllers[number_of_controller].periodic_qh[i].td.alternate_next_qtd_pointer = EHCI_INVALID_QTD_POINTER;
 }
 ehci_controllers[number_of_controller].periodic_qh[5].horizontal_pointer = EHCI_INVALID_QH_POINTER;
 
 //periodic list
 ehci_controllers[number_of_controller].periodic_list = (dword_t *) aligned_malloc(EHCI_NUMBER_OF_POINTERS_IN_PERIODIC_LIST*4, 0xFFF);
 for(dword_t i=0; i<EHCI_NUMBER_OF_POINTERS_IN_PERIODIC_LIST; i++) {
  ehci_controllers[number_of_controller].periodic_list[i] = EHCI_INVALID_PERIODIC_POINTER;
 }
 for(dword_t i = 0; i < EHCI_NUMBER_OF_POINTERS_IN_PERIODIC_LIST; i++) {
  if((i % 32) == 0) {
   ehci_controllers[number_of_controller].periodic_list[i] = ((dword_t)&ehci_controllers[number_of_controller].periodic_qh[0] | EHCI_PERIODIC_POINTER_TO_QH);
  }
  else if((i % 16) == 0) {
   ehci_controllers[number_of_controller].periodic_list[i] = ((dword_t)&ehci_controllers[number_of_controller].periodic_qh[1] | EHCI_PERIODIC_POINTER_TO_QH);
  }
  else if((i % 8) == 0) {
   ehci_controllers[number_of_controller].periodic_list[i] = ((dword_t)&ehci_controllers[number_of_controller].periodic_qh[2] | EHCI_PERIODIC_POINTER_TO_QH);
  }
  else if((i % 4) == 0) {
   ehci_controllers[number_of_controller].periodic_list[i] = ((dword_t)&ehci_controllers[number_of_controller].periodic_qh[3] | EHCI_PERIODIC_POINTER_TO_QH);
  }
  else if((i % 2) == 0) {
   ehci_controllers[number_of_controller].periodic_list[i] = ((dword_t)&ehci_controllers[number_of_controller].periodic_qh[4] | EHCI_PERIODIC_POINTER_TO_QH);
  }
  else {
   ehci_controllers[number_of_controller].periodic_list[i] = ((dword_t)&ehci_controllers[number_of_controller].periodic_qh[5] | EHCI_PERIODIC_POINTER_TO_QH);
  }
 }
 mmio_outd(oper_base+0x14, (dword_t)ehci_controllers[number_of_controller].periodic_list);
 
 //create empty Queue Head for asynchronous list
 struct ehci_queue_head_t *empty_queue_head = (struct ehci_queue_head_t *) aligned_calloc(256, 0x1F);
 empty_queue_head->horizontal_pointer = ((dword_t)empty_queue_head | EHCI_QH_POINTS_TO_QH);
 empty_queue_head->head_of_reclamation_list = 1;
 empty_queue_head->maximum_packet_length = 8;
 empty_queue_head->high_bandwidth_pipe_multiplier = 0b01;
 empty_queue_head->current_qtd_pointer = ((dword_t)empty_queue_head)+128;
 empty_queue_head->td.next_qtd_pointer = EHCI_INVALID_QTD_POINTER;
 empty_queue_head->td.alternate_next_qtd_pointer = EHCI_INVALID_QTD_POINTER;
 ehci_controllers[number_of_controller].asychronous_start_qh = empty_queue_head;
 ehci_controllers[number_of_controller].asychronous_end_qh = empty_queue_head;
 mmio_outd(oper_base+0x18, (dword_t)empty_queue_head);
 
 //run controller with asychronous and periodic schedule
 mmio_outd(oper_base+0x00, (1 << 0) | (1 << 4) | (1 << 5) | (1 << 16));

 //enable interrupts for transfers
 set_irq_handler(ehci_controllers[number_of_controller].irq, (dword_t)usb_irq);
 mmio_outd(oper_base+0x08, 0x23);
 
 //set flag to indicate that we initalized controller
 mmio_outd(oper_base+0x40, 0x1);
 
 //power all ports
 for(int i=0, port_base=(oper_base+0x44); i<ehci_controllers[number_of_controller].number_of_ports; i++, port_base+=0x04) {
  mmio_outd(port_base, (1 << 12));
 }
 wait(50);
}

/* irq handler */

byte_t ehci_acknowledge_interrupt(dword_t number_of_controller) {
 //read interrupt status
 volatile dword_t irq_status = mmio_ind(ehci_controllers[number_of_controller].operational_registers_base+0x04);

 //return if nothing happend
 if((irq_status & 0x3F) == 0) {
  return STATUS_FALSE;
 }

 //clear interrupt status
 mmio_outd(ehci_controllers[number_of_controller].operational_registers_base+0x04, irq_status);
 return STATUS_TRUE;
}

/* initalizing of ports */

byte_t ehci_check_port(dword_t number_of_controller, dword_t number_of_port) {
 //calculate register of port
 dword_t ehci_port = (ehci_controllers[number_of_controller].operational_registers_base+0x44+(number_of_port*4));
 dword_t ehci_port_value = mmio_ind(ehci_port);

 //is status change bit clear?
 if((ehci_port_value & (1 << 1))==0x0) {
  //check if this is zero address device that in not so far initalized
  if(usb_devices[0].is_used == STATUS_TRUE
     && usb_devices[0].controller_type == USB_EHCI
     && usb_devices[0].controller_number == number_of_controller
     && usb_devices[0].port_number == number_of_port
     && usb_devices[0].hub_address == 0
     && usb_devices[0].hub_port_number == 0
     && usb_devices[0].is_port_in_initalization == STATUS_FALSE) {
   return CONTROLLER_PORT_ZERO_ADDRESS_DEVICE;
  }

  return CONTROLLER_PORT_NO_CHANGE; //no, there was no change of device
 }

 //was device removed?
 if((ehci_port_value & (1 << 0))==0x0) {
  //clear status change bit
  mmio_outd(ehci_port, (1 << 1) | (1 << 12));

  //device was removed from this port
  return CONTROLLER_PORT_REMOVED_DEVICE;
 }

 //check conditions that would prevent initalizing of new USB device - we need zero device to be free and not too much USB devices to be connected
 if(usb_devices[0].is_used == STATUS_TRUE || number_of_usb_devices >= MAX_NUMBER_OF_USB_DEVICES) {
  return CONTROLLER_PORT_NO_CHANGE; //we can not initalize two devices at same time, but condition on this port is not cleared, so when needed condition(s) changes, we will initalize this device
 }

 //clear zero device entry
 clear_memory((dword_t)(&usb_devices[0]), sizeof(struct usb_device_t));

 //there was connected new device, so start initalization of this port
 usb_devices[0].is_used = STATUS_TRUE;
 usb_devices[0].controller_type = USB_EHCI;
 usb_devices[0].controller_number = number_of_controller;
 usb_devices[0].port_number = number_of_port;
 usb_devices[0].is_port_in_initalization = STATUS_TRUE;

 //clear status change - this can not be done in same write as start of reset
 mmio_outd(ehci_port, (1 << 1) | (1 << 12));

 //start reset of port
 mmio_outd(ehci_port, (1 << 8) | (1 << 12));
 create_task(ehci_stop_port_reset, TASK_TYPE_PERIODIC_INTERRUPT, 50);

 return CONTROLLER_PORT_NO_CHANGE;
}

void ehci_stop_port_reset(void) {
 //calculate register of port
 dword_t ehci_port = (ehci_controllers[usb_devices[0].controller_number].operational_registers_base+0x44+(usb_devices[0].port_number*4));

 //stop reset of port and clear status change
 mmio_outd(ehci_port, (1 << 1) | (1 << 12));

 //remove this task
 destroy_task(ehci_stop_port_reset);

 //wait 30 ms for port to enable
 create_task(ehci_check_if_port_is_enabled, TASK_TYPE_PERIODIC_INTERRUPT, 30);
}

void ehci_check_if_port_is_enabled(void) {
 //calculate register of port
 dword_t ehci_port = (ehci_controllers[usb_devices[0].controller_number].operational_registers_base+0x44+(usb_devices[0].port_number*4));
 dword_t ehci_port_value = mmio_ind(ehci_port);

 //check if port is enabled
 if((ehci_port_value & (1 << 2))==(1 << 2)) {
  l("\nEHCI port is enabled");
  
  //when port is enabled, it mean, that device is ready for transfers and have address 0, so fill all needed entries
  usb_devices[0].disable_device_on_port = ehci_disable_device_on_port;
  usb_devices[0].get_state_of_transfer = get_state_of_ehci_transfer;

  usb_devices[0].control_transfer_without_data = ehci_control_transfer_without_data;
  usb_devices[0].control_transfer_with_data = ehci_control_transfer_with_data;
  usb_devices[0].close_control_transfer = ehci_close_control_transfer;

  usb_devices[0].interrupt_transfer = ehci_interrupt_transfer;
  usb_devices[0].restart_interrupt_transfer = ehci_restart_interrupt_transfer;
  usb_devices[0].close_interrupt_transfer = ehci_close_interrupt_transfer;

  usb_devices[0].bulk_transfer = ehci_bulk_transfer;
  usb_devices[0].close_bulk_transfer = ehci_close_bulk_transfer;

  //set speed of connected device
  usb_devices[0].device_speed = USB_HIGH_SPEED;

  //port is now initalized
  usb_devices[0].is_port_in_initalization = STATUS_FALSE;
 }
 else {
  //pass low/full speed device to companion controller
  mmio_outd(ehci_port, (1 << 12) | (1 << 13));

  //wait 100 ms for port to pass device
  create_task(ehci_check_if_port_passed_device, TASK_TYPE_PERIODIC_INTERRUPT, 100);
 }

 //remove this task
 destroy_task(ehci_check_if_port_is_enabled);
}

void ehci_check_if_port_passed_device(void) {
 //calculate register of port
 dword_t ehci_port = (ehci_controllers[usb_devices[0].controller_number].operational_registers_base+0x44+(usb_devices[0].port_number*4));
 dword_t ehci_port_value = mmio_ind(ehci_port);

 //check device status
 if((ehci_port_value & (1 << 0))==(1 << 0)) {
  l("\nEHCI ERROR: device was not passed to companion controller ");
  lh(ehci_port_value);
 }
 else {
  //clear status change
  mmio_outd(ehci_port, (1 << 1) | (1 << 12) | (1 << 13));
 }

 //initalization of this port ended
 usb_devices[0].is_used = STATUS_FALSE;

 //remove this task
 destroy_task(ehci_check_if_port_passed_device);
}

void ehci_disable_device_on_port(dword_t number_of_controller, dword_t number_of_port) {
 //calculate register of port
 dword_t ehci_port = (ehci_controllers[usb_devices[0].controller_number].operational_registers_base+0x44+(usb_devices[0].port_number*4));

 //disable device
 mmio_outd(ehci_port, mmio_ind(ehci_port) & ~(1 << 2));
}

/* transfers */

void ehci_set_qh(struct ehci_queue_head_t *queue_head,
                 dword_t type_of_queue_head,
                 byte_t address,
                 byte_t endpoint,
                 word_t endpoint_size) {
 //clear queue head
 clear_memory((dword_t)queue_head, sizeof(struct ehci_queue_head_t));

 //set device address
 queue_head->address = address;

 //set endpoint
 queue_head->endpoint = endpoint;

 //set speed of transfer
 if(usb_devices[address].device_speed == USB_HIGH_SPEED) {
  queue_head->endpoint_speed = 0b10;
 }
 else if(usb_devices[address].device_speed == USB_LOW_SPEED) {
  queue_head->endpoint_speed = 0b01;
 }
 //else speed is USB_FULL_SPEED what is 0b00

 //take toggle bit from transfer descriptors
 queue_head->toggle_control = 1;

 //set size of endpoint
 queue_head->maximum_packet_length = endpoint_size;

 //set fields for non-high speed devices
 if(usb_devices[address].device_speed != USB_HIGH_SPEED) {
  //set hub address
  queue_head->hub_address = usb_devices[address].hub_address;

  //set hub port number
  queue_head->hub_port_number = usb_devices[address].hub_port_number;

  //control transfers
  if(endpoint == 0) {
   queue_head->low_full_speed_control_endpoint_flag = 1;
   queue_head->split_completion_mask = 0xFF;
  }
 }

 //set interrupt queue head
 if(type_of_queue_head == EHCI_INTERRUPT_QH) {
  queue_head->horizontal_pointer = EHCI_INVALID_QH_POINTER;
  
  //set interrupt schedule mask
  queue_head->interrupt_schedule_mask = 0x01;

  //set split completion for non-high speed devices
  if(usb_devices[address].device_speed != USB_HIGH_SPEED) {
   queue_head->split_completion_mask = 0x1C; //value from specification on Normal Case
  }

  //set pointer to Transfer Descriptor
  queue_head->td.next_qtd_pointer = ((dword_t)queue_head+128);
 }

 //set multiplier value
 queue_head->high_bandwidth_pipe_multiplier = 0b01;

 //set pointer to first Transfer Descriptor
 queue_head->current_qtd_pointer = ((dword_t)queue_head+128);

 //copy first Transfer Descriptor - it is always 128 bytes after Queue Head
 if(type_of_queue_head == EHCI_ASYCHRONOUS_QH) {
  copy_memory((dword_t)queue_head+128, (dword_t)&queue_head->td, sizeof(struct ehci_queue_transfer_descriptor_t));
 }
}

void ehci_set_td(byte_t *start_of_transfer_structures_memory,
                 dword_t td_number, dword_t next_td_number,
                 byte_t type,
                 byte_t toggle,
                 word_t length_of_transfer,
                 byte_t *buffer,
                 byte_t interrupt_on_complete) {
 //set pointer to first transfer descriptor
 struct ehci_queue_transfer_descriptor_t *transfer_descriptor = (struct ehci_queue_transfer_descriptor_t *) ((dword_t)start_of_transfer_structures_memory + 128 + (td_number*64));

 //clear transfer descriptor
 clear_memory((dword_t)transfer_descriptor, sizeof(struct ehci_queue_transfer_descriptor_t));

 //set link pointer
 if(next_td_number == 0) { //last transfer descriptor that do not point to anything else
  transfer_descriptor->next_qtd_pointer = EHCI_INVALID_QTD_POINTER;
  transfer_descriptor->alternate_next_qtd_pointer = EHCI_INVALID_QTD_POINTER;
 }
 else { //set pointer to given transfer descriptor
  transfer_descriptor->next_qtd_pointer = ((dword_t)start_of_transfer_structures_memory + 128 + (next_td_number*64));
  transfer_descriptor->alternate_next_qtd_pointer = ((dword_t)start_of_transfer_structures_memory + 128 + (next_td_number*64));
 }

 //set active status, so EHCI will transfer this descriptor
 transfer_descriptor->status_bits.active = 1;

 //set type of packet
 transfer_descriptor->packet_identification = type;

 //set error counter
 transfer_descriptor->error_counter = 0b11;

 //set if interrupt will be fired after completion of this transfer descriptor
 transfer_descriptor->interrupt_on_complete = interrupt_on_complete;

 //set length of transfer
 transfer_descriptor->total_bytes_to_transfer = length_of_transfer;

 //set toggle bit
 transfer_descriptor->toggle_bit = toggle; 

 //set buffer pointers
 transfer_descriptor->lower_buffer_pointer_0 = ((dword_t)buffer);
 transfer_descriptor->lower_buffer_pointer_1 = (((dword_t)buffer & 0xFFFFF000)+0x1000);
 transfer_descriptor->lower_buffer_pointer_2 = (transfer_descriptor->lower_buffer_pointer_1+0x1000);
 transfer_descriptor->lower_buffer_pointer_3 = (transfer_descriptor->lower_buffer_pointer_1+0x2000);
 transfer_descriptor->lower_buffer_pointer_4 = (transfer_descriptor->lower_buffer_pointer_1+0x3000);
}

void ehci_insert_asychronous_queue_head(dword_t number_of_controller, struct ehci_queue_head_t *queue_head) {
 cli();

 //insert Queue Head to end of list
 queue_head->horizontal_pointer = ((dword_t)ehci_controllers[number_of_controller].asychronous_start_qh | EHCI_QH_POINTS_TO_QH); //Queue Head at end needs to point to start of list
 ehci_controllers[number_of_controller].asychronous_end_qh->horizontal_pointer = ((dword_t)queue_head | EHCI_QH_POINTS_TO_QH); //insert Queue Head
 ehci_controllers[number_of_controller].asychronous_end_qh = queue_head; //update of last Queue Head pointer
 asm("wbinvd");

 sti();
}

void ehci_remove_asychronous_queue_head(dword_t number_of_controller, struct ehci_queue_head_t *queue_head) {
 cli();

 //mark qTDs in Queue Head as inactive
 if(queue_head->td.next_qtd_pointer != EHCI_INVALID_QTD_POINTER && queue_head->td.next_qtd_pointer != 0) {
  // struct ehci_queue_transfer_descriptor_t *next_qtd = (struct ehci_queue_transfer_descriptor_t *) queue_head->td.next_qtd_pointer;
  // next_qtd->status_bits.active = 0;
  l("\nRemoving active qTD");
 }
//  queue_head->td.status_bits.active = 0;

 //we need to find where is Queue Head in list
 struct ehci_queue_head_t *previous_queue_head = ehci_controllers[number_of_controller].asychronous_start_qh;
 struct ehci_queue_head_t *actual_queue_head = (struct ehci_queue_head_t *) (ehci_controllers[number_of_controller].asychronous_start_qh->horizontal_pointer & 0xFFFFFFE0);
 while((dword_t)actual_queue_head != (dword_t)ehci_controllers[number_of_controller].asychronous_start_qh) {
  //check if this is our Queue Head
  if((dword_t)actual_queue_head == (dword_t)queue_head) {
   //remove Queue Head
   previous_queue_head->horizontal_pointer = actual_queue_head->horizontal_pointer;

   //update last Queue Head pointer if needed
   if((dword_t)queue_head == (dword_t)ehci_controllers[number_of_controller].asychronous_end_qh) {
    ehci_controllers[number_of_controller].asychronous_end_qh = previous_queue_head;
   }

   //make sure that all changes are written directly to memory
   asm("wbinvd");

   //doorbell EHCI that something was removed from asychronous schedule
   mmio_outd(ehci_controllers[number_of_controller].operational_registers_base+0x04, (1 << 5));
   mmio_outd(ehci_controllers[number_of_controller].operational_registers_base+0x00, mmio_ind(ehci_controllers[number_of_controller].operational_registers_base+0x00) | (1 << 6));
   for(dword_t i = 0; i < 1000000; i++) {
    if((mmio_ind(ehci_controllers[number_of_controller].operational_registers_base+0x04) & (1 << 5)) == (1 << 5)) {
     mmio_outd(ehci_controllers[number_of_controller].operational_registers_base+0x04, (1 << 5));
     sti();
     return;
    }
   }

   sti();
   return;
  }

  //move to next Queue Head
  previous_queue_head = actual_queue_head;
  actual_queue_head = (struct ehci_queue_head_t *) (actual_queue_head->horizontal_pointer & 0xFFFFFFE0);
 }

}

byte_t get_state_of_ehci_transfer(byte_t device_address, void *transfer_descriptor_pointer, dword_t number_of_tds) {
 //set pointer to Queue Head
 struct ehci_queue_head_t *queue_head = (struct ehci_queue_head_t *) transfer_descriptor_pointer;

 //check actual status of transfer
 if((queue_head->td.status & 0x7C) != 0x00) {
  //log error
  l("\nEHCI transfer error: ");
  if(queue_head->td.status_bits.missed_microframe == 1) {
   l("Missed Microframe");
  }
  else if(queue_head->td.status_bits.transaction_error == 1) {
   l("Transaction Error");
  }
  else if(queue_head->td.status_bits.babble == 1) {
   l("Babble");
  }
  else if(queue_head->td.status_bits.data_buffer_error == 1) {
   l("Data Buffer Error");
  }
  else if(queue_head->td.status_bits.halted == 1) {
   l("Halted");
  }

  return USB_TRANSFER_ERROR;
 }
 else if(queue_head->td.next_qtd_pointer == EHCI_INVALID_QTD_POINTER && queue_head->td.status_bits.active == 0) {
  return USB_TRANSFER_DONE;
 }
 else {
  return USB_TRANSFER_NOT_TRANSFERRED;
 }
}

void ehci_control_transfer_without_data(byte_t device_address) {
 //allocate memory for Queue Head + SETUP td + ACK td
 byte_t *tms = (byte_t *) (aligned_calloc(128+(1+1)*64, 0x1F));

 //set SETUP transfer descriptor
 ehci_set_td(tms,
             0, 1, //TD on position 0, pointing to TD at position 1
             EHCI_PACKET_SETUP,
             USB_TOGGLE_0,
             8, //transfer size = 8
             (byte_t *)(&usb_devices[device_address].setup_buffer),
             STATUS_FALSE); //no interrupt

 //set ACK transfer descriptor
 ehci_set_td(tms,
             1, 0, //TD on position 1, not pointing to anything else
             EHCI_PACKET_IN,
             USB_TOGGLE_1,
             0, //transfer size = 0
             0, //nothing is transferred
             STATUS_TRUE); //interrupt when transferred

 //set Queue Head
 ehci_set_qh((struct ehci_queue_head_t *)tms,
             EHCI_ASYCHRONOUS_QH,
             device_address,
             0, //endpoint 0
             usb_devices[device_address].control_transfer.endpoint_size);

 //set transfer info, this will start checking of this transfer when IRQ will arrive
 usb_devices[device_address].control_transfer.is_running = STATUS_TRUE;
 usb_devices[device_address].control_transfer.tms_pointer = tms;
 usb_devices[device_address].control_transfer.first_td_pointer = tms;

 //start transfer
 ehci_insert_asychronous_queue_head(usb_devices[device_address].controller_number, (struct ehci_queue_head_t *)tms);
}

void ehci_control_transfer_with_data(byte_t device_address, byte_t transfer_direction, word_t buffer_size) {
 //load data of device to variables
 dword_t control_endpoint_size = usb_devices[device_address].control_transfer.endpoint_size;
 byte_t *buffer = usb_devices[device_address].control_transfer.transfer_buffer;

 //calculate how many transfer descriptors do we need
 dword_t number_of_data_td = (buffer_size / control_endpoint_size);
 if((buffer_size % control_endpoint_size) != 0) {
  number_of_data_td++;
 }

 //allocate memory for Queue Head + SETUP td + DATA tds (=number_of_data_td) + ACK td
 byte_t *tms = (byte_t *) (aligned_calloc(128+(1+number_of_data_td+1)*64, 0x1F));

 //set SETUP transfer descriptor
 ehci_set_td(tms,
             0, 1, //TD on position 0, pointing to TD at position 1
             EHCI_PACKET_SETUP,
             USB_TOGGLE_0,
             8, //transfer size = 8
             (byte_t *)(&usb_devices[device_address].setup_buffer),
             STATUS_FALSE); //no interrupt

 //set DATA transfer descriptors
 for(dword_t i = 1, toggle = 1, transfer_size = control_endpoint_size; i<(number_of_data_td+1); i++) {
  //set size of transfer descriptor, always control_endpoint_size except last transfer if it is smaller than control_endpoint_size
  if(buffer_size < control_endpoint_size) {
   transfer_size = buffer_size;
  }

  //set transfer descriptor
  if(transfer_direction == USB_TRANSFER_IN) {
   ehci_set_td(tms,
               i, (i+1), //TD on position i, pointing to next TD
               EHCI_PACKET_IN,
               toggle,
               transfer_size, //transfer size = 8
               (byte_t *)buffer,
               STATUS_FALSE); //no interrupt
  }
  else { //USB_TRANSFER_OUT
   ehci_set_td(tms,
               i, (i+1), //TD on position i, pointing to next TD
               EHCI_PACKET_OUT,
               toggle,
               transfer_size, //transfer size = 8
               (byte_t *)buffer,
               STATUS_FALSE); //no interrupt
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
  ehci_set_td(tms,
              (number_of_data_td+1), 0, //last TD not pointing to anything else
              EHCI_PACKET_OUT,
              USB_TOGGLE_1,
              0, //transfer size = 0
              0, //nothing is transferred
              STATUS_TRUE); //interrupt when transferred
 }
 else { //USB_TRANSFER_OUT
  ehci_set_td(tms,
              (number_of_data_td+1), 0, //last TD not pointing to anything else
              EHCI_PACKET_IN,
              USB_TOGGLE_1,
              0, //transfer size = 0
              0, //nothing is transferred
              STATUS_TRUE); //interrupt when transferred
 }

 //set Queue Head
 ehci_set_qh((struct ehci_queue_head_t *)tms,
             EHCI_ASYCHRONOUS_QH,
             device_address,
             0, //endpoint 0
             usb_devices[device_address].control_transfer.endpoint_size);

 //set transfer info, this will start checking of this transfer when IRQ will arrive
 usb_devices[device_address].control_transfer.is_running = STATUS_TRUE;
 usb_devices[device_address].control_transfer.tms_pointer = tms;
 usb_devices[device_address].control_transfer.first_td_pointer = tms;

 //start transfer
 ehci_insert_asychronous_queue_head(usb_devices[device_address].controller_number, (struct ehci_queue_head_t *)tms);
}

void ehci_close_control_transfer(byte_t device_address) {
 //remove Queue Head from transfer chain
 ehci_remove_asychronous_queue_head(usb_devices[device_address].controller_number, (struct ehci_queue_head_t *)usb_devices[device_address].control_transfer.tms_pointer);
}

void ehci_interrupt_transfer(byte_t device_address, byte_t transfer_direction, struct usb_interrupt_transfer_info_t *interrupt_transfer) {
 //recalculate interval
 dword_t interval = interrupt_transfer->interval;
 if(interrupt_transfer->interval == 0) {
  l("\nEHCI ERROR: invalid interrupt transfer interval");
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

 //allocate memory for Queue Head + INTERRUPT td
 byte_t *tms = (byte_t *) (aligned_calloc(128+64, 0x1F));

 //set INTERRUPT transfer descriptor
 if(transfer_direction == USB_TRANSFER_IN) {
  ehci_set_td(tms,
              0, 0, //TD on position 0, not pointing to anything
              EHCI_PACKET_IN,
              USB_TOGGLE_0,
              interrupt_transfer->endpoint_size, //interrupt transfer length
              (byte_t *)(interrupt_transfer->transfer_buffer),
              STATUS_TRUE); //interrupt when transferred
 }
 else { // USB_TRANSFER_OUT
  ehci_set_td(tms,
              0, 0, //TD on position 0, not pointing to anything
              EHCI_PACKET_OUT,
              USB_TOGGLE_0,
              interrupt_transfer->endpoint_size, //interrupt transfer length
              (byte_t *)(interrupt_transfer->transfer_buffer),
              STATUS_TRUE); //interrupt when transferred
 }

 //set Queue Head
 ehci_set_qh((struct ehci_queue_head_t *)tms,
             EHCI_INTERRUPT_QH,
             device_address,
             interrupt_transfer->endpoint, //interrupt endpoint
             interrupt_transfer->endpoint_size);
 
 //set transfer info, this will start checking of this transfer when IRQ will arrive
 interrupt_transfer->is_running = STATUS_TRUE;
 interrupt_transfer->tms_pointer = tms;
 interrupt_transfer->td_pointer = tms;

 //insert Queue Head to periodic list
 cli();

 //select correct Queue Head
 struct ehci_empty_queue_head_t *interval_queue_head;
 if(interval == 32) {
  interval_queue_head = &ehci_controllers[usb_devices[device_address].controller_number].periodic_qh[0];
 }
 else if(interval == 16) {
  interval_queue_head = &ehci_controllers[usb_devices[device_address].controller_number].periodic_qh[1];
 }
 else if(interval == 8) {
  interval_queue_head = &ehci_controllers[usb_devices[device_address].controller_number].periodic_qh[2];
 }
 else if(interval == 4) {
  interval_queue_head = &ehci_controllers[usb_devices[device_address].controller_number].periodic_qh[3];
 }
 else if(interval == 2) {
  interval_queue_head = &ehci_controllers[usb_devices[device_address].controller_number].periodic_qh[4];
 }
 else {
  interval_queue_head = &ehci_controllers[usb_devices[device_address].controller_number].periodic_qh[5];
 }

 //insert our Queue Head
 struct ehci_queue_head_t *queue_head = (struct ehci_queue_head_t *) tms;
 queue_head->horizontal_pointer = interval_queue_head->horizontal_pointer;
 interval_queue_head->horizontal_pointer = ((dword_t)queue_head | EHCI_QH_POINTS_TO_QH);

 sti();
}

void ehci_restart_interrupt_transfer(byte_t device_address, struct usb_interrupt_transfer_info_t *interrupt_transfer) {
 struct ehci_queue_head_t *qh = (struct ehci_queue_head_t *) interrupt_transfer->tms_pointer;
 struct ehci_queue_transfer_descriptor_t *td = (struct ehci_queue_transfer_descriptor_t *) ((dword_t)interrupt_transfer->tms_pointer+128);

 //set buffer pointers
 td->lower_buffer_pointer_0 = ((dword_t)interrupt_transfer->transfer_buffer);
 td->lower_buffer_pointer_1 = (((dword_t)interrupt_transfer->transfer_buffer & 0xFFFFF000)+0x1000);
 td->lower_buffer_pointer_2 = (td->lower_buffer_pointer_1+0x1000);
 td->lower_buffer_pointer_3 = (td->lower_buffer_pointer_1+0x2000);
 td->lower_buffer_pointer_4 = (td->lower_buffer_pointer_1+0x3000);

 //set transfer length
 td->total_bytes_to_transfer = interrupt_transfer->endpoint_size;

 //set buffer pointers
 qh->td.lower_buffer_pointer_0 = ((dword_t)interrupt_transfer->transfer_buffer);
 qh->td.lower_buffer_pointer_1 = (((dword_t)interrupt_transfer->transfer_buffer & 0xFFFFF000)+0x1000);
 qh->td.lower_buffer_pointer_2 = (qh->td.lower_buffer_pointer_1+0x1000);
 qh->td.lower_buffer_pointer_3 = (qh->td.lower_buffer_pointer_1+0x2000);
 qh->td.lower_buffer_pointer_4 = (qh->td.lower_buffer_pointer_1+0x3000);

 //set transfer length
 qh->td.total_bytes_to_transfer = interrupt_transfer->endpoint_size;

 //activate Transfer Descriptor
 td->status = 0x80;

 //activate Transfer Descriptor in Queue Head
 qh->td.status = 0x80;
}

void ehci_close_interrupt_transfer(byte_t device_address, struct usb_interrupt_transfer_info_t *interrupt_transfer) {
 //remove Queue Head from Periodic list
 struct ehci_queue_head_t *queue_head = (struct ehci_queue_head_t *) interrupt_transfer->tms_pointer;
 cli();

 //if this Queue Head was not transferred, close all Transfer Descriptors so controller will skip them
//  if(queue_head->element_pointer != UHCI_INVALID_QH_POINTER) {
//   struct uhci_transfer_descriptor_t *transfer_descriptor = (struct uhci_transfer_descriptor_t *) queue_head->element_pointer;
//   queue_head->element_pointer = UHCI_INVALID_QH_POINTER;

//   while(1) {
//    transfer_descriptor->depth_first = 0;
//    transfer_descriptor->terminate = 1;
//    transfer_descriptor->status = 0x00;
//    if(transfer_descriptor->link_pointer == 0) {
//     break;
//    }
//    else {
//     transfer_descriptor = (struct uhci_transfer_descriptor_t *) (transfer_descriptor->link_pointer<<4);
//    }
//   }
//  }

 //go through all Queue Heads to find our Queue Head
 struct ehci_empty_queue_head_t *queue_head_in_list = &ehci_controllers[usb_devices[device_address].controller_number].periodic_qh[0];
 while(1) {
  //end of list, our Queue Head was not inserted here
  if(queue_head_in_list->horizontal_pointer == EHCI_INVALID_QH_POINTER) {
   break;
  }
  //our Queue Head is inserted here
  else if((queue_head_in_list->horizontal_pointer & 0xFFFFFFF0) == (dword_t)queue_head) {
   queue_head_in_list->horizontal_pointer = queue_head->horizontal_pointer;
   break;
  }

  //go to next Queue Head
  queue_head_in_list = (struct ehci_empty_queue_head_t *) (queue_head_in_list->horizontal_pointer & 0xFFFFFFF0);
 }

 sti();
}

void ehci_bulk_transfer(byte_t device_address, byte_t transfer_direction, struct usb_bulk_transfer_info_t *bulk_transfer) {
 //get size of bulk endpoint
 dword_t endpoint_size = bulk_transfer->endpoint_size;
 if(endpoint_size > 512) {
  endpoint_size = 512;
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

 //transfer max 128 Transfer Descriptors at one Queue Head, so if size of transfer is bigger, it will be divided to multiple transfers
 if(number_of_tds > 128) {
  number_of_tds = 128;
  size_of_last_td = endpoint_size; //original last TD will not be transferred, and actual last TD will have equal size as all other TDs
 }

 //allocate memory for Queue Head + DATA tds
 byte_t *tms = (byte_t *) (aligned_calloc(128+(number_of_tds)*64, 0x1F));

 //set DATA transfer descriptors except for last one
 for(dword_t i=0; i<(number_of_tds-1); i++) {
  if(transfer_direction == USB_TRANSFER_IN) {
   ehci_set_td(tms,
               i, (i+1), //TD on position i, pointing to next TD
               EHCI_PACKET_IN,
               bulk_transfer->endpoint_toggle,
               endpoint_size, //transfer size is max size of endpoint
               (byte_t *)(bulk_transfer->transfer_buffer_pointer),
               STATUS_FALSE); //no interrupt
  }
  else { // USB_TRANSFER_OUT
   ehci_set_td(tms,
               i, (i+1), //TD on position i, pointing to next TD
               EHCI_PACKET_OUT,
               bulk_transfer->endpoint_toggle,
               endpoint_size, //transfer size is max size of endpoint
               (byte_t *)(bulk_transfer->transfer_buffer_pointer),
               STATUS_FALSE); //no interrupt
  }

  //change toggle
  bulk_transfer->endpoint_toggle = ((bulk_transfer->endpoint_toggle += 1) & 0x1);

  //move buffer pointer
  bulk_transfer->transfer_buffer_pointer += endpoint_size;
 }

 //set last DATA transfer descriptor
 if(transfer_direction == USB_TRANSFER_IN) {
  ehci_set_td(tms,
              (number_of_tds-1), 0, //TD on position i, not pointing to anything
              EHCI_PACKET_IN,
              bulk_transfer->endpoint_toggle,
              size_of_last_td, //size of last TD transfer
              (byte_t *)(bulk_transfer->transfer_buffer_pointer),
              STATUS_TRUE); //interrupt after transfer
 }
 else { // USB_TRANSFER_OUT
  ehci_set_td(tms,
              (number_of_tds-1), 0, //TD on position i, not pointing to anything
              EHCI_PACKET_OUT,
              bulk_transfer->endpoint_toggle,
              size_of_last_td, //size of last TD transfer
              (byte_t *)(bulk_transfer->transfer_buffer_pointer),
              STATUS_TRUE); //interrupt after transfer
 }

 //set Queue Head
 ehci_set_qh((struct ehci_queue_head_t *)tms,
             EHCI_ASYCHRONOUS_QH,
             device_address,
             bulk_transfer->endpoint,
             bulk_transfer->endpoint_size);

 //change toggle
 bulk_transfer->endpoint_toggle = ((bulk_transfer->endpoint_toggle += 1) & 0x1);

 //move buffer pointer
 bulk_transfer->transfer_buffer_pointer += endpoint_size;

 //change size of remaining untransferred data
 bulk_transfer->size_of_untransferred_data -= ((number_of_tds-1)*endpoint_size + size_of_last_td);
 
 //set transfer info, this will start checking of this transfer when IRQ will arrive
 bulk_transfer->is_running = STATUS_TRUE;
 bulk_transfer->tms_pointer = tms;
 bulk_transfer->td_pointer = tms;

 //insert Queue Head to transfer chain
 ehci_insert_asychronous_queue_head(usb_devices[device_address].controller_number, (struct ehci_queue_head_t *)tms);
}

void ehci_close_bulk_transfer(byte_t device_address, struct usb_bulk_transfer_info_t *bulk_transfer) {
 //remove Queue Head from transfer chain
 ehci_remove_asychronous_queue_head(usb_devices[device_address].controller_number, bulk_transfer->tms_pointer);
}