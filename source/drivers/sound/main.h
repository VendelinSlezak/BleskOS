//BleskOS

/*
* MIT License
* Copyright (c) 2023-2024 Vendelín Slezák
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#define SOUND_CARD_DRIVER_AC97 0
#define SOUND_CARD_DRIVER_HDA 1

#define MAX_NUMBER_OF_SOUND_CARDS 10
struct sound_card_info {
 byte_t driver;
 word_t vendor_id;
 word_t device_id;
 word_t io_base;
 word_t io_base_2;
 dword_t mmio_base;
}__attribute__((packed));
struct sound_card_info sound_cards_info[MAX_NUMBER_OF_SOUND_CARDS];
byte_t number_of_sound_cards;
#define NO_SOUND_CARD 0xFF
byte_t selected_sound_card;

byte_t sound_volume = 0;
dword_t sound_sample_rate = 0, sound_memory = 0, sound_length = 0, sound_position = 0;

void initalize_sound_card(void);
void sound_set_volume(byte_t volume);
byte_t is_supported_sound_format(byte_t channels, byte_t bits_per_channel, dword_t sample_rate);
void play_new_sound(dword_t sound_memory, dword_t channels, dword_t bits_per_sample, dword_t sample_rate, dword_t number_of_samples);
void pause_sound(void);
void play_sound(void);
