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

    /* components connected to PCI */
};
struct components_info_t *components;