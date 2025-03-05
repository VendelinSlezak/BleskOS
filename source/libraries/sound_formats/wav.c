//BleskOS

/*
* MIT License
* Copyright (c) 2023-2025 BleskOS developers
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

dword_t convert_wav_to_sound_data(dword_t wav_memory, dword_t wav_length) {
 dword_t *wav32 = (dword_t *) wav_memory;
 dword_t channels = 0, bits_per_sample = 0, sample_rate = 0, sound_memory = 0;
 wav_length += wav_memory;
 
 //check signature
 if(wav32[0]!=0x46464952 && wav32[2]!=0x45564157) {
  logf("\nWAV: invalid signature");
  return STATUS_ERROR;
 }
 
 //parse chunks
 wav32 = (dword_t *) (wav_memory+12);
 while((dword_t)wav32<wav_length && wav32[0]!=0x00000000) {
  if(wav32[0]==0x20746D66) { //fmt 
   //check sound format
   if((wav32[2] & 0xFFFF)!=WAV_FORMAT_PCM) {
    logf("\nWAV: not raw PCM");
    return STATUS_ERROR;
   }
   
   //read other data about sound
   channels = (wav32[2]>>16);
   sample_rate = wav32[3];
   bits_per_sample = (wav32[5]>>16);
  }
  else if(wav32[0]==0x61746164) { //data   
   if(channels==0 || sample_rate==0 || bits_per_sample==0) {
    logf("\nWAV: data chunk before fmt chunk");
    return STATUS_ERROR;
   }

   //convert PCM data
   dword_t converted_pcm_data_memory = convert_pcm_to_2_channels_16_bit_samples_48000_44100_sample_rate(((dword_t)wav32+8), wav32[1], channels, bits_per_sample, sample_rate);
   
   //copy sound data
   if(converted_pcm_data_memory==STATUS_ERROR) {
    sound_memory = create_sound(channels, bits_per_sample, sample_rate, wav32[1]);
    copy_memory(((dword_t)wav32+8), (get_sound_data_memory(sound_memory)), wav32[1]);
   }
   else {
    sound_memory = create_sound(2, 16, converted_pcm_data_sample_rate, converted_pcm_data_length);
    copy_memory(converted_pcm_data_memory, (get_sound_data_memory(sound_memory)), converted_pcm_data_length);
    free((void *)converted_pcm_data_memory);
   }
   
   return sound_memory;
  }
  else {
   logf("\nWAV: unknown chunk 0x%x", wav32[0]);
  }
  
  //go to next chunk
  wav32 = (dword_t *) ((dword_t)wav32+wav32[1]+8);
 }
 
 return STATUS_ERROR;
}

struct wav_info_t *read_wav_info(byte_t *wav_memory, dword_t wav_length) {
 struct wav_info_t *wav_info = (struct wav_info_t *) (calloc(sizeof(struct wav_info_t)));
 
 dword_t *wav32 = (dword_t *) wav_memory;
 dword_t end_of_wav = ((dword_t)wav_memory + wav_length);
 
 //check signature
 if(wav32[0]!=0x46464952 && wav32[2]!=0x45564157) {
  logf("\nWAV: invalid signature");
  return STATUS_ERROR;
 }
 
 //parse chunks
 wav32 = (dword_t *) ((dword_t)wav_memory+12);
 while((dword_t)wav32<end_of_wav && wav32[0]!=0x00000000) {
  if(wav32[0]==0x20746D66) { //fmt 
   //check sound format
   if((wav32[2] & 0xFFFF)!=WAV_FORMAT_PCM) {
    logf("\nWAV: not raw PCM");
    return STATUS_ERROR;
   }
   
   //read PCM data format
   wav_info->pcm_data_number_of_channels = (wav32[2]>>16);
   wav_info->pcm_data_sample_rate = wav32[3];
   wav_info->pcm_data_bits_per_sample = (wav32[5]>>16);

   //recalculate what sample rate we will use
   wav_info->output_pcm_data_sample_rate = 0;
   if(wav_info->pcm_data_sample_rate==48000 || wav_info->pcm_data_sample_rate==24000 || wav_info->pcm_data_sample_rate==12000) {
    wav_info->output_pcm_data_sample_rate = 48000;
   }
   else if(wav_info->pcm_data_sample_rate==44100 || wav_info->pcm_data_sample_rate==22050 || wav_info->pcm_data_sample_rate==11025) {
    wav_info->output_pcm_data_sample_rate = 44100;
   }
  }
  else if(wav32[0]==0x61746164) { //data   
   if(wav_info->pcm_data_number_of_channels==0 || wav_info->pcm_data_sample_rate==0 || wav_info->pcm_data_bits_per_sample==0) {
    logf("\nWAV: data chunk before fmt chunk");
    return STATUS_ERROR;
   }

   //read info about PCM data
   wav_info->start_of_pcm_data = ((dword_t)wav32+8);
   wav_info->length_of_pcm_data = wav32[1];
   if(wav_info->output_pcm_data_sample_rate!=0 && (wav_info->pcm_data_bits_per_sample==8 || wav_info->pcm_data_bits_per_sample==16)) {
    //recalculate length for 2 channels
    wav_info->length_of_output_pcm_data = ((wav_info->length_of_pcm_data/wav_info->pcm_data_number_of_channels)*2);

    //recalculate according 16 bits per sample
    if(wav_info->pcm_data_bits_per_sample==8) {
     wav_info->length_of_output_pcm_data *= 2;
    }

    //recalculate according to new sample rate
    wav_info->length_of_output_pcm_data = (wav_info->length_of_output_pcm_data*(wav_info->output_pcm_data_sample_rate/wav_info->pcm_data_sample_rate));
   }
   else {
    logf("Unplayable WAV: %d %d %d", wav_info->pcm_data_number_of_channels, wav_info->pcm_data_sample_rate, wav_info->pcm_data_bits_per_sample);
    wav_info->length_of_output_pcm_data = 0;
    return STATUS_ERROR;
   }
   
   return wav_info;
  }
  else {
   logf("\nWAV: unknown chunk 0x%x", wav32[0]);
  }
  
  //go to next chunk
  wav32 = (dword_t *) ((dword_t)wav32+wav32[1]+8);
 }
 
 return STATUS_ERROR;
}

dword_t play_wav(struct wav_info_t *wav_info, dword_t offset_to_first_byte_to_play) {
 //calculate mp3 PCM data offset
 dword_t offset_to_first_byte_to_pcm_data_in_original_wav = ((offset_to_first_byte_to_play/2/2)/(wav_info->output_pcm_data_sample_rate/wav_info->pcm_data_sample_rate)*(wav_info->pcm_data_number_of_channels*(wav_info->pcm_data_bits_per_sample/8)));
 
 //save wav info pointer
 actually_played_wav_info = wav_info;
 
 //play wav file
 play_sound_with_refilling_buffer((byte_t *)wav_info->start_of_pcm_data+offset_to_first_byte_to_pcm_data_in_original_wav, wav_info->length_of_pcm_data-offset_to_first_byte_to_pcm_data_in_original_wav, wav_info->length_of_output_pcm_data-offset_to_first_byte_to_play, wav_info->output_pcm_data_sample_rate, wav_info->output_pcm_data_sample_rate*4, wav_refill_buffer); //buffer contains 1 sec of PCM data
}

void wav_refill_buffer(byte_t *buffer) {
 //clear buffer
 clear_memory((dword_t)buffer, sound_buffer_refilling_info->buffer_size);

 //exit if we already converted all file
 if(sound_buffer_refilling_info->source_data_length==0) {
  return;
 }

 //fill buffer
 word_t first_channel_sample = 0, second_channel_sample = 0;
 dword_t number_of_frames_to_convert = 0, length_of_processed_data = 0;
 if(sound_buffer_refilling_info->source_data_length<actually_played_wav_info->pcm_data_sample_rate) {
  number_of_frames_to_convert = (sound_buffer_refilling_info->source_data_length/(actually_played_wav_info->pcm_data_number_of_channels*(actually_played_wav_info->pcm_data_bits_per_sample/8)));
 }
 else {
  number_of_frames_to_convert = actually_played_wav_info->pcm_data_sample_rate;
 }

 for(dword_t i=0; i<number_of_frames_to_convert; i++) {
  word_t *wav = (word_t *) (sound_buffer_refilling_info->source_data_pointer);

  //read frame
  if(actually_played_wav_info->pcm_data_bits_per_sample==16) {
   first_channel_sample = wav[0];
   second_channel_sample = wav[1];
  }
  else if(actually_played_wav_info->pcm_data_bits_per_sample==8) {
   first_channel_sample = (wav[0] & 0xFF)*0x40;
   second_channel_sample = (wav[0] >> 8)*0x40;
  }

  if(actually_played_wav_info->pcm_data_number_of_channels==1) {
   second_channel_sample = first_channel_sample;
  }
  
  //save frame according to new sample rate
  for(dword_t j=0; j<(actually_played_wav_info->output_pcm_data_sample_rate/actually_played_wav_info->pcm_data_sample_rate); j++) {
   buffer[0] = (first_channel_sample & 0xFF);
   buffer[1] = (first_channel_sample >> 8);
   buffer[2] = (second_channel_sample & 0xFF);
   buffer[3] = (second_channel_sample >> 8);
   buffer += 4;
  }

  //go to next frame
  sound_buffer_refilling_info->source_data_pointer = (sound_buffer_refilling_info->source_data_pointer+(actually_played_wav_info->pcm_data_number_of_channels*(actually_played_wav_info->pcm_data_bits_per_sample/8)));
  sound_buffer_refilling_info->source_data_length -= (actually_played_wav_info->pcm_data_number_of_channels*(actually_played_wav_info->pcm_data_bits_per_sample/8));
 }
}

void convert_sound_data_to_wav(byte_t *pcm_data_pointer, dword_t size_of_pcm_data_in_bytes, byte_t bits_per_sample, byte_t number_of_channels, word_t sample_rate) {
 dword_t wav_memory = (dword_t) malloc(size_of_pcm_data_in_bytes+44);
 dword_t *wav32 = (dword_t *) (wav_memory);

 //create info
 wav32[0] = 0x46464952; //'RIFF'
 wav32[1] = (size_of_pcm_data_in_bytes+44); //length of WAV file
 wav32[2] = 0x45564157; //'WAVE'
 wav32[3] = 0x20746D66; //'fmt '
 wav32[4] = 16; //chunk size
 wav32[5] = ((WAV_FORMAT_PCM) | (number_of_channels<<16)); //sound format and number of channels
 wav32[6] = sample_rate; //sample rate
 wav32[7] = ((sample_rate*bits_per_sample*number_of_channels)/8); //data rate
 wav32[8] = (((bits_per_sample/8)*number_of_channels) | (bits_per_sample<<16)); //data block size and bits per sample
 wav32[9] = 0x61746164; //'data'
 wav32[10] = size_of_pcm_data_in_bytes; //length of sound data

 //copy sound data
 copy_memory((dword_t)pcm_data_pointer, wav_memory+44, size_of_pcm_data_in_bytes);

 //set variables
 converted_file_memory = wav_memory;
 converted_file_size = (size_of_pcm_data_in_bytes+44);
}