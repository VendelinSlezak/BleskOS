//BleskOS

/*
* MIT License
* Copyright (c) 2023-2025 Vendelín Slezák
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

void set_pit(void) {
 //set PIT to 500 Hz = fire interrupt every 2 ms
 outb(0x43, 0x36);
 outb(0x40, (2386 & 0xFF));
 outb(0x40, (2386 >> 8));
 
 //set interrupt routine
 ticks = 0;
 ticks_of_processes = 0;
 set_irq_handler(0, (dword_t)pit_handler);
}

void pit_handler(void) {
 //this variable counts how long system runs
 time_of_system_running+=2;

 //this variable counts PIT interrupts
 ticks++;

 //this variable is used for some processes on background and is defined in source/system/processes_on_background.h
 ticks_of_processes++;
 
 //run processes that are on background of programs
 run_processes_on_background();

 //run tasks in scheduler
 scheduler_periodic_interrupt();
}