//BleskOS

/*
* MIT License
* Copyright (c) 2023-2025 BleskOS developers
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

void vesa_read_graphic_mode_info(void) {
    logf("\n\nDriver: VESA BIOS Extension");

    //bootloader loaded VESA information block on 0x3000 and VESA mode info on 0x3800
    struct vesa_info_block *vesa_info_block_data = (struct vesa_info_block *) (0x3000);
    struct vesa_mode_info_block *vesa_mode_info_block_data = (struct vesa_mode_info_block *) (0x3800);

    //check signature
    if(vesa_info_block_data->signature!=VESA_SIGNATURE) {
        logf("\nInvalid VBE info block");
        return;
    }

    //read info
    monitors[0].connected = STATUS_TRUE;
    monitors[0].linear_frame_buffer = vesa_mode_info_block_data->linear_frame_buffer_memory_pointer;
    if(vesa_info_block_data->major_version==0x03) { //VBE 3
        monitors[0].linear_frame_buffer_bpl = vesa_mode_info_block_data->vbe3_bytes_per_line;
    }
    else { //other VBE version
        monitors[0].linear_frame_buffer_bpl = vesa_mode_info_block_data->bytes_per_line;
    }
    monitors[0].width = vesa_mode_info_block_data->screen_width;
    monitors[0].height = vesa_mode_info_block_data->screen_height;
    monitors[0].bpp = vesa_mode_info_block_data->bits_per_pixel;

    //log
    logf("\nVBE version: %d.%d", vesa_info_block_data->major_version, vesa_info_block_data->minor_version);
    logf("\nOEM string: %s", (vesa_info_block_data->oem_string_segment*0x10 + vesa_info_block_data->oem_string_offset));
}