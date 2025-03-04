//BleskOS

/*
* MIT License
* Copyright (c) 2023-2025 BleskOS developers
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

dword_t convert_cdda_to_sound_data(dword_t cdda_memory, dword_t cdda_length) {
 dword_t sound_memory = create_sound(2, 16, 44100, cdda_length);
 copy_memory(cdda_memory, (get_sound_data_memory(sound_memory)), cdda_length);
 return sound_memory;
}

dword_t play_cdda(byte_t *cdda_memory, dword_t cdda_length, dword_t offset_to_first_byte_to_play) {
 play_sound_with_refilling_buffer((byte_t *)((dword_t)cdda_memory+offset_to_first_byte_to_play), cdda_length-offset_to_first_byte_to_play, cdda_length-offset_to_first_byte_to_play, 44100, 44100*4, cdda_refill_buffer); //buffer contains 1 sec of PCM data
}

void cdda_refill_buffer(byte_t *buffer) {
 //clear buffer
 clear_memory((dword_t)buffer, sound_buffer_refilling_info->buffer_size);
 if(sound_buffer_refilling_info->source_data_length==0) {
  return;
 }

 //because CDDA are PCM data in optimal format (16 bits per sample + 2 channels + 44100 sample rate), we need only to copy them to buffer
 if(sound_buffer_refilling_info->source_data_length>=sound_buffer_refilling_info->buffer_size) {
  copy_memory((dword_t)sound_buffer_refilling_info->source_data_pointer, (dword_t)buffer, sound_buffer_refilling_info->buffer_size);
  
  //move pointers
  sound_buffer_refilling_info->source_data_pointer = (byte_t *) ((dword_t)sound_buffer_refilling_info->source_data_pointer+sound_buffer_refilling_info->buffer_size);
  sound_buffer_refilling_info->source_data_length -= sound_buffer_refilling_info->buffer_size;
 }
 else {
  //copy last data
  copy_memory((dword_t)sound_buffer_refilling_info->source_data_pointer, (dword_t)buffer, sound_buffer_refilling_info->source_data_length);
  sound_buffer_refilling_info->source_data_length = 0;
 }
}