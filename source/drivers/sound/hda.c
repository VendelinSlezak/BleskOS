//BleskOS

/*
* MIT License
* Copyright (c) 2023-2024 Vendelín Slezák
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

void hda_initalize_sound_card(dword_t sound_card_number) {
 hda_base = sound_cards_info[sound_card_number].mmio_base;
 hda_communication_type = HDA_UNINITALIZED;

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
  log("\n\nHDA ERROR: card can not be set to operational state");
  return;
 }
 log("\n\nSound card High Definition Audio");

 //read capabilites
 hda_input_stream_base = (hda_base + 0x80);
 hda_output_stream_base = (hda_base + 0x80 + (0x20*((mmio_inw(hda_base + 0x00)>>8) & 0xF)) + 0x20); //skip input streams ports
 hda_output_buffer_list = aligned_calloc(16*2, 0x7F);

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
 hda_corb_mem = aligned_calloc(256*4, 0x7F);
 mmio_outd(hda_base + 0x40, hda_corb_mem); //CORB lower memory
 mmio_outd(hda_base + 0x44, 0); //CORB upper memory
 if((mmio_inb(hda_base + 0x4E) & 0x40)==0x40) {
  hda_corb_entries = 256;
  mmio_outb(hda_base + 0x4E, 0x2); //256 entries
  log("\nCORB: 256 entries");
 }
 else if((mmio_inb(hda_base + 0x4E) & 0x20)==0x20) {
  hda_corb_entries = 256;
  mmio_outb(hda_base + 0x4E, 0x1); //16 entries
  log("\nCORB: 16 entries");
 }
 else if((mmio_inb(hda_base + 0x4E) & 0x10)==0x10) {
  hda_corb_entries = 2;
  mmio_outb(hda_base + 0x4E, 0x0); //2 entries
  log("\nCORB: 2 entries");
 }
 else { //CORB/RIRB is not supported
  log("\nCORB: no size allowed");
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
 if((mmio_inw(hda_base + 0x4A) & 0x8000)==0x0000) { //CORB was not reseted
  log("\nHDA: CORB pointer can not be put to reset state");
  goto hda_use_pio_interface;
 }
 mmio_outw(hda_base + 0x4A, 0x0000); //go back to normal state
 ticks = 0;
 while(ticks<5) {
  asm("nop");
  if((mmio_inw(hda_base + 0x4A) & 0x8000)==0x0000) { //wait until is CORB in normal state
   break;
  }
 }
 if((mmio_inw(hda_base + 0x4A) & 0x8000)==0x8000) { //CORB is still in reset
  log("\nHDA: CORB pointer can not be put from reset state");
  goto hda_use_pio_interface;
 }
 mmio_outw(hda_base + 0x48, 0); //set write pointer
 hda_corb_pointer = 1;
 
 //configure RIRB
 hda_rirb_mem = aligned_calloc(256*8, 0x7F);
 mmio_outd(hda_base + 0x50, hda_rirb_mem); //RIRB lower memory
 mmio_outd(hda_base + 0x54, 0); //RIRB upper memory
 if((mmio_inb(hda_base + 0x5E) & 0x40)==0x40) {
  hda_rirb_entries = 256;
  mmio_outb(hda_base + 0x5E, 0x2); //256 entries
  log("\nRIRB: 256 entries");
 }
 else if((mmio_inb(hda_base + 0x5E) & 0x20)==0x20) {
  hda_rirb_entries = 256;
  mmio_outb(hda_base + 0x5E, 0x1); //16 entries
  log("\nRIRB: 16 entries");
 }
 else if((mmio_inb(hda_base + 0x5E) & 0x10)==0x10) {
  hda_rirb_entries = 2;
  mmio_outb(hda_base + 0x5E, 0x0); //2 entries
  log("\nRIRB: 2 entries");
 }
 else { //CORB/RIRB is not supported
  log("\nRIRB: no size allowed");
  goto hda_use_pio_interface;
 }
 mmio_outw(hda_base + 0x58, 0x8000); //reset write pointer
 wait(10);
 mmio_outw(hda_base + 0x5A, 0); //disable interrupts
 hda_rirb_pointer = 1;
 
 //start CORB and RIRB
 mmio_outb(hda_base + 0x4C, 0x2);
 mmio_outb(hda_base + 0x5C, 0x2);

 //find codec and working communication interface
 //TODO: find more codecs
 hda_is_initalized_useful_output = STATUS_FALSE;
 hda_communication_type = HDA_CORB_RIRB;
 for(dword_t codec_number=0, codec_id=0; codec_number<16; codec_number++) {
  codec_id = hda_send_verb(sound_card_number, codec_number, 0, 0xF00, 0);

  if(codec_id!=0) {
   log("\nHDA: CORB/RIRB communication interface");
   hda_initalize_codec(sound_card_number, codec_number);
   goto hda_initalization_complete;
  }
 }

 hda_use_pio_interface:
 hda_is_initalized_useful_output = STATUS_FALSE;
 hda_communication_type = HDA_PIO;
 for(dword_t codec_number=0, codec_id=0; codec_number<16; codec_number++) {
  codec_id = hda_send_verb(sound_card_number, codec_number, 0, 0xF00, 0);

  if(codec_id!=0) {
   log("\nHDA: PIO communication interface");
   hda_initalize_codec(sound_card_number, codec_number);
   goto hda_initalization_complete;
  }
 }

 //initalization of sound card is complete
 hda_initalization_complete:
 log("\nHDA: card sucessfully initalized");
}

dword_t hda_send_verb(dword_t sound_card_number, dword_t codec, dword_t node, dword_t verb, dword_t command) {
 dword_t *corb = (dword_t *) (hda_corb_mem);
 dword_t *rirb = (dword_t *) (hda_rirb_mem);
 dword_t value = ((codec<<28) | (node<<20) | (verb<<8) | (command));
 
 if(hda_communication_type==HDA_CORB_RIRB) { //CORB/RIRB interface
  //write verb
  corb[hda_corb_pointer] = value;
  
  //move write pointer
  mmio_outw(hda_base + 0x48, hda_corb_pointer);
  
  //wait for response
  ticks = 0;
  while(ticks<5) {
   asm("nop");
   if(mmio_inw(hda_base + 0x58)==hda_corb_pointer) {
    break;
   }
  }
  if(mmio_inw(hda_base + 0x58)!=hda_corb_pointer) {
   return STATUS_ERROR;
  }
  
  //read response
  value = rirb[hda_rirb_pointer*2];
  hda_corb_pointer++;
  if(hda_corb_pointer==hda_corb_entries) {
   hda_corb_pointer = 0;
  }
  hda_rirb_pointer++;
  if(hda_rirb_pointer==hda_rirb_entries) {
   hda_rirb_pointer = 0;
  }
  return value;
 }
 else if(hda_communication_type==HDA_PIO) { //PIO interface
  mmio_outw(hda_base + 0x68, 0x2);
  mmio_outd(hda_base + 0x60, value);
  mmio_outw(hda_base + 0x68, 0x1);
  ticks = 0;
  while(ticks<3) {
   asm("nop");
   if((mmio_inw(hda_base + 0x68) & 0x3)==0x2) {
    value = mmio_ind(hda_base + 0x64);
    mmio_outw(hda_base + 0x68, 0x2);
    return value;
   }
  }
  
  return STATUS_ERROR;
 }

 return STATUS_ERROR;
}

void hda_initalize_codec(dword_t sound_card_number, dword_t codec_number) {
 //test if this codec exist
 dword_t codec_id = hda_send_verb(sound_card_number, codec_number, 0, 0xF00, 0);
 if(codec_id==0) {
  return;
 }
 hda_codec_number = codec_number;

 //log basic codec info
 log("\nCodec ");
 log_var_with_space(codec_number);
 log("\nVendor: ");
 byte_t *vendor_string = (byte_t *) (get_pci_vendor_string(codec_id>>16));
 if(vendor_string[0]==0) {
  log_hex_specific_size((codec_id>>16), 4);
 }
 else {
  log(vendor_string);
 }
 log("\nNumber: ");
 log_hex_specific_size((codec_id & 0xFFFF), 4);

 //find Audio Function Groups
 dword_t subordinate_node_count_reponse = hda_send_verb(sound_card_number, codec_number, 0, 0xF00, 0x04);
 log("\nFirst Group node: ");
 log_var_with_space((subordinate_node_count_reponse>>16) & 0xFF);
 log("Number of Groups: ");
 log_var(subordinate_node_count_reponse & 0xFF);
 for(dword_t node = ((subordinate_node_count_reponse>>16) & 0xFF), last_node = (node+(subordinate_node_count_reponse & 0xFF)); node<last_node; node++) {
  if((hda_send_verb(sound_card_number, codec_number, node, 0xF00, 0x05) & 0x7F)==0x01) { //this is Audio Function Group
   //initalize this AFG
   hda_initalize_audio_function_group(sound_card_number, node);

   //TODO: initalize more AFG than one
   return;
  }
 }
 log("\nHDA ERROR: No AFG founded");
}

void hda_initalize_audio_function_group(dword_t sound_card_number, dword_t afg_node_number) {
 //reset AFG
 hda_send_verb(sound_card_number, hda_codec_number, afg_node_number, 0x7FF, 0x00);

 //enable power for AFG
 hda_send_verb(sound_card_number, hda_codec_number, afg_node_number, 0x705, 0x00);

 //read available info
 hda_afg_node_sample_capabilites = hda_send_verb(sound_card_number, hda_codec_number, afg_node_number, 0xF00, 0x0A);
 hda_afg_node_stream_format_capabilites = hda_send_verb(sound_card_number, hda_codec_number, afg_node_number, 0xF00, 0x0B);
 hda_afg_node_input_amp_capabilites = hda_send_verb(sound_card_number, hda_codec_number, afg_node_number, 0xF00, 0x0D);
 hda_afg_node_output_amp_capabilites = hda_send_verb(sound_card_number, hda_codec_number, afg_node_number, 0xF00, 0x12);

 //log AFG info
 log("\nAudio Function Group node ");
 log_var(afg_node_number);
 log("\nAFG sample capabilites: ");
 log_hex(hda_afg_node_sample_capabilites);
 log("\nAFG stream format capabilites: ");
 log_hex(hda_afg_node_stream_format_capabilites);
 log("\nAFG input amp capabilites: ");
 log_hex(hda_afg_node_input_amp_capabilites);
 log("\nAFG output amp capabilites: ");
 log_hex(hda_afg_node_output_amp_capabilites);

 //log all AFG nodes and find useful PINs
 log(("\n\nLIST OF ALL NODES IN AFG:"));
 dword_t subordinate_node_count_reponse = hda_send_verb(sound_card_number, hda_codec_number, afg_node_number, 0xF00, 0x04);
 dword_t pin_alternative_output_node_number = 0, pin_speaker_default_node_number = 0, pin_speaker_node_number = 0, pin_headphone_node_number = 0;
 for(dword_t node = ((subordinate_node_count_reponse>>16) & 0xFF), last_node = (node+(subordinate_node_count_reponse & 0xFF)), type_of_node = 0; node<last_node; node++) {
  log("\n");
  log_var_with_space(node);
  
  type_of_node = ((hda_send_verb(sound_card_number, hda_codec_number, node, 0xF00, 0x09) >> 20) & 0xF);

  if(type_of_node==HDA_WIDGET_AUDIO_OUTPUT) {
   log("Audio Output");

   //disable every audio output
   hda_send_verb(sound_card_number, hda_codec_number, node, 0x706, 0x00);
  }
  else if(type_of_node==HDA_WIDGET_AUDIO_INPUT) {
   log("Audio Input");
  }
  else if(type_of_node==HDA_WIDGET_AUDIO_MIXER) {
   log("Audio Mixer");
  }
  else if(type_of_node==HDA_WIDGET_AUDIO_SELECTOR) {
   log("Audio Selector");
  }
  else if(type_of_node==HDA_WIDGET_PIN_COMPLEX) {
   log("Pin Complex ");

   //read type of PIN
   type_of_node = ((hda_send_verb(sound_card_number, hda_codec_number, node, 0xF1C, 0x00) >> 20) & 0xF);
   if(type_of_node==HDA_PIN_LINE_OUT) {
    log("Line Out");

    //save this node, this variable contain number of last alternative output
    pin_alternative_output_node_number = node;
   }
   else if(type_of_node==HDA_PIN_SPEAKER) {
    log("Speaker ");

    //first speaker node is default speaker
    if(pin_speaker_node_number==0) {
     pin_speaker_default_node_number = node;
    }

    //find if there is device connected to this PIN
    if((hda_send_verb(sound_card_number, hda_codec_number, node, 0xF00, 0x09) & 0x4)==0x4) {
     //find if it is jack or fixed device
     if((hda_send_verb(sound_card_number, hda_codec_number, node, 0xF1C, 0x00)>>30)!=0x1) {
      //find if is device output capable
      if((hda_send_verb(sound_card_number, hda_codec_number, node, 0xF00, 0x0C) & 0x10)==0x10) {
       //there is connected device
       log("connected output device");

       //we will use first pin with connected device, so save node number only for first PIN
       if(pin_speaker_node_number==0) {
        pin_speaker_node_number = node;
       }
      }
      else {
       log("not output capable");
      }
     }
     else {
      log("not jack or fixed device");
     }
    }
    else {
     log("no output device");
    }
   }
   else if(type_of_node==HDA_PIN_HEADPHONE_OUT) {
    log("Headphone Out");

    //save node number
    //TODO: handle if there are multiple HP nodes
    pin_headphone_node_number = node;
   }
   else if(type_of_node==HDA_PIN_CD) {
    log("CD");

    //save this node, this variable contain number of last alternative output
    pin_alternative_output_node_number = node;
   }
   else if(type_of_node==HDA_PIN_SPDIF_OUT) {
    log("SPDIF Out");

    //save this node, this variable contain number of last alternative output
    pin_alternative_output_node_number = node;
   }
   else if(type_of_node==HDA_PIN_DIGITAL_OTHER_OUT) {
    log("Digital Other Out");

    //save this node, this variable contain number of last alternative output
    pin_alternative_output_node_number = node;
   }
   else if(type_of_node==HDA_PIN_MODEM_LINE_SIDE) {
    log("Modem Line Side");

    //save this node, this variable contain number of last alternative output
    pin_alternative_output_node_number = node;
   }
   else if(type_of_node==HDA_PIN_MODEM_HANDSET_SIDE) {
    log("Modem Handset Side");

    //save this node, this variable contain number of last alternative output
    pin_alternative_output_node_number = node;
   }
   else if(type_of_node==HDA_PIN_LINE_IN) {
    log("Line In");
   }
   else if(type_of_node==HDA_PIN_AUX) {
    log("AUX");
   }
   else if(type_of_node==HDA_PIN_MIC_IN) {
    log("Mic In");
   }
   else if(type_of_node==HDA_PIN_TELEPHONY) {
    log("Telephony");
   }
   else if(type_of_node==HDA_PIN_SPDIF_IN) {
    log("SPDIF In");
   }
   else if(type_of_node==HDA_PIN_DIGITAL_OTHER_IN) {
    log("Digital Other In");
   }
   else if(type_of_node==HDA_PIN_RESERVED) {
    log("Reserved");
   }
   else if(type_of_node==HDA_PIN_OTHER) {
    log("Other");
   }
  }
  else if(type_of_node==HDA_WIDGET_POWER_WIDGET) {
   log("Power Widget");
  }
  else if(type_of_node==HDA_WIDGET_VOLUME_KNOB) {
   log("Volume Knob");
  }
  else if(type_of_node==HDA_WIDGET_BEEP_GENERATOR) {
   log("Beep Generator");
  }
  else if(type_of_node==HDA_WIDGET_VENDOR_DEFINED) {
   log("Vendor defined");
  }
  else {
   log("Reserved type");
  }

  //log first four connected nodes
  dword_t connection_list_length = hda_send_verb(sound_card_number, hda_codec_number, node, 0xF00, 0x0E);
  dword_t connection_list_entries = hda_send_verb(sound_card_number, hda_codec_number, node, 0xF02, 0x00);
  if(connection_list_entries!=0) {
   log(" ");
   if((connection_list_length & 0x80)==0x00) { //short form
    for(dword_t i=0; i<4; i++) {
     if((connection_list_entries & 0xFF)==0) {
      break;
     }
     log_var_with_space(connection_list_entries & 0xFF);
     connection_list_entries >>= 8;
    }
   }
   else { //long form
    log_var_with_space(connection_list_entries & 0xFFFF);
    if((connection_list_entries>>16)!=0) {
     log_var(connection_list_entries>>16);
    }
   }
  }
 }

 //log founded PINs
 log("\n\n");
 if(pin_speaker_default_node_number!=0) {
  log("Speaker node: ");
  if(pin_speaker_node_number==0) { //if there are no PIN with connected device, device should be connected to default PIN
   log_var(pin_speaker_default_node_number);
  }
  else {
   log_var(pin_speaker_node_number);
  }
 }
 else {
  log("No speaker founded");
 }
 log("\n");
 if(pin_headphone_node_number!=0) {
  log("Headphone output node: ");
  log_var(pin_headphone_node_number);
 }
 else {
  log("No headphone output founded");
 }
 log("\n");
 if(pin_alternative_output_node_number!=0) {
  log("Alternative output node: ");
  log_var(pin_alternative_output_node_number);
 }
 else {
  log("No alternative output founded");
 }

 //initalize output PIN
 if(pin_speaker_default_node_number!=0) {
  hda_is_initalized_useful_output = STATUS_GOOD;

  if(pin_speaker_node_number!=0) {
   hda_initalize_output_pin(sound_card_number, pin_speaker_node_number); //we will initalize speaker with connected output device
  }
  else {
   hda_initalize_output_pin(sound_card_number, pin_speaker_default_node_number); //we will initalize default speaker
  }
 }
 else if(pin_alternative_output_node_number!=0) {
  hda_is_initalized_useful_output = STATUS_FALSE;
  hda_initalize_output_pin(sound_card_number, pin_alternative_output_node_number); //we will initalize alternative output
 }

 //TODO: initalize headphone PIN
}

void hda_initalize_output_pin(dword_t sound_card_number, dword_t pin_node_number) {
 log("\nInitalizing PIN ");
 log_var(pin_node_number);

 //reset variables
 hda_audio_output_node_number = 0;
 hda_audio_output_node_sample_capabilites = 0;
 hda_audio_output_node_stream_format_capabilites = 0;
 hda_output_amp_node_number = 0;
 hda_output_amp_node_capabilites = 0;

 //turn on power for PIN
 hda_send_verb(sound_card_number, hda_codec_number, pin_node_number, 0x705, 0x00);

 //disable unsolicited responses
 hda_send_verb(sound_card_number, hda_codec_number, pin_node_number, 0x708, 0x00);

 //enable PIN
 hda_send_verb(sound_card_number, hda_codec_number, pin_node_number, 0x707, (hda_send_verb(sound_card_number, hda_codec_number, pin_node_number, 0xF07, 0x00) | 0x80 | 0x40));

 //enable EAPD + L-R swap
 hda_send_verb(sound_card_number, hda_codec_number, pin_node_number, 0x70C, 0x6);

 //set maximal volume for PIN
 //TODO: does this work in every case?
 dword_t pin_output_amp_capabilites = hda_send_verb(sound_card_number, hda_codec_number, pin_node_number, 0xF00, 0x12);
 hda_send_verb(sound_card_number, hda_codec_number, pin_node_number, 0x300, (0xF000 | ((pin_output_amp_capabilites>>8) & 0x7F)));
 if(pin_output_amp_capabilites!=0) {
  //we will control volume by PIN node
  hda_output_amp_node_number = pin_node_number;
  hda_output_amp_node_capabilites = pin_output_amp_capabilites;
 }

 //start enabling path of nodes
 //TODO: more options than only through first connected node
 hda_length_of_node_path = 0;
 hda_send_verb(sound_card_number, hda_codec_number, pin_node_number, 0x701, 0x00); //select first node
 dword_t first_connected_node_number = (hda_send_verb(sound_card_number, hda_codec_number, pin_node_number, 0xF02, 0x00) & 0xFF);
 dword_t type_of_first_connected_node = ((hda_send_verb(sound_card_number, hda_codec_number, first_connected_node_number, 0xF00, 0x09) >> 20) & 0xF);
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
  log("\nHDA ERROR: PIN have connection ");
  log_var(first_connected_node_number);
 }
}

void hda_initalize_audio_output(dword_t sound_card_number, dword_t audio_output_node_number) {
 log("\nInitalizing Audio Output ");
 log_var(audio_output_node_number);
 hda_audio_output_node_number = audio_output_node_number;

 //turn on power for Audio Output
 hda_send_verb(sound_card_number, hda_codec_number, audio_output_node_number, 0x705, 0x00);

 //disable unsolicited responses
 hda_send_verb(sound_card_number, hda_codec_number, audio_output_node_number, 0x708, 0x00);

 //connect Audio Output to stream 1 channel 0
 hda_send_verb(sound_card_number, hda_codec_number, audio_output_node_number, 0x706, 0x10);

 //TODO:
 // hda_send_verb(sound_card_number, hda_codec_number, audio_output_node_number, 0x72D, 1);

 //set maximal volume for Audio Output
 //TODO: does this work in every case?
 dword_t audio_output_amp_capabilites = hda_send_verb(sound_card_number, hda_codec_number, audio_output_node_number, 0xF00, 0x12);
 hda_send_verb(sound_card_number, hda_codec_number, audio_output_node_number, 0x300, (0xF000 | ((audio_output_amp_capabilites>>8) & 0x7F)));
 if(audio_output_amp_capabilites!=0) {
  //we will control volume by Audio Output node
  hda_output_amp_node_number = audio_output_node_number;
  hda_output_amp_node_capabilites = audio_output_amp_capabilites;
 }

 //read info, if something is not present, take it from AFG node
 dword_t audio_output_sample_capabilites = hda_send_verb(sound_card_number, hda_codec_number, audio_output_node_number, 0xF00, 0x0A);
 if(audio_output_sample_capabilites==0) {
  hda_audio_output_node_sample_capabilites = hda_afg_node_sample_capabilites;
 }
 else {
  hda_audio_output_node_sample_capabilites = audio_output_sample_capabilites;
 }
 dword_t audio_output_stream_format_capabilites = hda_send_verb(sound_card_number, hda_codec_number, audio_output_node_number, 0xF00, 0x0B);
 if(audio_output_stream_format_capabilites==0) {
  hda_audio_output_node_stream_format_capabilites = hda_afg_node_stream_format_capabilites;
 }
 else {
  hda_audio_output_node_stream_format_capabilites = audio_output_stream_format_capabilites;
 }
 if(hda_output_amp_node_number==0) { //if nodes in path do not have output amp capabilites, volume will be controlled by Audio Output node by capabilites from AFG node
  hda_output_amp_node_number = audio_output_node_number;
  hda_output_amp_node_capabilites = hda_afg_node_output_amp_capabilites;
 }

 //because we are at end of node path, log all gathered info
 log("\nAudio Output path sucessfully initalized");
 log("\nSample Capabilites: ");
 log_hex(hda_audio_output_node_sample_capabilites);
 log("\nStream Format Capabilites: ");
 log_hex(hda_audio_output_node_stream_format_capabilites);
 log("\nVolume node: ");
 log_var(hda_output_amp_node_number);
 log("\nVolume capabilites: ");
 log_hex(hda_output_amp_node_capabilites);
}

void hda_initalize_audio_mixer(dword_t sound_card_number, dword_t audio_mixer_node_number) {
 if(hda_length_of_node_path>=10) {
  log("\nHDA ERROR: too long path");
  return;
 }
 log("\nInitalizing Audio Mixer ");
 log_var(audio_mixer_node_number);

 //turn on power for Audio Mixer
 hda_send_verb(sound_card_number, hda_codec_number, audio_mixer_node_number, 0x705, 0x00);

 //disable unsolicited responses
 hda_send_verb(sound_card_number, hda_codec_number, audio_mixer_node_number, 0x708, 0x00);

 //set maximal volume for Audio Mixer
 //TODO: does this work in every case?
 dword_t audio_mixer_amp_capabilites = hda_send_verb(sound_card_number, hda_codec_number, audio_mixer_node_number, 0xF00, 0x12);
 hda_send_verb(sound_card_number, hda_codec_number, audio_mixer_node_number, 0x300, (0xF000 | ((audio_mixer_amp_capabilites>>8) & 0x7F)));
 if(audio_mixer_amp_capabilites!=0) {
  //we will control volume by Audio Mixer node
  hda_output_amp_node_number = audio_mixer_node_number;
  hda_output_amp_node_capabilites = audio_mixer_amp_capabilites;
 }

 //continue in path
 //TODO: more options than only through first connected node
 hda_length_of_node_path++;
 dword_t first_connected_node_number = (hda_send_verb(sound_card_number, hda_codec_number, audio_mixer_node_number, 0xF02, 0x00) & 0xFF);
 dword_t type_of_first_connected_node = ((hda_send_verb(sound_card_number, hda_codec_number, first_connected_node_number, 0xF00, 0x09) >> 20) & 0xF);
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
  log("\nHDA ERROR: Mixer have connection ");
  log_var(first_connected_node_number);
 }
}

void hda_initalize_audio_selector(dword_t sound_card_number, dword_t audio_selector_node_number) {
 if(hda_length_of_node_path>=10) {
  log("\nHDA ERROR: too long path");
  return;
 }
 log("\nInitalizing Audio Selector ");
 log_var(audio_selector_node_number);

 //turn on power for Audio Selector
 hda_send_verb(sound_card_number, hda_codec_number, audio_selector_node_number, 0x705, 0x00);

 //disable unsolicited responses
 hda_send_verb(sound_card_number, hda_codec_number, audio_selector_node_number, 0x708, 0x00);

 //set maximal volume for Audio Selector
 //TODO: does this work in every case?
 dword_t audio_selector_amp_capabilites = hda_send_verb(sound_card_number, hda_codec_number, audio_selector_node_number, 0xF00, 0x12);
 hda_send_verb(sound_card_number, hda_codec_number, audio_selector_node_number, 0x300, (0xF000 | ((audio_selector_amp_capabilites>>8) & 0x7F)));
 if(audio_selector_amp_capabilites!=0) {
  //we will control volume by Audio Selector node
  hda_output_amp_node_number = audio_selector_node_number;
  hda_output_amp_node_capabilites = audio_selector_amp_capabilites;
 }

 //continue in path
 //TODO: more options than only through first connected node
 hda_length_of_node_path++;
 hda_send_verb(sound_card_number, hda_codec_number, audio_selector_node_number, 0x701, 0x00); //select first node
 dword_t first_connected_node_number = (hda_send_verb(sound_card_number, hda_codec_number, audio_selector_node_number, 0xF02, 0x00) & 0xFF);
 dword_t type_of_first_connected_node = ((hda_send_verb(sound_card_number, hda_codec_number, first_connected_node_number, 0xF00, 0x09) >> 20) & 0xF);
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
  log("\nHDA ERROR: Selector have connection ");
  log_var(first_connected_node_number);
 }
}

void hda_set_volume(dword_t sound_card_number, dword_t volume) {
 dword_t number_of_steps = ((hda_output_amp_node_capabilites>>8) & 0x7F);

 //zero volume
 if(volume==0) {
  hda_send_verb(sound_card_number, hda_codec_number, hda_output_amp_node_number, 0x300, (0xF000 | 0x80)); //mute with lowest volume
  return;
 }

 //max volume, or if there is fixed volume on node, unmute it
 if(volume==100 || number_of_steps==0) {
  hda_send_verb(sound_card_number, hda_codec_number, hda_output_amp_node_number, 0x300, (0xF000 | number_of_steps)); //unmute with highest volume
  return;
 }
 
 //recalculate volume scale 0-100 to node volume scale
 hda_send_verb(sound_card_number, hda_codec_number, hda_output_amp_node_number, 0x300, (0xB000 | (number_of_steps-((100-volume)*number_of_steps/100))));
}

byte_t hda_is_supported_channel_size(dword_t sound_card_number, byte_t size) {
 byte_t channel_sizes[5] = {8, 16, 20, 24, 32};
 dword_t mask=0x00010000;
 
 for(int i=0; i<5; i++) {
  if(channel_sizes[i]==size) {
   break;
  }
  mask <<= 1;
 }
 
 if((hda_audio_output_node_sample_capabilites & mask)==mask) {
  return STATUS_GOOD;
 }
 else {
  return STATUS_ERROR;
 }
}

byte_t hda_is_supported_sample_rate(dword_t sound_card_number, dword_t sample_rate) {
 dword_t sample_rates[11] = {8000, 11025, 16000, 22050, 32000, 44100, 48000, 88200, 96000, 176400, 192000};
 word_t mask=0x0000001;
 
 for(int i=0; i<11; i++) {
  if(sample_rates[i]==sample_rate) {
   break;
  }
  mask <<= 1;
 }
 
 if((hda_audio_output_node_sample_capabilites & mask)==mask) {
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

 //sample rate TODO: more
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

void hda_play_memory(dword_t sound_card_number, dword_t memory, dword_t sample_rate, dword_t channels, dword_t bits_per_sample, dword_t number_of_samples_in_one_channel) {
 if((hda_audio_output_node_stream_format_capabilites & 0x1)==0x0) {
  return; //this Audio Output do not support PCM sound data
 }

 //calculate length of data in bytes
 dword_t bytes_per_sample = (bits_per_sample/8); //8 bits/16 bits
 if(bits_per_sample>16) { //20 bits/24 bits/32 bits
  bytes_per_sample = 4;
 }
 hda_sound_length = (number_of_samples_in_one_channel*bytes_per_sample*channels);

 //stop stream
 hda_playing_state = 0;
 mmio_outb(hda_output_stream_base + 0x00, 0x00);
 ticks = 0;
 while(ticks<2) {
  asm("nop");
  if((mmio_inb(hda_output_stream_base + 0x00) & 0x2)==0x0) {
   break;
  }
 }
 if((mmio_inb(hda_output_stream_base + 0x00) & 0x2)==0x2) {
  log("\nHDA: can not stop stream");
  return;
 }

 //clear error bits
 mmio_outb(hda_output_stream_base + 0x03, 0x1C);
 
 //reset stream registers
 mmio_outb(hda_output_stream_base + 0x00, 0x01);
 ticks = 0;
 while(ticks<10) {
  asm("nop");
  if((mmio_inb(hda_output_stream_base + 0x00) & 0x1)==0x1) {
   break;
  }
 }
 if((mmio_inb(hda_output_stream_base + 0x00) & 0x1)==0x0) {
  log("\nHDA: can not start resetting stream");
  return;
 }
 wait(10);
 mmio_outb(hda_output_stream_base + 0x00, 0x00);
 ticks = 0;
 while(ticks<10) {
  asm("nop");
  if((mmio_inb(hda_output_stream_base + 0x00) & 0x1)==0x0) {
   break;
  }
 }
 if((mmio_inb(hda_output_stream_base + 0x00) & 0x1)==0x1) {
  log("\nHDA: can not stop resetting stream");
  return;
 }
 wait(10);

 //clear error bits
 mmio_outb(hda_output_stream_base + 0x03, 0x1C);

 //fill buffer entries
 //there have to be at least two entries in buffer, so we fill second entry with zeroes
 clear_memory(hda_output_buffer_list, 16*2);
 dword_t *buffer = (dword_t *) (hda_output_buffer_list);
 buffer[0]=memory;
 buffer[2]=hda_sound_length;

 //TODO:
 //?????
 //I really do not understand why, but if after setting buffer there is not 8+ ms wait when processor do something with memory, there are strange issues with playing stream, as if it remembers previous stream memory and try to play it
 reset_timer();
 while(get_timer_value_in_microseconds()<8) {
  redraw_screen();
 }
 //?????

 //set buffer registers
 mmio_outd(hda_output_stream_base + 0x18, hda_output_buffer_list);
 mmio_outd(hda_output_stream_base + 0x08, hda_sound_length);
 mmio_outw(hda_output_stream_base + 0x0C, 1); //there are two entries in buffer

 //set stream data format
 mmio_outw(hda_output_stream_base + 0x12, hda_return_sound_data_format(sample_rate, channels, bits_per_sample));

 //set Audio Output node data format
 hda_send_verb(sound_card_number, hda_codec_number, hda_audio_output_node_number, 0x200, hda_return_sound_data_format(sample_rate, channels, bits_per_sample));
 wait(10);

 //start streaming to stream 1
 mmio_outb(hda_output_stream_base + 0x02, 0x14);
 mmio_outb(hda_output_stream_base + 0x00, 0x02);
 hda_bytes_on_output_for_stopping_sound = 0;
 hda_playing_state = 1;

 //sound will be automatically stopped by drivers/system/processes_on_background.c
}

void hda_stop_sound(dword_t sound_card_number) {
 mmio_outb(hda_output_stream_base + 0x00, 0x00);
 hda_playing_state = 0;
}

void hda_resume_sound(dword_t sound_card_number) {
 mmio_outb(hda_output_stream_base + 0x00, 0x02);
 hda_playing_state = 1;
}