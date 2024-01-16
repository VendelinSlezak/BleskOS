//BleskOS

/*
* MIT License
* Copyright (c) 2023-2024 Vendelín Slezák
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

byte_t is_this_zip(dword_t zip_file_memory, dword_t zip_file_size) {
 dword_t *zip_end_of_central_directory_signature = (dword_t *) (zip_file_memory+zip_file_size-22);
 word_t *zip_comment_length = (word_t *) (zip_file_memory+zip_file_size-2);

 for(dword_t i=0; i<0xFFFF; i++) {
  if(*zip_end_of_central_directory_signature==0x06054B50 && *zip_comment_length==i) {
   //TODO: test of other signatures and too big values

   //check if this is ZIP64
   if(zip_end_of_central_directory_signature[4]==0xFFFFFFFF) {
    log("\nZIP: ZIP64 files are unsupported"); //TODO: support for ZIP64 files
    return STATUS_FALSE;
   }

   //save length of comment so other methods will not waste time for findid End Of Central Directory
   word_t *zip_edit_end_of_file = (word_t *) (zip_file_memory+zip_file_size-2);
   *zip_edit_end_of_file = i; 
   
   return STATUS_TRUE;
  }

  zip_end_of_central_directory_signature = (dword_t *) (((dword_t)zip_end_of_central_directory_signature)-1);
  zip_comment_length = (word_t *) (((dword_t)zip_comment_length)-1);
 }

 return STATUS_FALSE;
}

dword_t search_for_file_in_zip(dword_t zip_file_memory, dword_t zip_file_size, byte_t *searched_file_name) {
 word_t *zip_comment_length = (word_t *) (zip_file_memory+zip_file_size-2);
 dword_t *zip_end_of_central_directory = (dword_t *) (zip_file_memory+zip_file_size-(*zip_comment_length)-22);
 dword_t zip_central_directory_start_mem = (zip_file_memory+zip_end_of_central_directory[4]);
 dword_t zip_num_of_files = (zip_end_of_central_directory[2]>>16);
 dword_t *zip_file_entry = (dword_t *) (zip_central_directory_start_mem);
 dword_t searched_file_name_length = get_number_of_chars_in_ascii_string(searched_file_name);

 //search if there is entry with same name
 for(dword_t i=0; i<zip_num_of_files; i++) {
  byte_t *file_name = (byte_t *) (((dword_t)zip_file_entry)+46);
  dword_t file_name_length = (zip_file_entry[7] & 0xFFFF), extra_field_length = (zip_file_entry[7]>>16), file_comment_length = (zip_file_entry[8] & 0xFFFF);
  
  //compare file names
  if(searched_file_name_length==file_name_length) {
   for(dword_t j=0; j<file_name_length; j++) {
    if(file_name[j]!=searched_file_name[j]) {
     break;
    }
   }

   return i; //this entry is searched file
  }

  //next entry
  zip_file_entry = (dword_t *) (((dword_t)zip_file_entry)+46+file_name_length+extra_field_length+file_comment_length);
 }

 return ZIP_FILE_NOT_FOUNDED;
}

dword_t zip_extract_file(dword_t zip_file_memory, dword_t zip_file_size, dword_t entry_number) {
 word_t *zip_comment_length = (word_t *) (zip_file_memory+zip_file_size-2);
 dword_t *zip_end_of_central_directory = (dword_t *) (zip_file_memory+zip_file_size-(*zip_comment_length)-22);
 dword_t *zip_file_entry = (dword_t *) (zip_file_memory+zip_end_of_central_directory[4]);

 //skip entries to found our entry
 for(dword_t i=0; i<entry_number; i++) {
  word_t file_name_length = (zip_file_entry[7] & 0xFFFF), extra_field_length = (zip_file_entry[7]>>16), file_comment_length = (zip_file_entry[8] & 0xFFFF);
  zip_file_entry = (dword_t *) (((dword_t)zip_file_entry)+46+file_name_length+extra_field_length+file_comment_length);
 }

 //read variabiles
 dword_t file_data_compressed_data_length = zip_file_entry[5];
 dword_t file_data_uncompressed_data_length = zip_file_entry[6];

 //go to Local file header
 zip_file_entry = (dword_t *) (((dword_t)zip_file_entry)+42);
 dword_t *zip_local_file_header = (dword_t *) (zip_file_memory+(*zip_file_entry));
 if(*zip_local_file_header!=0x04034B50) {
  log("\nZIP: invalid signature in local file header");
  return STATUS_ERROR;
 }
 word_t compression_method = (zip_local_file_header[2] & 0xFFFF);
 if(!(compression_method==ZIP_NO_COMPRESSION || compression_method==ZIP_COMPRESSION_DEFLATE)) {
  log("\nZIP: unsupported compression method ");
  log_var(compression_method);
  return STATUS_ERROR;
 }
 word_t file_name_length = (zip_local_file_header[6]>>16), extra_field_length = (zip_local_file_header[7] & 0xFFFF);
 dword_t file_data_compressed_data_mem = (((dword_t)zip_local_file_header)+30+file_name_length+extra_field_length);

 //extract file
 dword_t file_memory = malloc(file_data_uncompressed_data_length);
 if(compression_method==ZIP_NO_COMPRESSION) {
  copy_memory(file_data_compressed_data_mem, file_memory, file_data_uncompressed_data_length);
 }
 else if(compression_method==ZIP_COMPRESSION_DEFLATE) {
  if(decode_deflate(file_data_compressed_data_mem, file_data_compressed_data_length, file_memory, file_data_uncompressed_data_length)==STATUS_ERROR) {
   free(file_memory);
   return STATUS_ERROR;
  }
 }

 return file_memory;
}