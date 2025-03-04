//BleskOS

/*
* MIT License
* Copyright (c) 2023-2025 BleskOS developers
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#define VMWARE_GRAPHIC_CARD_REGISTER_INDEX 0
#define VMWARE_GRAPHIC_CARD_REGISTER_DATA 1

#define VMWARE_GRAPHIC_CARD_INDEX_ID 0
#define VMWARE_GRAPHIC_CARD_INDEX_ENABLE 1
#define VMWARE_GRAPHIC_CARD_INDEX_WIDTH 2
#define VMWARE_GRAPHIC_CARD_INDEX_HEIGHT 3
#define VMWARE_GRAPHIC_CARD_INDEX_BPP 7
#define VMWARE_GRAPHIC_CARD_INDEX_BYTES_PER_LINE 12
#define VMWARE_GRAPHIC_CARD_INDEX_FB_START 13
#define VMWARE_GRAPHIC_CARD_INDEX_FB_OFFSET 14
#define VMWARE_GRAPHIC_CARD_INDEX_FIFO_MMIO 18
#define VMWARE_GRAPHIC_CARD_INDEX_FIFO_SIZE 19
#define VMWARE_GRAPHIC_CARD_INDEX_CONFIGURATION_DONE 20

#define VMWARE_GRAPHIC_CARD_FIFO_REGISTER_MIN 0
#define VMWARE_GRAPHIC_CARD_FIFO_REGISTER_MAX 1*4
#define VMWARE_GRAPHIC_CARD_FIFO_REGISTER_NEXT_CMD 2*4
#define VMWARE_GRAPHIC_CARD_FIFO_REGISTER_STOP 3*4

dword_t vmware_graphic_card_read(byte_t graphic_card_number, byte_t index);
void vmware_graphic_card_write(byte_t graphic_card_number, byte_t index, dword_t value);
void initalize_vmware_graphic_card(byte_t graphic_card_number);