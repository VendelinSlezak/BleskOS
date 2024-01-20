//BleskOS

/*
* MIT License
* Copyright (c) 2023-2024 Vendelín Slezák
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
  log("\nWAV: invalid signature");
  return STATUS_ERROR;
 }
 
 //parse chunks
 wav32 = (dword_t *) (wav_memory+12);
 while((dword_t)wav32<wav_length && wav32[0]!=0x00000000) {
  if(wav32[0]==0x20746D66) { //fmt 
   //check sound format
   if((wav32[2] & 0xFFFF)!=WAV_FORMAT_PCM) {
    log("\nWAV: not raw PCM");
    return STATUS_ERROR;
   }
   
   //read other data about sound
   channels = (wav32[2]>>16);
   sample_rate = wav32[3];
   bits_per_sample = (wav32[5]>>16);
  }
  else if(wav32[0]==0x61746164) { //data   
   if(channels==0 || sample_rate==0 || bits_per_sample==0) {
    log("\nWAV: data chunk before fmt chunk");
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
   }
   
   return sound_memory;
  }
  else {
   log("\nWAV: unknown chunk ");
   log_hex(wav32[0]);
  }
  
  //go to next chunk
  wav32 = (dword_t *) ((dword_t)wav32+wav32[1]+8);
 }
 
 return STATUS_ERROR;
}

void convert_sound_data_to_wav(dword_t sound_info_mem) {
 dword_t *sound_info = (dword_t *) (sound_info_mem);
 dword_t wav_memory = malloc(sound_info[SOUND_INFO_LENGTH_OF_DATA]+44);
 dword_t *wav32 = (dword_t *) (wav_memory);

 //create info
 wav32[0] = 0x46464952; //'RIFF'
 wav32[1] = (sound_info[SOUND_INFO_LENGTH_OF_DATA]+44); //length of WAV file
 wav32[2] = 0x45564157; //'WAVE'
 wav32[3] = 0x20746D66; //'fmt '
 wav32[4] = 16; //chunk size
 wav32[5] = ((WAV_FORMAT_PCM) | (sound_info[SOUND_INFO_NUMBER_OF_CHANNELS]<<16)); //sound format and number of channels
 wav32[6] = sound_info[SOUND_INFO_SAMPLE_RATE]; //sample rate
 wav32[7] = ((sound_info[SOUND_INFO_SAMPLE_RATE]*sound_info[SOUND_INFO_BITS_PER_SAMPLE]*sound_info[SOUND_INFO_NUMBER_OF_CHANNELS])/8); //data rate
 wav32[8] = ((sound_info[SOUND_INFO_BYTES_PER_SAMPLE]*sound_info[SOUND_INFO_NUMBER_OF_CHANNELS]) | (sound_info[SOUND_INFO_BITS_PER_SAMPLE]<<16)); //data block size and bits per sample
 wav32[9] = 0x61746164; //'data'
 wav32[10] = sound_info[SOUND_INFO_LENGTH_OF_DATA]; //length of sound data

 //copy sound data
 copy_memory(get_sound_data_memory(sound_info_mem), wav_memory+44, sound_info[SOUND_INFO_LENGTH_OF_DATA]);

 //set variables
 converted_file_memory = wav_memory;
 converted_file_size = (sound_info[SOUND_INFO_LENGTH_OF_DATA]+44);
}