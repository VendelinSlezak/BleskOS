//BleskOS

/*
* MIT License
* Copyright (c) 2023-2025 Vendelín Slezák
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

mp3dec_t *mp3_decoder_working_area;
mp3dec_frame_info_t *mp3_frame_info;
word_t *mp3_frame_pcm_output;

struct mp3_index_t {
 byte_t *mp3_frame_pointer;
 dword_t pcm_data_offset;
}__attribute__((packed));

struct mp3_info_t {
 byte_t *mp3_file;
 struct mp3_index_t *mp3_index;
 dword_t size_of_mp3_file;

 dword_t length_of_pcm_data;
 byte_t pcm_data_number_of_channels;
 word_t pcm_data_sample_rate;

 dword_t length_of_output_pcm_data; //number of bytes that sound card will play
 word_t output_pcm_data_sample_rate;

 dword_t output_pcm_data_buffer_size;
 byte_t number_of_frames_in_buffer;

 dword_t mp3_first_frame_pcm_data_offset; //used when playing starts from middle of file
}__attribute__((packed));

struct mp3_info_t *actually_played_mp3_info;

void initalize_mp3_decoder(void);
dword_t convert_mp3_to_sound_data(dword_t mp3_memory, dword_t mp3_length);
struct mp3_info_t *read_mp3_info(byte_t *mp3_memory, dword_t mp3_length);
dword_t play_mp3(struct mp3_info_t *mp3_info, dword_t offset_to_first_byte_to_play);
void mp3_refill_buffer(byte_t *buffer);