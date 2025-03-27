//BleskOS

/*
* MIT License
* Copyright (c) 2023-2025 BleskOS developers
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

void set_irq_handler(dword_t irq, dword_t handler) {
    extern dword_t irq_handlers[16][8];
    extern dword_t irq_handlers_number_of_methods[16];

    logf("\n[IDT] Request to connect handler to IRQ %d 0x%x", irq, handler);

    //refuse invalid irq number
    if(irq>15) {
        return;
    }

    //there can be max 8 methods on one IRQ
    if(irq_handlers_number_of_methods[irq] >= 8) {
        logf("\n[IDT] ERROR: IRQ handler methods are full");
        return;
    }

    //check if this method is not already connected
    for(dword_t i = 0; i < irq_handlers_number_of_methods[irq]; i++) {
        if(irq_handlers[irq][i] == handler) {
            logf(" Already connected handler");
            return;
        }
    }

    //set pointer to method
    irq_handlers[irq][irq_handlers_number_of_methods[irq]] = handler;
    irq_handlers_number_of_methods[irq]++;

    //unmask interrupt
    if(irq<8) {
        outb(0x21, (inb(0x21) & ~(1<<irq)));
    }
    else {
        outb(0xA1, (inb(0xA1) & ~(1<<(irq-8))));
    }
}