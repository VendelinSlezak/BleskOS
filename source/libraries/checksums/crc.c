//BleskOS

/*
* MIT License
* Copyright (c) 2023-2025 Vendelín Slezák
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

dword_t calculate_crc32_checksum(byte_t *memory, dword_t size) {
 dword_t crc32 = 0xFFFFFFFF; //starting value
 byte_t actual_byte = 0;

 //go through every byte
 for(dword_t i=0; i<size; i++) {
  actual_byte = memory[i]; //load byte

  //go through every bit
  for(dword_t j=0; j<8; j++) {
   if(((actual_byte^crc32) & 0x1)==0x1) {
    crc32 >>= 1;
    crc32 = (crc32^0xEDB88320);
   }
   else {
    crc32 >>= 1;
   }
   actual_byte >>= 1;
  }
 }

 return (~crc32);
}