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
 sound_set_volume(30);

 //allocate memory for playing sound
 sound_buffer_refilling_info = (struct sound_buffer_refilling_info_t *) (malloc(sizeof(struct sound_buffer_refilling_info_t)));
 pcm_data = (byte_t *) (malloc(SIZE_OF_PCM_DATA_BUFFER));
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

void play_sound_with_refilling_buffer(byte_t *source_data_pointer, dword_t source_data_length, dword_t size_of_full_pcm_output_in_bytes, dword_t sample_rate, dword_t size_of_buffer, void (*fill_buffer)(byte_t *buffer)) {
 //set info about sound
 sound_buffer_refilling_info->source_data_pointer = source_data_pointer;
 sound_buffer_refilling_info->source_data_length = source_data_length;
 sound_buffer_refilling_info->fill_buffer = fill_buffer;
 sound_buffer_refilling_info->buffer_size = size_of_buffer;
 sound_buffer_refilling_info->buffer_0_pointer = (byte_t *) pcm_data;
 sound_buffer_refilling_info->buffer_1_pointer = (byte_t *) ((dword_t)pcm_data+size_of_buffer);
 sound_buffer_refilling_info->actually_playing_buffer = SOUND_BUFFER_0;
 sound_buffer_refilling_info->last_filled_buffer = SOUND_BUFFER_0;
 sound_buffer_refilling_info->played_bytes_by_finished_buffers = 0;
 sound_buffer_refilling_info->played_bytes = 0;
 sound_buffer_refilling_info->size_of_full_pcm_output_in_bytes = size_of_full_pcm_output_in_bytes;

 //clear buffer
 clear_memory((dword_t)pcm_data, SIZE_OF_PCM_DATA_BUFFER);

 //load pcm data to first buffer
 (*fill_buffer)(pcm_data);
 
 //play pcm data buffer in endless loop
 if(selected_sound_card!=NO_SOUND_CARD) {
  if(sound_cards_info[selected_sound_card].driver==SOUND_CARD_DRIVER_AC97) {
   ac97_play_pcm_data_in_loop(sample_rate);
  }
  else if(sound_cards_info[selected_sound_card].driver==SOUND_CARD_DRIVER_HDA) {
   hda_play_pcm_data_in_loop(selected_sound_card, sample_rate);
  }
 }

 //add task for refilling buffer
 create_task(task_refill_sound_buffer, TASK_TYPE_PERIODIC_INTERRUPT, 10);
}

dword_t sound_get_actual_stream_position(void) {
 if(selected_sound_card!=NO_SOUND_CARD) {
  if(sound_cards_info[selected_sound_card].driver==SOUND_CARD_DRIVER_AC97) {
   return ac97_get_actual_stream_position();
  }
  else if(sound_cards_info[selected_sound_card].driver==SOUND_CARD_DRIVER_HDA) {
   return hda_get_actual_stream_position(selected_sound_card);
  }
 }
}

void task_refill_sound_buffer(void) {
 // SOUND_BUFFER_0 starts at ((dword_t)pcm_data) and ends at ((dword_t)pcm_data + sound_buffer_refilling_info->buffer_size)
 // SOUND_BUFFER_1 starts at ((dword_t)pcm_data + sound_buffer_refilling_info->buffer_size) and ends at ((dword_t)pcm_data + sound_buffer_refilling_info->buffer_size + sound_buffer_refilling_info->buffer_size)
 // sound card plays memory from ((dword_t)pcm_data) with length (sound_buffer_refilling_info->buffer_size*2) in loop
 // so by reading actual stream position we can determine which buffer is sound card actually playing
 // fill_buffer(byte_t *memory) fills byte_t *memory with sound_buffer_refilling_info->buffer_size bytes of new PCM data and it also moves sound_buffer_refilling_info->source_data_pointer and sound_buffer_refilling_info->source_data_length

 //calculate how many bytes were played
 if(sound_buffer_refilling_info->actually_playing_buffer==SOUND_BUFFER_0) {
  if(sound_get_actual_stream_position()<sound_buffer_refilling_info->buffer_size) {
   sound_buffer_refilling_info->played_bytes = (sound_buffer_refilling_info->played_bytes_by_finished_buffers+sound_get_actual_stream_position());
  }
  else {
   sound_buffer_refilling_info->played_bytes_by_finished_buffers += sound_buffer_refilling_info->buffer_size;
   sound_buffer_refilling_info->played_bytes = (sound_buffer_refilling_info->played_bytes_by_finished_buffers+sound_get_actual_stream_position()-sound_buffer_refilling_info->buffer_size);
   sound_buffer_refilling_info->actually_playing_buffer = SOUND_BUFFER_1;
  }
 }
 else {
  if(sound_get_actual_stream_position()>=sound_buffer_refilling_info->buffer_size) {
   sound_buffer_refilling_info->played_bytes = (sound_buffer_refilling_info->played_bytes_by_finished_buffers+sound_get_actual_stream_position()-sound_buffer_refilling_info->buffer_size);
  }
  else {
   sound_buffer_refilling_info->played_bytes_by_finished_buffers += sound_buffer_refilling_info->buffer_size;
   sound_buffer_refilling_info->played_bytes = (sound_buffer_refilling_info->played_bytes_by_finished_buffers+sound_get_actual_stream_position());
   sound_buffer_refilling_info->actually_playing_buffer = SOUND_BUFFER_0;
  }
 }

 //if is everything played, stop sound
 if(sound_buffer_refilling_info->played_bytes>=sound_buffer_refilling_info->size_of_full_pcm_output_in_bytes) {
  stop_sound();
  sound_buffer_refilling_info->played_bytes = sound_buffer_refilling_info->size_of_full_pcm_output_in_bytes;
 }

 //update values of actually played audio file
 audio_calculate_time_of_sound_data_offset(actually_played_audio_file, actually_played_audio_file->offset_to_start_of_playing+sound_buffer_refilling_info->played_bytes);

 //fill buffer with new data
 if(sound_buffer_refilling_info->actually_playing_buffer==sound_buffer_refilling_info->last_filled_buffer) {
  if(sound_buffer_refilling_info->last_filled_buffer==SOUND_BUFFER_0) {
   sound_buffer_refilling_info->fill_buffer((byte_t *)((dword_t)pcm_data+sound_buffer_refilling_info->buffer_size));
   sound_buffer_refilling_info->last_filled_buffer = SOUND_BUFFER_1;
  }
  else {
   sound_buffer_refilling_info->fill_buffer(pcm_data);
   sound_buffer_refilling_info->last_filled_buffer = SOUND_BUFFER_0;
  }
  asm("wbinvd"); //flush processor cache to RAM to be sure sound card will read correct data
 }
}

void stop_sound(void) {
 if(selected_sound_card!=NO_SOUND_CARD) {
  destroy_task(task_refill_sound_buffer);
  
  if(sound_cards_info[selected_sound_card].driver==SOUND_CARD_DRIVER_AC97) {
   ac97_stop_sound();
  }
  else if(sound_cards_info[selected_sound_card].driver==SOUND_CARD_DRIVER_HDA) {
   hda_stop_sound(selected_sound_card);
  }
 }
}
