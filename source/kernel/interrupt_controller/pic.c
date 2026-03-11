/* 
* BleskOS
*
* MIT License
* Copyright (c) 2023-2026 BleskOS developers
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

/* includes */
#include <kernel/cpu/commands.h>
#include <kernel/hardware/groups/logging/logging.h>

/* functions */
void initialize_pic(void) {
    // command to start initalization sequence
    outb(PORT_MASTER_PIC_COMMAND, 0x11);
    outb(PORT_SLAVE_PIC_COMMAND, 0x11);

    // we want to have IRQ0 - IRQ7 on IDT entries 32 - 39
    outb(PORT_MASTER_PIC_DATA, 32);

    // we want to have IRQ8 - IRQ15 on IDT entries 40 - 47
    outb(PORT_SLAVE_PIC_DATA, 40);

    // slave PIC interrupts are connected to IRQ2
    outb(PORT_MASTER_PIC_DATA, 0x04);

    // IRQ2 from master PIC is connected to slave PIC on IRQ9
    outb(PORT_SLAVE_PIC_DATA, 0x02);

    // we want 8086 mode
    outb(PORT_MASTER_PIC_DATA, 0x01);
    outb(PORT_SLAVE_PIC_DATA, 0x01);

    // mask all interrupts except for IRQ2 so slave PIC interrupts will work
    outb(PORT_MASTER_PIC_DATA, 0xFB);
    outb(PORT_SLAVE_PIC_DATA, 0xFF);

    log("\n[PIC] Initialization completed");
}

void disable_pic(void) {
    outb(PORT_MASTER_PIC_DATA, 0xFF);
    outb(PORT_SLAVE_PIC_DATA, 0xFF);
}

void pic_enable_interrupt(uint8_t irq_number) {
    uint8_t mask;
    if(irq_number < 8) {
        mask = inb(PORT_MASTER_PIC_DATA);
        mask &= ~(1 << irq_number);
        outb(PORT_MASTER_PIC_DATA, mask);
    }
    else {
        mask = inb(PORT_SLAVE_PIC_DATA);
        mask &= ~(1 << (irq_number - 8));
        outb(PORT_SLAVE_PIC_DATA, mask);
    }
}

void pic_send_eoi(uint8_t irq_number) {
    if(irq_number >= 8) {
        outb(PORT_SLAVE_PIC_COMMAND, 0x20); // send EOI to slave PIC
    }
    outb(PORT_MASTER_PIC_COMMAND, 0x20); // send EOI to master PIC
}