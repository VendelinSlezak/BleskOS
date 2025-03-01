//BleskOS

/*
* MIT License
* Copyright (c) 2023-2025 Vendelín Slezák
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
 hda_playing_state = 0;
 ac97_playing_state = 0;
 #ifndef NO_PROGRAMS
 bleskos_main_window_time_redraw = 0;
 media_viewer_sound_state = MEDIA_VIEWER_SOUND_NO_FILE;
 #endif

 //clear variables
 extern dword_t irq_handlers[16][8];
 clear_memory((dword_t)&irq_handlers, sizeof(irq_handlers));
 cli_level = 0;
 interrupt_handler_running = 0;

 //load IDT and enable interrupts
 load_idt();
}

void set_irq_handler(dword_t irq, dword_t handler) {
 extern dword_t irq_handlers[16][8];
 extern dword_t irq_handlers_number_of_methods[16];

 l("\nRequest to connect handler to IRQ "); lvw(irq); lhw(handler);

 //refuse invalid irq number
 if(irq>15) {
  return;
 }

 //there can be max 8 methods on one IRQ
 if(irq_handlers_number_of_methods[irq] >= 8) {
  l("\nERROR: IRQ handler methods are full");
  return;
 }

 //check if this method is not already connected
 for(dword_t i=0; i<irq_handlers_number_of_methods[irq]; i++) {
  if(irq_handlers[irq][i] == handler) {
   l("\nAlready connected handler");
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