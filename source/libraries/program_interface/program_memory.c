//BleskOS

/*
* MIT License
* Copyright (c) 2023-2024 Vendelín Slezák
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

dword_t create_program_interface_memory(dword_t draw_method_pointer, dword_t flags) {
 dword_t program_interface_memory = calloc(256+256*10);
 dword_t *program_interface = (dword_t *) (program_interface_memory);
 program_interface[PROGRAM_INTERFACE_DRAW_METHOD_POINTER] = draw_method_pointer;
 program_interface[PROGRAM_INTERFACE_FLAGS] = flags;
 return program_interface_memory;
}

dword_t get_program_value(dword_t value_offset) {
 dword_t *program_interface_memory = (dword_t *) (program_interface_memory_pointer);
 return program_interface_memory[value_offset];
}

void set_program_value(dword_t value_offset, dword_t value) {
 dword_t *program_interface_memory = (dword_t *) (program_interface_memory_pointer);
 program_interface_memory[value_offset] = value;
}

dword_t get_position_of_file_memory(void) {
 dword_t *program_interface_memory = (dword_t *) (program_interface_memory_pointer);
 return (program_interface_memory_pointer+program_interface_memory[PROGRAM_INTERFACE_SELECTED_FILE]*256);
}

dword_t get_specific_file_value(dword_t file_number, dword_t value_offset) {
 dword_t *program_interface_memory = (dword_t *) (program_interface_memory_pointer);
 program_interface_memory = (dword_t *) (program_interface_memory_pointer+file_number*256);
 return program_interface_memory[value_offset];
}

dword_t get_file_value(dword_t value_offset) {
 dword_t *program_interface_memory = (dword_t *) (program_interface_memory_pointer);
 program_interface_memory = (dword_t *) (program_interface_memory_pointer+program_interface_memory[PROGRAM_INTERFACE_SELECTED_FILE]*256);
 return program_interface_memory[value_offset];
}

void set_file_value(dword_t value_offset, dword_t value) {
 dword_t *program_interface_memory = (dword_t *) (program_interface_memory_pointer);
 program_interface_memory = (dword_t *) (program_interface_memory_pointer+program_interface_memory[PROGRAM_INTERFACE_SELECTED_FILE]*256);
 program_interface_memory[value_offset] = value;
}

word_t get_char_of_file_name(dword_t value_offset) {
 dword_t *program_interface_memory = (dword_t *) (program_interface_memory_pointer);
 program_interface_memory = (dword_t *) (program_interface_memory_pointer+program_interface_memory[PROGRAM_INTERFACE_SELECTED_FILE]*256+PROGRAM_INTERFACE_FILE_NAME*4);
 word_t *program_interface_file_name = (word_t *) ((dword_t)program_interface_memory);
 return program_interface_file_name[value_offset];
}

void set_char_of_file_name(dword_t value_offset, dword_t value) {
 dword_t *program_interface_memory = (dword_t *) (program_interface_memory_pointer);
 program_interface_memory = (dword_t *) (program_interface_memory_pointer+program_interface_memory[PROGRAM_INTERFACE_SELECTED_FILE]*256+PROGRAM_INTERFACE_FILE_NAME*4);
 word_t *program_interface_file_name = (word_t *) ((dword_t)program_interface_memory);
 program_interface_file_name[value_offset] = value;
}

void set_file_name_from_file_dialog(void) {
 word_t *file_name = file_dialog_file_descriptor->name;

 for(dword_t i=0; i<20; i++) {
  set_char_of_file_name(i, file_name[i]);
  if(file_name[i]==0) {
   break;
  }
 }
}

void add_file(word_t *file_name, byte_t device_type, byte_t device_number, byte_t partition, dword_t file_size, dword_t file_disk_pointer) {
 if(get_program_value(PROGRAM_INTERFACE_NUMBER_OF_FILES)>=10) {
  return;
 }

 set_program_value(PROGRAM_INTERFACE_NUMBER_OF_FILES, (get_program_value(PROGRAM_INTERFACE_NUMBER_OF_FILES)+1));
 set_program_value(PROGRAM_INTERFACE_SELECTED_FILE, get_program_value(PROGRAM_INTERFACE_NUMBER_OF_FILES));

 set_file_value(PROGRAM_INTERFACE_FILE_FLAGS, PROGRAM_INTERFACE_FILE_FLAG_SAVED); //file is saved
 set_file_value(PROGRAM_INTERFACE_FILE_DEVICE_TYPE, device_type);
 set_file_value(PROGRAM_INTERFACE_FILE_DEVICE_NUMBER, device_number);
 set_file_value(PROGRAM_INTERFACE_FILE_PARTITION, partition);
 set_file_value(PROGRAM_INTERFACE_FILE_SIZE, file_size);
 set_file_value(PROGRAM_INTERFACE_FILE_DISK_POINTER, file_disk_pointer);
 for(dword_t i=0; i<20; i++) {
  set_char_of_file_name(i, file_name[i]);
  if(file_name[i]==0) {
   break;
  }
 }

}

void change_file_name_byte_string(byte_t *name) {
 for(dword_t i=0; i<20; i++) {
  set_char_of_file_name(i, name[i]);
  if(name[i]==0) {
   break;
  }
 }
}

void change_file_name_word_string(word_t *name) {
 for(dword_t i=0; i<20; i++) {
  set_char_of_file_name(i, name[i]);
  if(name[i]==0) {
   break;
  }
 }
}

void remove_file(dword_t file_number) {
 if(file_number==10) {
  clear_memory(program_interface_memory_pointer+10*256, 256);
 }
 else {
  copy_memory(program_interface_memory_pointer+file_number*256+256, program_interface_memory_pointer+file_number*256, (10*256-file_number*256));
  clear_memory(program_interface_memory_pointer+10*256, 256);
 }

 set_program_value(PROGRAM_INTERFACE_NUMBER_OF_FILES, (get_program_value(PROGRAM_INTERFACE_NUMBER_OF_FILES)-1));
}