//BleskOS

/*
* MIT License
* Copyright (c) 2023-2024 Vendelín Slezák
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

void vesa_init_graphic(void) {
 //bootloader loaded VESA information block on 0x3000 and VESA mode info on 0x3800
 dword_t *vesa_info_block_32 = (dword_t *) (0x3000);
 if(*vesa_info_block_32!=0x41534556) { //'VESA' signature
  log("\nInvalid VESA info block\n");
  return;
 }

 dword_t *vesa_mode_info_32 = (dword_t *) 0x3828;
 graphic_screen_lfb = (*vesa_mode_info_32);
 byte_t *vesa_info_block_8 = (byte_t *) (0x3005);
 if(*vesa_info_block_32==0x03) { //VBE 3
  word_t *vesa_mode_info_16 = (word_t *) 0x3832;
  graphic_screen_bytes_per_line = ((dword_t)(*vesa_mode_info_16)); //in VBE 3 get bytes per line from here
 }
 else { //other VBE version
  word_t *vesa_mode_info_16 = (word_t *) 0x3810;
  graphic_screen_bytes_per_line = ((dword_t)(*vesa_mode_info_16));
 }
 word_t *vesa_mode_info_16 = (word_t *) 0x3812;
 graphic_screen_x = ((dword_t)(*vesa_mode_info_16));
 vesa_mode_info_16 = (word_t *) 0x3814;
 graphic_screen_y = ((dword_t)(*vesa_mode_info_16));
 byte_t *vesa_mode_info_8 = (byte_t *) 0x3819;
 graphic_screen_bpp = ((dword_t)(*vesa_mode_info_8));

 //log
 log("\nVBE ");
 word_t *vesa_info_block_16 = (word_t *) (0x3004);
 log_hex_specific_size(*vesa_info_block_16, 4);
 log("\n");
}
