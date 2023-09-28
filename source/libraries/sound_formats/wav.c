//BleskOS

/*
* MIT License
* Copyright (c) 2023 Vendelín Slezák
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
  log("WAV: invalid signature\n");
  return STATUS_ERROR;
 }
 
 //parse chunks
 wav32 = (dword_t *) (wav_memory+12);
 while((dword_t)wav32<wav_length && wav32[0]!=0x00000000) {
  if(wav32[0]==0x20746D66) { //fmt 
   //check sound format
   if((wav32[2] & 0xFFFF)!=WAV_FORMAT_PCM) {
    log("WAV: not raw PCM\n");
    return STATUS_ERROR;
   }
   
   //read other data about sound
   channels = (wav32[2]>>16);
   sample_rate = wav32[3];
   bits_per_sample = (wav32[5]>>16);
  }
  else if(wav32[0]==0x61746164) { //data   
   if(channels==0 || sample_rate==0 || bits_per_sample==0) {
    log("WAV: data chunk before fmt chunk\n");
    return STATUS_ERROR;
   }
   
   //allocate memory
   sound_memory = create_sound(channels, bits_per_sample, sample_rate, wav32[1]);
   
   //copy sound data
   copy_memory(((dword_t)wav32+8), (get_sound_data_memory(sound_memory)), wav32[1]);
   
   return sound_memory;
  }
  else {
   log("WAV: unknown chunk ");
   log_hex(wav32[0]);
   log("\n");
  }
  
  //go to next chunk
  wav32 = (dword_t *) ((dword_t)wav32+wav32[1]+8);
 }
 
 return STATUS_ERROR;
}
