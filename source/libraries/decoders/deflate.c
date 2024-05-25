//BleskOS

/*
* MIT License
* Copyright (c) 2023-2024 Vendelín Slezák
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

void initalize_deflate(void) { 
 //alocate memory for huffman tables
 deflate_encoding_huffman_table = create_huffman_table(19);
 deflate_huffman_tables_lengths_mem = calloc((288+32));
 deflate_fixed_huffman_table = create_huffman_table(288);
 deflate_fixed_huffman_table_distance = create_huffman_table(32);
 deflate_dynamic_huffman_table = create_huffman_table(286);
 deflate_dynamic_huffman_table_distance = create_huffman_table(30);
 
 //create fixed huffman table
 for(int code=0x00, result=256; result<280; code++, result++) {
  huffman_table_add_entry(deflate_fixed_huffman_table, 7, reverse_bits(code, 7), result);
 }
 for(int code=0x30, result=0; result<144; code++, result++) {
  huffman_table_add_entry(deflate_fixed_huffman_table, 8, reverse_bits(code, 8), result);
 }
 for(int code=0xC0, result=280; result<288; code++, result++) {
  huffman_table_add_entry(deflate_fixed_huffman_table, 8, reverse_bits(code, 8), result);
 }
 for(int code=0x190, result=144; result<256; code++, result++) {
  huffman_table_add_entry(deflate_fixed_huffman_table, 9, reverse_bits(code, 9), result);
 }
 
 //create fixed huffman table distance
 for(int code=0; code<32; code++) {
  huffman_table_add_entry(deflate_fixed_huffman_table_distance, 5, reverse_bits(code, 5), code);
 }
}

void deflate_create_canonical_huffman_table(dword_t table_pointer, dword_t lengths_pointer, dword_t num_of_entries) {
 byte_t *lengths_mem = (byte_t *) lengths_pointer;
 dword_t output_values[num_of_entries];
 byte_t swap_value, biggest_length_value;
 dword_t code_value;
 
 //initalize output entries - in DEFLATE always numbers counting from 0
 for(int i=0; i<num_of_entries; i++) {
  output_values[i] = i;
 }
 
 //find biggest length
 biggest_length_value = lengths_mem[0];
 for(int i=0; i<num_of_entries; i++) {
  if(lengths_mem[i]>biggest_length_value) {
   biggest_length_value = lengths_mem[i];
  }
 }
 if(biggest_length_value==0) { //no output
  return;
 }
 
 //create huffman table
 for(int code_length=1, code_value=0; code_length<=biggest_length_value; code_length++) {
  for(int i=0; i<num_of_entries; i++) {
   if(lengths_mem[i]==code_length) {
    huffman_table_add_entry(table_pointer, code_length, reverse_bits(code_value, code_length), output_values[i]);
    code_value++;
   }
  }
  
  code_value <<= 1;
 }
 
}

byte_t decode_deflate(dword_t input, dword_t input_length, dword_t output, dword_t output_length) {
 byte_t *output_ptr = (byte_t *) output;
 byte_t flag_end = 0;
 byte_t compression_type = 0;
 dword_t deflate_huffman_table = 0;
 dword_t deflate_huffman_table_distance = 0;
 
 input_length += input;
 output_length += output;
 bit_stream_pointer = input;
 bit_stream_pointer_shift = 0;
 decoded_stream_length = 0;
 
 //decode data
 while(flag_end==0 && bit_stream_pointer<=input_length && (dword_t)output_ptr<=output_length) {
  flag_end = bit_stream_read_value(1);
  compression_type = bit_stream_read_value(2);

  if(compression_type==0) {  
   //no compression
   if(bit_stream_pointer_shift!=0) {
    bit_stream_pointer++;
    bit_stream_pointer_shift=0;
   }
   dword_t len = bit_stream_read_value(16);
   dword_t nlen = bit_stream_read_value(16);
   copy_memory(bit_stream_pointer, ((dword_t)output_ptr), len); //output
   bit_stream_pointer += len;
   output_ptr += len;
  }
  else if(compression_type<3) {
   if(compression_type==1) {
    //fixed huffman table
    deflate_huffman_table = deflate_fixed_huffman_table;
    deflate_huffman_table_distance = deflate_fixed_huffman_table_distance;
   }
   else {
    //dynamic huffman table
    dword_t hlit = (bit_stream_read_value(5) + 257);
    dword_t hdist = (bit_stream_read_value(5) + 1);
    dword_t hclen = (bit_stream_read_value(4) + 4);
    
    //create huffman table that encode literal and distance huffman tables
    byte_t encoding_huffman_table_lengths[19];
    for(int i=0; i<19; i++) {
     encoding_huffman_table_lengths[i]=0;
    }
    for(int i=0; i<hclen; i++) {
     encoding_huffman_table_lengths[encoding_huffman_table_order[i]] = bit_stream_read_value(3);
    }
    reset_huffman_table(deflate_encoding_huffman_table);
    deflate_create_canonical_huffman_table(deflate_encoding_huffman_table, (dword_t)&encoding_huffman_table_lengths, 19);
    
    //decode literal and distance huffman tables lengths by encoding huffman table
    clear_memory(deflate_huffman_tables_lengths_mem, (288+32));
    dword_t decoded_values_of_tables = 0;
    byte_t *huffman_tables_lengths = (byte_t *) deflate_huffman_tables_lengths_mem;
    while(decoded_values_of_tables<(hlit+hdist)) {
     value32 = huffman_table_parse_one_value(deflate_encoding_huffman_table);
     
     if(value32<16) { //length value
      huffman_tables_lengths[decoded_values_of_tables] = (byte_t)value32;
      decoded_values_of_tables++;
     }
     else if(value32==16) { //repeat
      value8 = (3+bit_stream_read_value(2));
      decode_lz77(deflate_huffman_tables_lengths_mem+decoded_values_of_tables, 1, value8, deflate_huffman_tables_lengths_mem+decoded_values_of_tables, 256);
      decoded_values_of_tables += value8;
     }
     else if(value32==17) { //zeroes
      value8 = (3+bit_stream_read_value(3));
      clear_memory(deflate_huffman_tables_lengths_mem+decoded_values_of_tables, value8);
      decoded_values_of_tables += value8;
     }
     else if(value32==18) { //zeroes
      value8 = (11+bit_stream_read_value(7));
      clear_memory(deflate_huffman_tables_lengths_mem+decoded_values_of_tables, value8);
      decoded_values_of_tables += value8;
     }
     else {
      log("\nDEFLATE: invalid encoding huffman table value ");
      log_var(value32);
      return STATUS_ERROR;
     }
    }
    
    //create huffman tables
    reset_huffman_table(deflate_dynamic_huffman_table);
    reset_huffman_table(deflate_dynamic_huffman_table_distance);
    deflate_create_canonical_huffman_table(deflate_dynamic_huffman_table, deflate_huffman_tables_lengths_mem, hlit);
    deflate_create_canonical_huffman_table(deflate_dynamic_huffman_table_distance, (deflate_huffman_tables_lengths_mem+hlit), hdist);
    deflate_huffman_table = deflate_dynamic_huffman_table;
    deflate_huffman_table_distance = deflate_dynamic_huffman_table_distance;
   }
   
   //decode data with huffman tables
   while(bit_stream_pointer<=input_length && (dword_t)output_ptr<output_length) {
    value32 = huffman_table_parse_one_value(deflate_huffman_table);

    if(value32<256) { //put char to output stream
     *output_ptr = (byte_t)value32;
     output_ptr++;
     decoded_stream_length++;
    }
    else if(value32==256) { //end of block
     break;
    }
    else if(value32<287) { //repeat char with LZ77
     value32 -= 257;
     dword_t repeat = (deflate_lz77_repeat_base[value32] + bit_stream_read_value(deflate_lz77_length_extra_bits[value32]));
     dword_t distance_value = huffman_table_parse_one_value(deflate_huffman_table_distance);
     dword_t distance = (deflate_lz77_distance_base[distance_value] + bit_stream_read_value(deflate_lz77_distance_extra_bits[distance_value]));
     decode_lz77((dword_t)output_ptr, distance, repeat, (dword_t)output_ptr, 32769);
     output_ptr += repeat;
     decoded_stream_length += repeat;
    }
    else { //unknown value
     log("\nDEFLATE: invalid char from stream ");
     log_var(value32);
     return STATUS_ERROR;
    }
   }
   if((dword_t)output_ptr==output_length && huffman_table_parse_one_value_without_moving(deflate_huffman_table)==256) { //there might be case when ending byte is one byte outside of expected input range
    huffman_table_parse_one_value(deflate_huffman_table);
   }
  }
  else { //invalid compression type 3
   log("\nDEFLATE: invalid compression type 3");
   return STATUS_ERROR;
  }
 }

 return STATUS_GOOD;
}