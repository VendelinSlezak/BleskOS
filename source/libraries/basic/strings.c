//BleskOS

/*
* MIT License
* Copyright (c) 2023 Vendelín Slezák
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

byte_t are_equal_b_string_b_string(byte_t *string1, byte_t *string2) {
 while(*string1!=0 && *string2!=0) {
  if(*string1!=*string2) {
   return STATUS_FALSE;
  }

  string1++;
  string2++;
 }

 return STATUS_TRUE;
}

byte_t are_equal_w_string_b_string(word_t *string1, byte_t *string2) {
 while(*string1!=0 && *string2!=0) {
  if(*string1!=*string2) {
   return STATUS_FALSE;
  }

  string1++;
  string2++;
 }

 return STATUS_TRUE;
}

byte_t are_equal_b_string_w_string(byte_t *string1, word_t *string2) {
 while(*string1!=0 && *string2!=0) {
  if(*string1!=*string2) {
   return STATUS_FALSE;
  }

  string1++;
  string2++;
 }

 return STATUS_TRUE;
}

byte_t are_equal_w_string_w_string(word_t *string1, word_t *string2) {
 while(*string1!=0 && *string2!=0) {
  if(*string1!=*string2) {
   return STATUS_FALSE;
  }

  string1++;
  string2++;
 }

 return STATUS_TRUE;
}

byte_t is_char(word_t value) {
 if((value>='A' && value<='Z') || (value>='a' && value<='z') || (value>='0' && value<='9')) {
  return STATUS_TRUE;
 }
 else {
  return STATUS_FALSE;
 }
}

byte_t is_number(dword_t value) {
 if(value>='0' && value<='9') {
  return STATUS_TRUE;
 }
 
 return STATUS_FALSE;
}

byte_t is_hex_number(dword_t value) {
 if((value>='A' && value<='F') || (value>='a' && value<='f') || (value>='0' && value<='9')) {
  return STATUS_TRUE;
 }
 
 return STATUS_FALSE;
}

word_t get_unicode_char_with_diacritic(word_t value, word_t diacritic) {
 if(diacritic==UNICODE_COMBINING_ACUTE) {
  if(value=='a') {
   return 0x00E1;
  }
  else if(value=='e') {
   return 0x00E9;
  }
  else if(value=='i') {
   return 0x00ED;
  }
  else if(value=='l') {
   return 0x013A;
  }
  else if(value=='o') {
   return 0x00F3;
  }
  else if(value=='r') {
   return 0x0155;
  }
  else if(value=='u') {
   return 0x00FA;
  }
  else if(value=='y') {
   return 0x00FD;
  }
 }
 else if(diacritic==UNICODE_COMBINIG_CARON) {
  if(value=='c') {
   return 0x010D;
  }
  else if(value=='d') {
   return 0x010F;
  }
  else if(value=='e') {
   return 0x011B;
  }
  else if(value=='l') {
   return 0x013E;
  }
  else if(value=='n') {
   return 0x0148;
  }
  else if(value=='r') {
   return 0x0159;
  }
  else if(value=='s') {
   return 0x0161;
  }
  else if(value=='t') {
   return 0x0165;
  }
  else if(value=='z') {
   return 0x017E;
  }
 }

 //TODO: more chars

 return value;
}

dword_t get_number_of_chars_in_ascii_string(byte_t *string) {
 for(dword_t i=0; i<1000; i++) {
  if(string[i]==0) {
   return i;
  }
 }
}