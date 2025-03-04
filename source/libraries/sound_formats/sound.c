//BleskOS

/*
* MIT License
* Copyright (c) 2023-2025 BleskOS developers
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

/* delete later */
dword_t create_sound(dword_t channels, dword_t bits_per_sample, dword_t sample_rate, dword_t length_of_data) {
 dword_t sound_info_mem = (dword_t) aligned_calloc(SOUND_SIZE_OF_INFO_IN_BYTES+length_of_data, 0x7F);
 dword_t *sound_info = (dword_t *) sound_info_mem;

 sound_info[SOUND_INFO_NUMBER_OF_CHANNELS] = channels;
 sound_info[SOUND_INFO_BITS_PER_SAMPLE] = bits_per_sample;
 sound_info[SOUND_INFO_SAMPLE_RATE] = sample_rate;
 sound_info[SOUND_INFO_LENGTH_OF_DATA] = length_of_data;
 sound_info[SOUND_INFO_BYTES_PER_SAMPLE] = bits_per_sample/8;
 if(bits_per_sample%8!=0) {
  sound_info[SOUND_INFO_BYTES_PER_SAMPLE]++;
 }
 
 return sound_info_mem;
}

void delete_sound(dword_t sound_info_mem) {
 free((void *)sound_info_mem);
}

dword_t get_sound_data_memory(dword_t sound_info_mem) {
 return sound_info_mem+SOUND_SIZE_OF_INFO_IN_BYTES;
}

dword_t convert_pcm_to_2_channels_16_bit_samples_48000_44100_sample_rate(dword_t pcm_sound_memory, dword_t length_of_data, dword_t channels, dword_t bits_per_sample, dword_t sample_rate) {
 if(is_supported_sound_format(2, 16, 48000)==STATUS_FALSE || bits_per_sample==0 || bits_per_sample>32 || (bits_per_sample%8)!=0) {
  return STATUS_ERROR;
 }
 
 //test if we can also use second most basic format with sampling rate 44100
 dword_t can_44100_sample_rate_be_used = STATUS_FALSE;
 if(is_supported_sound_format(2, 16, 44100)==STATUS_TRUE) {
  can_44100_sample_rate_be_used = STATUS_TRUE;
 }

 //calculate basic values
 log("\nPCM: convert sound ");
 log_var_with_space(channels);
 log_var_with_space(bits_per_sample);
 log_var_with_space(sample_rate);
 dword_t bytes_per_sample = (bits_per_sample/8);
 dword_t number_of_samples = (length_of_data/channels/bytes_per_sample);
 dword_t how_many_times_to_multiply_sample = 0;
 converted_pcm_data_sample_rate = 48000;
 if(sample_rate==48000) {
  how_many_times_to_multiply_sample = 1;
 }
 else if(sample_rate==24000) {
  how_many_times_to_multiply_sample = 2;
 }
 else if(sample_rate==12000) {
  how_many_times_to_multiply_sample = 4;
 }
 else {
  if(can_44100_sample_rate_be_used==STATUS_TRUE) {
   if(sample_rate==44100) {
    how_many_times_to_multiply_sample = 1;
    converted_pcm_data_sample_rate = 44100;
   }
   else if(sample_rate==22050) {
    how_many_times_to_multiply_sample = 2;
    converted_pcm_data_sample_rate = 44100;
   }
   else if(sample_rate==11025) {
    how_many_times_to_multiply_sample = 4;
    converted_pcm_data_sample_rate = 44100;
   }
   else {
    how_many_times_to_multiply_sample = 0xFFFFFFFF; //simple multiplying can not be used TODO:
   }
  }
  else {
   how_many_times_to_multiply_sample = 0xFFFFFFFF; //simple multiplying can not be used TODO:
  }
 }

 //TODO:
 if(how_many_times_to_multiply_sample==0xFFFFFFFF) {
  return STATUS_ERROR;
 }
 dword_t converted_pcm_sound_memory = (dword_t) calloc(4*number_of_samples*how_many_times_to_multiply_sample);

 //convert PCM data
 byte_t *pcm_memory_input = (byte_t *) (pcm_sound_memory);
 word_t *pcm_memory_output = (word_t *) (converted_pcm_sound_memory);
 for(dword_t i=0, output_sample_first_channel = 0, output_sample_second_channel = 0; i<number_of_samples; i++) {
  //read samples from channels
  if(bits_per_sample==8) {
   output_sample_first_channel = pcm_memory_input[0];
   output_sample_first_channel *= 0x40;
   if(channels>1) {
    output_sample_second_channel = pcm_memory_input[1];
    output_sample_second_channel *= 0x40;
   }
  }
  else {
   output_sample_first_channel = (pcm_memory_input[0] | (pcm_memory_input[1]<<8));
   if(channels>1) {
    output_sample_second_channel = (pcm_memory_input[(bytes_per_sample)] | (pcm_memory_input[(bytes_per_sample+1)]<<8));
   }
  }
  if(channels==1) {
   output_sample_second_channel = output_sample_first_channel;
  }

  //put samples to output according to sample rate
  for(dword_t j=0; j<how_many_times_to_multiply_sample; j++) {
   *pcm_memory_output = output_sample_first_channel;
   pcm_memory_output++;
   *pcm_memory_output = output_sample_second_channel;
   pcm_memory_output++;
  }

  //skip sample
  pcm_memory_input += (bytes_per_sample*channels);
 }

 converted_pcm_data_length = (4*number_of_samples*how_many_times_to_multiply_sample);
 return converted_pcm_sound_memory;
}
/* up to here */

