//BleskOS

/*
* MIT License
* Copyright (c) 2023-2024 Vendelín Slezák
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

dword_t compare_file_extension(dword_t folder_mem, dword_t entry_number, dword_t extension_memory, dword_t extension_length) {
 word_t *extension = (word_t *) extension_memory;
 
 //find extension
 dword_t file_name_length = get_number_of_chars_in_unicode_string((word_t *)(folder_mem+entry_number*256+32));
 word_t *folder16 = (word_t *) (folder_mem+entry_number*256+32+file_name_length*2); //search from end of file name to find .
 for(int i=0; i<file_name_length; i++, folder16--) {
  if(*folder16=='.') {
   folder16++;
   
   //compare
   for(int j=0; j<extension_length; j++) {
    if(get_small_char_value(folder16[j])!=extension[j]) {
     return STATUS_FALSE;
    }
   }
   
   return STATUS_TRUE;
  }
 }
 
 return STATUS_FALSE; //extension not founded
}

void get_file_extension(dword_t folder_mem, dword_t entry_number) {
 word_t *folder16 = (word_t *) (folder_mem+entry_number*256+32);
 
 for(int i=0; i<10; i++) {
  file_extension[i]=0;
 }
 
 //find extension
 for(int i=0; i<100; i++, folder16++) {
  if(*folder16=='.') {
   folder16++;
   
   //compare
   for(int j=0; j<10; j++) {
    if(folder16[j]<0x5B && folder16[j]>0x40) {
     file_extension[j]=(folder16[j]+0x20); //convert to small chars
    }
    else {
     file_extension[j]=folder16[j];
    }
    if(file_extension[j]==0) {
     return;
    }
   }
  }
 }
}

word_t get_char_of_file_from_folder_entry_name(dword_t folder_mem, dword_t entry_number, dword_t char_offset) {
 word_t *folder16 = (word_t *) (folder_mem+entry_number*256+32+(char_offset*2));
 return *folder16;
}

void set_char_of_file_from_folder_entry_name(dword_t folder_mem, dword_t entry_number, dword_t char_offset, word_t char_value) {
 word_t *folder16 = (word_t *) (folder_mem+entry_number*256+32+(char_offset*2));
 *folder16 = char_value;
}

byte_t is_loaded_file_extension(byte_t *extension) {
 return are_equal_b_string_b_string((byte_t *)file_dialog_file_extension, extension);
}

dword_t get_file_attribute(dword_t folder_mem, dword_t entry_number) {
 byte_t *folder8 = (byte_t *) (folder_mem+entry_number*256);
 return folder8[11];
}

dword_t get_file_starting_entry(dword_t folder_mem, dword_t entry_number) {
 word_t *folder16 = (word_t *) (folder_mem+entry_number*256);
 return (folder16[10]<<16 | folder16[13]);
}

dword_t get_file_size(dword_t folder_mem, dword_t entry_number) {
 dword_t *folder32 = (dword_t *) (folder_mem+entry_number*256);
 return folder32[7];
}

void set_file_entry_size(dword_t folder_mem, dword_t entry_number, dword_t size) {
 dword_t *folder32 = (dword_t *) (folder_mem+entry_number*256);
 folder32[7]=size;
}