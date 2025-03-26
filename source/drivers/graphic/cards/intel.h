//BleskOS

/*
* MIT License
* Copyright (c) 2023-2025 BleskOS developers
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

// TODO: extract all devices from https://admin.pci-ids.ucw.cz/read/PC/8086
struct pci_supported_devices_list_t intel_graphic_card_supported_pci_devices[] = {
    { 0x8086, 0x0042 },
    { 0x8086, 0x0102 },
    { 0x8086, 0x0112 },
    { 0x8086, 0x0162 },
    { 0x8086, 0x0412 },
    { 0x8086, 0x2445 },
    { 0x8086, 0x2562 },
    { 0x8086, 0x2563 },
    { 0x8086, 0x2582 },
    { 0x8086, 0x258A },
    { 0x8086, 0x2A42 },
    { 0x8086, 0x2A43 },
    { 0x8086, 0x2A44 },
    { 0x8086, 0x2A45 },
    { 0x8086, 0x2A46 },
    { 0x8086, 0x2A47 },
    { 0x8086, 0x2A48 },
    { 0x8086, 0x2A49 },
    { 0x8086, 0x2A4A },
    { 0x8086, 0x2A4B },
    { 0x8086, 0x2A4C },
    { 0x8086, 0x2A4D },
    { 0x8086, 0x2A4E },
    { 0x8086, 0x2A4F },
    { 0x8086, 0x2A50 },
    { 0x8086, 0x2A51 },
    { 0x8086, 0x2A52 },
    { 0x8086, 0x2A53 },
    { 0x8086, 0x2A54 },
    { 0x8086, 0x2A55 },
    { 0x8086, 0x2A56 },
    { 0x8086, 0x2A57 },
    { 0x8086, 0x2A58 },
    { 0x8086, 0x2A59 },
    { 0x8086, 0x2A5A },
    { 0x8086, 0x2A5B },
    { 0x8086, 0x2A5C },
    { 0x8086, 0x2A5D },
    { 0x8086, 0x2A5E },
    { 0x8086, 0x2A5F },
    { 0x8086, 0x1616 },
    { 0x8086, 0x161E },
    { 0x8086, 0x1622 },
    { 0x8086, 0x1626 },
    { 0x8086, 0x1912 },
    { 0x8086, 0x1916 },
    { 0x8086, 0x1921 },
    { 0x8086, 0x1926 },
    { 0x8086, 0x3E92 },
    { 0x8086, 0x3E9B },
    { 0x8086, 0x4680 },
    { 0x8086, 0x46A6 },
    { 0x8086, 0x56A0 },
    { 0x8086, 0x56A1 },
    { 0x8086, 0x56A2 },
    { 0x8086, 0x56B0 },
    { 0x8086, 0x7D40 },
    { 0x8086, 0x7D45 },
    { 0x8086, 0x7D55 },
    { 0x8086, 0x7D60 },
    { 0x8086, 0x7DD5 },
    { 0x8086, 0x8A52 },
    { 0x8086, 0x8A56 },
    { 0x8086, 0x9A40 },
    { 0x8086, 0x9A49 },
    { 0x8086, 0x9B21 },
    { 0x8086, 0x9BC8 },
    { 0, 0 }
};

#define MAX_NUMBER_OF_INTEL_GRAPHIC_CARDS 4
struct intel_graphic_card_info_t {
    struct pci_device_info_t pci;
    dword_t base;
    byte_t *linear_frame_buffer;
};

#define GRAPHIC_CARD_INTEL_MMIO_GMBUS0 0x5100
#define GRAPHIC_CARD_INTEL_MMIO_GMBUS1 0x5104
#define GRAPHIC_CARD_INTEL_MMIO_GMBUS2 0x5108
#define GRAPHIC_CARD_INTEL_MMIO_GMBUS3 0x510C

#define GRAPHIC_CARD_INTEL_MMIO_BACKLIGHT_PWM_CONTROL 0x61254

void intel_graphic_card_add_new_pci_device(struct pci_device_info_t device);
void initalize_intel_graphic_card(dword_t graphic_card_number);
void graphic_card_intel_change_backlight(dword_t monitor_number, byte_t value);
void intel_try_read_edid(dword_t graphic_card_number);