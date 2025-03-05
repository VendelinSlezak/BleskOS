//BleskOS

/*
* MIT License
* Copyright (c) 2023-2025 BleskOS developers
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

void intalize_pic(void) {
    //command to start initalization sequence
    outb(0x20, 0x11);
    outb(0xA0, 0x11);

    //we want to have IRQ0 - IRQ7 on IDT entries 32 - 39
    outb(0x21, 32);

    //we want to have IRQ8 - IRQ15 on IDT entries 40 - 47
    outb(0xA1, 40);

    //slave PIC interrupts are connected to IRQ2
    outb(0x21, 0x04);

    //IRQ2 from master PIC is connected to slave PIC on IRQ9
    outb(0xA1, 0x02);

    //we want 8086 mode
    outb(0x21, 0x01);
    outb(0xA1, 0x01);

    //mask all interrupts except for IRQ2 so slave PIC interrupts will work
    outb(0x21, 0xFB);
    outb(0xA1, 0xFF);
}