//BleskOS

/*
* MIT License
* Copyright (c) 2023-2024 Vendelín Slezák
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

void initalize_ac97_sound_card(byte_t sound_card_number) {
 //load base address of registers
 ac97_nam_base = sound_cards_info[sound_card_number].io_base;
 ac97_nabm_base = sound_cards_info[sound_card_number].io_base_2;
 
 //resume from cold reset to normal operation and set 2 channels + 16 bit samples
 outd(ac97_nabm_base + AC97_NABM_IO_GLOBAL_CONTROL, (0b00<<22) | (0b00<<20) | (0<<2) | (1<<1));
 wait(20);
 
 //reset all streams
 outb(ac97_nabm_base + AC97_NABM_IO_PCM_INPUT_CONTROL, 0x2);
 outb(ac97_nabm_base + AC97_NABM_IO_PCM_OUTPUT_CONTROL, 0x2);
 outb(ac97_nabm_base + AC97_NABM_IO_MICROPHONE_INPUT_CONTROL, 0x2);
 wait(20);

 //reset all NAM registers
 outw(ac97_nam_base + AC97_NAM_IO_RESET, 0xFF);

 //detect if there is headphone output
 ac97_headphone_output_present = STATUS_FALSE;
 if((inw(ac97_nam_base + AC97_NAM_IO_CAPABILITES) & AC97_CAPABILITY_HEADPHONE_OUTPUT)==AC97_CAPABILITY_HEADPHONE_OUTPUT && inw(ac97_nam_base + AC97_NAM_IO_AUX_OUT_VOLUME)==0x8000) {
  ac97_headphone_output_present = STATUS_TRUE;
 }

 //set max PCM output volume
 outw(ac97_nam_base + AC97_NAM_IO_PCM_OUT_VOLUME, 0x0);
 
 //allocate memory for buffer
 ac97_buffer_memory_pointer = (struct ac97_buffer_entry *) (aligned_malloc(sizeof(struct ac97_buffer_entry)*32, 0xF));
 
 //read extended capabilities
 ac97_extended_capabilities = inw(ac97_nam_base + AC97_NAM_IO_EXTENDED_CAPABILITIES);
 if((ac97_extended_capabilities & AC97_EXTENDED_CAPABILITY_VARIABLE_SAMPLE_RATE)==AC97_EXTENDED_CAPABILITY_VARIABLE_SAMPLE_RATE) {
  outw(ac97_nam_base + AC97_NAM_IO_EXTENDED_FEATURES_CONTROL, AC97_EXTENDED_CAPABILITY_VARIABLE_SAMPLE_RATE); //enable variable sample rate
 }

 //get number of AUX_OUT volume steps
 ac97_aux_out_number_of_volume_steps = 31;
 outw(ac97_nam_base + AC97_NAM_IO_AUX_OUT_VOLUME, 0x2020);
 if((inw(ac97_nam_base + AC97_NAM_IO_AUX_OUT_VOLUME) & 0x2020)==0x2020) {
  ac97_aux_out_number_of_volume_steps = 63;
 }

 //set output 
 sound_volume = 0;
 if(ac97_is_headphone_connected()==STATUS_TRUE) {
  ac97_set_output(AC97_HEADPHONE_OUTPUT);
 }
 else {
  ac97_set_output(AC97_SPEAKER_OUTPUT);
 }

 //add task for checking headphone connection
 create_task(ac97_check_headphone_connection_change, TASK_TYPE_PERIODIC_INTERRUPT, 50);
 
 //log
 log("\n\nSound card AC97");
 log("\nCapabilities: "); log_hex_specific_size(inw(ac97_nam_base + AC97_NAM_IO_CAPABILITES), 4);
 log("\nExtended capabilities: "); log_hex_specific_size(ac97_extended_capabilities, 4);
}

byte_t ac97_is_headphone_connected(void) {
 //JACK_SENSE port is not part of official specification, but it seems to be supported by cards
 if(ac97_headphone_output_present==STATUS_TRUE && (inw(ac97_nam_base + AC97_NAM_IO_JACK_SENSE) & 0x8)==0x8) {
  return STATUS_TRUE;
 }
 else {
  return STATUS_FALSE;
 }
}

void ac97_set_volume_in_register(dword_t offset, byte_t number_of_volume_steps, byte_t volume) {
 if(volume==0) {
  outw(ac97_nam_base + offset, 0x8000); //mute
 }
 else {
  volume = ((100-volume)*number_of_volume_steps/100); //recalculate 0-100 scale to register volume scale
  outw(ac97_nam_base + offset, ((volume) | (volume<<8))); //set same volume for left and right
 }
}

void ac97_set_output(byte_t output) {
 if(output==AC97_SPEAKER_OUTPUT) {
  ac97_set_volume_in_register(AC97_NAM_IO_AUX_OUT_VOLUME, ac97_aux_out_number_of_volume_steps, 0);
  ac97_set_volume_in_register(AC97_NAM_IO_MASTER_VOLUME, AC97_SPEAKER_OUTPUT_NUMBER_OF_VOLUME_STEPS, sound_volume);
 }
 else if(output==AC97_HEADPHONE_OUTPUT) {
  ac97_set_volume_in_register(AC97_NAM_IO_MASTER_VOLUME, ac97_aux_out_number_of_volume_steps, 0);
  ac97_set_volume_in_register(AC97_NAM_IO_AUX_OUT_VOLUME, AC97_SPEAKER_OUTPUT_NUMBER_OF_VOLUME_STEPS, sound_volume);
 }
 else {
  return;
 }

 ac97_selected_output = output;
}

void ac97_set_volume(byte_t volume) {
 if(ac97_selected_output==AC97_SPEAKER_OUTPUT) {
  ac97_set_volume_in_register(AC97_NAM_IO_MASTER_VOLUME, ac97_aux_out_number_of_volume_steps, volume);
 }
 else if(ac97_selected_output==AC97_HEADPHONE_OUTPUT) {
  ac97_set_volume_in_register(AC97_NAM_IO_AUX_OUT_VOLUME, AC97_SPEAKER_OUTPUT_NUMBER_OF_VOLUME_STEPS, volume);
 }
}

byte_t ac97_is_supported_sample_rate(word_t sample_rate) {
 if(sample_rate==48000) { //48000 sample rate is always supported
  return STATUS_GOOD;
 }
 else if((ac97_extended_capabilities & AC97_EXTENDED_CAPABILITY_VARIABLE_SAMPLE_RATE)==AC97_EXTENDED_CAPABILITY_VARIABLE_SAMPLE_RATE) { //check if variable sound rate feature is present
  if(sample_rate==44100) { //44100 sample rate is always supported
   return STATUS_GOOD;
  }
  else { //other may or may not be supported, so we will check it
   //TODO: does this method always work?
   outw(ac97_nam_base + AC97_NAM_IO_VARIABLE_SAMPLE_RATE_FRONT_DAC, sample_rate);
   if(inw(ac97_nam_base + AC97_NAM_IO_VARIABLE_SAMPLE_RATE_FRONT_DAC)==sample_rate) {
    return STATUS_GOOD;
   }
  }
 }

 return STATUS_ERROR; //this sample rate is not supported
}

void ac97_set_sample_rate(word_t sample_rate) {
 //check if variable sample rate feature is present
 if((ac97_extended_capabilities & AC97_EXTENDED_CAPABILITY_VARIABLE_SAMPLE_RATE)==AC97_EXTENDED_CAPABILITY_VARIABLE_SAMPLE_RATE) {
  //set same variable rate on all outputs
  outw(ac97_nam_base + AC97_NAM_IO_VARIABLE_SAMPLE_RATE_FRONT_DAC, sample_rate);
  outw(ac97_nam_base + AC97_NAM_IO_VARIABLE_SAMPLE_RATE_SURR_DAC, sample_rate);
  outw(ac97_nam_base + AC97_NAM_IO_VARIABLE_SAMPLE_RATE_LFE_DAC, sample_rate);
  outw(ac97_nam_base + AC97_NAM_IO_VARIABLE_SAMPLE_RATE_LR_ADC, sample_rate);
 }
}

void ac97_fill_buffer_entry(void) {
 //return if we do not need to add entry / buffer is full
 if(ac97_number_of_samples_in_one_channel==0 || ac97_free_entry==inb(ac97_nabm_base + 0x14)) {
  return;
 }
 
 //fill entry
 if(ac97_number_of_samples_in_one_channel>0xFFFF) { //we will need more entries to play whole sound
  ac97_buffer_memory_pointer[ac97_free_entry].sample_memory = ac97_sound_memory;
  ac97_buffer_memory_pointer[ac97_free_entry].number_of_samples = 0xFFFE;
  ac97_buffer_memory_pointer[ac97_free_entry].reserved = 0;
  ac97_buffer_memory_pointer[ac97_free_entry].last_buffer_entry = 0;
  ac97_buffer_memory_pointer[ac97_free_entry].interrupt_on_completion = 0;

  ac97_sound_memory += 0xFFFF*2; //there are two channels
  ac97_number_of_samples_in_one_channel -= 0xFFFF;
 }
 else { //this is last entry, we do not need more entries to play whole sound
  ac97_buffer_memory_pointer[ac97_free_entry].sample_memory = ac97_sound_memory;
  ac97_buffer_memory_pointer[ac97_free_entry].number_of_samples = (ac97_number_of_samples_in_one_channel-1);
  ac97_buffer_memory_pointer[ac97_free_entry].reserved = 0;
  ac97_buffer_memory_pointer[ac97_free_entry].last_buffer_entry = 1;
  ac97_buffer_memory_pointer[ac97_free_entry].interrupt_on_completion = 0;

  ac97_sound_memory += ac97_number_of_samples_in_one_channel*2;
  ac97_number_of_samples_in_one_channel = 0;
  ac97_playing_state = AC97_BUFFER_FILLED; //we do not need more entries
 }
 
 //update Last Valid Entry register
 outb(ac97_nabm_base + AC97_NABM_IO_PCM_OUTPUT_LAST_VALID_ENTRY, ac97_free_entry);

 //next entry
 ac97_free_entry = ((ac97_free_entry+1) & 0x1F); //roll over 32 entries
}

void ac97_play_sound(void) {
 //always have at least one buffer filled
 ac97_fill_buffer_entry();

 //clear status
 outw(ac97_nabm_base + AC97_NABM_IO_PCM_OUTPUT_STATUS, 0x1C);

 //start streaming
 outb(ac97_nabm_base + AC97_NABM_IO_PCM_OUTPUT_CONTROL, 0x1);

 //other buffers will be filled while playing by code in drivers/system/processes_on_background.c
 ac97_playing_state = AC97_BUFFER_NOT_FILLED;
}

void ac97_stop_sound(void) {
 //stop streaming
 outb(ac97_nabm_base + AC97_NABM_IO_PCM_OUTPUT_CONTROL, 0x0);

 //clear status
 outw(ac97_nabm_base + AC97_NABM_IO_PCM_OUTPUT_STATUS, 0x1C);

 //buffers will not be filled by code in drivers/system/processes_on_background.c
 ac97_playing_state = AC97_BUFFER_FILLED;
}

void ac97_play_memory(dword_t sound_memory, dword_t sound_size, dword_t sample_rate) {
 //stop sound
 outb(ac97_nabm_base + AC97_NABM_IO_PCM_OUTPUT_CONTROL, 0x0);

 //set sample rate
 ac97_set_sample_rate(sample_rate);

 //reset stream
 outb(ac97_nabm_base + AC97_NABM_IO_PCM_OUTPUT_CONTROL, 0x2);
 ticks = 0;
 while((inb(ac97_nabm_base + AC97_NABM_IO_PCM_OUTPUT_CONTROL) & 0x2)==0x2) {
  asm("nop");
  if(ticks>50) { //stream was not reseted after 100 ms
   return;
  }
 }
 outb(ac97_nabm_base + AC97_NABM_IO_PCM_OUTPUT_CONTROL, 0x0);

 //clear buffer
 clear_memory((dword_t)ac97_buffer_memory_pointer, (sizeof(struct ac97_buffer_entry)*32));

 //set buffer address
 outd(ac97_nabm_base + AC97_NABM_IO_PCM_OUTPUT_BUFFER_BASE_ADDRESS, (dword_t)ac97_buffer_memory_pointer);

 //play sound
 ac97_sound_memory = sound_memory;
 ac97_number_of_samples_in_one_channel = (sound_size/2); //sound have 2 channels
 ac97_free_entry = 1; //playing will start from this entry
 ac97_play_sound(); 
}

void ac97_check_headphone_connection_change(void) {
 if(ac97_selected_output==AC97_SPEAKER_OUTPUT && ac97_is_headphone_connected()==STATUS_TRUE) { //headphone was connected
  ac97_set_output(AC97_HEADPHONE_OUTPUT);
 }
 else if(ac97_selected_output==AC97_HEADPHONE_OUTPUT && ac97_is_headphone_connected()==STATUS_FALSE) { //headphone was disconnected
  ac97_set_output(AC97_SPEAKER_OUTPUT);
 }
}