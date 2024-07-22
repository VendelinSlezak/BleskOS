//BleskOS

/*
* MIT License
* Copyright (c) 2023-2024 Vendelín Slezák
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

int abs(int i) {
 if(i<0) {
  i *= -1;
 }
 return i;
}

float fabs(float i) {
 if(i<0) {
  i *= -1;
 }
 return i;
}

int floor(double i) {
 if(i==0) {
  return 0;
 }
 else if(i>0) {
  return (int) i;
 }
 else if(((int) i)==i){
  return (int) i;
 }
 else {
  return ((int) i)-1;
 }
}

int floorf(float i) {
 if(i==0) {
  return 0;
 }
 else if(i>0) {
  return (int) i;
 }
 else if(((int) i)==i){
  return (int) i;
 }
 else {
  return ((int) i)-1;
 }
}

float ceilf(float number) {
 if((number-((int)number))==0) { //already rounded number
  return number;
 }
 else if(number>0) { //positive number
  return ((number/1)+1);
 }
 else { //negative number
  return (number/1);
 }
}

dword_t power(dword_t value, dword_t exponent) {
 if(exponent==0) {
  return 1;
 }
 else {
  exponent--;
  dword_t multiply_value = value;
  for(dword_t i=0; i<exponent; i++) {
   value *= multiply_value;
  }
  return value;
 }
}

float ldexp(float value, dword_t exponent) {
 if(exponent==0) {
  return 1;
 }
 else {
  exponent--;
  float multiply_value = value;
  for(dword_t i=0; i<exponent; i++) {
   value *= multiply_value;
  }
  return value;
 }
}

dword_t convert_big_endian_dword_to_little_endian(byte_t *mem) {
 return ((mem[0]<<24) | (mem[1]<<16) | (mem[2]<<8) | (mem[3]));
}

word_t convert_big_endian_word_to_little_endian(byte_t *mem) {
 return ((mem[0]<<8) | (mem[1]));
}

dword_t get_number_of_digits_in_number(dword_t number) {
 if(number==0) {
  return 1;
 }
 else {
  dword_t digits = 0;
  while(number>0) {
   number /= 10;
   digits++;
  }
  return digits;
 }
}

dword_t convert_byte_string_to_number(dword_t mem) {
 byte_t *string = (byte_t *) mem;
 dword_t number = 0;
 for(int i=0; i<10; i++) {
  if(string[i]<'0' || string[i]>'9') {
   return number;
  }
  number *= 10;
  number += (string[i]-'0');
 }
 
 return number;
}

dword_t convert_word_string_to_number(dword_t mem) {
 word_t *string = (word_t *) mem;
 dword_t number = 0;
 for(int i=0; i<10; i++) {
  if(string[i]<'0' || string[i]>'9') {
   return number;
  }
  number *= 10;
  number += (string[i]-'0');
 }
 
 return number;
}

float convert_word_string_to_float_number(dword_t mem) {
 word_t *string = (word_t *) mem;
 float number = 0, digit_base;

 number = convert_word_string_to_number(mem);
 while(*string>='0' && *string<='9') {
  string++;
 }
 if(*string=='.') {
  string++;
  
  digit_base = 0.1;
  
  for(int i=0; i<6; i++) {
   if(*string>='0' && *string<='9') {
    number += ((*string-'0')*digit_base);
    digit_base /= 10;
   }
   else {
    break;
   }
  }
 }
 
 return number;
}

dword_t convert_hex_word_string_to_number(dword_t mem) {
 word_t *string = (word_t *) mem;
 dword_t number = 0;
 for(int i=0; i<8; i++) {
  if((string[i]>='0' && string[i]<='9') || (string[i]>='a' && string[i]<='f') || (string[i]>='A' && string[i]<='F')) {
   number<<=4;
   if(string[i]>='0' && string[i]<='9') {
    number += (string[i]-'0');
   }
   else if(string[i]>='a' && string[i]<='f') {
    number += (string[i]-'a'+10);
   }
   else if(string[i]>='A' && string[i]<='F') {
    number += (string[i]-'A'+10);
   }
  }
  else {
   break;
  }
 }
 
 return number;
}

void convert_number_to_byte_string(dword_t number, dword_t mem) {
 byte_t *string = (byte_t *) mem;
 word_t number_string[11];
 dword_t number_digits = 0;
 
 for(int i=0; i<11; i++) {
  number_string[i]=0;
 }
 
 for(int i=10; i>0; i--) {
  number_string[i]=(number%10);
  number/=10;
  number_digits++;
  if(number==0) {
   break;
  }
 }
 
 for(int i=0; i<11; i++) {
  number_string[i]+='0';
 }
 
 for(int i=0; i<number_digits; i++) {
  string[i] = number_string[((11-number_digits)+i)];
 }
}

void convert_number_to_word_string(dword_t number, dword_t mem) {
 word_t *string = (word_t *) mem;
 word_t number_string[11];
 dword_t number_digits = 0;
 
 for(int i=0; i<11; i++) {
  number_string[i]=0;
 }
 
 for(int i=10; i>0; i--) {
  number_string[i]=(number%10);
  number/=10;
  number_digits++;
  if(number==0) {
   break;
  }
 }
 
 for(int i=0; i<11; i++) {
  number_string[i]+='0';
 }
 
 for(int i=0; i<number_digits; i++) {
  string[i] = number_string[((11-number_digits)+i)];
 }
}

void convert_unix_time(dword_t unix_time) {
 math_day = unix_time/86400;
 math_month = 0;
 math_year = 1970;

 //calculate year
 byte_t years_with_leap_second[26] = { 72, 73, 74, 75, 76, 77, 78, 79, 81, 82, 83, 85, 87, 89, 90, 92, 93, 94, 95, 97, 98, 5, 8, 12, 15, 16 };
 while(1) {
  if(math_year%400==0 || (math_year%4==0 && math_year%100!=0)) {
   if(math_day<366) {
    break;
   }
   else {
    math_day-=366;
   }
  }
  else {
   if(math_day<365) {
    break;
   }
   else {
    math_day-=365;
   }
  }

  math_year++;
  if(math_year==1972) {
   unix_time--; //there are two leap seconds in this year
  }
  for(dword_t i=0; i<26; i++) {
   if(years_with_leap_second[i]==(math_year%100)) {
    unix_time--; //leap second
    break;
   }
  }
 }

 //calculate month and day
 byte_t month_days[12] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
 if(math_year%400==0 || (math_year%4==0 && math_year%100!=0)) {
  month_days[1]++;
 }
 for(dword_t i=0; i<12; i++) {
  math_month++;

  if(math_day<month_days[i]) {
   math_day++;
   break;
  }

  math_day -= month_days[i];
 }
}