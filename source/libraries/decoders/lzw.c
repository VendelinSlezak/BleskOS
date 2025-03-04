//BleskOS

/*
* MIT License
* Copyright (c) 2023-2025 BleskOS developers
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

void initalize_lzw(void) {
 lzw_dictionary_memory = (dword_t) calloc(4096*8);
 
 // dictionary entry:
 // 4 bytes = pointer to memory where dictionary entry data starts
 // 2 bytes = length of dictionary entry data
 // 2 bytes = last value of dictionary entry data
 
 // so if we have in memory 0x1ABC data: 1, 2, 3
 // and dictionary entry look like this: 0x00001ABC 0x00080003
 // if we want to write this entry to output, it will produce 1, 2, 3, 8
 // this format was chosen because every new LZW dictionary entry contain data that are already in output + one new value
}

byte_t decode_lzw(dword_t init_number_of_values, dword_t input, dword_t input_length, dword_t output, dword_t output_length) {
 byte_t *output_ptr = (byte_t *) output;
 dword_t *dictionary = (dword_t *) lzw_dictionary_memory;
 dword_t init_number_of_read_bits = 0;
 for(int i=0, j=0; i<12; i++) {
  j = (init_number_of_values+2);
  j >>= i;
  if(j==0) {
   break;
  }
  init_number_of_read_bits++;
 }
 dword_t read_num_of_bits = init_number_of_read_bits, read_num_of_bits_shift_value = 0xFFFFFFFF;
 dword_t clear_dictionary_code = init_number_of_values;
 dword_t end_code = init_number_of_values+1;
 dword_t dictionary_entries = init_number_of_values+2;
 dword_t previous_code_mem = 0, previous_code_length = 0, previous_code_last_value = 0;
 
 //initalize values
 input_length += input;
 output_length += output;
 bit_stream_pointer = input;
 bit_stream_pointer_shift = 0;
 decoded_stream_length = 0;
 read_num_of_bits_shift_value <<= read_num_of_bits;
 read_num_of_bits_shift_value = ((~read_num_of_bits_shift_value)+1);
 
 while(bit_stream_pointer<=input_length && (dword_t)output_ptr<=output_length) {
  value32 = bit_stream_read_value(read_num_of_bits);
  
  if(value32==end_code) {
   break;
  }
  else if(value32==clear_dictionary_code) { //reset everything to init values
   read_num_of_bits = init_number_of_read_bits;
   value32 = bit_stream_read_value(read_num_of_bits);
   
   if(value32<clear_dictionary_code) { //dictionary is now clear, so next must not be value from dictionary
    //write value to output
    *output_ptr = value32;
    
    //save this output for next dictionary entry
    previous_code_mem = (dword_t)output_ptr;
    previous_code_length = 1;
    previous_code_last_value = value32;
    
    //reset dictionary
    dictionary_entries = init_number_of_values+2;
    clear_memory(lzw_dictionary_memory, 4096*8);
    
    //move output variables
    output_ptr++;
    decoded_stream_length++;
    
    //reset shift value
    read_num_of_bits_shift_value = 0xFFFFFFFF;
    read_num_of_bits_shift_value <<= read_num_of_bits;
    read_num_of_bits_shift_value = ((~read_num_of_bits_shift_value)+1);
   }
   else if(value32==end_code) {
    break;
   }
   else {
    return STATUS_ERROR;
   }
  }
  else if(value32<clear_dictionary_code) { //this is base value, not in dictionary
   //write value to output
   *output_ptr = value32;
   
   //add new entry to dictionary
   dictionary[dictionary_entries*2+0] = previous_code_mem;
   dictionary[dictionary_entries*2+1] = (previous_code_length | value32<<16);
   dictionary_entries++;
   
   //save this output for next dictionary entry
   previous_code_mem = (dword_t)output_ptr;
   previous_code_length = 1;
   previous_code_last_value = value32;
   
   //move output variables
   output_ptr++;
   decoded_stream_length++;
  }
  else if(value32<dictionary_entries) { //this is value from dictionary
   //write to output dictionary entry data except for last value
   copy_memory((dictionary[value32*2+0]), ((dword_t)output_ptr), (dictionary[value32*2+1] & 0xFFFF));
   
   //add new entry to dictionary
   dictionary[dictionary_entries*2+0] = previous_code_mem;
   dictionary[dictionary_entries*2+1] = (previous_code_length | output_ptr[0]<<16);
   dictionary_entries++;
   
   //save this output for next dictionary entry
   previous_code_mem = (dword_t)output_ptr;
   previous_code_last_value = output_ptr[0];
   previous_code_length = (dictionary[value32*2+1] & 0xFFFF)+1;
   
   //move output variables
   output_ptr += (dictionary[value32*2+1] & 0xFFFF);
   decoded_stream_length += (dictionary[value32*2+1] & 0xFFFF);
   
   //write last value to output
   *output_ptr = (dictionary[value32*2+1] >> 16);
   
   //move output variables
   output_ptr++;
   decoded_stream_length++;
  }
  else { //this is value that is not in dictionary
   //write to output data that was written last time
   copy_memory(previous_code_mem, ((dword_t)output_ptr), previous_code_length);
   
   //add new entry to dictionary
   dictionary[dictionary_entries*2+0] = previous_code_mem;
   dictionary[dictionary_entries*2+1] = (previous_code_length | previous_code_last_value<<16);
   dictionary_entries++;
   
   //save this output for next dictionary entry
   previous_code_mem = (dword_t)output_ptr;
   output_ptr += previous_code_length; //skip data that are in output
   previous_code_length++;
   //because this value is not in dictionary, last value will remain the same
   
   //write last value to output
   *output_ptr = previous_code_last_value;
   
   //move output variables
   output_ptr++;
   decoded_stream_length += previous_code_length;
  }
  
  //if dictionary exceeds max number that can be produced from actual read bits, we need to read one more bit
  if(read_num_of_bits<12 && dictionary_entries==read_num_of_bits_shift_value) { //dictionary can have max 4096 entries, so we will never pass 12 bits
   read_num_of_bits++;
   read_num_of_bits_shift_value <<= 1;
  }
 }

 return STATUS_GOOD;
}