struct audio_file_t *process_audio_file(byte_t audio_file_type, byte_t *audio_file_memory, dword_t audio_file_length) {
 //allocate audio file info structure
 struct audio_file_t *audio_file_info = (struct audio_file_t *) (malloc(sizeof(struct audio_file_t)));

 //save basic audio file info
 audio_file_info->type = audio_file_type;
 audio_file_info->file_pointer = audio_file_memory;
 audio_file_info->file_size = audio_file_length;

 //read informations specific by format
 if(audio_file_type==AUDIO_FILE_TYPE_WAV) {
  audio_file_info->file_info = read_wav_info(audio_file_memory, audio_file_length);

  if(audio_file_info->file_info==STATUS_ERROR) {
   free((void *)audio_file_info);
   return STATUS_ERROR;
  }

  struct wav_info_t *wav_info = (struct wav_info_t *) (audio_file_info->file_info);
  audio_file_info->sample_rate = wav_info->output_pcm_data_sample_rate;
  audio_file_info->output_length = wav_info->length_of_output_pcm_data;
 }
 else if(audio_file_type==AUDIO_FILE_TYPE_CDDA) {
  audio_file_info->sample_rate = 44100;
  audio_file_info->output_length = audio_file_length;
 }
 else if(audio_file_type==AUDIO_FILE_TYPE_MP3) {
  audio_file_info->file_info = read_mp3_info(audio_file_memory, audio_file_length);

  if(audio_file_info->file_info==STATUS_ERROR) {
   free((void *)audio_file_info);
   return STATUS_ERROR;
  }

  struct mp3_info_t *mp3_info = (struct mp3_info_t *) (audio_file_info->file_info);
  audio_file_info->sample_rate = mp3_info->output_pcm_data_sample_rate;
  audio_file_info->output_length = mp3_info->length_of_output_pcm_data;
 }

 //calculate time of audio file
 audio_file_info->length_seconds = ((audio_file_info->output_length/4)%(audio_file_info->sample_rate*60)/audio_file_info->sample_rate);
 audio_file_info->length_minutes = ((audio_file_info->output_length/4)%(audio_file_info->sample_rate*3600)/(audio_file_info->sample_rate*60));
 audio_file_info->length_hours = ((audio_file_info->output_length/4)/(audio_file_info->sample_rate*3600));

 //clear other variables
 audio_file_info->actually_played_position = 0;
 audio_file_info->played_length_seconds = 0;
 audio_file_info->played_length_minutes = 0;
 audio_file_info->played_length_hours = 0;

 return audio_file_info;
}

void play_audio_file(struct audio_file_t *audio_file_info, dword_t offset) {
 //offset bigger than audio length is invalid
 if(offset>=audio_file_info->output_length) {
  return;
 }

 //round offset to point to valid audio frame (16 bits per sample + 2 channels = 4 bytes = round to 0b00)
 offset &= 0xFFFFFFFC;

 //update variables in audio file info
 audio_calculate_time_of_sound_data_offset(audio_file_info, offset);
 audio_file_info->offset_to_start_of_playing = offset;

 //save audio file info pointer for updates during playing of file
 actually_played_audio_file = audio_file_info;

 //play audio file
 if(audio_file_info->type==AUDIO_FILE_TYPE_WAV) {
  play_wav(audio_file_info->file_info, offset);
 }
 else if(audio_file_info->type==AUDIO_FILE_TYPE_CDDA) {
  play_cdda(audio_file_info->file_pointer, audio_file_info->output_length, offset);
 }
 else if(audio_file_info->type==AUDIO_FILE_TYPE_MP3) {
  play_mp3(audio_file_info->file_info, offset);
 }
}

void audio_calculate_time_of_sound_data_offset(struct audio_file_t *audio_file_info, dword_t actually_played_position) {
 //save position
 audio_file_info->actually_played_position = actually_played_position;

 //calculate time of this position
 audio_file_info->played_length_seconds = ((audio_file_info->actually_played_position/4)%(audio_file_info->sample_rate*60)/audio_file_info->sample_rate);
 audio_file_info->played_length_minutes = ((audio_file_info->actually_played_position/4)%(audio_file_info->sample_rate*3600)/(audio_file_info->sample_rate*60));
 audio_file_info->played_length_hours = ((audio_file_info->actually_played_position/4)/(audio_file_info->sample_rate*3600));
}

void destroy_audio_file(struct audio_file_t *audio_file_info) {
 free((void *)audio_file_info->file_pointer);
 if(audio_file_info->type==AUDIO_FILE_TYPE_MP3) {
  struct mp3_info_t *mp3_info = (struct mp3_info_t *) (audio_file_info->file_info);
  free((void *)mp3_info->mp3_index);
 }
 free((void *)audio_file_info->file_info);
 free((void *)audio_file_info);
}