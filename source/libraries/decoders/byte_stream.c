//BleskOS

/*
* MIT License
* Copyright (c) 2023-2024 Vendelín Slezák
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

/*
 Byte stream is used when we have output without specified output length.
 This interface allows to write to this stream without worrying about memory allocation.

 name_of_struct->start_of_allocated_memory contain dword_t value where in memory is start of byte stream.
 name_of_struct->size_of_stream contain number of bytes in stream.

 Example:

 struct byte_stream_descriptor *byte_stream_1 = create_byte_stream(BYTE_STREAM_100_KB_BLOCK);
 add_byte_to_byte_stream(byte_stream_1, 1);
 add_byte_to_byte_stream(byte_stream_1, 2);
 add_byte_to_byte_stream(byte_stream_1, 3);
 ...
 destroy_byte_stream(byte_stream_1);
*/

struct byte_stream_descriptor *create_byte_stream(dword_t size_of_block) {
 struct byte_stream_descriptor *new_byte_stream_descriptor = (struct byte_stream_descriptor *) (calloc(sizeof(struct byte_stream_descriptor)));

 new_byte_stream_descriptor->start_of_allocated_memory = (calloc(size_of_block));
 new_byte_stream_descriptor->memory_pointer = (byte_t *) (new_byte_stream_descriptor->start_of_allocated_memory);
 new_byte_stream_descriptor->size_of_allocated_memory = size_of_block;
 new_byte_stream_descriptor->end_of_allocated_memory = (new_byte_stream_descriptor->start_of_allocated_memory+new_byte_stream_descriptor->size_of_allocated_memory);
 new_byte_stream_descriptor->size_of_stream = 0;
 new_byte_stream_descriptor->size_of_one_block = size_of_block;

 return new_byte_stream_descriptor;
}

void add_byte_to_byte_stream(struct byte_stream_descriptor *actual_byte_stream_descriptor, byte_t value) {
 if((dword_t)actual_byte_stream_descriptor->memory_pointer==actual_byte_stream_descriptor->end_of_allocated_memory) {
  //allocate more memory
  actual_byte_stream_descriptor->size_of_allocated_memory += actual_byte_stream_descriptor->size_of_one_block;
  actual_byte_stream_descriptor->start_of_allocated_memory = (realloc(actual_byte_stream_descriptor->start_of_allocated_memory, actual_byte_stream_descriptor->size_of_allocated_memory));
  actual_byte_stream_descriptor->end_of_allocated_memory = (actual_byte_stream_descriptor->start_of_allocated_memory+actual_byte_stream_descriptor->size_of_allocated_memory);
  actual_byte_stream_descriptor->memory_pointer = (byte_t *) (actual_byte_stream_descriptor->end_of_allocated_memory-actual_byte_stream_descriptor->size_of_one_block);
 }

 //write byte
 actual_byte_stream_descriptor->memory_pointer[0] = value;
 actual_byte_stream_descriptor->memory_pointer++;
 actual_byte_stream_descriptor->size_of_stream++;
}

void add_word_to_byte_stream(struct byte_stream_descriptor *actual_byte_stream_descriptor, word_t value) {
 add_byte_to_byte_stream(actual_byte_stream_descriptor, (value & 0xFF));
 add_byte_to_byte_stream(actual_byte_stream_descriptor, (value >> 8));
}

void add_dword_to_byte_stream(struct byte_stream_descriptor *actual_byte_stream_descriptor, dword_t value) {
 add_byte_to_byte_stream(actual_byte_stream_descriptor, (value & 0xFF));
 add_byte_to_byte_stream(actual_byte_stream_descriptor, ((value >> 8) & 0xFF));
 add_byte_to_byte_stream(actual_byte_stream_descriptor, ((value >> 16) & 0xFF));
 add_byte_to_byte_stream(actual_byte_stream_descriptor, ((value >> 24) & 0xFF));
}

void skip_bytes_in_byte_stream(struct byte_stream_descriptor *actual_byte_stream_descriptor, dword_t number_of_bytes) {
 for(dword_t i=0; i<number_of_bytes; i++) {
  add_byte_to_byte_stream(actual_byte_stream_descriptor, 0);
 }
}

void add_bytes_to_byte_stream(struct byte_stream_descriptor *actual_byte_stream_descriptor, byte_t *pointer, dword_t number_of_bytes) {
 for(dword_t i=0; i<number_of_bytes; i++) {
  add_byte_to_byte_stream(actual_byte_stream_descriptor, pointer[i]);
 }
}

void add_string_to_byte_stream(struct byte_stream_descriptor *actual_byte_stream_descriptor, byte_t *string) {
 for(dword_t i=0; i<get_number_of_chars_in_ascii_string(string); i++) {
  add_byte_to_byte_stream(actual_byte_stream_descriptor, string[i]);
 }
}

void add_number_to_byte_stream(struct byte_stream_descriptor *actual_byte_stream_descriptor, dword_t number) {
 clear_memory((dword_t)(&number_string), MATH_LENGTH_OF_NUMBER_STRING_ARRAY);
 convert_number_to_byte_string(number, (dword_t)(&number_string));
 add_string_to_byte_stream(actual_byte_stream_descriptor, (byte_t *)number_string);
}

void add_hex_number_to_byte_stream(struct byte_stream_descriptor *actual_byte_stream_descriptor, dword_t number, dword_t number_of_chars) {
 for(dword_t i=0, shift=((number_of_chars-1)*4); i<number_of_chars; i++, shift-=4) {
  if(((number>>shift) & 0xF)<0xA) {
   add_byte_to_byte_stream(actual_byte_stream_descriptor, ((number>>shift) & 0xF)+'0');
  }
  else {
   add_byte_to_byte_stream(actual_byte_stream_descriptor, ((number>>shift) & 0xF)-10+'A');
  }
 }
}

void create_free_space_in_byte_stream(struct byte_stream_descriptor *actual_byte_stream_descriptor, dword_t space_size) {
 if((actual_byte_stream_descriptor->size_of_allocated_memory-actual_byte_stream_descriptor->size_of_stream)<space_size) {
  //allocate enough memory for size of free space
  actual_byte_stream_descriptor->size_of_allocated_memory += ((space_size/actual_byte_stream_descriptor->size_of_one_block)*actual_byte_stream_descriptor->size_of_one_block+actual_byte_stream_descriptor->size_of_one_block);
  actual_byte_stream_descriptor->start_of_allocated_memory = (realloc(actual_byte_stream_descriptor->start_of_allocated_memory, actual_byte_stream_descriptor->size_of_allocated_memory));
  actual_byte_stream_descriptor->end_of_allocated_memory = (actual_byte_stream_descriptor->start_of_allocated_memory+actual_byte_stream_descriptor->size_of_allocated_memory);
 }
}

byte_t *close_byte_stream(struct byte_stream_descriptor *actual_byte_stream_descriptor) {
 actual_byte_stream_descriptor->start_of_allocated_memory = realloc(actual_byte_stream_descriptor->start_of_allocated_memory, actual_byte_stream_descriptor->size_of_stream);
 byte_t *stream_start = (byte_t *) actual_byte_stream_descriptor->start_of_allocated_memory;
 free((dword_t)actual_byte_stream_descriptor);
 return stream_start;
}

void destroy_byte_stream(struct byte_stream_descriptor *actual_byte_stream_descriptor) {
 free((dword_t)actual_byte_stream_descriptor->start_of_allocated_memory);
 free((dword_t)actual_byte_stream_descriptor);
}