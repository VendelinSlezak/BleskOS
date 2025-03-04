//BleskOS

/*
* MIT License
* Copyright (c) 2023-2025 BleskOS developers
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#define GRAPHIC_CARD_INTEL_MMIO_GMBUS0 0x5100
#define GRAPHIC_CARD_INTEL_MMIO_GMBUS1 0x5104
#define GRAPHIC_CARD_INTEL_MMIO_GMBUS2 0x5108
#define GRAPHIC_CARD_INTEL_MMIO_GMBUS3 0x510C

#define GRAPHIC_CARD_INTEL_MMIO_BACKLIGHT_PWM_CONTROL 0x61254

void initalize_intel_graphic_card(byte_t graphic_card_number);
void graphic_card_intel_change_backlight(byte_t percent);

void intel_try_read_edid(void);