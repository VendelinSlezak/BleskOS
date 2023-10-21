//BleskOS

/*
* MIT License
* Copyright (c) 2023 Vendelín Slezák
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

void initalize_hda(void) { 
 //set operational state
 mmio_outd(hda_base + 0x08, 0x1);
 wait(20);
 if((mmio_ind(hda_base + 0x08) & 0x1)==0x0) {
  return;
 }
 
 log("\nsound card High Definition Audio\n");
 
 //read capabilites
 hda_input_stream_base = (hda_base + 0x80);
 hda_output_stream_base = (hda_base + 0x80 + (0x20*((mmio_inw(hda_base + 0x00)>>8) & 0xF))); //skip input streams ports
 
 //disable interrupts
 mmio_outd(hda_base + 0x20, 0);
 
 //turn off dma position transfer
 mmio_outd(hda_base + 0x70, 0);
 mmio_outd(hda_base + 0x74, 0);
 
 //disable synchronization
 mmio_outd(hda_base + 0x34, 0);
 mmio_outd(hda_base + 0x38, 0);
 
 //configure input stream
 mmio_outd(hda_input_stream_base + 0x00, 0x00240000);
 hda_input_buffer_list = aligned_malloc(16*2, 0x7F);
 if(hda_input_buffer_list==0) {
  memory_error_debug(0x0000FF);
 }
 mmio_outw(hda_input_stream_base + 0x0C, 1); //two buffer entries
 mmio_outd(hda_input_stream_base + 0x18, hda_input_buffer_list);
 mmio_outd(hda_input_stream_base + 0x1C, 0);
 
 //configure output stream
 mmio_outd(hda_output_stream_base + 0x00, 0x00140000);
 hda_output_buffer_list = aligned_malloc(16*2, 0x7F);
 if(hda_output_buffer_list==0) {
  memory_error_debug(0x0000FF);
 }
 mmio_outw(hda_output_stream_base + 0x0C, 1); //two buffer entries
 mmio_outd(hda_output_stream_base + 0x18, hda_output_buffer_list);
 mmio_outd(hda_output_stream_base + 0x1C, 0);
 
 //stop CORB and RIRB
 mmio_outb(hda_base + 0x4C, 0x0);
 mmio_outb(hda_base + 0x5C, 0x0);
 
 //configure CORB
 hda_corb_mem = aligned_malloc(256*4, 0x7F);
 if(hda_corb_mem==0) {
  memory_error_debug(0x0000FF);
 }
 mmio_outd(hda_base + 0x40, hda_corb_mem); //CORB lower memory
 mmio_outd(hda_base + 0x44, 0); //CORB upper memory
 mmio_outb(hda_base + 0x4E, 0x2); //256 entries
 mmio_outw(hda_base + 0x4A, 0x8000); //reset read pointer
 wait(10);
 mmio_outw(hda_base + 0x4A, 0x0000);
 wait(10);
 mmio_outw(hda_base + 0x48, 0); //set write pointer
 hda_corb_pointer = 1;
 
 //configure RIRB
 hda_rirb_mem = aligned_malloc(256*8, 0x7F);
 if(hda_rirb_mem==0) {
  memory_error_debug(0x0000FF);
 }
 clear_memory(hda_rirb_mem, 256*8);
 mmio_outd(hda_base + 0x50, hda_rirb_mem); //CORB lower memory
 mmio_outd(hda_base + 0x54, 0); //CORB upper memory
 mmio_outb(hda_base + 0x5E, 0x2); //256 entries
 mmio_outw(hda_base + 0x58, 0x8000); //reset read pointer
 wait(10);
 mmio_outw(hda_base + 0x58, 0x0000);
 wait(10);
 mmio_outw(hda_base + 0x5A, 0); //disable interrupts
 hda_rirb_pointer = 1;
 
 //start CORB and RIRB
 mmio_outb(hda_base + 0x4C, 0x2);
 mmio_outb(hda_base + 0x5C, 0x2);
 
 //find codec and working interface
 hda_verb_interface = HDA_CORB_RIRB;
 hda_codec_number = 0xFFFFFFFF;
 for(int i=0; i<16; i++) {
  if(hda_send_verb(i, 0, 0xF00, 0)!=0) {
   log("CORB/RIRB interface\ncodec ID: ");
   log_hex(hda_send_verb(i, 0, 0xF00, 0));
   log("\n");
   hda_codec_number = i;
   break;
  }
 }
 if(hda_codec_number==0xFFFFFFFF) {
  //try PIO interface
  hda_verb_interface = HDA_PIO;
  for(int i=0; i<16; i++) {
   if(hda_send_verb(i, 0, 0xF00, 0)!=0) {
    log("PIO interface\ncodec ID: ");
    log_hex(hda_send_verb(i, 0, 0xF00, 0));
    log("\n");
    hda_codec_number = i;
    break;
   }
  }
  if(hda_codec_number==0xFFFFFFFF) {
   //ERROR: HDA do not respond or there is no codec
   return;
  }
 }
 
 //initalize codec
 hda_codec_nodes_types_mem = calloc(256);
 if(hda_codec_nodes_types_mem==0) {
  memory_error_debug(0x0000FF);
 }
 hda_codec_nodes_connection_mem = calloc(256*4);
 if(hda_codec_nodes_connection_mem==0) {
  memory_error_debug(0x0000FF);
 }
 hda_initalize_codec(hda_codec_number);
}

void hda_initalize_codec(dword_t codec) {
 byte_t *nodes_mem = (byte_t *) hda_codec_nodes_types_mem;
 dword_t *nodes_connection_mem = (dword_t *) hda_codec_nodes_connection_mem;
 dword_t response;
 
 log("codec nodes:\n");
 
 //find nodes
 for(int node=0; node<256; node++) {
  response = hda_send_verb(codec, node, 0xF00, 0x09);
  nodes_mem[node] = 0xFF;
  nodes_connection_mem[node] = 0xFFFFFFFF;
  
  if(response!=0 && response!=0xFFFFFFFF) {
   //type of node
   nodes_mem[node] = (byte_t)((response>>20) & 0xF);
   
   //type of pin
   if(nodes_mem[node]==HDA_NODE_OUTPUT) {
    hda_send_verb(codec, node, 0x706, 0x00); //disable by setting stream to 0
   }
   if(nodes_mem[node]==HDA_NODE_PIN) {
    nodes_mem[node] = (byte_t)(((hda_send_verb(codec, node, 0xF1C, 0x00)>>20) & 0xF)+0x10);
   }
   
   //get connection list
   nodes_connection_mem[node] = hda_send_verb(codec, node, 0xF02, 0x00);
   
   //turn on every widget
   hda_send_verb(codec, node, 0x705, 0x00);
   
   //LOG node
   log_var_with_space(node);
   if(nodes_mem[node]==0) {
    log("Audio Output");
   }
   else if(nodes_mem[node]==1) {
    log("Audio Input");
   }
   else if(nodes_mem[node]==2) {
    log("Audio Mixer");
   }
   else if(nodes_mem[node]==3) {
    log("Audio Selector");
   }
   else if(nodes_mem[node]==0x10) {
    log("Pin Line Out");
   }
   else if(nodes_mem[node]==0x11) {
    log("Pin Speaker");
   }
   else if(nodes_mem[node]==0x12) {
    log("Pin HP Out");
   }
   else if(nodes_mem[node]==0x13) {
    log("Pin CD");
   }
   else if(nodes_mem[node]==0x14) {
    log("Pin SPDIF Out");
   }
   else if(nodes_mem[node]==0x15) {
    log("Pin Digital Other Out");
   }
   else if(nodes_mem[node]==0x18) {
    log("Pin Line In");
   }
   else if(nodes_mem[node]==0x19) {
    log("Pin AUX");
   }
   else if(nodes_mem[node]==0x1A) {
    log("Pin Mic In");
   }
   else if(nodes_mem[node]==0x1D) {
    log("Pin Digital Other In");
   }
   else {
    log_hex_specific_size(nodes_mem[node], 2);
   }
   log(" ");
   for(dword_t i=0; i<4; i++) {
    if((nodes_connection_mem[node]>>(i*8))==0) {
     break;
    }
    log_var_with_space((nodes_connection_mem[node]>>(i*8)) & 0xFF);
   }
   log("\n");
  }
 }
 
 //find speaker with amp
 hda_output_pin_node=0xFF;
 for(int node=0; node<256; node++) {
  if(nodes_mem[node]==HDA_PIN_SPEAKER) {  
   response = hda_send_verb(codec, node, 0xF00, 0x09);
   
   //there is connected output device
   if((response & 0x4)==0x4) {
    response = hda_send_verb(codec, node, 0xF1C, 0x00);
    
    //pin have jack or fixed device
    if((response>>30)!=0x1) {
     response = hda_send_verb(codec, node, 0xF00, 0x0C);
     
     //pin is output capable
     if((response & 0x10)==0x10) {
      hda_output_pin_node = node;
      break;
     }
    }
   }
  }
 }
 
 //if there is no speaker, found line out
 if(hda_output_pin_node==0xFF) {
  for(int node=0; node<256; node++) {
   if(nodes_mem[node]==HDA_PIN_LINE_OUT && nodes_connection_mem[node]!=0) {
    hda_output_pin_node = node;
    break;
   }
  }
 }

 //if there is no line out, found headphone out
 if(hda_output_pin_node==0xFF) {
  for(int node=0; node<256; node++) {
   if(nodes_mem[node]==HDA_PIN_HEADPHONE && nodes_connection_mem[node]!=0) {
    hda_output_pin_node = node;
    break;
   }
  }
 }

 //if there is no line out, found digital other out
 if(hda_output_pin_node==0xFF) {
  for(int node=0; node<256; node++) {
   if(nodes_mem[node]==HDA_PIN_DIGITAL_OTHER_OUT && nodes_connection_mem[node]!=0) {
    hda_output_pin_node = node;
    break;
   }
  }
 }

 //ERROR: no output pin was founded
 if(hda_output_pin_node==0xFF) {
  return;
 }

 //enable pin
 response = hda_send_verb(codec, hda_output_pin_node, 0xF07, 0x00);
 hda_send_verb(codec, hda_output_pin_node, 0x707, (response | 0x80 | 0x40));
  
 //power amplifier + L-R swap
 hda_send_verb(codec, hda_output_pin_node, 0x70C, 0x6);
  
 //set max volume
 hda_node_set_volume(codec, hda_output_pin_node, 100);
 
 //search if is pin directly connected to audio output
 hda_output_audio_node = 0xFF;
 response = (hda_send_verb(codec, hda_output_pin_node, 0xF00, 0x0E) & 0x7F);
 for(int i=0, shift=0; i<response; i++, shift+=8) {
  hda_node_in_path = ((nodes_connection_mem[hda_output_pin_node]>>shift) & 0xFF);
  if(nodes_mem[hda_node_in_path]==HDA_NODE_OUTPUT) {
   hda_output_audio_node = hda_node_in_path;
   break;
  }
 }
 //search if some mixer connected to pin is connected to audio output
 if(hda_output_audio_node==0xFF) {
  for(int i=0, shift=0; i<response; i++, shift+=8) {
   hda_node_in_path = ((nodes_connection_mem[hda_output_pin_node]>>shift) & 0xFF);
   if(nodes_mem[hda_node_in_path]==HDA_NODE_MIXER) {
    for(int j=0, num_of_connections=(hda_send_verb(codec, hda_output_pin_node, 0xF00, 0x0E) & 0x7F), shift2=0; j<num_of_connections; j++, shift2+=8) {
     hda_output_audio_node = ((nodes_connection_mem[hda_node_in_path]>>shift2) & 0xFF);
     break;
    }
   }
  }
 }
 //TODO: selectors and longer paths
 if(hda_output_audio_node==0xFF) {
  return; //ERROR: no audio output founded
 }
 
 //EAPD + L-R swap
 hda_send_verb(codec, hda_output_audio_node, 0x70C, 0x6);

 //connect audio output to stream 1 from channel 0
 hda_send_verb(codec, hda_output_audio_node, 0x706, 0x10);
 
 //set data format to 16 bit 2 channels
 hda_send_verb(codec, hda_output_audio_node, 0x200, 0x11);
 
 //unmute audio output
 hda_node_set_volume(codec, hda_output_audio_node, 100);
 
 //read output audio capabilites
 hda_output_sound_capabilites = hda_send_verb(codec, hda_output_audio_node, 0xF00, 0x0A);
 
 //LOG
 if(nodes_mem[hda_node_in_path]==HDA_NODE_OUTPUT) {
  log("output->pin ");
  log_var_with_space(hda_output_audio_node);
  log_var(hda_output_pin_node);
 }
 if(nodes_mem[hda_node_in_path]==HDA_NODE_MIXER) {
  log("output->mixer->pin ");
  log_var_with_space(hda_output_audio_node);
  log_var_with_space(hda_node_in_path);
  log_var(hda_output_pin_node);
  
  log("\n");
  hda_node_set_volume(codec, hda_node_in_path, 100);
 }
 
 log("\n");
}

void hda_node_set_volume(dword_t codec, dword_t node, dword_t volume) {
 dword_t volume_scale = ((hda_send_verb(codec, node, 0xF00, 0x12)>>8) & 0x7F);
 
 //fixed volume
 if(volume_scale==0) {
  hda_send_verb(codec, node, 0x300, (0xF000 | 0x00)); //unmute
  return;
 }
 
 //set maximal volume
 if(volume==100) {
  hda_send_verb(codec, node, 0x300, (0xF000 | volume_scale));
  return;
 }
 
 //recalculate volume scale 0-100 to node volume scale
 hda_send_verb(codec, node, 0x300, (0xF000 | (volume_scale-((100-volume)*volume_scale/100))));
}

//TODO: set sample rate

byte_t hda_is_supported_channel_size(byte_t size) {
 byte_t channel_sizes[5] = {8, 16, 20, 24, 32};
 dword_t mask=0x00010000;
 
 for(int i=0; i<5; i++) {
  if(channel_sizes[i]==size) {
   break;
  }
  mask <<= 1;
 }
 
 if((hda_output_sound_capabilites & mask)==mask) {
  return STATUS_GOOD;
 }
 else {
  return STATUS_ERROR;
 }
}

byte_t hda_is_supported_sample_rate(dword_t sample_rate) {
 dword_t sample_rates[11] = {8000, 11025, 16000, 22050, 32000, 44100, 48000, 88200, 96000, 176400, 192000};
 word_t mask=1;
 
 for(int i=0; i<11; i++) {
  if(sample_rates[i]==sample_rate) {
   break;
  }
  mask <<= 1;
 }
 
 if((hda_output_sound_capabilites & mask)==mask) {
  return STATUS_GOOD;
 }
 else {
  return STATUS_ERROR;
 }
}

dword_t hda_send_verb(dword_t codec, dword_t node, dword_t verb, dword_t command) {
 dword_t *corb = (dword_t *) hda_corb_mem;
 dword_t *rirb = (dword_t *) hda_rirb_mem;
 dword_t value = ((codec<<28) | (node<<20) | (verb<<8) | (command));
 
 //PIO interface
 if(hda_verb_interface==HDA_PIO) {
  mmio_outw(hda_base + 0x68, 0x2);
  mmio_outd(hda_base + 0x60, value);
  mmio_outw(hda_base + 0x68, 0x1);
  ticks = 0;
  while(ticks<3) {
   if((mmio_inw(hda_base + 0x68) & 0x3)==0x2) {
    value = mmio_ind(hda_base + 0x64);
    mmio_outw(hda_base + 0x68, 0x2);
    return value;
   }
  }
  
  return 0;
 }
 else { //CORB/RIRB interface
  //write verb
  corb[hda_corb_pointer] = value;
  
  //move write pointer
  mmio_outw(hda_base + 0x48, hda_corb_pointer);
  
  //wait for response
  ticks = 0;
  while(ticks<5) {
   if(mmio_inw(hda_base + 0x58)==hda_corb_pointer) {
    break;
   }
  }
  
  //read response
  value = rirb[hda_rirb_pointer*2];
  hda_corb_pointer = ((hda_corb_pointer+1) & 0xFF);
  hda_rirb_pointer = ((hda_rirb_pointer+1) & 0xFF);
  return value;
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

void hda_play_memory(dword_t memory, dword_t sample_rate, dword_t channels, dword_t bits_per_sample, dword_t number_of_samples_in_one_channel) {
 dword_t *buffer = (dword_t *) hda_output_buffer_list;
 
 //stop stream
 mmio_outd(hda_output_stream_base + 0x00, 0x00100000);
 
 //reset stream
 mmio_outd(hda_output_stream_base + 0x00, 0x00100001);
 wait(5);
 mmio_outd(hda_output_stream_base + 0x00, 0x00100000);
 wait(5);
 
 //buffer memory
 mmio_outd(hda_output_stream_base + 0x18, hda_output_buffer_list);
 mmio_outd(hda_output_stream_base + 0x1C, 0);
 
 //length of data
 dword_t bytes_per_sample = (bits_per_sample/8); //8 bits/16 bits
 if(bits_per_sample>16) { //20 bits/24 bits/32 bits
  bytes_per_sample = 4;
 }
 hda_sound_length = number_of_samples_in_one_channel*bytes_per_sample*channels;
 mmio_outd(hda_output_stream_base + 0x08, hda_sound_length*2); //we need at least two buffer descriptors
 
 //last valid buffer entry
 mmio_outw(hda_output_stream_base + 0x0C, 1); //two entries
 
 //set stream data format
 mmio_outw(hda_output_stream_base + 0x12, hda_return_sound_data_format(sample_rate, channels, bits_per_sample));

 //set audio output node data format
 hda_send_verb(hda_codec_number, hda_output_audio_node, 0x200, hda_return_sound_data_format(sample_rate, channels, bits_per_sample));

 //buffer entries
 buffer[0]=memory;
 buffer[1]=0;
 buffer[2]=hda_sound_length;
 buffer[3]=0;
 
 buffer[4+0]=memory;
 buffer[4+1]=0;
 buffer[4+2]=hda_sound_length;
 buffer[4+3]=0;
 
 //start streaming to stream 1
 mmio_outd(hda_output_stream_base + 0x00, 0x00100002);
 hda_playing_state = 1;
}

void hda_stop_sound(void) {
 mmio_outd(hda_output_stream_base + 0x00, 0x00100000);
 hda_playing_state = 0;
}

void hda_resume_sound(void) {
 mmio_outd(hda_output_stream_base + 0x00, 0x00100002);
 hda_playing_state = 1;
}