//BleskOS

/*
* MIT License
* Copyright (c) 2023-2025 BleskOS developers
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

void vmware_graphic_card_add_new_pci_device(struct pci_device_info_t device) {
    // check number of already connected cards
    if(components->n_vmware_graphic_card >= MAX_NUMBER_OF_VMWARE_GRAPHIC_CARDS) {
        return;
    }

    // log driver
    logf("%s", __FILE__);

    // save basic device informations
    copy_memory((dword_t)&device, (dword_t)&components->vmware_graphic_card[components->n_vmware_graphic_card].pci, sizeof(struct pci_device_info_t));

    // read other device infromations
    components->vmware_graphic_card[components->n_vmware_graphic_card].base = pci_get_io(device, PCI_BAR0);

    // configure PCI
    pci_set_bits(device, 0x04, PCI_STATUS_BUSMASTERING | PCI_STATUS_MMIO | PCI_STATUS_IO);

    // update number of devices
    components->p_graphic_card = 1;
    components->n_vmware_graphic_card++;
}

dword_t vmware_graphic_card_read(dword_t graphic_card_number, byte_t index) {
    word_t base = components->vmware_graphic_card[graphic_card_number].base;
    outd(base+VMWARE_GRAPHIC_CARD_REGISTER_INDEX, index);
    return ind(base+VMWARE_GRAPHIC_CARD_REGISTER_DATA);
}

void vmware_graphic_card_write(dword_t graphic_card_number, byte_t index, dword_t value) {
    word_t base = components->vmware_graphic_card[graphic_card_number].base;
    outd(base+VMWARE_GRAPHIC_CARD_REGISTER_INDEX, index);
    outd(base+VMWARE_GRAPHIC_CARD_REGISTER_DATA, value);
}

void initalize_vmware_graphic_card(dword_t graphic_card_number) {
    // try all possible IDs from newest to oldest version
    logf("\n\nVMWare SVGA ");
    byte_t initalized = STATUS_FALSE;
    for(dword_t i = 0x90000002; i >= 0x90000000; i--) {
        vmware_graphic_card_write(graphic_card_number, VMWARE_GRAPHIC_CARD_INDEX_ID, i);
        if(vmware_graphic_card_read(graphic_card_number, VMWARE_GRAPHIC_CARD_INDEX_ID) == i) {
            initalized = STATUS_TRUE;
            logf("version %d", (i & 0x3));
            break;
        }
    }
    if(initalized == STATUS_FALSE) {
        logf("unknown version");
        return;
    }

    // read Linear Frame Buffer
    monitors[0].linear_frame_buffer = (byte_t *) (vmware_graphic_card_read(graphic_card_number, VMWARE_GRAPHIC_CARD_INDEX_FB_START) + vmware_graphic_card_read(graphic_card_number, VMWARE_GRAPHIC_CARD_INDEX_FB_OFFSET));
    logf("\nLFB: 0x%x %d", vmware_graphic_card_read(graphic_card_number, VMWARE_GRAPHIC_CARD_INDEX_FB_START), vmware_graphic_card_read(graphic_card_number, VMWARE_GRAPHIC_CARD_INDEX_FB_OFFSET));

    // initalize FIFO
    dword_t fifo_base = vmware_graphic_card_read(graphic_card_number, VMWARE_GRAPHIC_CARD_INDEX_FIFO_MMIO);
    mmio_outd(fifo_base+VMWARE_GRAPHIC_CARD_FIFO_REGISTER_MIN, 293*4);
    mmio_outd(fifo_base+VMWARE_GRAPHIC_CARD_FIFO_REGISTER_NEXT_CMD, 293*4);
    mmio_outd(fifo_base+VMWARE_GRAPHIC_CARD_FIFO_REGISTER_STOP, 293*4);
    mmio_outd(fifo_base+VMWARE_GRAPHIC_CARD_FIFO_REGISTER_MAX, vmware_graphic_card_read(graphic_card_number, VMWARE_GRAPHIC_CARD_INDEX_FIFO_SIZE));
    vmware_graphic_card_write(graphic_card_number, VMWARE_GRAPHIC_CARD_INDEX_CONFIGURATION_DONE, 1);

    // set graphic mode 1024x768x32
    vmware_graphic_card_write(graphic_card_number, VMWARE_GRAPHIC_CARD_INDEX_WIDTH, 1024);
    vmware_graphic_card_write(graphic_card_number, VMWARE_GRAPHIC_CARD_INDEX_HEIGHT, 768);
    vmware_graphic_card_write(graphic_card_number, VMWARE_GRAPHIC_CARD_INDEX_BPP, 32);

    // set new graphic mode
    vmware_graphic_card_write(graphic_card_number, VMWARE_GRAPHIC_CARD_INDEX_ENABLE, 1);

    // save monitor informations
    monitors[0].connected = STATUS_TRUE;
    monitors[0].card_number = graphic_card_number;
    monitors[0].width = 1024;
    monitors[0].height = 768;
    monitors[0].bpp = 32;
    monitors[0].linear_frame_buffer_bpl = vmware_graphic_card_read(graphic_card_number, VMWARE_GRAPHIC_CARD_INDEX_BYTES_PER_LINE);
    monitors[0].linear_frame_buffer_bpl = 1024*4;

    // TODO: send continuous update commands
}