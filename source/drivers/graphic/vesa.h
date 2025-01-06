//BleskOS

/*
* MIT License
* Copyright (c) 2023-2025 Vendelín Slezák
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#define VESA_SIGNATURE 0x41534556

struct vesa_info_block {
 dword_t signature;
 byte_t minor_version;
 byte_t major_version;
 word_t oem_string_offset;
 word_t oem_string_segment;
 dword_t capabilities_of_graphic_controller;
 word_t video_mode_list_offset;
 word_t video_mode_list_segment;
 word_t number_of_64_kb_blocks;
 word_t software_revision;
 word_t vendor_name_string_offset;
 word_t vendor_name_string_segment;
 word_t product_name_string_offset;
 word_t product_name_string_segment;
 word_t product_revision_string_offset;
 word_t product_revision_string_segment;
 byte_t reserved[222];
 byte_t data_area[256];
}__attribute__((packed));

struct vesa_mode_info_block {
 word_t mode_attributes;
 byte_t window_a_attributes;
 byte_t window_b_attributes;
 word_t window_granularity;
 word_t window_size;
 word_t window_a_start_segment;
 word_t window_b_start_segment;
 word_t window_function_offset;
 word_t window_function_segment;
 word_t bytes_per_line;

 word_t screen_width;
 word_t screen_height;
 byte_t character_cell_width;
 byte_t character_cell_height;
 byte_t number_of_planes;
 byte_t bits_per_pixel;
 byte_t number_of_banks;
 byte_t type_of_memory_model;
 byte_t bank_size_in_kb;
 byte_t number_of_images;
 byte_t reserved;

 byte_t red_mask_size_in_bits;
 byte_t red_mask_bit_position;
 byte_t green_mask_size_in_bits;
 byte_t green_mask_bit_position;
 byte_t blue_mask_size_in_bits;
 byte_t blue_mask_bit_position;
 byte_t reserved_mask_size_in_bits;
 byte_t reserved_mask_bit_position;
 byte_t direct_color_mode_attributes;

 byte_t *linear_frame_buffer_memory_pointer;
 dword_t reserved2;
 word_t reserved3;

 word_t vbe3_bytes_per_line;
 byte_t number_of_images_for_banked_modes;
 byte_t number_of_images_for_linear_modes;
 byte_t linear_modes_red_mask_size_in_bits;
 byte_t linear_modes_red_mask_bit_position;
 byte_t linear_modes_green_mask_size_in_bits;
 byte_t linear_modes_green_mask_bit_position;
 byte_t linear_modes_blue_mask_size_in_bits;
 byte_t linear_modes_blue_mask_bit_position;
 byte_t linear_modes_reserved_mask_size_in_bits;
 byte_t linear_modes_reserved_mask_bit_position;
 dword_t maximum_pixel_clock;

 byte_t reserved4[189];
}__attribute__((packed));

void vesa_read_graphic_mode_info(void);