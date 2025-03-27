//BleskOS

/*
* MIT License
* Copyright (c) 2023-2025 BleskOS developers
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

/* TODO: rewrite drivers to add output and input sources to these structures */
struct audio_output_t {
    dword_t output_number;
    dword_t device_number;
    void (*activate_audio_output)(dword_t device_number, dword_t output_number);

    dword_t gain;

    dword_t present;
    dword_t removable;
    byte_t (*check_presence)(dword_t device_number, dword_t output_number);
};
struct audio_input_t {
    dword_t input_number;
    dword_t device_number;
    void (*activate_audio_input)(dword_t device_number, dword_t input_number);

    dword_t gain;

    dword_t present;
    dword_t removable;
    byte_t (*check_presence)(dword_t device_number, dword_t output_number);
};

#define SOUND_CARD_DRIVER_AC97 0
#define SOUND_CARD_DRIVER_HDA 1

#define MAX_NUMBER_OF_SOUND_CARDS 10
struct sound_card_info_t {
 byte_t driver;
 word_t vendor_id;
 word_t device_id;
 word_t io_base;
 word_t io_base_2;
 dword_t mmio_base;
}__attribute__((packed));
struct sound_card_info_t sound_cards_info[MAX_NUMBER_OF_SOUND_CARDS];
byte_t number_of_sound_cards;
#define NO_SOUND_CARD 0xFF
byte_t selected_sound_card;
byte_t selected_sound_card_driver_type;

byte_t sound_volume = 0;
dword_t sound_sample_rate = 0, sound_memory = 0, sound_length = 0, sound_position = 0;

#define SIZE_OF_PCM_DATA_BUFFER 4*48000*4 //four seconds for 2 channels with 16 bit samples
#define SOUND_BUFFER_0 0
#define SOUND_BUFFER_1 1
struct sound_buffer_refilling_info_t {
 byte_t *source_data_pointer;
 dword_t source_data_length;
 void (*fill_buffer)(byte_t *buffer);
 dword_t buffer_size;
 byte_t *buffer_0_pointer;
 byte_t *buffer_1_pointer;
 byte_t actually_playing_buffer: 4;
 byte_t last_filled_buffer: 4;
 dword_t played_bytes_by_finished_buffers;
 dword_t played_bytes;
 dword_t size_of_full_pcm_output_in_bytes;
}__attribute__((packed));

struct sound_buffer_refilling_info_t *sound_buffer_refilling_info;
byte_t *pcm_data;

void initalize_sound_card(void);
void sound_set_volume(byte_t volume);
byte_t is_supported_sound_format(byte_t channels, byte_t bits_per_channel, dword_t sample_rate);

dword_t sound_get_actual_stream_position(void);
void play_sound_with_refilling_buffer(byte_t *source_data_pointer, dword_t source_data_length, dword_t size_of_full_pcm_output_in_bytes, dword_t sample_rate, dword_t size_of_buffer, void (*fill_buffer)(byte_t *buffer));
void task_refill_sound_buffer(void);
void stop_sound(void);