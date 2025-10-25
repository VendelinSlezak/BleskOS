/* 
* BleskOS
*
* MIT License
* Copyright (c) 2023-2025 BleskOS developers
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

/* includes */
#include <kernel/x86/kernel.h>
#include <hardware/groups/logging/logging.h>
#include <kernel/x86/cpu/commands.h>
#include <kernel/x86/cpu/idt.h>
#include <kernel/x86/interrupt_controllers/main.h>

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

    // set functions
    connect_legacy_interrupt_to_handler = pic_connect_legacy_interrupt_to_handler;

    // log
    log("\n[PIC] Initialization completed");
}

void pic_connect_legacy_interrupt_to_handler(dword_t irq, void (*handler)(void)) {
    // check if this is valid request
    if(irq >= 16) {
        log("\n[PIC] ERROR: Request to connect legacy IRQ %d", irq);
        return;
    }

    // connect handler to IDT table
    if(idt_functions[32+irq] != 0) {
        // TODO: connect multiple handlers on one interrupt
        kernel_panic("Connecting multiple handlers on one interrupt");
    }
    else {
        log("\n[PIC] Registered IRQ %d", irq);
        idt_functions[32+irq] = handler;
    }

    // unmask interrupt
    if(irq < 8) {
        outb(PORT_MASTER_PIC_DATA, (inb(PORT_MASTER_PIC_DATA) & ~(1 << irq)));
    }
    else {
        outb(PORT_SLAVE_PIC_DATA, (inb(PORT_SLAVE_PIC_DATA) & ~(1 << (irq - 8))));
    }
}