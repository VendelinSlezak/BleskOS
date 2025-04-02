//BleskOS

/*
* MIT License
* Copyright (c) 2023-2025 BleskOS developers
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

// p_* = presence of component that can be present only once STATUS_FALSE / STATUS_TRUE
// n_* = number of components of this type

struct components_info_t {
    /* components that has to be present if BleskOS runs */
    struct cpu_info_t cpu;
    struct ram_info_t ram;
    struct cmos_info_t cmos;
    struct pci_info_t pci;

    /* system components */
    dword_t p_ebda;
    struct ebda_info_t ebda;
    dword_t p_acpi;
    struct acpi_info_t acpi;
    dword_t p_pic;
    dword_t p_apic;
    dword_t p_pit;
    dword_t p_hpet;
    struct hpet_info_t hpet;

    /* components connected to fixed ports */
    dword_t p_e9_debug_device;
    dword_t p_8042_controller;
    dword_t p_bga;
    struct bga_info_t bga;

    /* components connected to PCI */
    dword_t p_graphic_card;
    dword_t n_vmware_graphic_card;
    struct vmware_graphic_card_info_t vmware_graphic_card[MAX_NUMBER_OF_VMWARE_GRAPHIC_CARDS];
    dword_t n_intel_graphic_card;
    struct intel_graphic_card_info_t intel_graphic_card[MAX_NUMBER_OF_INTEL_GRAPHIC_CARDS];

    dword_t p_sound_card;
    dword_t n_ac97;
    struct ac97_info_t ac97[MAX_NUMBER_OF_AC97_SOUND_CARDS];
    dword_t n_hda;
    struct hda_info_t hda[MAX_NUMBER_OF_HDA_SOUND_CARDS];

    dword_t n_ide;
    struct ide_info_t ide[MAX_NUMBER_OF_IDE_CONTROLLERS];
    dword_t n_ahci;
    struct ahci_info_t ahci[MAX_NUMBER_OF_AHCI_CONTROLLERS];

    dword_t n_intel_e1000;
    struct intel_e1000_info_t intel_e1000[MAX_NUMBER_OF_INTEL_E1000_CARDS];
    dword_t n_amd_pcnet;
    struct amd_pcnet_info_t amd_pcnet[MAX_NUMBER_OF_AMD_PCNET_CARDS];
    dword_t n_realtek_8139;
    struct realtek_8139_info_t realtek_8139[MAX_NUMBER_OF_REALTEK_8139_CARDS];
    dword_t n_realtek_8169;
    struct realtek_8169_info_t realtek_8169[MAX_NUMBER_OF_REALTEK_8169_CARDS];

    dword_t n_uhci;
    struct uhci_info_t uhci[MAX_NUMBER_OF_UHCI_CONTROLLERS];
    dword_t n_ohci;
    struct ohci_info_t ohci[MAX_NUMBER_OF_OHCI_CONTROLLERS];
    dword_t n_ehci;
    struct ehci_info_t ehci[MAX_NUMBER_OF_EHCI_CONTROLLERS];
    dword_t n_xhci;
    struct xhci_info_t xhci[MAX_NUMBER_OF_XHCI_CONTROLLERS];

    dword_t n_serial_port;
    struct serial_port_info_t serial_port[MAX_NUMBER_OF_SERIAL_PORTS];
};
struct components_info_t *components;