//BleskOS

/*
* MIT License
* Copyright (c) 2023-2025 BleskOS developers
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

void intel_graphic_card_add_new_pci_device(struct pci_device_info_t device) {
    // check number of already connected cards
    if(components->n_intel_graphic_card >= MAX_NUMBER_OF_INTEL_GRAPHIC_CARDS) {
        return;
    }

    // log driver
    logf("%s", __FILE__);

    // save basic device informations
    copy_memory((dword_t)&device, (dword_t)&components->intel_graphic_card[components->n_intel_graphic_card].pci, sizeof(struct pci_device_info_t));

    // read other device infromations
    components->intel_graphic_card[components->n_intel_graphic_card].base = pci_get_mmio(device, PCI_BAR0);
    components->intel_graphic_card[components->n_intel_graphic_card].linear_frame_buffer = (byte_t *) pci_get_mmio(device, PCI_BAR2);

    // configure PCI
    pci_set_bits(device, 0x04, PCI_STATUS_BUSMASTERING | PCI_STATUS_MMIO | PCI_STATUS_IO);

    // update number of devices
    components->p_graphic_card = 1;
    components->n_intel_graphic_card++;
}

void initalize_intel_graphic_card(dword_t graphic_card_number) {
    dword_t base = components->intel_graphic_card[graphic_card_number].base;

    // this driver now can not change video mode
    
    // read EDID loaded by bootloader TODO: try read EDID from card
    is_bootloader_edid_present = parse_edid_data(0x2000);
    if(is_bootloader_edid_present == STATUS_FALSE) {
        logf("\n\nBootloader did not load EDID");
    }
    else {
        logf("\n\nBOOTLOADER EDID");
        log_edid_data();
    }

    // set monitor values
    monitors[0].card_number = graphic_card_number;

    // find if this driver can change backlight
    word_t max_backlight_value = (mmio_ind(base+GRAPHIC_CARD_INTEL_MMIO_BACKLIGHT_PWM_CONTROL) >> 16);
    if(max_backlight_value != 0 && max_backlight_value != 0xFFFF) {
        monitors[0].change_backlight = graphic_card_intel_change_backlight;
        graphic_card_intel_change_backlight(graphic_card_number, 100);
    }
}

void graphic_card_intel_change_backlight(dword_t monitor_number, byte_t value) {
    dword_t base = components->intel_graphic_card[monitors[monitor_number].card_number].base;
    word_t max_backlight_value = ((mmio_ind(base+GRAPHIC_CARD_INTEL_MMIO_BACKLIGHT_PWM_CONTROL) >> 16)-0x20); //too low values will shutdown display, so we are avoiding them
    mmio_outd(base+GRAPHIC_CARD_INTEL_MMIO_BACKLIGHT_PWM_CONTROL, ((max_backlight_value << 16) | (0x20+((max_backlight_value*value/100) & 0xFFFE))));
    monitors[monitor_number].backlight = value;
}

void intel_try_read_edid(dword_t graphic_card_number) {
    dword_t *edid_memory_pointer = (dword_t *) calloc(128);
    dword_t base = components->intel_graphic_card[graphic_card_number].base;

    //set type of device
    mmio_outd(base+GRAPHIC_CARD_INTEL_MMIO_GMBUS0, 0b011);

    //read 128 bytes from offset 0x50
    mmio_outd(base+GRAPHIC_CARD_INTEL_MMIO_GMBUS1, ((1 << 30) | (0b011 << 25) | (128 << 16) | (0x50 << 1) | 0x1));
    for(dword_t i = 0; i < 32; i++) {
        //wait for response
        ticks = 0;
        while(ticks < 10) {
            asm("nop");
            if((mmio_ind(base+GRAPHIC_CARD_INTEL_MMIO_GMBUS2) & (1<<11))==(1<<11)) {
                break;
            }
            if((mmio_ind(base+GRAPHIC_CARD_INTEL_MMIO_GMBUS2) & (1<<10))==(1<<10)) {
                logf("\nNAK error");
                break;
            }
        }

        //read 4 bytes
        edid_memory_pointer[i] = mmio_ind(base+GRAPHIC_CARD_INTEL_MMIO_GMBUS3);
    }

    //stop transfer
    mmio_outd(base+GRAPHIC_CARD_INTEL_MMIO_GMBUS1, ((1 << 30) | (0b100 << 27)));

    //parse received EDID data
    parse_edid_data((dword_t)edid_memory_pointer);
    
    free((void *)edid_memory_pointer);
}