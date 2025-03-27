//BleskOS

/*
* MIT License
* Copyright (c) 2023-2025 BleskOS developers
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

void initalize_usb_controllers(void) {
    //allocate memory for USB devices
    usb_devices = (struct usb_device_t *) calloc(sizeof(struct usb_device_t)*MAX_NUMBER_OF_USB_DEVICES);
    number_of_usb_devices = 0;
    usb_device_change_event = STATUS_FALSE;

    //initalize all detected xHCI controllers
    for(dword_t i = 0; i < components->n_xhci; i++) {
        initalize_xhci_controller(i);
    }

    //initalize all detected EHCI controllers
    for(dword_t i = 0; i < components->n_ehci; i++) {
        initalize_ehci_controller(i);
    }

    //initalize all detected UHCI controllers
    for(dword_t i = 0; i < components->n_uhci; i++) {
        initalize_uhci_controller(i);
    }

    //initalize all detected OHCI controllers
    for(dword_t i = 0; i < components->n_ohci; i++) {
        initalize_ohci_controller(i);
    }

    //check actual state of USB ports and start initalization of connected devices
    check_usb_ports();

    //create task to check status of ports four times per second
    create_task(check_usb_ports, TASK_TYPE_PERIODIC_INTERRUPT, 250);

    //create task to check initalization status of interfaces four times per second that will initalize all uninitalized interfaces
    create_task(check_usb_interface_initalization, TASK_TYPE_PERIODIC_INTERRUPT, 250);

    //create task that will check pressing keys that are holded by user
    create_task(usb_keyboards_check_pressed_keys, TASK_TYPE_PERIODIC_INTERRUPT, 50);

    //create task that will check USB hub ports
    create_task(usb_hub_check_connections, TASK_TYPE_PERIODIC_INTERRUPT, 200);

    //create task that will monitor unit state of USB MSD
    create_task(usb_msd_monitor_unit_state, TASK_TYPE_USER_INPUT, 1000);
}

void check_usb_ports(void) {
 //check all EHCI ports
 for(dword_t controller=0; controller<components->n_ehci; controller++) {
  for(dword_t port=0; port<components->ehci[controller].number_of_ports; port++) {
   usb_process_port_status(ehci_check_port(controller, port), USB_EHCI, controller, port);
  }
 }

 //check all UHCI ports
 for(dword_t controller=0; controller<components->n_uhci; controller++) {
  for(dword_t port=0; port<components->uhci[controller].number_of_ports; port++) {
   usb_process_port_status(uhci_check_port(controller, port), USB_UHCI, controller, port);
  }
 }

 //check all OHCI ports
 for(dword_t controller=0; controller<components->n_ohci; controller++) {
  for(dword_t port=0; port<components->ohci[controller].number_of_ports; port++) {
   usb_process_port_status(ohci_check_port(controller, port), USB_OHCI, controller, port);
  }
 }
}

void usb_process_port_status(byte_t port_status, byte_t type_of_controller, byte_t controller_number, byte_t port_number) {
 //this device is in zero address state and is not being initalized
 if(port_status == CONTROLLER_PORT_ZERO_ADDRESS_DEVICE && usb_devices[0].control_transfer.is_running == STATUS_FALSE) {
  initalize_zero_address_device();
 }
 else if(port_status == CONTROLLER_PORT_REMOVED_DEVICE) {
  //remove all devices connected through this port
  for(dword_t i=0; i<MAX_NUMBER_OF_USB_DEVICES; i++) {
   if(usb_devices[i].controller_type == type_of_controller && usb_devices[i].controller_number == controller_number && usb_devices[i].port_number == port_number) {
    usb_remove_device(i);
   }
  }
 }
}

void check_usb_interface_initalization(void) {
 for(dword_t i=1; i<MAX_NUMBER_OF_USB_DEVICES; i++) {
  //check if this is device with uninitalized interfaces that is not currently initalizing one
  if(usb_devices[i].is_used == STATUS_TRUE && usb_devices[i].are_interfaces_initalized == STATUS_FALSE && usb_devices[i].is_interface_in_initalization == STATUS_FALSE) {
   //check which interface is not initalized
   if(usb_devices[i].mouse.is_present == STATUS_TRUE && usb_devices[i].mouse.is_initalized == STATUS_FALSE) {
    usb_devices[i].mouse.initalize(i); //start initalization
    continue;
   }
   else if(usb_devices[i].keyboard.is_present == STATUS_TRUE && usb_devices[i].keyboard.is_initalized == STATUS_FALSE) {
    usb_devices[i].keyboard.initalize(i); //start initalization
    continue;
   }
   else if(usb_devices[i].hub.is_present == STATUS_TRUE && usb_devices[i].hub.is_initalized == STATUS_FALSE) {
    usb_devices[i].hub.initalize(i); //start initalization
    continue;
   }
   else if(usb_devices[i].msd.is_present == STATUS_TRUE && usb_devices[i].msd.is_initalized == STATUS_FALSE) {
    usb_devices[i].msd.initalize(i); //start initalization
    continue;
   }

   //no interface is uninitalized what mean that all are initalized and device can be used
   usb_devices[i].are_interfaces_initalized = STATUS_TRUE;
   usb_device_change_event = STATUS_TRUE;
  }

  //check if there is device with interface that needs to wait to finalize initalization
  if(usb_devices[i].is_used == STATUS_TRUE && usb_devices[i].is_interface_in_initalization == STATUS_TRUE) {
   //check which interface is in initalization
   if(usb_devices[i].hub.is_present == STATUS_TRUE
      && usb_devices[i].hub.is_initalized == STATUS_FALSE
      && usb_devices[i].hub.timeout_for_finalizing_initalization != 0
      && time_of_system_running >= usb_devices[i].hub.timeout_for_finalizing_initalization) {
    //initalization is finalized
    usb_devices[i].hub.is_initalized = STATUS_TRUE;
    usb_devices[i].is_interface_in_initalization = STATUS_FALSE;
    usb_devices[i].hub.timeout_for_finalizing_initalization = 0;

    //log
    logf("\nUSB hub was initalized");

    //check devices on hub
    usb_hub_check_connections();
   }
  }
 }

 //TODO: check transfer timeouts
 for(dword_t i=0; i<MAX_NUMBER_OF_USB_DEVICES; i++) {
  //check if there is some USB device
  if(usb_devices[i].is_used == STATUS_TRUE) {
   /* check control transfer */
   if(usb_devices[i].control_transfer.is_running == STATUS_TRUE && time_of_system_running >= usb_devices[i].control_transfer.timeout) {
    //read actual state of transfer
    byte_t status_of_transfer = usb_devices[i].get_state_of_transfer(i, usb_devices[i].control_transfer.first_td_pointer, usb_devices[i].control_transfer.number_of_tds);

    //this transfer was already transfered
    if(status_of_transfer == USB_TRANSFER_DONE || status_of_transfer == USB_TRANSFER_ERROR) {
     logf("\nCalled USB IRQ routine");
     usb_irq();
     continue;
    }

    //transfer was not transferred in time
    logf("\nControl transfer error: timeout ");
    usb_devices[i].control_transfer.transfer_error(i);
   }

   /* check bulk transfers */
   if(usb_devices[i].msd.is_present == STATUS_TRUE) {
    if(usb_devices[i].msd.bulk_only.out.is_running == STATUS_TRUE) {
     usb_check_bulk_transfer(i, (struct usb_bulk_transfer_info_t *)&usb_devices[i].msd.bulk_only.out, USB_TRANSFER_OUT);
    }
    if(usb_devices[i].msd.bulk_only.in.is_running == STATUS_TRUE) {
     usb_check_bulk_transfer(i, (struct usb_bulk_transfer_info_t *)&usb_devices[i].msd.bulk_only.in, USB_TRANSFER_IN);
    }
   }
  }
 }
}

