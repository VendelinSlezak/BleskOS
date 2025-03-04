//BleskOS

/*
* MIT License
* Copyright (c) 2023-2025 BleskOS developers
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

void reset_timer(void) {
 if(hpet_base!=0) { //we use HPET
  hpet_reset_counter();
 }
 else { //we use PIT
  ticks = 0;
  while(ticks==0) { //PIT interrupt will increase ticks value
   asm("hlt");
  }
  ticks = 0;
 }
}

dword_t get_timer_value_in_microseconds(void) {
 if(hpet_base!=0) { //we use HPET
  return hpet_return_time_in_microseconds();
 }
 else { //we use PIT
  return (ticks*MILISECONDS_PER_ONE_PIT_TICK*1000);
 }
}

dword_t get_timer_value_in_milliseconds(void) {
 if(hpet_base!=0) { //we use HPET
  return (hpet_return_time_in_microseconds()/1000);
 }
 else { //we use PIT
  return (ticks*MILISECONDS_PER_ONE_PIT_TICK);
 }
}

void wait(dword_t milliseconds) {
 //because PIT interrupt is fired every 2 milliseconds, we need to convert number and add some extra time to be sure that we do not wait less
 milliseconds /= 2;
 milliseconds += 2;

 ticks = 0;
 while(ticks<milliseconds) {
  asm("hlt");
 }
}