#ifndef BUILD_KERNEL_X86_INTERRUPT_CONTROLLERS_PIC_H
#define BUILD_KERNEL_X86_INTERRUPT_CONTROLLERS_PIC_H

/* 
* BleskOS
*
* MIT License
* Copyright (c) 2023-2025 BleskOS developers
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#define PORT_MASTER_PIC_COMMAND 0x20
#define PORT_MASTER_PIC_DATA 0x21
#define PORT_SLAVE_PIC_COMMAND 0xA0
#define PORT_SLAVE_PIC_DATA 0xA1
void initialize_pic(void);
void pic_connect_legacy_interrupt_to_handler(dword_t irq, void (*handler)(void));

#endif /* BUILD_KERNEL_X86_INTERRUPT_CONTROLLERS_PIC_H */
