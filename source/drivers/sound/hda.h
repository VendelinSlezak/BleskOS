//BleskOS

/*
* MIT License
* Copyright (c) 2023-2025 BleskOS developers
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#define MAX_NUMBER_OF_HDA_SOUND_CARDS 4
struct hda_info_t {
    struct pci_device_info_t pci;

    dword_t base;
    dword_t input_stream_base;
    dword_t output_stream_base;
    dword_t communication_type;
    dword_t codec_number;
    dword_t is_initalized_useful_output;
    dword_t selected_output_node;

    dword_t *corb_mem;
    dword_t corb_pointer;
    dword_t corb_number_of_entries;
    dword_t *rirb_mem;
    dword_t rirb_pointer;
    dword_t rirb_number_of_entries;

    dword_t *output_buffer_list;
    dword_t sound_length;
    dword_t bytes_on_output_for_stopping_sound;
    dword_t length_of_node_path;

    dword_t afg_node_sample_capabilities;
    dword_t afg_node_stream_format_capabilities;
    dword_t afg_node_input_amp_capabilities;
    dword_t afg_node_output_amp_capabilities;

    dword_t audio_output_node_number;
    dword_t audio_output_node_sample_capabilities;
    dword_t audio_output_node_stream_format_capabilities;

    dword_t output_amp_node_number;
    dword_t output_amp_node_capabilities;

    dword_t second_audio_output_node_number;
    dword_t second_audio_output_node_sample_capabilities;
    dword_t second_audio_output_node_stream_format_capabilities;
    dword_t second_output_amp_node_number;
    dword_t second_output_amp_node_capabilities;

    dword_t pin_output_node_number;
    dword_t pin_headphone_node_number;
};

// TODO: rewrite this
dword_t selected_hda_card;

#define HDA_UNINITALIZED 0
#define HDA_CORB_RIRB 1
#define HDA_PIO 2

#define HDA_WIDGET_AUDIO_OUTPUT 0x0
#define HDA_WIDGET_AUDIO_INPUT 0x1
#define HDA_WIDGET_AUDIO_MIXER 0x2
#define HDA_WIDGET_AUDIO_SELECTOR 0x3
#define HDA_WIDGET_PIN_COMPLEX 0x4
#define HDA_WIDGET_POWER_WIDGET 0x5
#define HDA_WIDGET_VOLUME_KNOB 0x6
#define HDA_WIDGET_BEEP_GENERATOR 0x7
#define HDA_WIDGET_VENDOR_DEFINED 0xF

#define HDA_PIN_LINE_OUT 0x0
#define HDA_PIN_SPEAKER 0x1
#define HDA_PIN_HEADPHONE_OUT 0x2
#define HDA_PIN_CD 0x3
#define HDA_PIN_SPDIF_OUT 0x4
#define HDA_PIN_DIGITAL_OTHER_OUT 0x5
#define HDA_PIN_MODEM_LINE_SIDE 0x6
#define HDA_PIN_MODEM_HANDSET_SIDE 0x7
#define HDA_PIN_LINE_IN 0x8
#define HDA_PIN_AUX 0x9
#define HDA_PIN_MIC_IN 0xA
#define HDA_PIN_TELEPHONY 0xB
#define HDA_PIN_SPDIF_IN 0xC
#define HDA_PIN_DIGITAL_OTHER_IN 0xD
#define HDA_PIN_RESERVED 0xE
#define HDA_PIN_OTHER 0xF

#define HDA_OUTPUT_NODE 0x1
#define HDA_INPUT_NODE 0x2

void hda_add_new_pci_device(struct pci_device_info_t device);

void hda_initalize_sound_card(dword_t sound_card_number);
dword_t hda_send_verb(dword_t codec, dword_t node, dword_t verb, dword_t command);

byte_t hda_get_node_type(dword_t codec, dword_t node);
word_t hda_get_node_connection_entry(dword_t codec, dword_t node, dword_t connection_entry_number);
void hda_set_node_gain(dword_t codec, dword_t node, dword_t node_type, dword_t capabilities, dword_t gain);
void hda_enable_pin_output(dword_t codec, dword_t pin_node);
void hda_disable_pin_output(dword_t codec, dword_t pin_node);
byte_t hda_is_headphone_connected(void);

void hda_initalize_codec(dword_t sound_card_number, dword_t codec_number);
void hda_initalize_audio_function_group(dword_t sound_card_number, dword_t afg_node_number);
void hda_initalize_output_pin(dword_t sound_card_number, dword_t pin_node_number);
void hda_initalize_audio_output(dword_t sound_card_number, dword_t audio_output_node_number);
void hda_initalize_audio_mixer(dword_t sound_card_number, dword_t audio_mixer_node_number);
void hda_initalize_audio_selector(dword_t sound_card_number, dword_t audio_selector_node_number);

void hda_set_volume(dword_t sound_card_number, dword_t volume);
void hda_check_headphone_connection_change(void);

byte_t hda_is_supported_channel_size(dword_t sound_card_number, byte_t size);
byte_t hda_is_supported_sample_rate(dword_t sound_card_number, dword_t sample_rate);
word_t hda_return_sound_data_format(dword_t sample_rate, dword_t channels, dword_t bits_per_sample);
void hda_play_pcm_data_in_loop(dword_t sound_card_number, dword_t sample_rate);
void hda_stop_sound(dword_t sound_card_number);
dword_t hda_get_actual_stream_position(dword_t sound_card_number);