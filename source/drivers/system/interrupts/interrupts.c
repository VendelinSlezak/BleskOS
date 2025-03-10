//BleskOS

/*
* MIT License
* Copyright (c) 2023-2025 BleskOS developers
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

void initalize_interrupts(void) {
    // TODO: add code for APIC
    // TODO: add code to test presence of PIC
    intalize_pic();

    // reset variables
    extern dword_t irq_handlers[16][8];
    clear_memory((dword_t)&irq_handlers, sizeof(irq_handlers));
    cli_level = 0;
    interrupt_handler_running = 0;

    // load IDT and enable interrupts
    extern void load_idt(void);
    load_idt();
}

void cli(void) {
    if(interrupt_handler_running == 1) {
        return;
    }

    if(cli_level == 0) {
        asm("cli");
    }

    cli_level++;
}

void sti(void) {
    if(interrupt_handler_running == 1) {
        return;
    }
    
    cli_level--;

    if(cli_level == 0) {
        asm("sti");
    }
}