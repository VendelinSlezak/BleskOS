//BleskOS

/*
* MIT License
* Copyright (c) 2023-2025 Vendelín Slezák
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#define AC97_BUFFER_NOT_FILLED 1
#define AC97_BUFFER_FILLED 0

#define AC97_NAM_IO_RESET 0x00
#define AC97_NAM_IO_CAPABILITES 0x00
 #define AC97_CAPABILITY_HEADPHONE_OUTPUT 0x10
#define AC97_NAM_IO_MASTER_VOLUME 0x02
#define AC97_NAM_IO_AUX_OUT_VOLUME 0x04
#define AC97_NAM_IO_PCM_OUT_VOLUME 0x18
#define AC97_NAM_IO_EXTENDED_CAPABILITIES 0x28
 #define AC97_EXTENDED_CAPABILITY_VARIABLE_SAMPLE_RATE 0x1
#define AC97_NAM_IO_EXTENDED_FEATURES_CONTROL 0x2A
#define AC97_NAM_IO_VARIABLE_SAMPLE_RATE_FRONT_DAC 0x2C
#define AC97_NAM_IO_VARIABLE_SAMPLE_RATE_SURR_DAC 0x2E
#define AC97_NAM_IO_VARIABLE_SAMPLE_RATE_LFE_DAC 0x30
#define AC97_NAM_IO_VARIABLE_SAMPLE_RATE_LR_ADC 0x32
#define AC97_NAM_IO_JACK_SENSE 0x72

#define AC97_NABM_IO_PCM_OUTPUT_BUFFER_BASE_ADDRESS 0x10
#define AC97_NABM_IO_PCM_OUTPUT_CURRENTLY_PROCESSED_ENTRY 0x14
#define AC97_NABM_IO_PCM_OUTPUT_LAST_VALID_ENTRY 0x15
#define AC97_NABM_IO_PCM_OUTPUT_STATUS 0x16
#define AC97_NABM_IO_PCM_OUTPUT_CURRENT_ENTRY_POSITION 0x18
#define AC97_NABM_IO_PCM_OUTPUT_CONTROL 0x1B
#define AC97_NABM_IO_GLOBAL_CONTROL 0x2C

#define AC97_NABM_IO_PCM_INPUT_CONTROL 0x0B
#define AC97_NABM_IO_MICROPHONE_INPUT_CONTROL 0x2B

#define AC97_SPEAKER_OUTPUT 0
#define AC97_HEADPHONE_OUTPUT 1

#define AC97_SPEAKER_OUTPUT_NUMBER_OF_VOLUME_STEPS 31

struct ac97_buffer_entry {
 dword_t sample_memory;
 word_t number_of_samples;
 word_t reserved: 14;
 byte_t last_buffer_entry: 1;
 byte_t interrupt_on_completion: 1;
}__attribute__((packed));

struct ac97_buffer_entry *ac97_buffer_memory_pointer;
byte_t ac97_headphone_output_present, ac97_free_entry, ac97_playing_state, ac97_selected_output, ac97_aux_out_number_of_volume_steps;
word_t ac97_nam_base, ac97_nabm_base, ac97_extended_capabilities;
dword_t ac97_sound_memory, ac97_number_of_samples_in_one_channel;

void initalize_ac97_sound_card(byte_t sound_card_number);
byte_t ac97_is_headphone_connected(void);
void ac97_set_volume_in_register(dword_t offset, byte_t number_of_volume_steps, byte_t volume);
void ac97_set_output(byte_t output);
void ac97_set_volume(byte_t volume);
byte_t ac97_is_supported_sample_rate(word_t sample_rate);
void ac97_set_sample_rate(word_t sample_rate);
void ac97_check_headphone_connection_change(void);
void ac97_play_pcm_data_in_loop(dword_t sample_rate);
void task_ac97_play_buffer_in_loop(void);
dword_t ac97_get_actual_stream_position(void);
void ac97_stop_sound(void);