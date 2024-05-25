//BleskOS

/*
* MIT License
* Copyright (c) 2023-2024 Vendelín Slezák
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#define WAV_FORMAT_PCM 0x0001

struct wav_info_t {
 dword_t start_of_pcm_data;
 dword_t length_of_pcm_data;
 byte_t pcm_data_number_of_channels;
 word_t pcm_data_sample_rate;
 byte_t pcm_data_bits_per_sample;

 dword_t length_of_output_pcm_data; //number of bytes that sound card will play
 word_t output_pcm_data_sample_rate;
}__attribute__((packed));

struct wav_info_t *actually_played_wav_info;

dword_t convert_wav_to_sound_data(dword_t wav_memory, dword_t wav_length);
struct wav_info_t *read_wav_info(byte_t *wav_memory, dword_t wav_length);
dword_t play_wav(struct wav_info_t *wav_info, dword_t offset_to_first_byte_to_play);
void wav_refill_buffer(byte_t *buffer);
void convert_sound_data_to_wav(byte_t *pcm_data_pointer, dword_t size_of_pcm_data_in_bytes, byte_t bits_per_sample, byte_t number_of_channels, word_t sample_rate);