void usb_remove_device(byte_t device_address) {
 logf("\nUSB: Device %d removed", device_address);

 usb_devices[device_address].is_used = STATUS_FALSE;

 //close control transfer
 if(usb_devices[device_address].control_transfer.is_running == STATUS_TRUE) {
  usb_close_control_transfer(device_address);
 }

 //close interrupt transfers
 if(usb_devices[device_address].mouse.is_present == STATUS_TRUE
    && usb_devices[device_address].mouse.is_initalized == STATUS_TRUE
    && usb_devices[device_address].mouse.interrupt_transfer.is_running == STATUS_TRUE) {
  usb_close_interrupt_transfer(device_address, (struct usb_interrupt_transfer_info_t *)&usb_devices[device_address].mouse.interrupt_transfer);
 }
 if(usb_devices[device_address].keyboard.is_present == STATUS_TRUE
    && usb_devices[device_address].keyboard.is_initalized == STATUS_TRUE
    && usb_devices[device_address].keyboard.interrupt_transfer.is_running == STATUS_TRUE) {
  usb_close_interrupt_transfer(device_address, (struct usb_interrupt_transfer_info_t *)&usb_devices[device_address].keyboard.interrupt_transfer);
 }

 //close bulk transfers
 if(usb_devices[device_address].msd.is_present == STATUS_TRUE
    && usb_devices[device_address].msd.bulk_only.is_running == STATUS_TRUE) {
  usb_bulk_only_close_transfer(device_address, (struct usb_bbb_t *)&usb_devices[device_address].msd.bulk_only);
 }

 //remove connected paritions
 if(usb_devices[device_address].msd.is_present == STATUS_TRUE) {
  remove_partitions_of_medium_from_list(MEDIUM_USB_MSD, device_address);
 }

 //remove devices connected to hub
 if(usb_devices[device_address].hub.is_present == STATUS_TRUE && usb_devices[device_address].hub.is_initalized == STATUS_TRUE) {
  logf("\nRemoving HUB from hub");
  
  //go through all USB devices
  for(dword_t i=0; i<MAX_NUMBER_OF_USB_DEVICES; i++) {
   //check if this is device we are looking for
   if(usb_devices[i].is_used == STATUS_TRUE && usb_devices[i].hub_address == device_address) {
    usb_remove_device(i);
   }
  }
 }

 //clear device entry
 clear_memory((dword_t)&usb_devices[device_address], sizeof(struct usb_device_t));

 //decrease number of connected USB devices
 if(device_address != 0) {
  number_of_usb_devices--;
 }
 usb_device_change_event = STATUS_TRUE;
}

