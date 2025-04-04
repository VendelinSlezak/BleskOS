//BleskOS

/*
* MIT License
* Copyright (c) 2023-2025 BleskOS developers
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

void hda_add_new_pci_device(struct pci_device_info_t device) {
    // check number of already connected cards
    if(components->n_hda >= MAX_NUMBER_OF_HDA_SOUND_CARDS) {
        return;
    }

    // log driver
    logf("%s", __FILE__);

    // save basic device informations
    copy_memory((dword_t)&device, (dword_t)&components->hda[components->n_hda].pci, sizeof(struct pci_device_info_t));

    // read other device informations
    components->hda[components->n_hda].base = pci_get_mmio(device, PCI_BAR0);

    // configure PCI
    pci_set_bits(device, 0x04, PCI_STATUS_BUSMASTERING | PCI_STATUS_MMIO);

    // update number of devices
    components->p_sound_card = 1;
    components->n_hda++;
}

void hda_initalize_sound_card(dword_t sound_card_number) {
    selected_hda_card = sound_card_number;
    //log
    logf("\n\nDriver: High Definition Audio\nDevice: PCI bus %d:%d:%d:%d",
        components->hda[sound_card_number].pci.segment,
        components->hda[sound_card_number].pci.bus,
        components->hda[sound_card_number].pci.device,
        components->hda[sound_card_number].pci.function);
    dword_t hda_base = components->hda[sound_card_number].base;
    components->hda[sound_card_number].communication_type = HDA_UNINITALIZED;
    components->hda[sound_card_number].is_initalized_useful_output = STATUS_FALSE;

 //reset card and set operational state
 mmio_outd(hda_base + 0x08, 0x0);
 ticks = 0;
 while(ticks<10) {
  asm("nop");
  if((mmio_ind(hda_base + 0x08) & 0x1)==0x0) {
   break;
  }
 }
 mmio_outd(hda_base + 0x08, 0x1);
 ticks = 0;
 while(ticks<10) {
  asm("nop");
  if((mmio_ind(hda_base + 0x08) & 0x1)==0x1) {
   break;
  }
 }
 if((mmio_ind(hda_base + 0x08) & 0x1)==0x0) {
  logf("\nHDA ERROR: card can not be set to operational state");
  return;
 }

 //read capabilities
 logf("\nVersion: %d.%d", mmio_inb(hda_base + 0x03), mmio_inb(hda_base + 0x02));
 components->hda[sound_card_number].input_stream_base = (hda_base + 0x80);
 components->hda[sound_card_number].output_stream_base = (hda_base + 0x80 + (0x20*((mmio_inw(hda_base + 0x00)>>8) & 0xF))); //skip input streams ports
 components->hda[sound_card_number].output_buffer_list = (dword_t *) (aligned_calloc(16*2, 0x7F));

 //disable interrupts
 mmio_outd(hda_base + 0x20, 0);
 
 //turn off dma position transfer
 mmio_outd(hda_base + 0x70, 0);
 mmio_outd(hda_base + 0x74, 0);
 
 //disable synchronization
 mmio_outd(hda_base + 0x34, 0);
 mmio_outd(hda_base + 0x38, 0);

 //stop CORB and RIRB
 mmio_outb(hda_base + 0x4C, 0x0);
 mmio_outb(hda_base + 0x5C, 0x0);
 
 //configure CORB
 components->hda[sound_card_number].corb_mem = (dword_t *) (aligned_calloc(256*4, 0x7F));
 mmio_outd(hda_base + 0x40, (dword_t)components->hda[sound_card_number].corb_mem); //CORB lower memory
 mmio_outd(hda_base + 0x44, 0); //CORB upper memory
 if((mmio_inb(hda_base + 0x4E) & 0x40)==0x40) {
  components->hda[sound_card_number].corb_number_of_entries = 256;
  mmio_outb(hda_base + 0x4E, 0x2); //256 entries
  logf("\nCORB: 256 entries");
 }
 else if((mmio_inb(hda_base + 0x4E) & 0x20)==0x20) {
  components->hda[sound_card_number].corb_number_of_entries = 16;
  mmio_outb(hda_base + 0x4E, 0x1); //16 entries
  logf("\nCORB: 16 entries");
 }
 else if((mmio_inb(hda_base + 0x4E) & 0x10)==0x10) {
  components->hda[sound_card_number].corb_number_of_entries = 2;
  mmio_outb(hda_base + 0x4E, 0x0); //2 entries
  logf("\nCORB: 2 entries");
 }
 else { //CORB/RIRB is not supported
  logf("\nCORB: no size allowed");
  goto hda_use_pio_interface;
 }
 mmio_outw(hda_base + 0x4A, 0x8000); //reset read pointer
 ticks = 0;
 while(ticks<5) {
  asm("nop");
  if((mmio_inw(hda_base + 0x4A) & 0x8000)==0x8000) { //wait until reset is complete
   break;
  }
 }
 if((mmio_inw(hda_base + 0x4A) & 0x8000)==0x0000) { //CORB read pointer was not reseted
  logf("\nHDA: CORB pointer can not be put to reset state");
  goto hda_use_pio_interface;
 }
 mmio_outw(hda_base + 0x4A, 0x0000); //go back to normal state
 ticks = 0;
 while(ticks<5) {
  asm("nop");
  if((mmio_inw(hda_base + 0x4A) & 0x8000)==0x0000) { //wait until is CORB read pointer in normal state
   break;
  }
 }
 if((mmio_inw(hda_base + 0x4A) & 0x8000)==0x8000) { //CORB read pointer is still in reset
  logf("\nHDA: CORB pointer can not be put from reset state");
  goto hda_use_pio_interface;
 }
 mmio_outw(hda_base + 0x48, 0); //set write pointer
 components->hda[sound_card_number].corb_pointer = 1;
 
 //configure RIRB
 components->hda[sound_card_number].rirb_mem = (dword_t *) (aligned_calloc(256*8, 0x7F));
 mmio_outd(hda_base + 0x50, (dword_t)components->hda[sound_card_number].rirb_mem); //RIRB lower memory
 mmio_outd(hda_base + 0x54, 0); //RIRB upper memory
 if((mmio_inb(hda_base + 0x5E) & 0x40)==0x40) {
  components->hda[sound_card_number].rirb_number_of_entries = 256;
  mmio_outb(hda_base + 0x5E, 0x2); //256 entries
  logf("\nRIRB: 256 entries");
 }
 else if((mmio_inb(hda_base + 0x5E) & 0x20)==0x20) {
  components->hda[sound_card_number].rirb_number_of_entries = 16;
  mmio_outb(hda_base + 0x5E, 0x1); //16 entries
  logf("\nRIRB: 16 entries");
 }
 else if((mmio_inb(hda_base + 0x5E) & 0x10)==0x10) {
  components->hda[sound_card_number].rirb_number_of_entries = 2;
  mmio_outb(hda_base + 0x5E, 0x0); //2 entries
  logf("\nRIRB: 2 entries");
 }
 else { //CORB/RIRB is not supported
  logf("\nRIRB: no size allowed");
  goto hda_use_pio_interface;
 }
 mmio_outw(hda_base + 0x58, 0x8000); //reset write pointer
 wait(10);
 mmio_outw(hda_base + 0x5A, 0); //disable interrupts
 components->hda[sound_card_number].rirb_pointer = 1;
 
 //start CORB and RIRB
 mmio_outb(hda_base + 0x4C, 0x2);
 mmio_outb(hda_base + 0x5C, 0x2);

 //find codec and working communication interface
 //TODO: find more codecs
 for(dword_t codec_number = 0, codec_id = 0; codec_number < 16; codec_number++) {
  components->hda[sound_card_number].communication_type = HDA_CORB_RIRB;
  codec_id = hda_send_verb(codec_number, 0, 0xF00, 0);

  if(codec_id != 0) {
   logf("\nHDA: CORB/RIRB communication interface");
   hda_initalize_codec(sound_card_number, codec_number);
   return; //initalization is complete
  }
 }

 hda_use_pio_interface:
 //stop CORB and RIRB
 mmio_outb(hda_base + 0x4C, 0x0);
 mmio_outb(hda_base + 0x5C, 0x0);

 for(dword_t codec_number = 0, codec_id = 0; codec_number < 16; codec_number++) {
  components->hda[sound_card_number].communication_type = HDA_PIO;
  codec_id = hda_send_verb(codec_number, 0, 0xF00, 0);

  if(codec_id != 0) {
   logf("\nHDA: PIO communication interface");
   hda_initalize_codec(sound_card_number, codec_number);
   return; //initalization is complete
  }
 }
}

dword_t hda_send_verb(dword_t codec, dword_t node, dword_t verb, dword_t command) {
 dword_t value = ((codec<<28) | (node<<20) | (verb<<8) | (command));
 
 if(components->hda[selected_hda_card].communication_type==HDA_CORB_RIRB) { //CORB/RIRB interface
  //write verb
  components->hda[selected_hda_card].corb_mem[components->hda[selected_hda_card].corb_pointer] = value;
  
  //move write pointer
  mmio_outw(components->hda[selected_hda_card].base + 0x48, components->hda[selected_hda_card].corb_pointer);
  
  //wait for response
  ticks = 0;
  while(ticks<5) {
   asm("nop");
   if(mmio_inw(components->hda[selected_hda_card].base + 0x58)==components->hda[selected_hda_card].corb_pointer) {
    break;
   }
  }
  if(mmio_inw(components->hda[selected_hda_card].base + 0x58)!=components->hda[selected_hda_card].corb_pointer) {
   logf("\nHDA ERROR: no response");
   components->hda[selected_hda_card].communication_type = HDA_UNINITALIZED;
   return STATUS_ERROR;
  }
  
  //read response
  value = components->hda[selected_hda_card].rirb_mem[components->hda[selected_hda_card].rirb_pointer*2];

  //move pointers
  components->hda[selected_hda_card].corb_pointer++;
  if(components->hda[selected_hda_card].corb_pointer == components->hda[selected_hda_card].corb_number_of_entries) {
   components->hda[selected_hda_card].corb_pointer = 0;
  }
  components->hda[selected_hda_card].rirb_pointer++;
  if(components->hda[selected_hda_card].rirb_pointer == components->hda[selected_hda_card].rirb_number_of_entries) {
   components->hda[selected_hda_card].rirb_pointer = 0;
  }

  //return response
  return value;
 }
 else if(components->hda[selected_hda_card].communication_type==HDA_PIO) { //PIO interface
  //clear Immediate Result Valid bit
  mmio_outw(components->hda[selected_hda_card].base + 0x68, 0x2);

  //write verb
  mmio_outd(components->hda[selected_hda_card].base + 0x60, value);

  //start verb transfer
  mmio_outw(components->hda[selected_hda_card].base + 0x68, 0x1);

  //pool for response
  ticks = 0;
  while(ticks<3) {
   asm("nop");

   //wait for Immediate Result Valid bit = set and Immediate Command Busy bit = clear
   if((mmio_inw(components->hda[selected_hda_card].base + 0x68) & 0x3)==0x2) {
    //clear Immediate Result Valid bit
    mmio_outw(components->hda[selected_hda_card].base + 0x68, 0x2);

    //return response
    return mmio_ind(components->hda[selected_hda_card].base + 0x64);
   }
  }
  
  //there was no response after 6 ms
  logf("\nHDA ERROR: no response");
  components->hda[selected_hda_card].communication_type = HDA_UNINITALIZED;
  return STATUS_ERROR;
 }

 return STATUS_ERROR;
}

byte_t hda_get_node_type(dword_t codec, dword_t node) {
 return ((hda_send_verb(codec, node, 0xF00, 0x09) >> 20) & 0xF);
}

word_t hda_get_node_connection_entry(dword_t codec, dword_t node, dword_t connection_entry_number) {
 //read connection capabilities
 dword_t connection_list_capabilities = hda_send_verb(codec, node, 0xF00, 0x0E);

 //test if this connection even exist
 if(connection_entry_number >= (connection_list_capabilities & 0x7F)) {
  return 0x0000;
 }

 //return number of connected node
 if((connection_list_capabilities & 0x80) == 0x00) { //short form
  return ((hda_send_verb(codec, node, 0xF02, ((connection_entry_number/4)*4)) >> ((connection_entry_number%4)*8)) & 0xFF);
 }
 else { //long form
  return ((hda_send_verb(codec, node, 0xF02, ((connection_entry_number/2)*2)) >> ((connection_entry_number%2)*16)) & 0xFFFF);
 }
}

void hda_set_node_gain(dword_t codec, dword_t node, dword_t node_type, dword_t capabilities, dword_t gain) {
 //this will apply to left and right
 dword_t payload = 0x3000;

 //set type of node
 if((node_type & HDA_OUTPUT_NODE) == HDA_OUTPUT_NODE) {
  payload |= 0x8000;
 }
 if((node_type & HDA_INPUT_NODE) == HDA_INPUT_NODE) {
  payload |= 0x4000;
 }

 //set number of gain
 if(gain == 0 && (capabilities & 0x80000000) == 0x80000000) {
  payload |= 0x80; //mute
 }
 else {
  payload |= (((capabilities>>8) & 0x7F)*gain/100); //recalculate range 0-100 to range of node steps
 }

 //change gain
 hda_send_verb(codec, node, 0x300, payload);
}

void hda_enable_pin_output(dword_t codec, dword_t pin_node) {
 hda_send_verb(codec, pin_node, 0x707, (hda_send_verb(codec, pin_node, 0xF07, 0x00) | 0x40));
}

void hda_disable_pin_output(dword_t codec, dword_t pin_node) {
 hda_send_verb(codec, pin_node, 0x707, (hda_send_verb(codec, pin_node, 0xF07, 0x00) & ~0x40));
}

byte_t hda_is_headphone_connected(void) {
 if(components->hda[selected_hda_card].pin_headphone_node_number != 0
    && (hda_send_verb(components->hda[selected_hda_card].codec_number, components->hda[selected_hda_card].pin_headphone_node_number, 0xF09, 0x00) & 0x80000000) == 0x80000000) {
  return STATUS_TRUE;
 }
 else {
  return STATUS_FALSE;
 }
}

void hda_initalize_codec(dword_t sound_card_number, dword_t codec_number) {
 //test if this codec exist
 dword_t codec_id = hda_send_verb(codec_number, 0, 0xF00, 0);
 if(codec_id == 0x00000000) {
  return;
 }
 components->hda[sound_card_number].codec_number = codec_number;

 //log basic codec info
 logf("\nCodec %d ", codec_number);
 logf("\nVendor: %04x %s", (codec_id>>16), (pci_get_vendor_name(codec_id>>16)));
 logf("\nNumber: %04x", (codec_id & 0xFFFF), 4);

 //find Audio Function Groups
 dword_t subordinate_node_count_reponse = hda_send_verb(codec_number, 0, 0xF00, 0x04);
 logf("\nFirst Group node: %d Number of Groups: %d", (subordinate_node_count_reponse>>16) & 0xFF, subordinate_node_count_reponse & 0xFF);
 for(dword_t node = ((subordinate_node_count_reponse>>16) & 0xFF), last_node = (node+(subordinate_node_count_reponse & 0xFF)); node<last_node; node++) {
  if((hda_send_verb(codec_number, node, 0xF00, 0x05) & 0x7F)==0x01) { //this is Audio Function Group
   hda_initalize_audio_function_group(sound_card_number, node); //initalize Audio Function Group
   return;
  }
 }
 logf("\nHDA ERROR: No AFG founded");
}

void hda_initalize_audio_function_group(dword_t sound_card_number, dword_t afg_node_number) {
 //reset AFG
 hda_send_verb(components->hda[sound_card_number].codec_number, afg_node_number, 0x7FF, 0x00);

 //enable power for AFG
 hda_send_verb(components->hda[sound_card_number].codec_number, afg_node_number, 0x705, 0x00);

 //disable unsolicited responses
 hda_send_verb(components->hda[sound_card_number].codec_number, afg_node_number, 0x708, 0x00);

 //read available info
 components->hda[sound_card_number].afg_node_sample_capabilities = hda_send_verb(components->hda[sound_card_number].codec_number, afg_node_number, 0xF00, 0x0A);
 components->hda[sound_card_number].afg_node_stream_format_capabilities = hda_send_verb(components->hda[sound_card_number].codec_number, afg_node_number, 0xF00, 0x0B);
 components->hda[sound_card_number].afg_node_input_amp_capabilities = hda_send_verb(components->hda[sound_card_number].codec_number, afg_node_number, 0xF00, 0x0D);
 components->hda[sound_card_number].afg_node_output_amp_capabilities = hda_send_verb(components->hda[sound_card_number].codec_number, afg_node_number, 0xF00, 0x12);

 //log AFG info
 logf("\nAudio Function Group node %d", afg_node_number);
 logf("\nAFG sample capabilities: 0x%x", components->hda[sound_card_number].afg_node_sample_capabilities);
 logf("\nAFG stream format capabilities: 0x%x", components->hda[sound_card_number].afg_node_stream_format_capabilities);
 logf("\nAFG input amp capabilities: 0x%x", components->hda[sound_card_number].afg_node_input_amp_capabilities);
 logf("\nAFG output amp capabilities: 0x%x", components->hda[sound_card_number].afg_node_output_amp_capabilities);

 //log all AFG nodes and find useful PINs
 logf("\n\nLIST OF ALL NODES IN AFG:");
 dword_t subordinate_node_count_reponse = hda_send_verb(components->hda[sound_card_number].codec_number, afg_node_number, 0xF00, 0x04);
 dword_t pin_alternative_output_node_number = 0, pin_speaker_default_node_number = 0, pin_speaker_node_number = 0, pin_headphone_node_number = 0;
 components->hda[sound_card_number].pin_output_node_number = 0;
 components->hda[sound_card_number].pin_headphone_node_number = 0;
 for(dword_t node = ((subordinate_node_count_reponse>>16) & 0xFF), last_node = (node+(subordinate_node_count_reponse & 0xFF)), type_of_node = 0; node < last_node; node++) {
  //log number of node
  logf("\n%d ", node);
  
  //get type of node
  type_of_node = hda_get_node_type(components->hda[sound_card_number].codec_number, node);

  //process node
  if(type_of_node==HDA_WIDGET_AUDIO_OUTPUT) {
   logf("Audio Output");

   //disable every audio output by connecting it to stream 0
   hda_send_verb(components->hda[sound_card_number].codec_number, node, 0x706, 0x00);
  }
  else if(type_of_node==HDA_WIDGET_AUDIO_INPUT) {
   logf("Audio Input");
  }
  else if(type_of_node==HDA_WIDGET_AUDIO_MIXER) {
   logf("Audio Mixer");
  }
  else if(type_of_node==HDA_WIDGET_AUDIO_SELECTOR) {
   logf("Audio Selector");
  }
  else if(type_of_node==HDA_WIDGET_PIN_COMPLEX) {
   logf("Pin Complex ");

   //read type of PIN
   type_of_node = ((hda_send_verb(components->hda[sound_card_number].codec_number, node, 0xF1C, 0x00) >> 20) & 0xF);
   if(type_of_node==HDA_PIN_LINE_OUT) {
    logf("Line Out");

    //save this node, this variable contain number of last alternative output
    pin_alternative_output_node_number = node;
   }
   else if(type_of_node==HDA_PIN_SPEAKER) {
    logf("Speaker ");

    //first speaker node is default speaker
    if(pin_speaker_default_node_number==0) {
     pin_speaker_default_node_number = node;
    }

    //find if there is device connected to this PIN
    if((hda_send_verb(components->hda[sound_card_number].codec_number, node, 0xF00, 0x09) & 0x4)==0x4) {
     //find if it is jack or fixed device
     if((hda_send_verb(components->hda[sound_card_number].codec_number, node, 0xF1C, 0x00)>>30)!=0x1) {
      //find if is device output capable
      if((hda_send_verb(components->hda[sound_card_number].codec_number, node, 0xF00, 0x0C) & 0x10)==0x10) {
       //there is connected device
       logf("connected output device");

       //we will use first pin with connected device, so save node number only for first PIN
       if(pin_speaker_node_number==0) {
        pin_speaker_node_number = node;
       }
      }
      else {
       logf("not output capable");
      }
     }
     else {
      logf("not jack or fixed device");
     }
    }
    else {
     logf("no output device");
    }
   }
   else if(type_of_node==HDA_PIN_HEADPHONE_OUT) {
    logf("Headphone Out");

    //save node number
    //TODO: handle if there are multiple HP nodes
    pin_headphone_node_number = node;
   }
   else if(type_of_node==HDA_PIN_CD) {
    logf("CD");

    //save this node, this variable contain number of last alternative output
    pin_alternative_output_node_number = node;
   }
   else if(type_of_node==HDA_PIN_SPDIF_OUT) {
    logf("SPDIF Out");

    //save this node, this variable contain number of last alternative output
    pin_alternative_output_node_number = node;
   }
   else if(type_of_node==HDA_PIN_DIGITAL_OTHER_OUT) {
    logf("Digital Other Out");

    //save this node, this variable contain number of last alternative output
    pin_alternative_output_node_number = node;
   }
   else if(type_of_node==HDA_PIN_MODEM_LINE_SIDE) {
    logf("Modem Line Side");

    //save this node, this variable contain number of last alternative output
    pin_alternative_output_node_number = node;
   }
   else if(type_of_node==HDA_PIN_MODEM_HANDSET_SIDE) {
    logf("Modem Handset Side");

    //save this node, this variable contain number of last alternative output
    pin_alternative_output_node_number = node;
   }
   else if(type_of_node==HDA_PIN_LINE_IN) {
    logf("Line In");
   }
   else if(type_of_node==HDA_PIN_AUX) {
    logf("AUX");
   }
   else if(type_of_node==HDA_PIN_MIC_IN) {
    logf("Mic In");
   }
   else if(type_of_node==HDA_PIN_TELEPHONY) {
    logf("Telephony");
   }
   else if(type_of_node==HDA_PIN_SPDIF_IN) {
    logf("SPDIF In");
   }
   else if(type_of_node==HDA_PIN_DIGITAL_OTHER_IN) {
    logf("Digital Other In");
   }
   else if(type_of_node==HDA_PIN_RESERVED) {
    logf("Reserved");
   }
   else if(type_of_node==HDA_PIN_OTHER) {
    logf("Other");
   }
  }
  else if(type_of_node==HDA_WIDGET_POWER_WIDGET) {
   logf("Power Widget");
  }
  else if(type_of_node==HDA_WIDGET_VOLUME_KNOB) {
   logf("Volume Knob");
  }
  else if(type_of_node==HDA_WIDGET_BEEP_GENERATOR) {
   logf("Beep Generator");
  }
  else if(type_of_node==HDA_WIDGET_VENDOR_DEFINED) {
   logf("Vendor defined");
  }
  else {
   logf("Reserved type");
  }

  //log all connected nodes
  logf(" ");
  byte_t connection_entry_number = 0;
  word_t connection_entry_node = hda_get_node_connection_entry(components->hda[sound_card_number].codec_number, node, 0);
  while(connection_entry_node!=0x0000) {
   logf("%d ", connection_entry_node);
   connection_entry_number++;
   connection_entry_node = hda_get_node_connection_entry(components->hda[sound_card_number].codec_number, node, connection_entry_number);
  }
 }

 //reset variables of second path
 components->hda[sound_card_number].second_audio_output_node_number = 0;
 components->hda[sound_card_number].second_audio_output_node_sample_capabilities = 0;
 components->hda[sound_card_number].second_audio_output_node_stream_format_capabilities = 0;
 components->hda[sound_card_number].second_output_amp_node_number = 0;
 components->hda[sound_card_number].second_output_amp_node_capabilities = 0;

 //initalize output PINs
 logf("\n");
 if(pin_speaker_default_node_number!=0) {
  //initalize speaker
  components->hda[sound_card_number].is_initalized_useful_output = STATUS_TRUE;
  if(pin_speaker_node_number!=0) {
   logf("\nSpeaker output");
   hda_initalize_output_pin(sound_card_number, pin_speaker_node_number); //initalize speaker with connected output device
   components->hda[sound_card_number].pin_output_node_number = pin_speaker_node_number; //save speaker node number
  }
  else {
   logf("\nDefault speaker output");
   hda_initalize_output_pin(sound_card_number, pin_speaker_default_node_number); //initalize default speaker
   components->hda[sound_card_number].pin_output_node_number = pin_speaker_default_node_number; //save speaker node number
  }

  //save speaker path
  components->hda[sound_card_number].second_audio_output_node_number = components->hda[sound_card_number].audio_output_node_number;
  components->hda[sound_card_number].second_audio_output_node_sample_capabilities = components->hda[sound_card_number].audio_output_node_sample_capabilities;
  components->hda[sound_card_number].second_audio_output_node_stream_format_capabilities = components->hda[sound_card_number].audio_output_node_stream_format_capabilities;
  components->hda[sound_card_number].second_output_amp_node_number = components->hda[sound_card_number].output_amp_node_number;
  components->hda[sound_card_number].second_output_amp_node_capabilities = components->hda[sound_card_number].output_amp_node_capabilities;

  //if codec has also headphone output, initalize it
  if(pin_headphone_node_number!=0) {
   logf("\n\nHeadphone output");
   hda_initalize_output_pin(sound_card_number, pin_headphone_node_number); //initalize headphone output
   components->hda[sound_card_number].pin_headphone_node_number = pin_headphone_node_number; //save headphone node number

   //if first path and second path share nodes, left only info for first path
   if(components->hda[sound_card_number].audio_output_node_number==components->hda[sound_card_number].second_audio_output_node_number) {
    components->hda[sound_card_number].second_audio_output_node_number = 0;
   }
   if(components->hda[sound_card_number].output_amp_node_number==components->hda[sound_card_number].second_output_amp_node_number) {
    components->hda[sound_card_number].second_output_amp_node_number = 0;
   }

   //find headphone connection status
   if(hda_is_headphone_connected()==STATUS_TRUE) {
    hda_disable_pin_output(components->hda[sound_card_number].codec_number, components->hda[sound_card_number].pin_output_node_number);
    components->hda[sound_card_number].selected_output_node = components->hda[sound_card_number].pin_headphone_node_number;
   }
   else {
    components->hda[sound_card_number].selected_output_node = components->hda[sound_card_number].pin_output_node_number;
   }

   //add task for checking headphone connection
   create_task(hda_check_headphone_connection_change, TASK_TYPE_USER_INPUT, 50);
  }
 }
 else if(pin_headphone_node_number!=0) { //codec do not have speaker, but only headphone output
  logf("\nHeadphone output");
  components->hda[sound_card_number].is_initalized_useful_output = STATUS_TRUE;
  hda_initalize_output_pin(sound_card_number, pin_headphone_node_number); //initalize headphone output
  components->hda[sound_card_number].pin_output_node_number = pin_headphone_node_number; //save headphone node number
 }
 else if(pin_alternative_output_node_number!=0) { //codec have only alternative output
  logf("\nAlternative output");
  components->hda[sound_card_number].is_initalized_useful_output = STATUS_FALSE;
  hda_initalize_output_pin(sound_card_number, pin_alternative_output_node_number); //initalize alternative output
  components->hda[sound_card_number].pin_output_node_number = pin_alternative_output_node_number; //save alternative output node number
 }
 else {
  logf("\nCodec do not have any output PINs");
 }
}

void hda_initalize_output_pin(dword_t sound_card_number, dword_t pin_node_number) {
 logf("\nInitalizing PIN %d", pin_node_number);

 //reset variables of first path
 components->hda[sound_card_number].audio_output_node_number = 0;
 components->hda[sound_card_number].audio_output_node_sample_capabilities = 0;
 components->hda[sound_card_number].audio_output_node_stream_format_capabilities = 0;
 components->hda[sound_card_number].output_amp_node_number = 0;
 components->hda[sound_card_number].output_amp_node_capabilities = 0;

 //turn on power for PIN
 hda_send_verb(components->hda[sound_card_number].codec_number, pin_node_number, 0x705, 0x00);

 //disable unsolicited responses
 hda_send_verb(components->hda[sound_card_number].codec_number, pin_node_number, 0x708, 0x00);

 //disable any processing
 hda_send_verb(components->hda[sound_card_number].codec_number, pin_node_number, 0x703, 0x00);

 //enable PIN
 hda_send_verb(components->hda[sound_card_number].codec_number, pin_node_number, 0x707, (hda_send_verb(components->hda[sound_card_number].codec_number, pin_node_number, 0xF07, 0x00) | 0x80 | 0x40));

 //enable EAPD + L-R swap
 hda_send_verb(components->hda[sound_card_number].codec_number, pin_node_number, 0x70C, 0x6);

 //set maximal volume for PIN
 dword_t pin_output_amp_capabilities = hda_send_verb(components->hda[sound_card_number].codec_number, pin_node_number, 0xF00, 0x12);
 hda_set_node_gain(components->hda[sound_card_number].codec_number, pin_node_number, HDA_OUTPUT_NODE, pin_output_amp_capabilities, 100);
 if(pin_output_amp_capabilities!=0) {
  //we will control volume by PIN node
  components->hda[sound_card_number].output_amp_node_number = pin_node_number;
  components->hda[sound_card_number].output_amp_node_capabilities = pin_output_amp_capabilities;
 }

 //start enabling path of nodes
 components->hda[sound_card_number].length_of_node_path = 0;
 hda_send_verb(components->hda[sound_card_number].codec_number, pin_node_number, 0x701, 0x00); //select first node
 dword_t first_connected_node_number = hda_get_node_connection_entry(components->hda[sound_card_number].codec_number, pin_node_number, 0); //get first node number
 dword_t type_of_first_connected_node = hda_get_node_type(components->hda[sound_card_number].codec_number, first_connected_node_number); //get type of first node
 if(type_of_first_connected_node==HDA_WIDGET_AUDIO_OUTPUT) {
  hda_initalize_audio_output(sound_card_number, first_connected_node_number);
 }
 else if(type_of_first_connected_node==HDA_WIDGET_AUDIO_MIXER) {
  hda_initalize_audio_mixer(sound_card_number, first_connected_node_number);
 }
 else if(type_of_first_connected_node==HDA_WIDGET_AUDIO_SELECTOR) {
  hda_initalize_audio_selector(sound_card_number, first_connected_node_number);
 }
 else {
  logf("\nHDA ERROR: PIN have connection %d", first_connected_node_number);
 }
}

void hda_initalize_audio_output(dword_t sound_card_number, dword_t audio_output_node_number) {
 logf("\nInitalizing Audio Output %d", audio_output_node_number);
 components->hda[sound_card_number].audio_output_node_number = audio_output_node_number;

 //turn on power for Audio Output
 hda_send_verb(components->hda[sound_card_number].codec_number, audio_output_node_number, 0x705, 0x00);

 //disable unsolicited responses
 hda_send_verb(components->hda[sound_card_number].codec_number, audio_output_node_number, 0x708, 0x00);

 //disable any processing
 hda_send_verb(components->hda[sound_card_number].codec_number, audio_output_node_number, 0x703, 0x00);

 //connect Audio Output to stream 1 channel 0
 hda_send_verb(components->hda[sound_card_number].codec_number, audio_output_node_number, 0x706, 0x10);

 //set maximal volume for Audio Output
 dword_t audio_output_amp_capabilities = hda_send_verb(components->hda[sound_card_number].codec_number, audio_output_node_number, 0xF00, 0x12);
 hda_set_node_gain(components->hda[sound_card_number].codec_number, audio_output_node_number, HDA_OUTPUT_NODE, audio_output_amp_capabilities, 100);
 if(audio_output_amp_capabilities!=0) {
  //we will control volume by Audio Output node
  components->hda[sound_card_number].output_amp_node_number = audio_output_node_number;
  components->hda[sound_card_number].output_amp_node_capabilities = audio_output_amp_capabilities;
 }

 //read info, if something is not present, take it from AFG node
 dword_t audio_output_sample_capabilities = hda_send_verb(components->hda[sound_card_number].codec_number, audio_output_node_number, 0xF00, 0x0A);
 if(audio_output_sample_capabilities==0) {
  components->hda[sound_card_number].audio_output_node_sample_capabilities = components->hda[sound_card_number].afg_node_sample_capabilities;
 }
 else {
  components->hda[sound_card_number].audio_output_node_sample_capabilities = audio_output_sample_capabilities;
 }
 dword_t audio_output_stream_format_capabilities = hda_send_verb(components->hda[sound_card_number].codec_number, audio_output_node_number, 0xF00, 0x0B);
 if(audio_output_stream_format_capabilities==0) {
  components->hda[sound_card_number].audio_output_node_stream_format_capabilities = components->hda[sound_card_number].afg_node_stream_format_capabilities;
 }
 else {
  components->hda[sound_card_number].audio_output_node_stream_format_capabilities = audio_output_stream_format_capabilities;
 }
 if(components->hda[sound_card_number].output_amp_node_number==0) { //if nodes in path do not have output amp capabilities, volume will be controlled by Audio Output node with capabilities taken from AFG node
  components->hda[sound_card_number].output_amp_node_number = audio_output_node_number;
  components->hda[sound_card_number].output_amp_node_capabilities = components->hda[sound_card_number].afg_node_output_amp_capabilities;
 }

 //because we are at end of node path, log all gathered info
 logf("\nSample Capabilites: 0x%x", components->hda[sound_card_number].audio_output_node_sample_capabilities);
 logf("\nStream Format Capabilites: 0x%x", components->hda[sound_card_number].audio_output_node_stream_format_capabilities);
 logf("\nVolume node: %d", components->hda[sound_card_number].output_amp_node_number);
 logf("\nVolume capabilities: 0x%x", components->hda[sound_card_number].output_amp_node_capabilities);
}

void hda_initalize_audio_mixer(dword_t sound_card_number, dword_t audio_mixer_node_number) {
 if(components->hda[sound_card_number].length_of_node_path>=10) {
  logf("\nHDA ERROR: too long path");
  return;
 }
 logf("\nInitalizing Audio Mixer %d", audio_mixer_node_number);

 //turn on power for Audio Mixer
 hda_send_verb(components->hda[sound_card_number].codec_number, audio_mixer_node_number, 0x705, 0x00);

 //disable unsolicited responses
 hda_send_verb(components->hda[sound_card_number].codec_number, audio_mixer_node_number, 0x708, 0x00);

 //set maximal volume for Audio Mixer
 dword_t audio_mixer_amp_capabilities = hda_send_verb(components->hda[sound_card_number].codec_number, audio_mixer_node_number, 0xF00, 0x12);
 hda_set_node_gain(components->hda[sound_card_number].codec_number, audio_mixer_node_number, HDA_OUTPUT_NODE, audio_mixer_amp_capabilities, 100);
 if(audio_mixer_amp_capabilities!=0) {
  //we will control volume by Audio Mixer node
  components->hda[sound_card_number].output_amp_node_number = audio_mixer_node_number;
  components->hda[sound_card_number].output_amp_node_capabilities = audio_mixer_amp_capabilities;
 }

 //continue in path
 components->hda[sound_card_number].length_of_node_path++;
 dword_t first_connected_node_number = hda_get_node_connection_entry(components->hda[sound_card_number].codec_number, audio_mixer_node_number, 0); //get first node number
 dword_t type_of_first_connected_node = hda_get_node_type(components->hda[sound_card_number].codec_number, first_connected_node_number); //get type of first node
 if(type_of_first_connected_node==HDA_WIDGET_AUDIO_OUTPUT) {
  hda_initalize_audio_output(sound_card_number, first_connected_node_number);
 }
 else if(type_of_first_connected_node==HDA_WIDGET_AUDIO_MIXER) {
  hda_initalize_audio_mixer(sound_card_number, first_connected_node_number);
 }
 else if(type_of_first_connected_node==HDA_WIDGET_AUDIO_SELECTOR) {
  hda_initalize_audio_selector(sound_card_number, first_connected_node_number);
 }
 else {
  logf("\nHDA ERROR: Mixer have connection %d", first_connected_node_number);
 }
}

void hda_initalize_audio_selector(dword_t sound_card_number, dword_t audio_selector_node_number) {
 if(components->hda[sound_card_number].length_of_node_path>=10) {
  logf("\nHDA ERROR: too long path");
  return;
 }
 logf("\nInitalizing Audio Selector %d", audio_selector_node_number);

 //turn on power for Audio Selector
 hda_send_verb(components->hda[sound_card_number].codec_number, audio_selector_node_number, 0x705, 0x00);

 //disable unsolicited responses
 hda_send_verb(components->hda[sound_card_number].codec_number, audio_selector_node_number, 0x708, 0x00);

 //disable any processing
 hda_send_verb(components->hda[sound_card_number].codec_number, audio_selector_node_number, 0x703, 0x00);

 //set maximal volume for Audio Selector
 dword_t audio_selector_amp_capabilities = hda_send_verb(components->hda[sound_card_number].codec_number, audio_selector_node_number, 0xF00, 0x12);
 hda_set_node_gain(components->hda[sound_card_number].codec_number, audio_selector_node_number, HDA_OUTPUT_NODE, audio_selector_amp_capabilities, 100);
 if(audio_selector_amp_capabilities!=0) {
  //we will control volume by Audio Selector node
  components->hda[sound_card_number].output_amp_node_number = audio_selector_node_number;
  components->hda[sound_card_number].output_amp_node_capabilities = audio_selector_amp_capabilities;
 }

 //continue in path
 components->hda[sound_card_number].length_of_node_path++;
 hda_send_verb(components->hda[sound_card_number].codec_number, audio_selector_node_number, 0x701, 0x00); //select first node
 dword_t first_connected_node_number = hda_get_node_connection_entry(components->hda[sound_card_number].codec_number, audio_selector_node_number, 0); //get first node number
 dword_t type_of_first_connected_node = hda_get_node_type(components->hda[sound_card_number].codec_number, first_connected_node_number); //get type of first node
 if(type_of_first_connected_node==HDA_WIDGET_AUDIO_OUTPUT) {
  hda_initalize_audio_output(sound_card_number, first_connected_node_number);
 }
 else if(type_of_first_connected_node==HDA_WIDGET_AUDIO_MIXER) {
  hda_initalize_audio_mixer(sound_card_number, first_connected_node_number);
 }
 else if(type_of_first_connected_node==HDA_WIDGET_AUDIO_SELECTOR) {
  hda_initalize_audio_selector(sound_card_number, first_connected_node_number);
 }
 else {
  logf("\nHDA ERROR: Selector have connection %d", first_connected_node_number);
 }
}

void hda_set_volume(dword_t sound_card_number, dword_t volume) {
 hda_set_node_gain(components->hda[sound_card_number].codec_number, components->hda[sound_card_number].output_amp_node_number, HDA_OUTPUT_NODE, components->hda[sound_card_number].output_amp_node_capabilities, volume);
 if(components->hda[sound_card_number].second_output_amp_node_number!=0) {
  hda_set_node_gain(components->hda[sound_card_number].codec_number, components->hda[sound_card_number].second_output_amp_node_number, HDA_OUTPUT_NODE, components->hda[sound_card_number].second_output_amp_node_capabilities, volume);
 }
}

void hda_check_headphone_connection_change(void) {
 if(components->hda[selected_hda_card].selected_output_node==components->hda[selected_hda_card].pin_output_node_number && hda_is_headphone_connected()==STATUS_TRUE) { //headphone was connected
  hda_disable_pin_output(components->hda[selected_hda_card].codec_number, components->hda[selected_hda_card].pin_output_node_number);
  components->hda[selected_hda_card].selected_output_node = components->hda[selected_hda_card].pin_headphone_node_number;
 }
 else if(components->hda[selected_hda_card].selected_output_node==components->hda[selected_hda_card].pin_headphone_node_number && hda_is_headphone_connected()==STATUS_FALSE) { //headphone was disconnected
  hda_enable_pin_output(components->hda[selected_hda_card].codec_number, components->hda[selected_hda_card].pin_output_node_number);
  components->hda[selected_hda_card].selected_output_node = components->hda[selected_hda_card].pin_output_node_number;
 }
}

byte_t hda_is_supported_channel_size(dword_t sound_card_number, byte_t size) {
 byte_t channel_sizes[5] = {8, 16, 20, 24, 32};
 dword_t mask=0x00010000;
 
 //get bit of requested size in capabilities
 for(int i=0; i<5; i++) {
  if(channel_sizes[i]==size) {
   break;
  }
  mask <<= 1;
 }
 
 if((components->hda[sound_card_number].audio_output_node_sample_capabilities & mask)==mask) {
  return STATUS_GOOD;
 }
 else {
  return STATUS_ERROR;
 }
}

byte_t hda_is_supported_sample_rate(dword_t sound_card_number, dword_t sample_rate) {
 dword_t sample_rates[11] = {8000, 11025, 16000, 22050, 32000, 44100, 48000, 88200, 96000, 176400, 192000};
 word_t mask=0x0000001;
 
 //get bit of requested sample rate in capabilities
 for(int i=0; i<11; i++) {
  if(sample_rates[i]==sample_rate) {
   break;
  }
  mask <<= 1;
 }
 
 if((components->hda[sound_card_number].audio_output_node_sample_capabilities & mask)==mask) {
  return STATUS_GOOD;
 }
 else {
  return STATUS_ERROR;
 }
}

word_t hda_return_sound_data_format(dword_t sample_rate, dword_t channels, dword_t bits_per_sample) {
 word_t data_format = 0;

 //channels
 data_format = (channels-1);

 //bits per sample
 if(bits_per_sample==16) {
  data_format |= ((0b001)<<4);
 }
 else if(bits_per_sample==20) {
  data_format |= ((0b010)<<4);
 }
 else if(bits_per_sample==24) {
  data_format |= ((0b011)<<4);
 }
 else if(bits_per_sample==32) {
  data_format |= ((0b100)<<4);
 }

 //sample rate
 if(sample_rate==48000) {
  data_format |= ((0b0000000)<<8);
 }
 else if(sample_rate==44100) {
  data_format |= ((0b1000000)<<8);
 }
 else if(sample_rate==32000) {
  data_format |= ((0b0001010)<<8);
 }
 else if(sample_rate==22050) {
  data_format |= ((0b1000001)<<8);
 }
 else if(sample_rate==16000) {
  data_format |= ((0b0000010)<<8);
 }
 else if(sample_rate==11025) {
  data_format |= ((0b1000011)<<8);
 }
 else if(sample_rate==8000) {
  data_format |= ((0b0000101)<<8);
 }
 else if(sample_rate==88200) {
  data_format |= ((0b1001000)<<8);
 }
 else if(sample_rate==96000) {
  data_format |= ((0b0001000)<<8);
 }
 else if(sample_rate==176400) {
  data_format |= ((0b1011000)<<8);
 }
 else if(sample_rate==192000) {
  data_format |= ((0b0011000)<<8);
 }

 return data_format;
}

void hda_play_pcm_data_in_loop(dword_t sound_card_number, dword_t sample_rate) {
 if((components->hda[sound_card_number].audio_output_node_stream_format_capabilities & 0x1)==0x0) {
  return; //this Audio Output do not support PCM sound data
 }

 //stop stream
 mmio_outb(components->hda[sound_card_number].output_stream_base + 0x00, 0x00);
 ticks = 0;
 while(ticks<2) {
  asm("nop");
  if((mmio_inb(components->hda[sound_card_number].output_stream_base + 0x00) & 0x2)==0x0) {
   break;
  }
 }
 if((mmio_inb(components->hda[sound_card_number].output_stream_base + 0x00) & 0x2)==0x2) {
  logf("\nHDA: can not stop stream");
  return;
 }
 
 //reset stream registers
 mmio_outb(components->hda[sound_card_number].output_stream_base + 0x00, 0x01);
 ticks = 0;
 while(ticks<10) {
  asm("nop");
  if((mmio_inb(components->hda[sound_card_number].output_stream_base + 0x00) & 0x1)==0x1) {
   break;
  }
 }
 if((mmio_inb(components->hda[sound_card_number].output_stream_base + 0x00) & 0x1)==0x0) {
  logf("\nHDA: can not start resetting stream");
 }
 wait(5);
 mmio_outb(components->hda[sound_card_number].output_stream_base + 0x00, 0x00);
 ticks = 0;
 while(ticks<10) {
  asm("nop");
  if((mmio_inb(components->hda[sound_card_number].output_stream_base + 0x00) & 0x1)==0x0) {
   break;
  }
 }
 if((mmio_inb(components->hda[sound_card_number].output_stream_base + 0x00) & 0x1)==0x1) {
  logf("\nHDA: can not stop resetting stream");
  return;
 }
 wait(5);

 //clear error bits
 mmio_outb(components->hda[sound_card_number].output_stream_base + 0x03, 0x1C);

 //fill buffer entries - there have to be at least two entries in buffer, so we fill second entry with zeroes
 clear_memory((dword_t)components->hda[sound_card_number].output_buffer_list, 16*2);
 components->hda[sound_card_number].output_buffer_list[0]=((dword_t)pcm_data);
 components->hda[sound_card_number].output_buffer_list[2]=(sound_buffer_refilling_info->buffer_size*2); //from point of view of HDA card there is only one buffer, but thread will recognize first half of buffer as SOUND_BUFFER_0 and second half as SOUND_BUFFER_2
 asm("wbinvd"); //flush processor cache to RAM to be sure sound card will read correct data

 //set buffer registers
 mmio_outd(components->hda[sound_card_number].output_stream_base + 0x18, (dword_t)components->hda[sound_card_number].output_buffer_list);
 mmio_outd(components->hda[sound_card_number].output_stream_base + 0x08, (sound_buffer_refilling_info->buffer_size*2));
 mmio_outw(components->hda[sound_card_number].output_stream_base + 0x0C, 1); //there are two entries in buffer

 //set stream data format
 mmio_outw(components->hda[sound_card_number].output_stream_base + 0x12, hda_return_sound_data_format(sample_rate, 2, 16));

 //set Audio Output node data format
 hda_send_verb(components->hda[sound_card_number].codec_number, components->hda[sound_card_number].audio_output_node_number, 0x200, hda_return_sound_data_format(sample_rate, 2, 16));
 if(components->hda[sound_card_number].second_audio_output_node_number!=0) {
  hda_send_verb(components->hda[sound_card_number].codec_number, components->hda[sound_card_number].second_audio_output_node_number, 0x200, hda_return_sound_data_format(sample_rate, 2, 16));
 }
 wait(10);

 //start streaming to stream 1
 mmio_outb(components->hda[sound_card_number].output_stream_base + 0x02, 0x14);
 mmio_outb(components->hda[sound_card_number].output_stream_base + 0x00, 0x02);
}

void hda_stop_sound(dword_t sound_card_number) {
 mmio_outb(components->hda[sound_card_number].output_stream_base + 0x00, 0x00);
}

dword_t hda_get_actual_stream_position(dword_t sound_card_number) {
 return mmio_ind(components->hda[sound_card_number].output_stream_base + 0x04);
}