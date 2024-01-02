//BleskOS

/*
* MIT License
* Copyright (c) 2023-2024 Vendelín Slezák
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#define AC97_BUFFER_NOT_FILLED 1
#define AC97_BUFFER_FILLED 0

byte_t ac97_present = 0, ac97_variabile_sound_rate_present = 0, ac97_free_entry = 0, ac97_playing_state = AC97_BUFFER_FILLED;
word_t ac97_nam_base, ac97_nabm_base;
dword_t ac97_buffer_entries_memory;
dword_t ac97_sound_memory, ac97_sound_num_of_samples;

void initalize_ac97(void);
void ac97_set_volume(byte_t volume);
byte_t ac97_is_supported_sample_rate(word_t sample_rate);
void ac97_set_sample_rate(word_t sample_rate);
void ac97_play_sound(void);
void ac97_stop_sound(void);
void ac97_clear_buffer(void);
void ac97_fill_buffer_entry(void);
void ac97_play_memory(dword_t sound_memory, dword_t number_of_samples_in_one_channel);
void ac97_stop_playing_memory(void);
