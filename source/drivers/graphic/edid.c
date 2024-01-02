//BleskOS

/*
* MIT License
* Copyright (c) 2023-2024 Vendelín Slezák
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

void parse_edid_informations(void) {
 dword_t *edid32 = (dword_t *) 0x2000; //here bootloader loaded EDID
 
 //check signature
 if(edid32[0]!=0xFFFFFF00 || edid32[1]!=0x00FFFFFF) {
  log("\nno EDID founded\n");
  is_edid_present = STATUS_FALSE;
  return;
 }
 else {
  is_edid_present = STATUS_TRUE;
 }
 
 //we will read first timing
 byte_t *edid8 = (byte_t *) (0x2000+54);
 horizontal_active = (edid8[2] | ((edid8[4]>>4)<<8));
 horizontal_blank = (edid8[3] | ((edid8[4] & 0xF)<<8));
 horizontal_sync_offset = (edid8[8] | ((edid8[11]>>6)<<8));
 horizontal_sync_pulse = (edid8[9] | (((edid8[11]>>4) & 0x3)<<8));
 vertical_active = (edid8[5] | ((edid8[7]>>4)<<8));
 vertical_blank = (edid8[6] | ((edid8[7] & 0xF)<<8));
 vertical_sync_offset = ((edid8[10]>>4) | (((edid8[11]>>2) & 0x3)<<8));
 vertical_sync_pulse = ((edid8[10] & 0xF) | ((edid8[11] & 0x3)<<8));
 
 //calculate variabiles
 mode_horizontal_active = horizontal_active;
 mode_horizontal_sync_start = (horizontal_active + horizontal_sync_offset);
 mode_horizontal_sync_end = (mode_horizontal_sync_start + horizontal_sync_pulse);
 mode_horizontal_total = (horizontal_active + horizontal_blank);
 mode_vertical_active = vertical_active;
 mode_vertical_sync_start = (vertical_active + vertical_sync_offset);
 mode_vertical_sync_end = (mode_vertical_sync_start + vertical_sync_pulse);
 mode_vertical_total = (vertical_active + vertical_blank);
 
 //log output
 log("\nHORIZONTAL active: ");
 log_var_with_space(horizontal_active);
 log("blank: ");
 log_var_with_space(horizontal_blank);
 log("sync offset: ");
 log_var_with_space(horizontal_sync_offset);
 log("sync pulse: ");
 log_var(horizontal_sync_pulse);
 log("\nVERTICAL active: ");
 log_var_with_space(vertical_active);
 log("blank: ");
 log_var_with_space(vertical_blank);
 log("sync offset: ");
 log_var_with_space(vertical_sync_offset);
 log("sync pulse: ");
 log_var(vertical_sync_pulse);
 log("\n");
}