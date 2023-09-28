//BleskOS

/*
* MIT License
* Copyright (c) 2023 Vendelín Slezák
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

void read_time_format(void) {
 //time format is in Status register B
 outb(0x70, 0x0B);
 byte_t status_register_b = (inb(0x71));

 //parse bits, 0 = STATUS_FALSE, 1 = STATUS_TRUE
 time_format_24_hours = ((status_register_b>>1) & 0x1);
 time_format_binary = ((status_register_b>>2) & 0x1);
}

byte_t read_time_register(byte_t register_number) {
 outb(0x70, register_number);
 if(time_format_binary==STATUS_TRUE) {
  return inb(0x71);
 }
 else { //BCD format
  byte_t value = inb(0x71);
  return ((value & 0xF) + ((value >> 4)*10));
 }
}

void read_time_second(void) {
 time_second = read_time_register(0x00);
}

void read_time(void) {
 read_time_second();
 time_minute = read_time_register(0x02);
 time_weekday = read_time_register(0x06);
 time_day = read_time_register(0x07);
 time_month = read_time_register(0x08);
 time_year = read_time_register(0x09);

 //hour needs special handling because of 12/24 hour format
 if(time_format_24_hours==STATUS_TRUE) {
  time_hour = read_time_register(0x04);
 }
 else {
  outb(0x70, 0x04);
  time_hour = inb(0x71);
  if((time_hour & 0x80)==0x00) { //AM
   if(time_format_binary==STATUS_FALSE) { //convert if in BCD format
    time_hour = ((time_hour & 0xF) + ((time_hour >> 4)*10));
   }
  }
  else { //PM
   time_hour &= ~0x80; //clear PM bit

   if(time_format_binary==STATUS_FALSE) { //convert if in BCD format
    time_hour = ((time_hour & 0xF) + ((time_hour >> 4)*10));
   }

   //convert to actual hour
   if(time_hour==12) {
    time_hour = 0; //midnight
   }
   else {
    time_hour += 12;
   }
  }
 }
 
 //convert year number to real year
 if(time_year<80) {
  time_year += 2000;
 }
 else {
  time_year += 1900;
 }
}
