//BleskOS

/*
* MIT License
* Copyright (c) 2023-2025 BleskOS developers
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

/* initalization of UHCI controller at boot */

void initalize_uhci_controller(dword_t number_of_controller) {
 //log
 l("\n\nUHCI controller ");

 //Host Controller Reset
 outw(uhci_controllers[number_of_controller].base+0x0, (1 << 1));
 volatile dword_t timeout = (time_of_system_running+50);
 while((inw(uhci_controllers[number_of_controller].base+0x0) & (1 << 1))==(1 << 1)) {
  asm("nop");
  if(time_of_system_running >= timeout) {
   l("Host Controller Reset error");
   return;
  }
 }
 l("reset in "); lv(50-(timeout-time_of_system_running));

 //Global Reset
 outw(uhci_controllers[number_of_controller].base+0x0, (1 << 2));
 wait(20); //specification says we need to wait at least 10ms
 outw(uhci_controllers[number_of_controller].base+0x0, 0);

 //create main transfer Queue Heads
 uhci_controllers[number_of_controller].queue_head = (struct uhci_queue_head_t *) aligned_calloc(sizeof(struct uhci_queue_head_t)*6, 0xF);
 for(dword_t i=0; i<6; i++) {
  uhci_controllers[number_of_controller].queue_head[i].head_pointer = ((dword_t)&uhci_controllers[number_of_controller].queue_head[i+1] | UHCI_QH_POINTS_TO_QH);
  uhci_controllers[number_of_controller].queue_head[i].element_pointer = UHCI_INVALID_QH_POINTER;
 }
 uhci_controllers[number_of_controller].queue_head[5].head_pointer = UHCI_INVALID_QH_POINTER;

 //initalize frame list
 uhci_controllers[number_of_controller].frame_list = (dword_t *) aligned_malloc(UHCI_NUMBER_OF_FRAMES_IN_FRAME_LIST*4, 0xFFF);
 for(dword_t i = 0; i < UHCI_NUMBER_OF_FRAMES_IN_FRAME_LIST; i++) {
  if((i % 32) == 0) {
   uhci_controllers[number_of_controller].frame_list[i] = ((dword_t)&uhci_controllers[number_of_controller].queue_head[0] | UHCI_FRAME_POINTS_TO_QH);
  }
  else if((i % 16) == 0) {
   uhci_controllers[number_of_controller].frame_list[i] = ((dword_t)&uhci_controllers[number_of_controller].queue_head[1] | UHCI_FRAME_POINTS_TO_QH);
  }
  else if((i % 8) == 0) {
   uhci_controllers[number_of_controller].frame_list[i] = ((dword_t)&uhci_controllers[number_of_controller].queue_head[2] | UHCI_FRAME_POINTS_TO_QH);
  }
  else if((i % 4) == 0) {
   uhci_controllers[number_of_controller].frame_list[i] = ((dword_t)&uhci_controllers[number_of_controller].queue_head[3] | UHCI_FRAME_POINTS_TO_QH);
  }
  else if((i % 2) == 0) {
   uhci_controllers[number_of_controller].frame_list[i] = ((dword_t)&uhci_controllers[number_of_controller].queue_head[4] | UHCI_FRAME_POINTS_TO_QH);
  }
  else {
   uhci_controllers[number_of_controller].frame_list[i] = ((dword_t)&uhci_controllers[number_of_controller].queue_head[5] | UHCI_FRAME_POINTS_TO_QH);
  }
 }
 outd(uhci_controllers[number_of_controller].base+0x8, (dword_t)uhci_controllers[number_of_controller].frame_list);

 //set Start Of Frame
 outb(uhci_controllers[number_of_controller].base+0xC, 0x40);
 
 //find number of ports
 uhci_controllers[number_of_controller].number_of_ports = 0;
 for(dword_t i=0, port_number=0x10; i<16; i++, port_number+=0x2) {
  //check if this is valid port register
  if(inw(uhci_controllers[number_of_controller].base+port_number)==0xFFFF || (inw(uhci_controllers[number_of_controller].base+port_number) & 0x80)!=0x80) {
   break;
  }
  
  //if it is, we found port
  uhci_controllers[number_of_controller].number_of_ports++;
 }

 //enable interrupts
 set_irq_handler(uhci_controllers[number_of_controller].irq, (dword_t)usb_irq);
 outw(uhci_controllers[number_of_controller].base+0x4, 0x000C);
 
 //start controller
 outw(uhci_controllers[number_of_controller].base+0x0, (1 << 0) | (1 << 7)); //run execution of frame list, max packet size is 64 bytes
}