void usb_remove_hub_device(byte_t hub_address, byte_t hub_port_number) {
 
}

/* interrupt routine */

void usb_irq(void) {
 //acknowledge interrupt on all controllers
 byte_t is_usb_interrupt = STATUS_FALSE;
 for(dword_t i=0; i<components->n_uhci; i++) {
  is_usb_interrupt |= uhci_acknowledge_interrupt(i);
 }
 for(dword_t i=0; i<components->n_ohci; i++) {
  is_usb_interrupt |= ohci_acknowledge_interrupt(i);
 }
 for(dword_t i=0; i<components->n_ehci; i++) {
  is_usb_interrupt |= ehci_acknowledge_interrupt(i);
 }
 if(is_usb_interrupt == STATUS_FALSE) {
  return;
 }

 //check for completed transfers
 for(dword_t i=0; i<MAX_NUMBER_OF_USB_DEVICES; i++) {
  //check if there is some USB device
  if(usb_devices[i].is_used == STATUS_TRUE) {
   /* check control transfer */
   if(usb_devices[i].control_transfer.is_running == STATUS_TRUE) {
    //read actual state of transfer
    byte_t status_of_transfer = usb_devices[i].get_state_of_transfer(i, usb_devices[i].control_transfer.first_td_pointer, usb_devices[i].control_transfer.number_of_tds);

    //transfer was successfully transferred
    if(status_of_transfer == USB_TRANSFER_DONE) {
     usb_devices[i].control_transfer.transfer_successfull(i);
    }
    //there was error during transfer
    else if(status_of_transfer == USB_TRANSFER_ERROR) {
     usb_devices[i].control_transfer.transfer_error(i);
    }
    //transfer was not transferred in time
    else if(time_of_system_running >= usb_devices[i].control_transfer.timeout) {
     logf("\nControl transfer error: timeout ");
     usb_devices[i].control_transfer.transfer_error(i);
    }
   }

   /* check interrupt transfers */
   if(usb_devices[i].mouse.is_present == STATUS_TRUE && usb_devices[i].mouse.interrupt_transfer.is_running == STATUS_TRUE) {
    usb_check_interrupt_transfer(i, (struct usb_interrupt_transfer_info_t *)&usb_devices[i].mouse.interrupt_transfer);
   }
   if(usb_devices[i].keyboard.is_present == STATUS_TRUE) {
    if(usb_devices[i].keyboard.interrupt_transfer.is_running == STATUS_TRUE) {
     usb_check_interrupt_transfer(i, (struct usb_interrupt_transfer_info_t *)&usb_devices[i].keyboard.interrupt_transfer);
    }
    if(usb_devices[i].keyboard.interrupt_led_transfer.is_running == STATUS_TRUE) {
     usb_check_interrupt_transfer(i, (struct usb_interrupt_transfer_info_t *)&usb_devices[i].keyboard.interrupt_led_transfer);
    }
   }

   /* check bulk transfers */
   if(usb_devices[i].msd.is_present == STATUS_TRUE) {
    if(usb_devices[i].msd.bulk_only.out.is_running == STATUS_TRUE) {
     usb_check_bulk_transfer(i, (struct usb_bulk_transfer_info_t *)&usb_devices[i].msd.bulk_only.out, USB_TRANSFER_OUT);
    }
    if(usb_devices[i].msd.bulk_only.in.is_running == STATUS_TRUE) {
     usb_check_bulk_transfer(i, (struct usb_bulk_transfer_info_t *)&usb_devices[i].msd.bulk_only.in, USB_TRANSFER_IN);
    }
   }
  }
 }
}

