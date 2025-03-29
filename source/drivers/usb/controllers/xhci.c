//BleskOS

/*
* MIT License
* Copyright (c) 2023-2025 BleskOS developers
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

void xhci_add_new_pci_device(struct pci_device_info_t device) {
    // check number of already connected cards
    if(components->n_xhci >= MAX_NUMBER_OF_XHCI_CONTROLLERS) {
        return;
    }

    // log driver
    logf("%s", __FILE__);

    // save basic device informations
    copy_memory((dword_t)&device, (dword_t)&components->xhci[components->n_xhci].pci, sizeof(struct pci_device_info_t));

    // read other device informations
    components->xhci[components->n_xhci].base = pci_get_mmio(device, PCI_BAR0);

    // configure PCI
    pci_set_bits(device, 0x04, PCI_STATUS_BUSMASTERING | PCI_STATUS_MMIO);

    // disable BIOS legacy emulation
    dword_t xhci_bios_register_offset = ((mmio_ind(components->xhci[components->n_xhci].base+0x10)>>16)*4);
    if(xhci_bios_register_offset != 0 && (mmio_ind(components->xhci[components->n_xhci].base+xhci_bios_register_offset) & 0xFF)==0x01) {
        mmio_outd(components->xhci[components->n_xhci].base+xhci_bios_register_offset, (mmio_ind(components->xhci[components->n_xhci].base+xhci_bios_register_offset) & ~(1 << 16)) | (1 << 24)); //set OS ownership
    }

    // update number of devices
    components->n_xhci++;
}

/* initalization of xHCI controller at boot */

void initalize_xhci_controller(dword_t number_of_controller) {
    // log
    logf("\n\nDriver: xHCI Controller\nDevice: PCI bus %d:%d:%d:%d",
        components->xhci[number_of_controller].pci.segment,
        components->xhci[number_of_controller].pci.bus,
        components->xhci[number_of_controller].pci.device,
        components->xhci[number_of_controller].pci.function);

    //disable BIOS ownership
    dword_t xhci_bios_register_offset = ((mmio_ind(components->xhci[number_of_controller].base+0x10)>>16)*4);
    if(xhci_bios_register_offset != 0 && (mmio_ind(components->xhci[number_of_controller].base+xhci_bios_register_offset) & 0xFF)==0x01) {
        // wait for BIOS handoff
        do {
            asm("nop");

            if(((mmio_ind(components->xhci[number_of_controller].base+xhci_bios_register_offset) & ((1 << 16) | (1 << 24))) == (1 << 24))) {
                logf("\nBIOS ownership released successfully");
                break;
            }
        } while(time_of_system_running < components->xhci[number_of_controller].pci.bios_handoff_timeout);
        if(((mmio_ind(components->xhci[number_of_controller].base+xhci_bios_register_offset) & ((1 << 16) | (1 << 24))) != (1 << 24))
            && time_of_system_running >= components->xhci[number_of_controller].pci.bios_handoff_timeout) {
            logf("\nERROR: xHCI controller is still in BIOS ownership %x", mmio_ind(components->xhci[number_of_controller].base+xhci_bios_register_offset));
            return;
        }
    }
}