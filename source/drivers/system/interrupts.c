//BleskOS

/*
* MIT License
* Copyright (c) 2023-2024 Vendelín Slezák
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

void set_interrupts(void) {
 extern void load_idt(void);

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
 
 //disable all variables that affect IRQ0 routine
 ticks = 0;
 ticks_of_processes = 0;
 bleskos_main_window_time_redraw = 0;
 hda_playing_state = 0;
 ac97_playing_state = 0;
 usb_mouse[0].controller_type = USB_NO_DEVICE_ATTACHED;
 media_viewer_sound_state = MEDIA_VIEWER_SOUND_NO_FILE;
 
  //load IDT and enable interrupts
 load_idt();
}

void set_irq_handler(dword_t irq, dword_t handler) {
 extern dword_t irq_handlers[16];

 //refuse invalid irq number
 if(irq>15) {
  return;
 }

 //set pointer to method
 irq_handlers[irq] = handler;

 //unmask interrupt
 if(irq<8) {
  outb(0x21, (inb(0x21) & ~(1<<irq)));
 }
 else {
  outb(0xA1, (inb(0xA1) & ~(1<<(irq-8))));
 }
}

void isr_handler(dword_t isr_number) {
 clear_screen(RED);
 print("Serious error occured", 10, 10, BLACK);
 print("Error type: ISR", 10, 30, BLACK);
 extern dword_t isr_string_array[32];
 if(isr_number<19) {
  print((byte_t *)(isr_string_array[isr_number]), 10, 50, BLACK);
 }
 else {
  print("Reserved exception ", 10, 50, BLACK);
  print_var(isr_number, 10+20*8, 50, BLACK);
 }
 redraw_screen();

 while(1) { 
  asm("hlt"); //halt forever
 }
}