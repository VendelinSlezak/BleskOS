//BleskOS

/*
* MIT License
* Copyright (c) 2023 Vendelín Slezák
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

dword_t create_huffman_table(dword_t number_of_entries) { 
 value32 = calloc((number_of_entries+1)*8);
 
 dword_t *huffman_table = (dword_t *) value32;
 huffman_table[0] = 2; //pointer to first entry
 huffman_table[1] = number_of_entries;
 
 // huffman table:
 // 4 bytes = pointer to first free entry in way that it can be accessed like huffman_table[huffman_table[0]]
 // 4 bytes = number of all entries
 // 8 bytes*number_of_entries = entries
 //  one entry:
 //  - 1 byte = number of bits in code
 //  - 3 bytes = code
 //  - 4 bytes = result of code
 
 return value32;
}

void reset_huffman_table(dword_t table_pointer) {
 dword_t *huffman_table = (dword_t *) table_pointer;
 
 clear_memory(table_pointer+8, huffman_table[1]*8);
 huffman_table[0] = 2;
}

void huffman_table_add_entry(dword_t table_pointer, dword_t length, dword_t code, dword_t result) {
 dword_t *huffman_table = (dword_t *) table_pointer;
 
 if(huffman_table[0]>=(huffman_table[1]*2+2)) {
  return; //table if full
 }
 
 // e.g. if code 0b110(0x6) will result to 0x18, entry will look like this:
 // 0x00000603
 // 0x00000018
 
 //create entry
 huffman_table[huffman_table[0]+0] = (length | (code<<8));
 huffman_table[huffman_table[0]+1] = result;
 
 //next entry
 huffman_table[0]+=2;
}

dword_t huffman_table_parse_one_value(dword_t table_pointer) {
 dword_t *huffman_table = (dword_t *) table_pointer;
 dword_t *bit_stream = (dword_t *) bit_stream_pointer;
 
 if(huffman_table[0]<2) {
  return 0; //this was not pointer to huffman table
 }
 
 //read data
 dword_t value = (*bit_stream>>bit_stream_pointer_shift);
 
 //find value in huffman table
 dword_t huffman_table_entries=((huffman_table[0]-2)/2);
 huffman_table += 2;
 for(dword_t i=0; i<huffman_table_entries; i++, huffman_table+=2) {
  if((value & bit_stream_mask[(huffman_table[0] & 0xFF)])==(huffman_table[0]>>8)) {
   //move pointers
   bit_stream_pointer_shift += (huffman_table[0] & 0xFF);
   bit_stream_pointer += (bit_stream_pointer_shift>>3);
   bit_stream_pointer_shift &= 0x7;
   
   return huffman_table[1];
  }
 }
 
 //value was not founded
 return 0;
}

void huffman_table_add_one_value(dword_t table_pointer, dword_t value) {
 dword_t *huffman_table = (dword_t *) table_pointer;

 //find value in huffman table
 dword_t huffman_table_entries=((huffman_table[0]-2)/2);
 huffman_table += 2;
 for(dword_t i=0; i<huffman_table_entries; i++, huffman_table+=2) {
  if(huffman_table[1]==value) {
   //write value
   bit_stream_write_value((huffman_table[0] & 0xFF), (huffman_table[0]>>8));
  }
 }
}
