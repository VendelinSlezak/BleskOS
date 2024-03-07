//BleskOS

/*
* MIT License
* Copyright (c) 2023-2024 Vendelín Slezák
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

byte_t parse_edid_data(dword_t edid_data_memory) {
 struct edid *edid_data = (struct edid *) (edid_data_memory);
 
 //check signature
 if(edid_data->signature!=EDID_SIGNATURE) {
  return STATUS_FALSE;
 }
 
 //read first timing
 edid_horizontal_active = (edid_data->detailed_timing_description_1.horizontal_active_time_lower_bits | edid_data->detailed_timing_description_1.horizontal_active_time_upper_bits<<8);
 edid_horizontal_blank = (edid_data->detailed_timing_description_1.horizontal_blanking_time_lower_bits | edid_data->detailed_timing_description_1.horizontal_blanking_time_upper_bits<<8);
 edid_horizontal_sync_offset = (edid_data->detailed_timing_description_1.horizontal_sync_offset_lower_bits | edid_data->detailed_timing_description_1.horizontal_sync_offset_upper_bits<<8);
 edid_horizontal_sync_pulse = (edid_data->detailed_timing_description_1.horizontal_sync_pulse_lower_bits | edid_data->detailed_timing_description_1.horizontal_sync_pulse_upper_bits<<8);
 edid_vertical_active = (edid_data->detailed_timing_description_1.vertical_active_time_lower_bits | edid_data->detailed_timing_description_1.vertical_active_time_upper_bits<<8);
 edid_vertical_blank = (edid_data->detailed_timing_description_1.vertical_blanking_time_lower_bits | edid_data->detailed_timing_description_1.vertical_blanking_time_upper_bits<<8);
 edid_vertical_sync_offset = (edid_data->detailed_timing_description_1.vertical_sync_offset_lower_bits | edid_data->detailed_timing_description_1.vertical_sync_offset_upper_bits<<4);
 edid_vertical_sync_pulse = (edid_data->detailed_timing_description_1.vertical_sync_pulse_lower_bits | edid_data->detailed_timing_description_1.vertical_sync_pulse_upper_bits<<4);

 //calculate variables
 mode_horizontal_active = edid_horizontal_active;
 mode_horizontal_sync_start = (edid_horizontal_active + edid_horizontal_sync_offset);
 mode_horizontal_sync_end = (mode_horizontal_sync_start + edid_horizontal_sync_pulse);
 mode_horizontal_total = (edid_horizontal_active + edid_horizontal_blank);
 mode_vertical_active = edid_vertical_active;
 mode_vertical_sync_start = (edid_vertical_active + edid_vertical_sync_offset);
 mode_vertical_sync_end = (mode_vertical_sync_start + edid_vertical_sync_pulse);
 mode_vertical_total = (edid_vertical_active + edid_vertical_blank);

 return STATUS_TRUE;
}

void log_edid_data(void) {
 log("\nHORIZONTAL active: ");
 log_var_with_space(edid_horizontal_active);
 log("blank: ");
 log_var_with_space(edid_horizontal_blank);
 log("sync offset: ");
 log_var_with_space(edid_horizontal_sync_offset);
 log("sync pulse: ");
 log_var(edid_horizontal_sync_pulse);

 log("\nVERTICAL active: ");
 log_var_with_space(edid_vertical_active);
 log("blank: ");
 log_var_with_space(edid_vertical_blank);
 log("sync offset: ");
 log_var_with_space(edid_vertical_sync_offset);
 log("sync pulse: ");
 log_var(edid_vertical_sync_pulse);
}