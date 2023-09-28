//BleskOS

/*
* MIT License
* Copyright (c) 2023 Vendelín Slezák
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

dword_t create_sound(dword_t channels, dword_t bits_per_sample, dword_t sample_rate, dword_t length_of_data) {
 dword_t sound_info_mem = aligned_malloc(SOUND_SIZE_OF_INFO_IN_BYTES+length_of_data, 0x7F);
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
 free(sound_info_mem);
}

dword_t get_sound_data_memory(dword_t sound_info_mem) {
 return sound_info_mem+SOUND_SIZE_OF_INFO_IN_BYTES;
}
