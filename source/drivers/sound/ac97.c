//BleskOS

/*
* MIT License
* Copyright (c) 2023-2024 Vendelín Slezák
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

void initalize_ac97(void) {
 if(ac97_present!=DEVICE_PRESENT) {
  return;
 }
 
 log("\nsound card AC97\n");
 
 //resume from cold reset
 outd(ac97_nabm_base + 0x2C, 0x2);
 wait(20);
 
 //reset all streams
 outb(ac97_nabm_base + 0x0B, 0x2);
 outb(ac97_nabm_base + 0x1B, 0x2);
 outb(ac97_nabm_base + 0x2B, 0x2);
 wait(20);
 outb(ac97_nabm_base + 0x15, 0);

 //register reset
 outw(ac97_nam_base + 0x0, 0xFF);

 //set volume
 outw(ac97_nam_base + 0x2, 0x0);
 outw(ac97_nam_base + 0x18, 0x0);
 
 //allocate memory for 32 entries
 ac97_buffer_entries_memory = aligned_malloc(8*32, 0xF); //0x7?
 if(ac97_buffer_entries_memory==0) {
  memory_error_debug(0x0000FF);
 }
 outd(ac97_nabm_base + 0x10, ac97_buffer_entries_memory);
 
 //set variabiles
 ac97_variabile_sound_rate_present = (inw(ac97_nam_base + 0x28) & 0x1);
 sound_volume = 100;
 ac97_free_entry = 0;
}

void ac97_set_volume(byte_t volume) {
 if(volume==100) {
  outw(ac97_nam_base + 0x18, 0); //full volume
  return;
 }
 if(volume==0) {
  outw(ac97_nam_base + 0x18, 0x8000); //mute
  return;
 }
 
 //recalculate 0-100 scale to AC97 0-32 volume scale
 outw(ac97_nam_base + 0x18, (((100-volume)*32/100) | (((100-volume)*32/100)<<8)));
}

byte_t ac97_is_supported_sample_rate(word_t sample_rate) {
 if(ac97_variabile_sound_rate_present==0x1) {
  if(sample_rate==48000 || sample_rate==44100) {
   return STATUS_GOOD;
  }
  else if(sample_rate==32000 || sample_rate==22050 || sample_rate==16000 || sample_rate==11025 || sample_rate==8000) {
   //TODO: check if these sample rates are supported
   return STATUS_ERROR;
  }
  else {
   return STATUS_ERROR;
  }
 }
 else {
  if(sample_rate==48000) {
   return STATUS_GOOD;
  }
  else {
   return STATUS_ERROR;
  }
 }
}

void ac97_set_sample_rate(word_t sample_rate) {
 if(ac97_variabile_sound_rate_present==0x1) {
  outw(ac97_nam_base + 0x2A, 0x1); //enable variabile sample rate
  outw(ac97_nam_base + 0x2C, sample_rate);
  outw(ac97_nam_base + 0x2E, sample_rate);
  outw(ac97_nam_base + 0x30, sample_rate);
  outw(ac97_nam_base + 0x32, sample_rate);
 }
 else {
  return;
 }
}

void ac97_play_sound(void) {
 ac97_fill_buffer_entry();
 outb(ac97_nabm_base + 0x1B, 0x1);
 outw(ac97_nabm_base + 0x16, 0x1C);
 ac97_playing_state = AC97_BUFFER_NOT_FILLED;
}

void ac97_stop_sound(void) {
 outb(ac97_nabm_base + 0x1B, 0x0);
 outw(ac97_nabm_base + 0x16, 0x1C);
 ac97_playing_state = AC97_BUFFER_FILLED;
}

void ac97_clear_buffer(void) {
 dword_t *buffer = (dword_t *) ac97_buffer_entries_memory;
 
 for(int i=0; i<64; i++) {
  *buffer = 0;
  buffer++;
 }
 
 ac97_free_entry = ((inb(ac97_nabm_base + 0x1A)+1) & 0x1F);
}

void ac97_fill_buffer_entry(void) {
 dword_t *buffer = (dword_t *) ac97_buffer_entries_memory;
 
 //test if we have free entry
 if(ac97_sound_num_of_samples==0 || ac97_free_entry==inb(ac97_nabm_base + 0x14)) {
  return;
 }
 
 //fill entry
 if(ac97_sound_num_of_samples>0xFFFF) {
  buffer[ac97_free_entry*2]=ac97_sound_memory;
  buffer[ac97_free_entry*2+1]=0xFFFE;
  ac97_sound_memory += 0xFFFF*2;
  ac97_sound_num_of_samples -= 0xFFFF;
 }
 else {
  buffer[ac97_free_entry*2]=ac97_sound_memory;
  buffer[ac97_free_entry*2+1]=((ac97_sound_num_of_samples-1) | 0x40000000);
  ac97_sound_memory += ac97_sound_num_of_samples*2;
  ac97_sound_num_of_samples = 0;
  ac97_playing_state = AC97_BUFFER_FILLED;
 }
 
 //update Last Valid Entry register
 outb(ac97_nabm_base + 0x15, ac97_free_entry);
 
 //next entry
 ac97_free_entry = ((ac97_free_entry+1) & 0x1F);
}

void ac97_play_memory(dword_t sound_memory, dword_t number_of_samples_in_one_channel) {  
 //reset stream
 outb(ac97_nabm_base + 0x1B, 0x2);
 ticks = 0;
 while((inb(ac97_nabm_base + 0x1B) & 0x2)==0x2) {
  if(ticks>100) {
   return;
  }
 }
 outb(ac97_nabm_base + 0x15, 0);
 outd(ac97_nabm_base + 0x10, ac97_buffer_entries_memory);
 ac97_free_entry = 0;
 
 ac97_clear_buffer();
 ac97_sound_memory = sound_memory;
 ac97_sound_num_of_samples = (number_of_samples_in_one_channel*2);
 ac97_play_sound(); 
}

void ac97_stop_playing_memory(void) {
 ac97_stop_sound();
 ac97_clear_buffer();
}
