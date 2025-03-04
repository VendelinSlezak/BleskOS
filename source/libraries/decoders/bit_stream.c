//BleskOS

/*
* MIT License
* Copyright (c) 2023-2025 BleskOS developers
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

dword_t bit_stream_read_value(byte_t bits) {
 dword_t *bit_stream = (dword_t *) bit_stream_pointer;
 
 //parse value
 value32 = ((bit_stream[0]>>bit_stream_pointer_shift) & bit_stream_mask[bits]);
 
 //move pointers
 bit_stream_pointer_shift += bits;
 bit_stream_pointer += (bit_stream_pointer_shift>>3);
 bit_stream_pointer_shift &= 0x7;
 
 return value32;
}

dword_t bit_stream_read_value_without_moving(byte_t bits) {
 dword_t *bit_stream = (dword_t *) bit_stream_pointer;
 
 return ((bit_stream[0]>>bit_stream_pointer_shift) & bit_stream_mask[bits]);
}

void bit_stream_write_value(byte_t bits, dword_t value) {
 dword_t *bit_stream = (dword_t *) bit_stream_pointer;
 
 //write value
 *bit_stream |= (value<<bit_stream_pointer_shift);
 
 //move pointers
 bit_stream_pointer_shift += bits;
 bit_stream_pointer += (bit_stream_pointer_shift>>3);
 bit_stream_pointer_shift &= 0x7;
}

dword_t reverse_bits(dword_t value, byte_t length) {
 dword_t result = 0;
 
 for(int i=0, shift=(length-1), mask=0x1; i<length; i++, shift-=2, mask<<=1) {
  if(shift<0) {
   result |= ((value & mask)>>(shift*-1));
  }
  else {
   result |= ((value & mask)<<shift);
  }
 }
 
 return result;
}