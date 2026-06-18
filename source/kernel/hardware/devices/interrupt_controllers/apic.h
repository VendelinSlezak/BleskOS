/* 
* BleskOS
*
* MIT License
* Copyright (c) 2023-2026 BleskOS developers
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#define MAX_NUMBER_OF_IOAPICS_ON_SYSTEM 256

#define APIC_DELIVERY_MODE_NORMAL 0
#define APIC_DELIVERY_MODE_LOW_PRIORITY 1
#define APIC_DELIVERY_MODE_SMI 2
#define APIC_DELIVERY_MODE_NMI 4
#define APIC_DELIVERY_MODE_INIT 5
#define APIC_DELIVERY_MODE_EXTERNAL 7

#define APIC_PHYSICAL_DESTINATION 0
#define APIC_LOGICAL_DESTINATION 1

typedef struct {
    uint8_t id;
    uint32_t mmio_address;
    uint32_t base_gsi;
    uint32_t volatile *port_register;
    uint32_t volatile *port_data;
    uint32_t redirection_entries;
} io_apic_info_t;

extern uint32_t number_of_io_apics;
extern io_apic_info_t *io_apics;