void usb_check_interrupt_transfer(byte_t device_address, struct usb_interrupt_transfer_info_t *interrupt_transfer) {
 //check if this transaction is halted
 if(interrupt_transfer->line_of_errors >= 20) {
  return;
 }

 //read state of interrupt transfer
 byte_t status_of_transfer = usb_devices[device_address].get_state_of_transfer(device_address, interrupt_transfer->td_pointer, 1);

 //check if transfer is done
 if(status_of_transfer == USB_TRANSFER_DONE) {
  interrupt_transfer->line_of_errors = 0;
  interrupt_transfer->transfer_successfull(device_address);
  usb_devices[device_address].restart_interrupt_transfer(device_address, interrupt_transfer);
 }
 //if there was error, we only need to restart transfer
 else if(status_of_transfer == USB_TRANSFER_ERROR) {
  interrupt_transfer->line_of_errors++;
  if(interrupt_transfer->line_of_errors >= 20) {
   logf("\nUSB: Interrupt transfer halted because of line of errors");
  }
  else {
   usb_devices[device_address].restart_interrupt_transfer(device_address, interrupt_transfer);
  }
 }
}

void usb_check_bulk_transfer(byte_t device_address, struct usb_bulk_transfer_info_t *bulk_transfer, byte_t transfer_type) {
 //read state of interrupt transfer
 byte_t status_of_transfer = usb_devices[device_address].get_state_of_transfer(device_address, bulk_transfer->td_pointer, bulk_transfer->number_of_tds);

 //check if transfer is done
 if(status_of_transfer == USB_TRANSFER_DONE) {
  //check if whole transfer is done
  if(bulk_transfer->size_of_untransferred_data == 0) {
   bulk_transfer->transfer_successfull(device_address, bulk_transfer->transfer_info);
  }
  else {
   //close previous transfer
   usb_close_bulk_transfer(device_address, bulk_transfer);

   //start new transfer - all values for it are already set
   usb_devices[device_address].bulk_transfer(device_address, transfer_type, bulk_transfer);
  }
  
 }
 //there was error during transfer
 else if(status_of_transfer == USB_TRANSFER_ERROR) {
  bulk_transfer->transfer_error(device_address, bulk_transfer->transfer_info);
 }
 //transfer was not transferred in time
 else if(time_of_system_running >= bulk_transfer->timeout) {
  logf("\nBulk transfer error: timeout ");
  bulk_transfer->transfer_error(device_address, bulk_transfer->transfer_info);
 }
}

