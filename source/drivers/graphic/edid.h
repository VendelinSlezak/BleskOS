//BleskOS

/*
* MIT License
* Copyright (c) 2023-2025 Vendelín Slezák
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#define EDID_SIGNATURE 0x00FFFFFFFFFFFF00

struct edid_detailed_timing_description {
 byte_t horizontal_frequency;
 byte_t vertical_frequency;
 byte_t horizontal_active_time_lower_bits;
 byte_t horizontal_blanking_time_lower_bits;
 byte_t horizontal_blanking_time_upper_bits: 4;
 byte_t horizontal_active_time_upper_bits: 4;
 byte_t vertical_active_time_lower_bits;
 byte_t vertical_blanking_time_lower_bits;
 byte_t vertical_blanking_time_upper_bits: 4;
 byte_t vertical_active_time_upper_bits: 4;
 byte_t horizontal_sync_offset_lower_bits;
 byte_t horizontal_sync_pulse_lower_bits;
 byte_t vertical_sync_pulse_lower_bits: 4;
 byte_t vertical_sync_offset_lower_bits: 4;
 byte_t vertical_sync_pulse_upper_bits: 2;
 byte_t vertical_sync_offset_upper_bits: 2;
 byte_t horizontal_sync_pulse_upper_bits: 2;
 byte_t horizontal_sync_offset_upper_bits: 2;
 byte_t horizontal_image_size_in_mm_lower_bits;
 byte_t vertical_image_size_in_mm_lower_bits;
 byte_t vertical_image_size_in_mm_upper_bits: 4;
 byte_t horizontal_image_size_in_mm_upper_bits: 4;
 byte_t horizontal_border;
 byte_t vertical_border;
 byte_t type_of_display;
}__attribute__((packed));

struct edid {
 qword_t signature;
 word_t manufacture_id;
 word_t edid_id_code;
 dword_t serial_number;
 byte_t manufacture_week;
 byte_t manufacture_year;
 byte_t edid_version;
 byte_t edid_revision;
 byte_t video_input_type;
 byte_t max_horizontal_size_in_cm;
 byte_t max_vertical_size_in_cm;
 byte_t gama_factor;
 byte_t dpms_flags;
 byte_t chroma_information[10];
 byte_t estabilished_timing_1;
 byte_t estabilished_timing_2;
 byte_t manufacture_reserved_timing;
 word_t standard_timing_identification[8];
 struct edid_detailed_timing_description detailed_timing_description_1;
 struct edid_detailed_timing_description detailed_timing_description_2;
 struct edid_detailed_timing_description detailed_timing_description_3;
 struct edid_detailed_timing_description detailed_timing_description_4;
 byte_t reserved;
 byte_t checksum;
}__attribute__((packed));

byte_t is_bootloader_edid_present;
word_t edid_horizontal_active, edid_horizontal_blank, edid_horizontal_sync_offset, edid_horizontal_sync_pulse, edid_vertical_active, edid_vertical_blank, edid_vertical_sync_offset, edid_vertical_sync_pulse;
word_t mode_horizontal_active, mode_horizontal_sync_start, mode_horizontal_sync_end, mode_horizontal_total, mode_vertical_active, mode_vertical_sync_start, mode_vertical_sync_end, mode_vertical_total;

byte_t parse_edid_data(dword_t edid_data_memory);
void log_edid_data(void);