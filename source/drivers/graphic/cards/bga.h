//BleskOS

/*
* MIT License
* Copyright (c) 2023-2025 BleskOS developers
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

struct pci_supported_devices_list_t bga_supported_pci_devices[] = {
    {0x1234, 0x1111},
    {0, 0}
};

struct bga_info_t {
    word_t id;
    byte_t *linear_frame_buffer;
};

#define BGA_REGISTER_INDEX 0x01CE
#define BGA_REGISTER_DATA 0x01CF

#define BGA_INDEX_ID 0
#define BGA_INDEX_WIDTH 1
#define BGA_INDEX_HEIGHT 2
#define BGA_INDEX_BPP 3
#define BGA_INDEX_ENABLE 4

void bga_add_new_pci_device(struct pci_device_info_t device);
word_t bga_read(byte_t index);
void bga_write(byte_t index, word_t value);
void check_presence_of_bga(void);
void initalize_bga(void);