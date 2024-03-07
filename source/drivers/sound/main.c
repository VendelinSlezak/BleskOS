//BleskOS

/*
* MIT License
* Copyright (c) 2023-2024 Vendelín Slezák
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

void initalize_sound_card(void) {
 //found usable sound card and initalize it
 selected_sound_card = NO_SOUND_CARD;
 for(dword_t i=0; i<number_of_sound_cards; i++) {
  if(sound_cards_info[i].driver==SOUND_CARD_DRIVER_AC97) {
   initalize_ac97_sound_card(i); //we will initalize only one AC97 sound card
   selected_sound_card = i;
   break;
  }
  else if(sound_cards_info[i].driver==SOUND_CARD_DRIVER_HDA) {
   hda_initalize_sound_card(i); //we will initalize only one HDA sound card
   selected_sound_card = i;
   if(hda_is_initalized_useful_output==STATUS_TRUE) {
    break;
   }
   //else we will try to find another sound card that have useful output
  }
 }
 
 //set sound
 sound_set_volume(50);
}

void sound_set_volume(byte_t volume) {
 //set volume on sound card
 if(selected_sound_card!=NO_SOUND_CARD) {
  if(sound_cards_info[selected_sound_card].driver==SOUND_CARD_DRIVER_AC97) {
   ac97_set_volume(volume);
  }
  else if(sound_cards_info[selected_sound_card].driver==SOUND_CARD_DRIVER_HDA) {
   hda_set_volume(selected_sound_card, volume);
  }
 }

 //update global variable
 sound_volume = volume;
}

byte_t is_supported_sound_format(byte_t channels, byte_t bits_per_channel, dword_t sample_rate) {
 if(selected_sound_card==NO_SOUND_CARD) {
  return STATUS_ERROR;
 }

 if(sound_cards_info[selected_sound_card].driver==SOUND_CARD_DRIVER_AC97) {
  if(channels==2 && bits_per_channel==16) {
   return ac97_is_supported_sample_rate(sample_rate);
  }
  else {
   return STATUS_ERROR;
  }
 }
 else if(sound_cards_info[selected_sound_card].driver==SOUND_CARD_DRIVER_HDA) {
  if(channels==2 && hda_is_supported_channel_size(selected_sound_card, bits_per_channel)==STATUS_GOOD && hda_is_supported_sample_rate(selected_sound_card, sample_rate)==STATUS_GOOD) {
   return STATUS_GOOD;
  }
  else {
   return STATUS_ERROR;
  }
 }
 
 return STATUS_ERROR; 
}

void play_new_sound(dword_t sound_memory, dword_t channels, dword_t bits_per_sample, dword_t sample_rate, dword_t number_of_samples) {
 if(selected_sound_card!=NO_SOUND_CARD) {
  if(sound_cards_info[selected_sound_card].driver==SOUND_CARD_DRIVER_AC97) {
   ac97_play_memory(sound_memory, number_of_samples*(bits_per_sample/8), sample_rate);
  }
  else if(sound_cards_info[selected_sound_card].driver==SOUND_CARD_DRIVER_HDA) {
   hda_play_memory(selected_sound_card, sound_memory, sample_rate, channels, bits_per_sample, (number_of_samples/channels));
  }
 }
}

void pause_sound(void) {
 if(selected_sound_card!=NO_SOUND_CARD) {
  if(sound_cards_info[selected_sound_card].driver==SOUND_CARD_DRIVER_AC97) {
   ac97_stop_sound();
  }
  else if(sound_cards_info[selected_sound_card].driver==SOUND_CARD_DRIVER_HDA) {
   hda_stop_sound(selected_sound_card);
  }
 }
}

void play_sound(void) {
 if(selected_sound_card!=NO_SOUND_CARD) {
  if(sound_cards_info[selected_sound_card].driver==SOUND_CARD_DRIVER_AC97) {
   ac97_play_sound();
  }
  else if(sound_cards_info[selected_sound_card].driver==SOUND_CARD_DRIVER_HDA) {
   hda_resume_sound(selected_sound_card);
  }
 }
}