//BleskOS

/*
* MIT License
* Copyright (c) 2023-2025 BleskOS developers
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

byte_t read_ps2_data(void) {
 for(int i=0; i<200; i++) {
  wait(1);
  if((inb(0x64) & 0x1)==0x1) {
   break;
  }
 }
 
 return inb(0x60);
}

void write_ps2_data(byte_t value) {
 for(int i=0; i<200; i++) {
  wait(1);
  if((inb(0x64) & 0x2)==0x0) {
   break;
  }
 }
 
 outb(0x60, value);
}

void write_ps2_command(byte_t value) {
 for(int i=0; i<200; i++) {
  wait(1);
  if((inb(0x64) & 0x2)==0x0) {
   break;
  }
 }
 
 outb(0x64, value);
}

void write_to_first_ps2_channel(byte_t value) {
 ps2_first_channel_buffer_pointer = 0;
 ps2_first_channel_buffer[0] = 0;
 write_ps2_data(value);
}

byte_t ps2_first_channel_wait_for_ack(void) {
 ticks = 0;
 while(ticks<200) {
  asm("nop");
  if(ps2_first_channel_buffer_pointer>0) { //we wait for first byte of response
   if(ps2_first_channel_buffer[0]==0xFA) {
    return STATUS_GOOD; //acknowledge sended
   }
   else {
    return STATUS_ERROR; //acknowledge was not sended
   }
  }
 }

 return STATUS_ERROR;
}

byte_t ps2_first_channel_wait_for_response(void) {
 ticks = 0;
 while(ticks<250) { //processing command, especially reset can took some time, so we wait 500 milliseconds
  asm("nop");
  if(ps2_first_channel_buffer_pointer>1) {
   wait(20); //when we receive first byte of response, we wait longer for receiving all bytes that device sends
   return STATUS_GOOD;
  }
 }

 return STATUS_ERROR;
}

void write_to_second_ps2_channel(byte_t value) {
 ps2_second_channel_buffer_pointer = 0;
 ps2_second_channel_buffer[0] = 0;
 write_ps2_command(0xD4);
 write_ps2_data(value);
}

byte_t ps2_second_channel_wait_for_ack(void) {
 ticks = 0;
 while(ticks<200) {
  asm("nop");
  if(ps2_second_channel_buffer_pointer>0) { //we wait for second byte of response
   if(ps2_second_channel_buffer[0]==0xFA) {
    return STATUS_GOOD; //acknowledge sended
   }
   else {
    return STATUS_ERROR; //acknowledge was not sended
   }
  }
 }

 return STATUS_ERROR;
}

byte_t ps2_second_channel_wait_for_response(void) {
 ticks = 0;
 while(ticks<250) { //processing command, especially reset can took some time, so we wait 500 milliseconds
  asm("nop");
  if(ps2_second_channel_buffer_pointer>1) {
   wait(20); //when we receive first byte of response, we wait longer for receiving all bytes that device sends
   return STATUS_GOOD;
  }
 }

 return STATUS_ERROR;
}

void initalize_ps2_controller(void) {
 byte_t controller_configuration_byte = 0;

 //check presence of PS/2 controller
 if(ps2_controller_present==DEVICE_NOT_PRESENT) {
  return;
 }
 else {
  log("\n\nPS/2 controller ");
 }

 //initalize variables
 ps2_first_channel_present = DEVICE_PRESENCE_IS_NOT_KNOWN;
 ps2_first_channel_device = PS2_CHANNEL_NO_DEVICE_CONNECTED;
 ps2_first_channel_buffer_pointer = 0;
 ps2_second_channel_present = DEVICE_PRESENCE_IS_NOT_KNOWN;
 ps2_second_channel_device = PS2_CHANNEL_NO_DEVICE_CONNECTED;
 ps2_second_channel_buffer_pointer = 0;
 
 //disable PS/2 channels
 write_ps2_command(0xAD);
 write_ps2_command(0xA7);

 //clear buffer of data port
 for(dword_t i=0; i<1000; i++) {
  if((inb(0x64) & 0x1)==0x1) { //there are data on data port
   inb(0x60);
  }
  else {
   break;
  }
 }
 if((inb(0x64) & 0x1)==0x1) { //error: data port was not cleared
  log("has locked data port");
  return;
 }

 //disable interrupts of both channels
 write_ps2_command(0x20);
 controller_configuration_byte = read_ps2_data();
 write_ps2_command(0x60);
 write_ps2_data(controller_configuration_byte & ~((1<<0) | (1<<1)));

 //check presence of channels
 if((controller_configuration_byte & (1<<4))==0) { //first channel should be disabled, if it is not, it can not exist
  ps2_first_channel_present = DEVICE_NOT_PRESENT;
 }
 if((controller_configuration_byte & (1<<5))==0) { //second channel should be disabled, if it is not, it can not exist
  ps2_second_channel_present = DEVICE_NOT_PRESENT;
 }

 //perform controller self-test
 write_ps2_command(0xAA);
 if(read_ps2_data()==0x55) {
  //self-test succesfull
  log("self-test succesfull");
 }
 else {
  //self-test failed
  log("self-test failed");
  ps2_first_channel_present = DEVICE_NOT_PRESENT;
  ps2_second_channel_present = DEVICE_NOT_PRESENT;
  return;
 }

 //enable PS/2 channels
 write_ps2_command(0xAE);
 write_ps2_command(0xA8);

 //check presence of channels
 write_ps2_command(0x20);
 controller_configuration_byte = read_ps2_data();
 if(ps2_first_channel_present==DEVICE_PRESENCE_IS_NOT_KNOWN) { //we do not know first channel presence yet
  if((controller_configuration_byte & (1<<4))==0) {
   ps2_first_channel_present = DEVICE_PRESENT; //first channel is enabled, so it exist
  }
  else {
   ps2_first_channel_present = DEVICE_NOT_PRESENT; //first channel is disabled, so it do not exist
  }
 }
 if(ps2_second_channel_present==DEVICE_PRESENCE_IS_NOT_KNOWN) { //we do not know second channel presence yet
  if((controller_configuration_byte & (1<<5))==0) {
   ps2_second_channel_present = DEVICE_PRESENT; //second channel is enabled, so it exist
  }
  else {
   ps2_second_channel_present = DEVICE_NOT_PRESENT; //second channel is disabled, so it do not exist
  }
 }

 //disable PS/2 channels
 write_ps2_command(0xAD);
 write_ps2_command(0xA7);

 //test first channel
 if(ps2_first_channel_present==DEVICE_PRESENT) {
  write_ps2_command(0xAB);
  if(read_ps2_data()==0x00) { //test successfull
   log("\nfirst channel test successfull");
  }
  else { //test failed
   log("\nfirst channel test failed");
   ps2_first_channel_present = DEVICE_PRESENT_BUT_ERROR_STATE;
  }
 }

 //test second channel
 if(ps2_second_channel_present==DEVICE_PRESENT) {
  write_ps2_command(0xA9);
  if(read_ps2_data()==0x00) { //test successfull
   log("\nsecond channel test successfull");
  }
  else { //test failed
   log("\nsecond channel test failed");
   ps2_second_channel_present = DEVICE_PRESENT_BUT_ERROR_STATE;
  }
 }

 //enable interrupts
 write_ps2_command(0x20);
 controller_configuration_byte = read_ps2_data();
 if(ps2_first_channel_present==DEVICE_PRESENT) {
  set_irq_handler(1, (dword_t)ps2_first_channel_irq_handler);
  controller_configuration_byte |= ((1<<0) | (1<<6)); //enable interrupt and translation to scan code set 1
 }
 if(ps2_second_channel_present==DEVICE_PRESENT) {
  set_irq_handler(12, (dword_t)ps2_second_channel_irq_handler);
  controller_configuration_byte |= (1<<1); //enable interrupt
 }
 write_ps2_command(0x60);
 write_ps2_data(controller_configuration_byte);

 //enable channels
 if(ps2_first_channel_present==DEVICE_PRESENT) {
  write_ps2_command(0xAE);
  write_to_first_ps2_channel(0xFF); //reset
  if(ps2_first_channel_wait_for_ack()==STATUS_ERROR) {
   log("\nfirst channel no ACK after reset");
  }
  if(ps2_first_channel_wait_for_response()==STATUS_ERROR) {
   log("\nfirst channel no response after reset");
  }
 }
 if(ps2_second_channel_present==DEVICE_PRESENT) {
  write_ps2_command(0xA8);
  write_to_second_ps2_channel(0xFF); //reset
  if(ps2_second_channel_wait_for_ack()==STATUS_ERROR) {
   log("\nsecond channel no ACK after reset");
  }
  if(ps2_second_channel_wait_for_response()==STATUS_ERROR) {
   log("\nsecond channel no response after reset");
  }
 }

 //check what type of device is connected on first channel
 if(ps2_first_channel_present==DEVICE_PRESENT) {
  //after reset device should not be in streaming mode, but to be sure disable streaming
  write_to_first_ps2_channel(0xF5);
  if(ps2_first_channel_wait_for_ack()==STATUS_ERROR) {
   log("\nfirst channel no ACK after 0xF5");
  }

  //read device ID
  ps2_first_channel_buffer[1] = 0xFF;
  write_to_first_ps2_channel(0xF2);
  log("\n");
  if(ps2_first_channel_wait_for_ack()==STATUS_GOOD) {
   if(ps2_first_channel_wait_for_response()==STATUS_GOOD) {
    if(ps2_first_channel_buffer[1]==0xAB || ps2_first_channel_buffer[1]==0xAC) {
     ps2_first_channel_device = PS2_CHANNEL_KEYBOARD_CONNECTED;
    }
    else {
     ps2_first_channel_device = ps2_first_channel_buffer[1]; //unknown device
    }

    log("first channel device ID: ");
    for(dword_t i=1; i<ps2_first_channel_buffer_pointer; i++) {
     log_hex_specific_size_with_space(ps2_first_channel_buffer[i], 2);
    }
   }
   else {
    log("first channel device did not send ID data");
   }
  }
  else {
   log("first channel device did not send ID ack");
  }
 }

 //check what type of device is connected on second channel
 if(ps2_second_channel_present==DEVICE_PRESENT) {
  //after reset device should not be in streaming mode, but to be sure disable streaming
  write_to_second_ps2_channel(0xF5);
  if(ps2_second_channel_wait_for_ack()==STATUS_ERROR) {
   log("\nsecond channel no ACK after 0xF5");
  }

  //read device ID
  ps2_second_channel_buffer[1] = 0xFF;
  write_to_second_ps2_channel(0xF2);
  log("\n");
  if(ps2_second_channel_wait_for_ack()==STATUS_GOOD) {
   if(ps2_second_channel_wait_for_response()==STATUS_GOOD) {
    if(ps2_second_channel_buffer[1]==0x00 || ps2_second_channel_buffer[1]==0x03 || ps2_second_channel_buffer[1]==0x04) {
     ps2_second_channel_device = PS2_CHANNEL_MOUSE_CONNECTED;
    }
    else {
     ps2_second_channel_device = ps2_second_channel_buffer[1]; //unknown device
    }
   
    log("second channel device ID: ");
    for(dword_t i=1; i<ps2_second_channel_buffer_pointer; i++) {
     log_hex_specific_size_with_space(ps2_second_channel_buffer[i], 2);
    }
   }
   else {
    log("second channel device did not send ID data");
   }
  }
  else {
   log("second channel device did not send ID ack");
  }
 }
}

void ps2_first_channel_irq_handler(void) {
 //read data
 ps2_first_channel_buffer[ps2_first_channel_buffer_pointer] = inb(0x60);

 //move pointer
 ps2_first_channel_buffer_pointer++;
 if(ps2_first_channel_buffer_pointer>=10) {
  log("\nERROR: PS/2 first channel buffer is full");
  ps2_first_channel_buffer_pointer = 0;
 }

 //process data
 if(ps2_first_channel_device==PS2_CHANNEL_KEYBOARD_INITALIZED) {
  dword_t key_value = 0;

  //if data starts with 0xE1, it means that keyboard will send two more bytes
  if(ps2_first_channel_buffer[0]==0xE1) {
   if(ps2_first_channel_buffer_pointer>=3) {
    key_value = (ps2_first_channel_buffer[0] | ps2_first_channel_buffer[1]<<8 | 0xE1<<16);
   }
   else {
    return; //wait for rest of data
   }
  }
  //if data starts with 0xE0, it means that keyboard will send one more byte
  else if(ps2_first_channel_buffer[0]==0xE0) {
   if(ps2_first_channel_buffer_pointer>=2) {
    key_value = (ps2_first_channel_buffer[1] | 0xE0<<8);
   }
   else {
    return; //wait for rest of data
   }
  }
  //normal data in one byte
  else {
   key_value = ps2_first_channel_buffer[0];
  }

  //process pressed key
  keyboard_process_code(key_value);

  //save key to list
  ps2_keyboard_process_key_value(key_value);

  //clear variables
  ps2_first_channel_buffer_pointer = 0;

  return;
 }
}

void ps2_second_channel_irq_handler(void) {
 //read data
 ps2_second_channel_buffer[ps2_second_channel_buffer_pointer] = inb(0x60);

 //move pointer
 ps2_second_channel_buffer_pointer++;
 if(ps2_second_channel_buffer_pointer>=10) {
  log("\nERROR: PS/2 second channel buffer is full");
  ps2_second_channel_buffer_pointer = 0;
 }

 //process data
 if(ps2_second_channel_device==PS2_CHANNEL_MOUSE_INITALIZED && ps2_mouse_enable==STATUS_TRUE) {
  if(ps2_second_channel_buffer_pointer >= ps2_second_channel_mouse_data_bytes) {
   //if code waits for data from mouse, copy them
   if(ps2_mouse_wait==1) {
    for(dword_t i=0; i<ps2_second_channel_mouse_data_bytes; i++) {
     ps2_mouse_data[i] = ps2_second_channel_buffer[i];
    }
   }

   //set variables
   ps2_second_channel_buffer_pointer = 0;
   ps2_mouse_wait = 0;
  }
 }
}