/* USB control transfers */

void usb_device_set_control_transfer_variables(byte_t device_address, void (*transfer_successfull)(byte_t device_address), void (*transfer_error)(byte_t device_address), dword_t timeout) {
 usb_devices[device_address].control_transfer.transfer_successfull = transfer_successfull;
 usb_devices[device_address].control_transfer.transfer_error = transfer_error;
 usb_devices[device_address].control_transfer.timeout = (time_of_system_running+timeout);
}

void usb_device_set_setup_buffer(byte_t device_address, byte_t request_type, byte_t request, word_t value, word_t index, word_t length) {
 usb_devices[device_address].setup_buffer.request_type = request_type;
 usb_devices[device_address].setup_buffer.request = request;
 usb_devices[device_address].setup_buffer.value = value;
 usb_devices[device_address].setup_buffer.index = index;
 usb_devices[device_address].setup_buffer.length = length;
}

void usb_control_transfer_without_data(byte_t device_address, void (*transfer_successfull)(byte_t device_address), void (*transfer_error)(byte_t device_address), dword_t timeout, byte_t request_type, byte_t request, word_t value, word_t index) {
 //set transfer variables
 usb_device_set_control_transfer_variables(device_address, transfer_successfull, transfer_error, timeout);

 //set SETUP packet
 usb_device_set_setup_buffer(device_address, request_type, request, value, index, 0);

 //start transfer
 usb_devices[device_address].control_transfer_without_data(device_address);
}

void usb_control_transfer_in(byte_t device_address, void (*transfer_successfull)(byte_t device_address), void (*transfer_error)(byte_t device_address), dword_t timeout, byte_t request_type, byte_t request, word_t value, word_t index, word_t length) {
 //set transfer variables
 usb_device_set_control_transfer_variables(device_address, transfer_successfull, transfer_error, timeout);

 //set SETUP packet
 usb_device_set_setup_buffer(device_address, request_type, request, value, index, length);

 //allocate memory for transferred data
 usb_devices[device_address].control_transfer.transfer_buffer = (void *) calloc(length);

 //start transfer
 usb_devices[device_address].control_transfer_with_data(device_address, USB_TRANSFER_IN, length);
}

void usb_control_transfer_out(byte_t device_address, void (*transfer_successfull)(byte_t device_address), void (*transfer_error)(byte_t device_address), dword_t timeout, byte_t request_type, byte_t request, word_t value, word_t index, word_t length, byte_t *buffer) {
 //set transfer variables
 usb_device_set_control_transfer_variables(device_address, transfer_successfull, transfer_error, timeout);

 //set SETUP packet
 usb_device_set_setup_buffer(device_address, request_type, request, value, index, length);

 //set memory for transferred data
 byte_t *transfer_buffer = (byte_t *) (calloc(length));
 for(dword_t i=0; i<length; i++) {
  transfer_buffer[i] = buffer[i];
 }
 usb_devices[device_address].control_transfer.transfer_buffer = (void *) transfer_buffer;

 //start transfer
 usb_devices[device_address].control_transfer_with_data(device_address, USB_TRANSFER_OUT, length);
}

void usb_close_control_transfer(byte_t device_address) {
 //close transfer on controller
 usb_devices[device_address].close_control_transfer(device_address);

 //free memory of structures that described transfer
 free((void *)usb_devices[device_address].control_transfer.tms_pointer);
 usb_devices[device_address].control_transfer.tms_pointer = 0;

 //free buffer of transfer
 if(usb_devices[device_address].control_transfer.transfer_buffer != 0) {
  free((void *)usb_devices[device_address].control_transfer.transfer_buffer);
 }
 usb_devices[device_address].control_transfer.transfer_buffer = 0;

 //device now isn't in control transfer
 usb_devices[device_address].control_transfer.is_running = STATUS_FALSE;
}

