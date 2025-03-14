//BleskOS

/*
* MIT License
* Copyright (c) 2023-2025 BleskOS developers
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

void decode_lz77(dword_t input, dword_t back, dword_t repeat, dword_t output, dword_t output_length_limit) {
 byte_t *output_ptr = (byte_t *) output;
 byte_t *input_ptr = (byte_t *) (input-back);
 output_length_limit += output;
 
 for(int i=0; i<repeat; i++) {
  *output_ptr=*input_ptr;
  output_ptr++;
  input_ptr++;
  
  if((dword_t)output_ptr>=output_length_limit) {
   return;
  }
 }
}