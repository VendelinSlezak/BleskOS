//BleskOS

/*
* MIT License
* Copyright (c) 2023-2024 Vendelín Slezák
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

void initalize_sound_card(void) {
 if(ac97_present==DEVICE_PRESENT) {
  initalize_ac97();
 }
 else if(hda_sound_card_pointer!=0) {
  //TODO: initalize more sound cards if present
  selected_hda_sound_card = (hda_sound_card_pointer-1); //initalize last founded sound card
  hda_initalize_sound_card(selected_hda_sound_card);
 }
 
 sound_set_volume(50);
}

void sound_set_volume(byte_t volume) {
 sound_volume = volume;
 
 if(ac97_present==DEVICE_PRESENT) {
  ac97_set_volume(volume);
 }
 else if(hda_sound_card_pointer!=0) {
  hda_set_volume(selected_hda_sound_card, volume);
 }
}

byte_t is_supported_sound_format(byte_t channels, byte_t bits_per_channel, dword_t sample_rate) {
 if(ac97_present==DEVICE_PRESENT) {
  if(channels==2 && bits_per_channel==16) {
   return ac97_is_supported_sample_rate(sample_rate);
  }
  else {
   return STATUS_ERROR;
  }
 }
 else if(hda_sound_card_pointer!=0) {
  if(channels==2) { //TODO: more channels
   if(hda_is_supported_channel_size(selected_hda_sound_card, bits_per_channel)==STATUS_GOOD || hda_is_supported_sample_rate(selected_hda_sound_card, sample_rate)) {
    return STATUS_GOOD;
   }
   else {
    return STATUS_ERROR;
   }
  }
  else {
   return STATUS_ERROR;
  }
 }
 
 return STATUS_ERROR; 
}

void play_new_sound(dword_t sound_memory, dword_t channels, dword_t bits_per_sample, dword_t sample_rate, dword_t number_of_samples) {
 if(ac97_present==DEVICE_PRESENT) {
  ac97_stop_sound();
  ac97_set_sample_rate(sample_rate);
  ac97_play_memory(sound_memory, (number_of_samples/channels));
 }
 else if(hda_sound_card_pointer!=0) {
  hda_play_memory(selected_hda_sound_card, sound_memory, sample_rate, channels, bits_per_sample, (number_of_samples/channels));
 }
}

void pause_sound(void) {
 if(ac97_present==DEVICE_PRESENT) {
  ac97_stop_sound();
 }
 else if(hda_sound_card_pointer!=0) {
  hda_stop_sound(selected_hda_sound_card);
 }
}

void play_sound(void) {
 if(ac97_present==DEVICE_PRESENT) {
  ac97_play_sound();
 }
 else if(hda_sound_card_pointer!=0) {
  hda_resume_sound(selected_hda_sound_card);
 }
}