/* USB interrupt transfers */

void usb_interrupt_transfer_in(byte_t device_address, struct usb_interrupt_transfer_info_t *interrupt_transfer, void (*transfer_successfull)(byte_t device_address)) {
 //set method
 interrupt_transfer->transfer_successfull = transfer_successfull;

 //start transfer
 usb_devices[device_address].interrupt_transfer(device_address, USB_TRANSFER_IN, interrupt_transfer);
}

void usb_interrupt_transfer_out(byte_t device_address, struct usb_interrupt_transfer_info_t *interrupt_transfer, byte_t *buffer, void (*transfer_successfull)(byte_t device_address)) {
 //set method
 interrupt_transfer->transfer_successfull = transfer_successfull;
 interrupt_transfer->transfer_buffer = buffer;

 //start transfer
 usb_devices[device_address].interrupt_transfer(device_address, USB_TRANSFER_OUT, interrupt_transfer);
}

void usb_close_interrupt_transfer(byte_t device_address, struct usb_interrupt_transfer_info_t *interrupt_transfer) {
 //close transfer on controller
 usb_devices[device_address].close_interrupt_transfer(device_address, interrupt_transfer);

 //free memory of structures that described transfer
 free((void *)interrupt_transfer->tms_pointer);
 interrupt_transfer->tms_pointer = 0;

 //free buffer of transfer
 free((void *)interrupt_transfer->transfer_buffer);
 interrupt_transfer->transfer_buffer = 0;

 //interrupt transfer is now closed
 interrupt_transfer->is_running = STATUS_FALSE;
}

/* USB bulk transfers */

void usb_bulk_transfer_out(byte_t device_address, struct usb_bulk_transfer_info_t *bulk_transfer, void (*transfer_successfull)(byte_t device_address, void *transfer_info), void (*transfer_error)(byte_t device_address, void *transfer_info), void *transfer_info, dword_t timeout, byte_t *buffer, dword_t length) {
 //set bulk transfer variables
 bulk_transfer->transfer_successfull = transfer_successfull;
 bulk_transfer->transfer_error = transfer_error;
 bulk_transfer->transfer_info = transfer_info;
 bulk_transfer->transfer_buffer_pointer = buffer;
 bulk_transfer->size_of_untransferred_data = length;
 bulk_transfer->timeout = (time_of_system_running+timeout);

 //start transfer
 usb_devices[device_address].bulk_transfer(device_address, USB_TRANSFER_OUT, bulk_transfer);
}

void usb_bulk_transfer_in(byte_t device_address, struct usb_bulk_transfer_info_t *bulk_transfer, void (*transfer_successfull)(byte_t device_address, void *transfer_info), void (*transfer_error)(byte_t device_address, void *transfer_info), void *transfer_info, dword_t timeout, byte_t *buffer, dword_t length) {
 //set bulk transfer variables
 bulk_transfer->transfer_successfull = transfer_successfull;
 bulk_transfer->transfer_error = transfer_error;
 bulk_transfer->transfer_info = transfer_info;
 bulk_transfer->transfer_buffer_pointer = buffer;
 bulk_transfer->size_of_untransferred_data = length;
 bulk_transfer->timeout = (time_of_system_running+timeout);

 //start transfer
 usb_devices[device_address].bulk_transfer(device_address, USB_TRANSFER_IN, bulk_transfer);
}

void usb_close_bulk_transfer(byte_t device_address, struct usb_bulk_transfer_info_t *bulk_transfer) {
 //close transfer on controller
 usb_devices[device_address].close_bulk_transfer(device_address, bulk_transfer);

 //free memory of structures that described transfer
 free((void *)bulk_transfer->tms_pointer);
 bulk_transfer->tms_pointer = 0;

 //bulk transfer is now closed
 bulk_transfer->is_running = STATUS_FALSE;
}