/* irq handler */

byte_t uhci_acknowledge_interrupt(dword_t number_of_controller) {
 //read interrupt status
 volatile word_t irq_status = inw(uhci_controllers[number_of_controller].base+0x2);

 //return if nothing happend
 if(irq_status == 0) {
  return STATUS_FALSE;
 }

 //clear interrupt status
 outw(uhci_controllers[number_of_controller].base+0x2, irq_status);
 return STATUS_TRUE;
}

/* initalizing of ports */

byte_t uhci_check_port(dword_t number_of_controller, dword_t number_of_port) {
 //calculate register of port
 word_t uhci_port = (uhci_controllers[number_of_controller].base+0x10+(number_of_port*2));
 word_t uhci_port_value = inw(uhci_port);

 //is status change bit clear?
 if((uhci_port_value & (1 << 1))==0x0) {
  //check if this is zero address device that is not initalized
  if(usb_devices[0].is_used == STATUS_TRUE
     && usb_devices[0].controller_type == USB_UHCI
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

 //was device removed?
 if((uhci_port_value & (1 << 0))==0x0) {
  //clear status change bit
  outw(uhci_port, uhci_port_value | (1 << 1));

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
 usb_devices[0].controller_type = USB_UHCI;
 usb_devices[0].controller_number = number_of_controller;
 usb_devices[0].port_number = number_of_port;
 usb_devices[0].is_port_in_initalization = STATUS_TRUE;

 //start reset of port
 outw(uhci_port, (1 << 9));

 //wait 100ms to stop reset
 create_task(uhci_stop_port_reset, TASK_TYPE_PERIODIC_INTERRUPT, 100);

 return CONTROLLER_PORT_NO_CHANGE;
}

void uhci_stop_port_reset(void) {
 //calculate register of port
 word_t uhci_port = (uhci_controllers[usb_devices[0].controller_number].base+0x10+(usb_devices[0].port_number*2));

 //stop reset of port
 outw(uhci_port, 0x0);

 //remove this task
 destroy_task(uhci_stop_port_reset);

 //wait 50 ms to start enabling port
 create_task(uhci_enable_port, TASK_TYPE_PERIODIC_INTERRUPT, 50);
}

void uhci_enable_port(void) {
 //calculate register of port
 word_t uhci_port = (uhci_controllers[usb_devices[0].controller_number].base+0x10+(usb_devices[0].port_number*2));

 //clear status change
 outw(uhci_port, (1 << 1));

 //start enabling of port - this can not be done in same write as clearing status change
 outw(uhci_port, (1 << 2));

 //remove this task
 destroy_task(uhci_enable_port);

 //wait until is port enabled or not with 100ms timeout
 usb_devices[0].timeout = (time_of_system_running+100);
 create_task(uhci_check_if_port_is_enabled, TASK_TYPE_PERIODIC_INTERRUPT, 1);
}

void uhci_check_if_port_is_enabled(void) {
 //calculate register of port
 word_t uhci_port = (uhci_controllers[usb_devices[0].controller_number].base+0x10+(usb_devices[0].port_number*2));
 word_t uhci_port_value = inw(uhci_port);

 //check if port is enabled
 if((uhci_port_value & (1 << 2))==(1 << 2)) {
  l("\nUHCI port is enabled in "); lvw(100-(usb_devices[0].timeout-time_of_system_running));
  
  //when port is enabled, it mean, that device have address 0 and is ready for transfers, so fill all needed entries
  usb_devices[0].disable_device_on_port = uhci_disable_device_on_port;
  usb_devices[0].get_state_of_transfer = get_state_of_uhci_transfer;

  usb_devices[0].control_transfer_without_data = uhci_control_transfer_without_data;
  usb_devices[0].control_transfer_with_data = uhci_control_transfer_with_data;
  usb_devices[0].close_control_transfer = uhci_close_control_transfer;

  usb_devices[0].interrupt_transfer = uhci_interrupt_transfer;
  usb_devices[0].restart_interrupt_transfer = uhci_restart_interrupt_transfer;
  usb_devices[0].close_interrupt_transfer = uhci_close_interrupt_transfer;

  usb_devices[0].bulk_transfer = uhci_bulk_transfer;
  usb_devices[0].close_bulk_transfer = uhci_close_bulk_transfer;

  //get speed of connected device
  if((uhci_port_value & (1 << 8))==(1 << 8)) {
   usb_devices[0].device_speed = USB_LOW_SPEED;
  }
  else {
   usb_devices[0].device_speed = USB_FULL_SPEED;
  }

  //port is now initalized
  usb_devices[0].is_port_in_initalization = STATUS_FALSE;

  //remove this task
  destroy_task(uhci_check_if_port_is_enabled);
 }
 else if(time_of_system_running >= usb_devices[0].timeout) {
  //timeout error
  l("\nERROR: UHCI device was not enabled");

  //disable device
  uhci_disable_device_on_port(usb_devices[0].controller_number, usb_devices[0].port_number);

  //initalization of port ended, and device is not in zero address state
  usb_devices[0].is_used = STATUS_FALSE;

  //remove this task
  destroy_task(uhci_check_if_port_is_enabled);
 }
}

void uhci_disable_device_on_port(dword_t number_of_controller, dword_t number_of_port) {
 //calculate register of port
 word_t uhci_port = (uhci_controllers[number_of_controller].base+0x10+(number_of_port*2));

 //disable device
 outw(uhci_port, inw(uhci_port) & ~(1 << 2));
}

/* transfers */

void uhci_set_td(byte_t *start_of_td_memory,
                 dword_t td_number, dword_t next_td_number,
                 byte_t speed,
                 byte_t type,
                 byte_t address,
                 byte_t endpoint,
                 byte_t toggle,
                 word_t length_of_transfer,
                 byte_t *buffer,
                 byte_t interrupt_on_complete,
                 byte_t is_isochronous) {
 //set pointer to first transfer descriptor
 struct uhci_transfer_descriptor_t *transfer_descriptor = (struct uhci_transfer_descriptor_t *) ((dword_t)start_of_td_memory + td_number*sizeof(struct uhci_transfer_descriptor_t));

 //clear transfer descriptor
 clear_memory((dword_t)transfer_descriptor, sizeof(struct uhci_transfer_descriptor_t));

 //set link pointer
 if(next_td_number == 0) { //last transfer descriptor that do not point to anything else
  transfer_descriptor->terminate = 1;
 }
 else { //set pointer to given transfer descriptor
  transfer_descriptor->depth_first = 1;
  transfer_descriptor->link_pointer = (((dword_t)start_of_td_memory + next_td_number*sizeof(struct uhci_transfer_descriptor_t)) >> 4);
 }

 //set active status, so UHCI will transfer this descriptor
 transfer_descriptor->status_bits.active = 1;

 //set if interrupt will be fired after completion of this transfer descriptor
 transfer_descriptor->interrupt_on_complete = interrupt_on_complete;

 //set if this is isochronous transfer
 transfer_descriptor->isochronous_transfer = is_isochronous;

 //set speed of transfer
 if(speed == USB_LOW_SPEED) {
  transfer_descriptor->low_speed_transfer = 1;
 }
 //else transfer_descriptor->low_speed_transfer is 0 = USB_FULL_SPEED / USB_HIGH_SPEED

 //enable short packet detection
 transfer_descriptor->enable_short_packet_detect = 1;

 //set type of packet
 transfer_descriptor->packet_identification = type;

 //set address and endpoint of device
 transfer_descriptor->address = address;
 transfer_descriptor->endpoint = endpoint;

 //set toggle bit
 transfer_descriptor->toggle_bit = toggle;

 //set length of transfer
 transfer_descriptor->length_of_transfer = (length_of_transfer-1);

 //set buffer pointer
 transfer_descriptor->buffer = buffer;
}

void uhci_insert_queue_head(dword_t number_of_controller, struct uhci_queue_head_t *queue_head, dword_t interval) {
 cli();

 //select correct Queue Head
 struct uhci_queue_head_t *interval_queue_head;
 if(interval == 32) {
  interval_queue_head = &uhci_controllers[number_of_controller].queue_head[0];
 }
 else if(interval == 16) {
  interval_queue_head = &uhci_controllers[number_of_controller].queue_head[1];
 }
 else if(interval == 8) {
  interval_queue_head = &uhci_controllers[number_of_controller].queue_head[2];
 }
 else if(interval == 4) {
  interval_queue_head = &uhci_controllers[number_of_controller].queue_head[3];
 }
 else if(interval == 2) {
  interval_queue_head = &uhci_controllers[number_of_controller].queue_head[4];
 }
 else {
  interval_queue_head = &uhci_controllers[number_of_controller].queue_head[5];
 }

 //insert our Queue Head
 queue_head->head_pointer = interval_queue_head->head_pointer;
 interval_queue_head->head_pointer = ((dword_t)queue_head | UHCI_QH_POINTS_TO_QH);

 sti();
}

void uhci_remove_queue_head(dword_t number_of_controller, struct uhci_queue_head_t *queue_head) {
 cli();

 //if this Queue Head was not transferred, close all Transfer Descriptors so controller will skip them
 if(queue_head->element_pointer != UHCI_INVALID_QH_POINTER) {
  struct uhci_transfer_descriptor_t *transfer_descriptor = (struct uhci_transfer_descriptor_t *) queue_head->element_pointer;
  queue_head->element_pointer = UHCI_INVALID_QH_POINTER;

  while(1) {
   transfer_descriptor->depth_first = 0;
   transfer_descriptor->terminate = 1;
   transfer_descriptor->status = 0x00;
   if(transfer_descriptor->link_pointer == 0) {
    break;
   }
   else {
    transfer_descriptor = (struct uhci_transfer_descriptor_t *) (transfer_descriptor->link_pointer<<4);
   }
  }
 }

 //go through all Queue Heads to find our Queue Head
 struct uhci_queue_head_t *queue_head_in_list = &uhci_controllers[number_of_controller].queue_head[0];
 while(1) {
  //end of list, our Queue Head was not inserted here
  if(queue_head_in_list->head_pointer == UHCI_INVALID_QH_POINTER) {
   break;
  }
  //our Queue Head is inserted here
  else if((queue_head_in_list->head_pointer & 0xFFFFFFF0) == (dword_t)queue_head) {
   queue_head_in_list->head_pointer = queue_head->head_pointer;
   break;
  }

  //go to next Queue Head
  queue_head_in_list = (struct uhci_queue_head_t *) (queue_head_in_list->head_pointer & 0xFFFFFFF0);
 }

 sti();
}

byte_t get_state_of_uhci_transfer(byte_t device_address, void *transfer_descriptor_pointer, dword_t number_of_tds) {
 //set pointer to first Transfer Descriptor
 struct uhci_transfer_descriptor_t *transfer_descriptor = (struct uhci_transfer_descriptor_t *) transfer_descriptor_pointer;

 //check state of all transfer descriptors
 for(dword_t i=0; i<number_of_tds; i++) {
  //check if last descriptor is successfully transferred
  if(transfer_descriptor[i].terminate == 1 && transfer_descriptor[i].status == 0x00) {
   return USB_TRANSFER_DONE;
  }
  //check if there is error
  else if(transfer_descriptor[i].status_bits.active == 0 && transfer_descriptor[i].status != 0x00) {
   //log error
   l("\nUHCI transfer error: ");
   if(transfer_descriptor[i].status_bits.stalled == 1) {
    l("stalled");
   }
   else if(transfer_descriptor[i].status_bits.data_buffer_error == 1) {
    l("data buffer error");
   }
   else if(transfer_descriptor[i].status_bits.babble_detected == 1) {
    l("babble");
   }
   else if(transfer_descriptor[i].status_bits.nak_received == 1) {
    l("NAK");
   }
   else if(transfer_descriptor[i].status_bits.crc_timeout_error == 1) {
    l("CRC/timeout error");
   }
   else if(transfer_descriptor[i].status_bits.bitstuff_error == 1) {
    l("bitstuff error");
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

void uhci_control_transfer_without_data(byte_t device_address) {
 //load data of device to variables
 dword_t device_speed = usb_devices[device_address].device_speed;

 //allocate memory for Queue Head + SETUP td + ACK td
 struct uhci_transfer_memory_structure_t *tms = (struct uhci_transfer_memory_structure_t *) (aligned_calloc(sizeof(struct uhci_queue_head_t)+sizeof(struct uhci_transfer_descriptor_t)*(1+1), 0xF));

 //set Queue Head
 tms->queue_head.head_pointer = UHCI_INVALID_QH_POINTER; //terminate transfer
 tms->queue_head.element_pointer = (dword_t)(&tms->transfer_descriptor); //SETUP transfer descriptor

 //set SETUP transfer descriptor
 uhci_set_td((byte_t *)(&tms->transfer_descriptor), //pointer to first TD
             0, 1, //TD on position 0, pointing to TD at position 1
             device_speed,
             UHCI_PACKET_SETUP,
             device_address,
             0, //endpoint 0
             USB_TOGGLE_0,
             8, //setup buffer is always 8 bytes long
             (byte_t *)(&usb_devices[device_address].setup_buffer),
             STATUS_FALSE, //no interrupt
             STATUS_FALSE); //not isochronous

 //set ACK transfer descriptor
 uhci_set_td((byte_t *)(&tms->transfer_descriptor), //pointer to first TD
             1, 0, //TD on position 1, not pointing to anything else
             device_speed,
             UHCI_PACKET_IN,
             device_address,
             0, //endpoint 0
             USB_TOGGLE_1,
             0, //transfer size = 0
             0, //nothing is transferred
             STATUS_TRUE,  //interrupt when transferred
             STATUS_FALSE); //not isochronous

 //set transfer info, this will start checking of this transfer when IRQ will arrive
 usb_devices[device_address].control_transfer.is_running = STATUS_TRUE;
 usb_devices[device_address].control_transfer.tms_pointer = tms;
 usb_devices[device_address].control_transfer.first_td_pointer = (&tms->transfer_descriptor);
 usb_devices[device_address].control_transfer.number_of_tds = (1+1);

 //start transfer
 uhci_insert_queue_head(usb_devices[device_address].controller_number, &tms->queue_head, 1);
}

void uhci_control_transfer_with_data(byte_t device_address, byte_t transfer_direction, word_t buffer_size) {
 //load data of device to variables
 dword_t control_endpoint_size = usb_devices[device_address].control_transfer.endpoint_size;
 dword_t device_speed = usb_devices[device_address].device_speed;
 byte_t *buffer = usb_devices[device_address].control_transfer.transfer_buffer;

 //calculate how many transfer descriptors do we need
 dword_t number_of_data_td = (buffer_size / control_endpoint_size);
 if((buffer_size % control_endpoint_size) != 0) {
  number_of_data_td++;
 }

 //allocate memory for Queue Head + SETUP td + DATA tds (=number_of_data_td) + ACK td
 struct uhci_transfer_memory_structure_t *tms = (struct uhci_transfer_memory_structure_t *) (aligned_calloc(sizeof(struct uhci_queue_head_t)+sizeof(struct uhci_transfer_descriptor_t)*(1+number_of_data_td+1), 0xF));

 //set Queue Head
 tms->queue_head.head_pointer = UHCI_INVALID_QH_POINTER; //terminate transfer
 tms->queue_head.element_pointer = (dword_t)(&tms->transfer_descriptor); //SETUP transfer descriptor

 //set SETUP transfer descriptor
 uhci_set_td((byte_t *)(&tms->transfer_descriptor), //pointer to first TD
             0, 1, //TD on position 0, pointing to TD at position 1
             device_speed,
             UHCI_PACKET_SETUP,
             device_address,
             0, //endpoint 0
             USB_TOGGLE_0,
             8, //setup buffer is always 8 bytes long
             (byte_t *)(&usb_devices[device_address].setup_buffer),
             STATUS_FALSE, //no interrupt
             STATUS_FALSE); //not isochronous

 //set DATA transfer descriptors
 for(dword_t i = 1, toggle = 1, transfer_size = control_endpoint_size; i<(number_of_data_td+1); i++) {
  //set size of transfer descriptor, always control_endpoint_size except last transfer if it is smaller than control_endpoint_size
  if(buffer_size < control_endpoint_size) {
   transfer_size = buffer_size;
  }

  //set transfer descriptor
  if(transfer_direction == USB_TRANSFER_IN) {
   uhci_set_td((byte_t *)(&tms->transfer_descriptor), //pointer to first TD
             i, (i+1), //TD on position i, pointing to next TD
             device_speed,
             UHCI_PACKET_IN,
             device_address,
             0, //endpoint 0
             toggle,
             transfer_size,
             (byte_t *)buffer,
             STATUS_FALSE,  //no interrupt
             STATUS_FALSE); //not isochronous
  }
  else { //USB_TRANSFER_OUT
   uhci_set_td((byte_t *)(&tms->transfer_descriptor), //pointer to first TD
             i, (i+1), //TD on position i, pointing to next TD
             device_speed,
             UHCI_PACKET_OUT,
             device_address,
             0, //endpoint 0
             toggle,
             transfer_size,
             (byte_t *)buffer,
             STATUS_FALSE,  //no interrupt
             STATUS_FALSE); //not isochronous
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
  uhci_set_td((byte_t *)(&tms->transfer_descriptor), //pointer to first TD
              (number_of_data_td+1), 0, //last TD not pointing to anything else
              device_speed,
              UHCI_PACKET_OUT,
              device_address,
              0, //endpoint 0
              USB_TOGGLE_1,
              0, //transfer size = 0
              0, //nothing is transferred
              STATUS_TRUE,  //interrupt when transferred
              STATUS_FALSE); //not isochronous
 }
 else { //USB_TRANSFER_OUT
  uhci_set_td((byte_t *)(&tms->transfer_descriptor), //pointer to first TD
              (number_of_data_td+1), 0, //last TD not pointing to anything else
              device_speed,
              UHCI_PACKET_IN,
              device_address,
              0, //endpoint 0
              USB_TOGGLE_1,
              0, //transfer size = 0
              0, //nothing is transferred
              STATUS_TRUE,  //interrupt when transferred
              STATUS_FALSE); //not isochronous
 }

 //set transfer info, this will start checking of this transfer when IRQ will arrive
 usb_devices[device_address].control_transfer.is_running = STATUS_TRUE;
 usb_devices[device_address].control_transfer.tms_pointer = tms;
 usb_devices[device_address].control_transfer.first_td_pointer = (&tms->transfer_descriptor);
 usb_devices[device_address].control_transfer.number_of_tds = (1+number_of_data_td+1);

 //start transfer
 uhci_insert_queue_head(usb_devices[device_address].controller_number, &tms->queue_head, 1);
}

void uhci_close_control_transfer(byte_t device_address) {
 //remove Queue Head from transfer chain
 uhci_remove_queue_head(usb_devices[device_address].controller_number, (struct uhci_queue_head_t *)usb_devices[device_address].control_transfer.tms_pointer);
}

void uhci_interrupt_transfer(byte_t device_address, byte_t transfer_direction, struct usb_interrupt_transfer_info_t *interrupt_transfer) {
 //recalculate interval
 dword_t interval = interrupt_transfer->interval;
 if(interrupt_transfer->interval == 0) {
  l("\nUHCI ERROR: invalid interrupt transfer interval");
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
 struct uhci_transfer_memory_structure_t *tms = (struct uhci_transfer_memory_structure_t *) (aligned_calloc(sizeof(struct uhci_queue_head_t)+sizeof(struct uhci_transfer_descriptor_t), 0xF));

 //set Queue Head
 tms->queue_head.head_pointer = UHCI_INVALID_QH_POINTER; //terminate transfer
 tms->queue_head.element_pointer = (dword_t)(&tms->transfer_descriptor); //INTERRUPT transfer descriptor

 //set INTERRUPT transfer descriptor
 if(transfer_direction == USB_TRANSFER_IN) {
  uhci_set_td((byte_t *)(&tms->transfer_descriptor), //pointer to first TD
             0, 0, //TD on position 0, not pointing to anything
             usb_devices[device_address].device_speed,
             UHCI_PACKET_IN,
             device_address,
             interrupt_transfer->endpoint, //interrupt endpoint
             USB_TOGGLE_0,
             interrupt_transfer->endpoint_size, //interrupt transfer length
             (byte_t *)(interrupt_transfer->transfer_buffer),
             STATUS_TRUE, //interrupt after transfer
             STATUS_FALSE); //not isochronous
 }
 else { // USB_TRANSFER_OUT
  uhci_set_td((byte_t *)(&tms->transfer_descriptor), //pointer to first TD
             0, 0, //TD on position 0, not pointing to anything
             usb_devices[device_address].device_speed,
             UHCI_PACKET_OUT,
             device_address,
             interrupt_transfer->endpoint, //interrupt endpoint
             USB_TOGGLE_0,
             interrupt_transfer->endpoint_size, //interrupt transfer length
             (byte_t *)(interrupt_transfer->transfer_buffer),
             STATUS_TRUE, //interrupt after transfer
             STATUS_FALSE); //not isochronous
 }
 
 //set transfer info, this will start checking of this transfer when IRQ will arrive
 interrupt_transfer->is_running = STATUS_TRUE;
 interrupt_transfer->tms_pointer = tms;
 interrupt_transfer->td_pointer = (&tms->transfer_descriptor);

 //insert Queue Head to transfer chain
 uhci_insert_queue_head(usb_devices[device_address].controller_number, (&tms->queue_head), interval);
}

void uhci_restart_interrupt_transfer(byte_t device_address, struct usb_interrupt_transfer_info_t *interrupt_transfer) {
 struct uhci_queue_head_t *qh = (struct uhci_queue_head_t *) interrupt_transfer->tms_pointer;
 struct uhci_transfer_descriptor_t *td = (struct uhci_transfer_descriptor_t *) interrupt_transfer->td_pointer;

 //reverse toggle bit
 td->toggle_bit = ((td->toggle_bit + 1) & 0x1);

 //clear entry
 td->number_of_transferred_bytes = 0;
 td->status = 0;
 td->error_counter = 0;

 //activate Transfer Descriptor
 td->status_bits.active = 1;

 //activate Queue Head
 qh->element_pointer = ((dword_t)td);
}

void uhci_close_interrupt_transfer(byte_t device_address, struct usb_interrupt_transfer_info_t *interrupt_transfer) {
 //remove Queue Head from transfer chain
 uhci_remove_queue_head(usb_devices[device_address].controller_number, interrupt_transfer->tms_pointer);
}

void uhci_bulk_transfer(byte_t device_address, byte_t transfer_direction, struct usb_bulk_transfer_info_t *bulk_transfer) {
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

 //transfer max 128 Transfer Descriptors at one Queue Head, so if size of transfer is bigger, it will be divided to multiple transfers
 if(number_of_tds > 128) {
  number_of_tds = 128;
  size_of_last_td = endpoint_size; //original last TD will not be transferred, and actual last TD will have equal size as all other TDs
 }

 //allocate memory for Queue Head + DATA tds
 struct uhci_transfer_memory_structure_t *tms = (struct uhci_transfer_memory_structure_t *) (aligned_calloc(sizeof(struct uhci_queue_head_t)+sizeof(struct uhci_transfer_descriptor_t)*number_of_tds, 0xF));

 //set Queue Head
 tms->queue_head.head_pointer = UHCI_INVALID_QH_POINTER; //terminate transfer
 tms->queue_head.element_pointer = (dword_t)(&tms->transfer_descriptor); //DATA transfer descriptor

 //set DATA transfer descriptors except for last one
 for(dword_t i=0; i<(number_of_tds-1); i++) {
  if(transfer_direction == USB_TRANSFER_IN) {
   uhci_set_td((byte_t *)(&tms->transfer_descriptor), //pointer to first TD
              i, (i+1), //TD on position i, pointing to next TD
              usb_devices[device_address].device_speed,
              UHCI_PACKET_IN,
              device_address,
              bulk_transfer->endpoint, //bulk endpoint
              bulk_transfer->endpoint_toggle, //bulk endpoint toggle
              endpoint_size, //endpoint size
              (byte_t *)(bulk_transfer->transfer_buffer_pointer),
              STATUS_FALSE, //no interrupt
              STATUS_FALSE); //not isochronous
  }
  else { // USB_TRANSFER_OUT
   uhci_set_td((byte_t *)(&tms->transfer_descriptor), //pointer to first TD
              i, (i+1), //TD on position i, pointing to next TD
              usb_devices[device_address].device_speed,
              UHCI_PACKET_OUT,
              device_address,
              bulk_transfer->endpoint, //bulk endpoint
              bulk_transfer->endpoint_toggle, //bulk endpoint toggle
              endpoint_size, //endpoint size
              (byte_t *)(bulk_transfer->transfer_buffer_pointer),
              STATUS_FALSE, //no interrupt
              STATUS_FALSE); //not isochronous
  }

  //change toggle
  bulk_transfer->endpoint_toggle = ((bulk_transfer->endpoint_toggle += 1) & 0x1);

  //move buffer pointer
  bulk_transfer->transfer_buffer_pointer += endpoint_size;
 }

 //set last DATA transfer descriptor
 if(transfer_direction == USB_TRANSFER_IN) {
  uhci_set_td((byte_t *)(&tms->transfer_descriptor), //pointer to first TD
             (number_of_tds-1), 0, //TD on position i, not pointing to anything
             usb_devices[device_address].device_speed,
             UHCI_PACKET_IN,
             device_address,
             bulk_transfer->endpoint, //bulk endpoint
             bulk_transfer->endpoint_toggle, //bulk endpoint toggle
             size_of_last_td, //size of last TD transfer
             (byte_t *)(bulk_transfer->transfer_buffer_pointer),
             STATUS_TRUE, //interrupt after transfer
             STATUS_FALSE); //not isochronous
 }
 else { // USB_TRANSFER_OUT
  uhci_set_td((byte_t *)(&tms->transfer_descriptor), //pointer to first TD
             (number_of_tds-1), 0, //TD on position i, not pointing to anything
             usb_devices[device_address].device_speed,
             UHCI_PACKET_OUT,
             device_address,
             bulk_transfer->endpoint, //bulk endpoint
             bulk_transfer->endpoint_toggle, //bulk endpoint toggle
             size_of_last_td, //size of last TD transfer
             (byte_t *)(bulk_transfer->transfer_buffer_pointer),
             STATUS_TRUE, //interrupt after transfer
             STATUS_FALSE); //not isochronous
 }

 //change toggle
 bulk_transfer->endpoint_toggle = ((bulk_transfer->endpoint_toggle += 1) & 0x1);

 //move buffer pointer
 bulk_transfer->transfer_buffer_pointer += endpoint_size;

 //change size of remaining untransferred data
 bulk_transfer->size_of_untransferred_data -= ((number_of_tds-1)*endpoint_size + size_of_last_td);
 
 //set transfer info, this will start checking of this transfer when IRQ will arrive
 bulk_transfer->is_running = STATUS_TRUE;
 bulk_transfer->tms_pointer = tms;
 bulk_transfer->td_pointer = (&tms->transfer_descriptor);
 bulk_transfer->number_of_tds = number_of_tds;

 //insert Queue Head to transfer chain
 uhci_insert_queue_head(usb_devices[device_address].controller_number, (&tms->queue_head), 1);
}

void uhci_close_bulk_transfer(byte_t device_address, struct usb_bulk_transfer_info_t *bulk_transfer) {
 //remove Queue Head from transfer chain
 uhci_remove_queue_head(usb_devices[device_address].controller_number, bulk_transfer->tms_pointer);
}

/* deep debugger */

void ddbg_show_devregs_uhci(dword_t number_of_controller) {
 if(number_of_controller >= number_of_uhci_controllers) {
  ddbg_printf("Invalid UHCI controller number");
  return;
 }

 ddbg_printf("UHCI controller %d registers", number_of_controller);
 ddbg_printf("\n\nBase: %04x", uhci_controllers[number_of_controller].base);

 ddbg_print_parsed_mem("USBCMD", ddbg_uhci_reg_usbcmd, inw(uhci_controllers[number_of_controller].base + 0x00));

 ddbg_print_parsed_mem("USBSTS", ddbg_uhci_reg_usbsts, inw(uhci_controllers[number_of_controller].base + 0x02));

 ddbg_print_parsed_mem("USBINTR", ddbg_uhci_reg_usbintr, inw(uhci_controllers[number_of_controller].base + 0x04));

 word_t frame = inw(uhci_controllers[number_of_controller].base + 0x06);
 ddbg_printf("\n\nFrame Number: %d\nProcessed Frame List: %d", frame, (frame & 0x3FF));

 ddbg_printf("\n\nFrame List Base Address: %x", ind(uhci_controllers[number_of_controller].base + 0x08));

 ddbg_printf("\n\nStart Of Frame Modify: %d", inb(uhci_controllers[number_of_controller].base + 0x0C));

 for(dword_t i=0; i<uhci_controllers[number_of_controller].number_of_ports; i++) {
  ddbg_printf("\n\nUSB Port %d", i+1);
  ddbg_print_parsed_mem(0, ddbg_uhci_reg_port, inw(uhci_controllers[number_of_controller].base + 0x10 + i*2));
 }

 ddbg_printf("\n\nRegister After Ports: %04x", inw(uhci_controllers[number_of_controller].base + 0x10 + uhci_controllers[number_of_controller].number_of_ports*2 + 0x02));
}