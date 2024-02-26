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
  if(*zip_end_of_central_directory_signature==ZIP_END_OF_CENTRAL_DIRECTORY_SIGNATURE && *zip_comment_length==i) {
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
   byte_t file_founded = STATUS_TRUE;

   for(dword_t j=0; j<file_name_length; j++) {
    if(file_name[j]!=searched_file_name[j]) {
     file_founded = STATUS_FALSE;
     break;
    }
   }

   if(file_founded==STATUS_TRUE) {
    return i; //this entry is file we are searching for
   }
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

 //read variables
 dword_t file_data_compressed_data_length = zip_file_entry[5];
 dword_t file_data_uncompressed_data_length = zip_file_entry[6];

 //go to Local file header
 zip_file_entry = (dword_t *) (((dword_t)zip_file_entry)+42);
 dword_t *zip_local_file_header = (dword_t *) (zip_file_memory+(*zip_file_entry));
 if(*zip_local_file_header!=ZIP_LOCAL_FILE_HEADER_SIGNATURE) {
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
   log("\nZIP: DEFLATE error");
   free(file_memory);
   return STATUS_ERROR;
  }
 }

 zip_extracted_file_size = file_data_uncompressed_data_length;
 return file_memory;
}

dword_t create_zip_file(dword_t number_of_files_inside, dword_t size_of_all_files) {
 new_zip_file_pointer = calloc((number_of_files_inside*sizeof(struct zip_local_file_header))+(number_of_files_inside*256)+(number_of_files_inside*sizeof(struct zip_central_directory_file_header))+(number_of_files_inside*256)+size_of_all_files+(sizeof(struct zip_end_of_central_directory)));
 new_zip_file_size = 0;
 new_zip_file_list_of_relative_offsets = (dword_t *) (calloc((number_of_files_inside+1)*4));
 new_zip_actual_processed_file_number = 0;
 new_zip_number_of_files = number_of_files_inside;
 new_zip_size_of_central_directory = 0;
 return new_zip_file_pointer;
}

void zip_add_file(byte_t *name, dword_t memory, dword_t size) {
 struct zip_local_file_header *local_file_header = (struct zip_local_file_header *) (new_zip_file_pointer);
 
 //create local file header
 local_file_header->signature = ZIP_LOCAL_FILE_HEADER_SIGNATURE;
 local_file_header->version_for_extracting = 0x0014;
 local_file_header->general_purpose_flag = 0;
 local_file_header->compression_method = ZIP_NO_COMPRESSION;
 local_file_header->file_last_modification_time = 0;
 local_file_header->file_last_modification_date = 0;
 local_file_header->crc32 = calculate_crc32_checksum((byte_t *)memory, size);
 local_file_header->uncompressed_size = size;
 local_file_header->compressed_size = size;
 local_file_header->file_name_length = get_number_of_chars_in_ascii_string(name);
 local_file_header->extra_field_length = 0;
 copy_memory((dword_t)name, new_zip_file_pointer+sizeof(struct zip_local_file_header), local_file_header->file_name_length);

 //add file content
 copy_memory(memory, new_zip_file_pointer+sizeof(struct zip_local_file_header)+local_file_header->file_name_length, size);

 //save relative offset
 new_zip_file_list_of_relative_offsets[new_zip_actual_processed_file_number+1] = (new_zip_file_list_of_relative_offsets[new_zip_actual_processed_file_number]+(sizeof(struct zip_local_file_header)+local_file_header->file_name_length+size));
 new_zip_actual_processed_file_number++;

 //move variables
 new_zip_file_pointer += (sizeof(struct zip_local_file_header)+local_file_header->file_name_length+size);
 new_zip_file_size += (sizeof(struct zip_local_file_header)+local_file_header->file_name_length+size);
}

void zip_start_central_directory(void) {
 new_zip_actual_processed_file_number = 0;
}

void zip_add_central_directory_file_header(byte_t *name, dword_t memory, dword_t size) {
 struct zip_central_directory_file_header *central_directory_file_header = (struct zip_central_directory_file_header *) (new_zip_file_pointer);

 //create central directory file header
 central_directory_file_header->signature = ZIP_CENTRAL_DIRECTORY_FILE_HEADER_SIGNATURE;
 central_directory_file_header->version_made_by = 0x0014;
 central_directory_file_header->version_for_extracting = 0x0014;
 central_directory_file_header->general_purpose_flag = 0;
 central_directory_file_header->compression_method = ZIP_NO_COMPRESSION;
 central_directory_file_header->file_last_modification_time = 0;
 central_directory_file_header->file_last_modification_date = 0;
 central_directory_file_header->crc32 = calculate_crc32_checksum((byte_t *)memory, size);
 central_directory_file_header->uncompressed_size = size;
 central_directory_file_header->compressed_size = size;
 central_directory_file_header->file_name_length = get_number_of_chars_in_ascii_string(name);
 central_directory_file_header->extra_field_length = 0;
 central_directory_file_header->file_comment_length = 0;
 central_directory_file_header->disk_number_of_file = 0;
 central_directory_file_header->internal_file_attributes = 0;
 central_directory_file_header->external_file_attributes = 0;
 central_directory_file_header->relative_offset_to_local_file_header = new_zip_file_list_of_relative_offsets[new_zip_actual_processed_file_number];
 copy_memory((dword_t)name, new_zip_file_pointer+sizeof(struct zip_central_directory_file_header), central_directory_file_header->file_name_length);

 //move variables
 new_zip_file_pointer += (sizeof(struct zip_central_directory_file_header)+central_directory_file_header->file_name_length);
 new_zip_size_of_central_directory += (sizeof(struct zip_central_directory_file_header)+central_directory_file_header->file_name_length);
 new_zip_file_size += (sizeof(struct zip_central_directory_file_header)+central_directory_file_header->file_name_length);

 //move to next file
 new_zip_actual_processed_file_number++;
}

void finish_zip_file(void) {
 struct zip_end_of_central_directory *end_of_central_directory = (struct zip_end_of_central_directory *) (new_zip_file_pointer);
 
 //create end of central directory 
 end_of_central_directory->signature = ZIP_END_OF_CENTRAL_DIRECTORY_SIGNATURE;
 end_of_central_directory->number_of_this_disk = 0;
 end_of_central_directory->disk_of_central_directory = 0;
 end_of_central_directory->number_of_central_directory_records_on_this_disk = new_zip_number_of_files;
 end_of_central_directory->total_number_of_central_directory_records = new_zip_number_of_files;
 end_of_central_directory->size_of_central_directory = new_zip_size_of_central_directory;
 end_of_central_directory->relative_offset_to_central_directory = new_zip_file_list_of_relative_offsets[new_zip_actual_processed_file_number];
 end_of_central_directory->comment_length = 0;

 //move variables
 new_zip_file_size += sizeof(struct zip_end_of_central_directory);

 //free allocated memory
 free((dword_t)new_zip_file_list_of_relative_offsets);
}