//BleskOS

/*
* MIT License
* Copyright (c) 2023-2024 Vendelín Slezák
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

struct byte_stream_descriptor_t *create_descriptor_of_file_sectors(word_t size_of_one_sector) {
 //create stream
 struct byte_stream_descriptor_t *descriptor_of_file_sectors_stream = create_byte_stream(BYTE_STREAM_100_KB_BLOCK);

 //add header
 struct descriptor_of_file_sectors_t descriptor_of_file_sectors_header;
 clear_memory((dword_t)(&descriptor_of_file_sectors_header), sizeof(struct descriptor_of_file_sectors_t));
 descriptor_of_file_sectors_header.size_of_one_sector = size_of_one_sector;
 add_bytes_to_byte_stream(descriptor_of_file_sectors_stream, (byte_t *)(&descriptor_of_file_sectors_header), sizeof(struct descriptor_of_file_sectors_t));

 return descriptor_of_file_sectors_stream;
}

void add_sectors_to_descriptor_of_file_sectors(struct byte_stream_descriptor_t *descriptor_of_file_sectors_stream, dword_t first_sector, dword_t number_of_sectors) {
 struct descriptor_of_file_sectors_t *descriptor_of_file_sectors = (struct descriptor_of_file_sectors_t *) (descriptor_of_file_sectors_stream->start_of_allocated_memory);
 
 if(descriptor_of_file_sectors->number_of_runs==0) {
  //add first run
  add_dword_to_byte_stream(descriptor_of_file_sectors_stream, first_sector);
  add_dword_to_byte_stream(descriptor_of_file_sectors_stream, number_of_sectors);

  //update header
  descriptor_of_file_sectors->number_of_sectors = number_of_sectors;
  descriptor_of_file_sectors->number_of_runs = 1;
 }
 else {
  //test if we can add these sectors to last run
  struct file_sectors_run_t *last_run = (struct file_sectors_run_t *) (((dword_t)descriptor_of_file_sectors_stream->memory_pointer)-sizeof(struct file_sectors_run_t));
  if((last_run->first_sector_of_run+last_run->number_of_sectors_in_run)==first_sector) {
   //extend last run
   last_run->number_of_sectors_in_run += number_of_sectors;

   //update header
   descriptor_of_file_sectors->number_of_sectors += number_of_sectors;
  }
  else {
   //add new run
   add_dword_to_byte_stream(descriptor_of_file_sectors_stream, first_sector);
   add_dword_to_byte_stream(descriptor_of_file_sectors_stream, number_of_sectors);

   //update header
   descriptor_of_file_sectors->number_of_sectors += number_of_sectors;
   descriptor_of_file_sectors->number_of_runs++;
  }
 }
}

byte_t descriptor_of_file_sector_read_sectors(struct descriptor_of_file_sectors_t *descriptor_of_file_sectors, dword_t first_sector, dword_t number_of_sectors, byte_t *memory) {
 if(descriptor_of_file_sectors->size_of_one_sector==2352) {
  return read_audio_cd(first_sector, number_of_sectors, (dword_t)memory);
 }
 else {
  return read_storage_medium(first_sector, number_of_sectors, (dword_t)memory);
 }
}

byte_t *read_whole_descriptor_of_file_sector(struct byte_stream_descriptor_t *descriptor_of_file_sectors_stream) {
 struct descriptor_of_file_sectors_t *descriptor_of_file_sectors = (struct descriptor_of_file_sectors_t *) (descriptor_of_file_sectors_stream->start_of_allocated_memory);
 
 //allocate memory for file
 byte_t *file_memory = (byte_t *) calloc(descriptor_of_file_sectors->number_of_sectors*descriptor_of_file_sectors->size_of_one_sector);
 byte_t *file_memory_pointer = file_memory;

 //set reading speed according to medium
 byte_t number_of_sectors_per_one_read = 0;
 if(storage_medium==MEDIUM_HARD_DISK) {
  number_of_sectors_per_one_read = 128;
 }
 else if(storage_medium==MEDIUM_OPTICAL_DRIVE) {
  if(descriptor_of_file_sectors->size_of_one_sector==2352) {
   number_of_sectors_per_one_read = 27;
  }
  else {
   number_of_sectors_per_one_read = 16;
  }
 }
 else if(storage_medium==MEDIUM_USB_MSD) {
  number_of_sectors_per_one_read = 128;
 }
 else {
  number_of_sectors_per_one_read = 1;
 }

 //set variables
 keyboard_code_of_pressed_key = 0;
 dofs_full_task_number_of_bytes = (descriptor_of_file_sectors->number_of_sectors*descriptor_of_file_sectors->size_of_one_sector);
 dofs_task_completed_number_of_bytes = 0;

 //read all runs
 for(dword_t run=0; run<descriptor_of_file_sectors->number_of_runs; run++) {
  //check if user cancelled this request
  if(dofs_can_user_cancel_action==USER_CAN_CANCEL_THIS_DOFS_ACTION && keyboard_code_of_pressed_key==KEY_ESC) {
   free((dword_t)file_memory);
   return STATUS_ERROR;
  }

  //we can read run on one request
  if(descriptor_of_file_sectors->runs[run].number_of_sectors_in_run<=number_of_sectors_per_one_read) {
   //read data
   if(descriptor_of_file_sector_read_sectors(descriptor_of_file_sectors, descriptor_of_file_sectors->runs[run].first_sector_of_run, descriptor_of_file_sectors->runs[run].number_of_sectors_in_run, file_memory_pointer)==STATUS_ERROR) {
    free((dword_t)file_memory);
    return STATUS_ERROR;
   }

   //move variables
   file_memory_pointer = (byte_t *) (((dword_t)file_memory_pointer)+(descriptor_of_file_sectors->runs[run].number_of_sectors_in_run*descriptor_of_file_sectors->size_of_one_sector));
   dofs_task_completed_number_of_bytes += (descriptor_of_file_sectors->runs[run].number_of_sectors_in_run*descriptor_of_file_sectors->size_of_one_sector);
  }
  else { //we need to read run on multiple requests
   //read basic values
   dword_t first_sector_of_run = descriptor_of_file_sectors->runs[run].first_sector_of_run;
   dword_t sectors_of_run = descriptor_of_file_sectors->runs[run].number_of_sectors_in_run;

   //read run
   while(sectors_of_run>0) {
    //check if user cancelled this request
    if(dofs_can_user_cancel_action==USER_CAN_CANCEL_THIS_DOFS_ACTION && keyboard_code_of_pressed_key==KEY_ESC) {
     free((dword_t)file_memory);
     return STATUS_ERROR;
    }

    if(sectors_of_run>number_of_sectors_per_one_read) {
     //read data
     if(descriptor_of_file_sector_read_sectors(descriptor_of_file_sectors, first_sector_of_run, number_of_sectors_per_one_read, file_memory_pointer)==STATUS_ERROR) {
      free((dword_t)file_memory);
      return STATUS_ERROR;
     }
     
     //move variables
     file_memory_pointer = (byte_t *) (((dword_t)file_memory_pointer)+(number_of_sectors_per_one_read*descriptor_of_file_sectors->size_of_one_sector));
     first_sector_of_run += number_of_sectors_per_one_read;
     sectors_of_run -= number_of_sectors_per_one_read;
     dofs_task_completed_number_of_bytes += (number_of_sectors_per_one_read*descriptor_of_file_sectors->size_of_one_sector);
    }
    else {
     //check if user cancelled this request
     if(dofs_can_user_cancel_action==USER_CAN_CANCEL_THIS_DOFS_ACTION && keyboard_code_of_pressed_key==KEY_ESC) {
      free((dword_t)file_memory);
      return STATUS_ERROR;
     }

     //read last data of this run
     if(descriptor_of_file_sector_read_sectors(descriptor_of_file_sectors, first_sector_of_run, sectors_of_run, file_memory_pointer)==STATUS_ERROR) {
      free((dword_t)file_memory);
      return STATUS_ERROR;
     }

     //move variables
     file_memory_pointer = (byte_t *) (((dword_t)file_memory_pointer)+(sectors_of_run*descriptor_of_file_sectors->size_of_one_sector));
     dofs_task_completed_number_of_bytes += (sectors_of_run*descriptor_of_file_sectors->size_of_one_sector);

     //we readed everything from this run
     break;
    }
   }
  }
 }

 return file_memory;
}

byte_t write_whole_descriptor_of_file_sector(struct byte_stream_descriptor_t *descriptor_of_file_sectors_stream, byte_t *file_memory) {
 struct descriptor_of_file_sectors_t *descriptor_of_file_sectors = (struct descriptor_of_file_sectors_t *) (descriptor_of_file_sectors_stream->start_of_allocated_memory);

 //set writing speed according to medium
 byte_t number_of_sectors_per_one_write = 0;
 if(storage_medium==MEDIUM_HARD_DISK) {
  number_of_sectors_per_one_write = 128;
 }
 else if(storage_medium==MEDIUM_USB_MSD) {
  number_of_sectors_per_one_write = 128;
 }
 else {
  number_of_sectors_per_one_write = 1;
 }

 //set variables
 keyboard_code_of_pressed_key = 0;
 dofs_full_task_number_of_bytes = (descriptor_of_file_sectors->number_of_sectors*descriptor_of_file_sectors->size_of_one_sector);
 dofs_task_completed_number_of_bytes = 0;

 //write all runs
 for(dword_t run=0; run<descriptor_of_file_sectors->number_of_runs; run++) {
  //check if user cancelled this request
  if(dofs_can_user_cancel_action==USER_CAN_CANCEL_THIS_DOFS_ACTION && keyboard_code_of_pressed_key==KEY_ESC) {
   return STATUS_ERROR;
  }

  //we can write run on one request
  if(descriptor_of_file_sectors->runs[run].number_of_sectors_in_run<=number_of_sectors_per_one_write) {
   //write data
   if(write_storage_medium(descriptor_of_file_sectors->runs[run].first_sector_of_run, descriptor_of_file_sectors->runs[run].number_of_sectors_in_run, (dword_t)file_memory)==STATUS_ERROR) {
    return STATUS_ERROR;
   }

   //move variables
   file_memory = (byte_t *) (((dword_t)file_memory)+(descriptor_of_file_sectors->runs[run].number_of_sectors_in_run*descriptor_of_file_sectors->size_of_one_sector));
   dofs_task_completed_number_of_bytes += (descriptor_of_file_sectors->runs[run].number_of_sectors_in_run*descriptor_of_file_sectors->size_of_one_sector);
  }
  else { //we need to write run on multiple requests
   //read basic values
   dword_t first_sector_of_run = descriptor_of_file_sectors->runs[run].first_sector_of_run;
   dword_t sectors_of_run = descriptor_of_file_sectors->runs[run].number_of_sectors_in_run;

   //write run
   while(sectors_of_run>0) {
    //check if user cancelled this request
    if(dofs_can_user_cancel_action==USER_CAN_CANCEL_THIS_DOFS_ACTION && keyboard_code_of_pressed_key==KEY_ESC) {
     return STATUS_ERROR;
    }

    if(sectors_of_run>number_of_sectors_per_one_write) {
     //write data
     if(write_storage_medium(first_sector_of_run, number_of_sectors_per_one_write, (dword_t)file_memory)==STATUS_ERROR) {
      return STATUS_ERROR;
     }
     
     //move variables
     file_memory = (byte_t *) (((dword_t)file_memory)+(number_of_sectors_per_one_write*descriptor_of_file_sectors->size_of_one_sector));
     first_sector_of_run += number_of_sectors_per_one_write;
     sectors_of_run -= number_of_sectors_per_one_write;
     dofs_task_completed_number_of_bytes += (number_of_sectors_per_one_write*descriptor_of_file_sectors->size_of_one_sector);
    }
    else {
     //check if user cancelled this request
     if(dofs_can_user_cancel_action==USER_CAN_CANCEL_THIS_DOFS_ACTION && keyboard_code_of_pressed_key==KEY_ESC) {
      return STATUS_ERROR;
     }

     //write last data of this run
     if(write_storage_medium(first_sector_of_run, sectors_of_run, (dword_t)file_memory)==STATUS_ERROR) {
      return STATUS_ERROR;
     }

     //move variables
     file_memory = (byte_t *) (((dword_t)file_memory)+(sectors_of_run*descriptor_of_file_sectors->size_of_one_sector));
     dofs_task_completed_number_of_bytes += (sectors_of_run*descriptor_of_file_sectors->size_of_one_sector);

     //we wrote everything from this run
     break;
    }
   }
  }
 }

 return STATUS_GOOD;
}

void dofs_show_progress_in_reading(void) {
 draw_message_window(430, 15+23+20+23+15);
 program_element_layout_initalize_for_window(430, 15+23+20+23+15);
 program_element_layout_add_border_to_area(FIRST_AREA, 15);

 //add first label
 if(dofs_task_completed_number_of_bytes==0) {
  //print label to center of message window
  program_element_layout_calculate_floating_element_position(FIRST_AREA, 400, 23+20+23, ELEMENT_MIDDLE_ALIGNMENT, ELEMENT_CENTER_ALIGNMENT, (sizeof("Reading informations about file data...")-1)*8, 8);
  print("Reading informations about file data...", element_x, element_y, BLACK);
 }
 else {
  //print label at top
  add_label(FIRST_AREA, ELEMENT_MIDDLE_ALIGNMENT, "Reading file data...");
  program_element_move_horizontally(FIRST_AREA, 15);

  //draw square
  program_element_layout_calculate_element_position(FIRST_AREA, ELEMENT_LEFT_ALIGNMENT, program_element_layout_areas_info[FIRST_AREA].width, 20);
  if(dofs_full_task_number_of_bytes>=1024) {
   draw_full_square(element_x, element_y, element_width*(dofs_task_completed_number_of_bytes/1024)/(dofs_full_task_number_of_bytes/1024), element_height, 0x000CFF);
  }
  draw_empty_square(element_x, element_y, element_width, element_height, BLACK);

  //print number of processed MB on square
  byte_t output_string[10];
  clear_memory((dword_t)output_string, 10);
  convert_number_to_byte_string((dofs_task_completed_number_of_bytes/1024/1024), (dword_t)output_string);
  output_string[get_number_of_chars_in_ascii_string(output_string)] = '.';
  convert_number_to_byte_string((dofs_task_completed_number_of_bytes/1024/103%10), (dword_t)output_string+get_number_of_chars_in_ascii_string(output_string));
  output_string[get_number_of_chars_in_ascii_string(output_string)] = ' ';
  output_string[get_number_of_chars_in_ascii_string(output_string)] = 'M';
  output_string[get_number_of_chars_in_ascii_string(output_string)] = 'B';
  program_element_layout_calculate_floating_element_position(FIRST_AREA, 400, 20, ELEMENT_MIDDLE_ALIGNMENT, ELEMENT_CENTER_ALIGNMENT, get_number_of_chars_in_ascii_string(output_string)*8, 8);
  print(output_string, element_x, element_y, BLACK);
  program_element_move_horizontally(FIRST_AREA, 15);

  //print label at bottom
  add_label(FIRST_AREA, ELEMENT_MIDDLE_ALIGNMENT, "You can cancel action by holding ESC");
 }

 //show message window on screen
 redraw_message_window(430, 15+23+20+23+15);
}

void dofs_show_progress_in_writing(void) {
 draw_message_window(430, 15+23+20+23+15);
 program_element_layout_initalize_for_window(430, 15+23+20+23+15);
 program_element_layout_add_border_to_area(FIRST_AREA, 15);

 //add first label
 if(dofs_task_completed_number_of_bytes==0) {
  //print label to center of message window
  program_element_layout_calculate_floating_element_position(FIRST_AREA, 400, 23+20+23, ELEMENT_MIDDLE_ALIGNMENT, ELEMENT_CENTER_ALIGNMENT, (sizeof("Finding free space on medium...")-1)*8, 8);
  print("Finding free space on medium...", element_x, element_y, BLACK);
 }
 else if(dofs_task_completed_number_of_bytes<dofs_full_task_number_of_bytes) {
  //print label at top
  add_label(FIRST_AREA, ELEMENT_MIDDLE_ALIGNMENT, "Writing file data...");
  program_element_move_horizontally(FIRST_AREA, 15);

  //draw square
  program_element_layout_calculate_element_position(FIRST_AREA, ELEMENT_LEFT_ALIGNMENT, program_element_layout_areas_info[FIRST_AREA].width, 20);
  if(dofs_full_task_number_of_bytes>=1024) {
   draw_full_square(element_x, element_y, element_width*(dofs_task_completed_number_of_bytes/1024)/(dofs_full_task_number_of_bytes/1024), element_height, 0x000CFF);
  }
  draw_empty_square(element_x, element_y, element_width, element_height, BLACK);

  //print number of processed MB on square
  byte_t output_string[10];
  clear_memory((dword_t)output_string, 10);
  convert_number_to_byte_string((dofs_task_completed_number_of_bytes/1024/1024), (dword_t)output_string);
  output_string[get_number_of_chars_in_ascii_string(output_string)] = '.';
  convert_number_to_byte_string((dofs_task_completed_number_of_bytes/1024/103%10), (dword_t)output_string+get_number_of_chars_in_ascii_string(output_string));
  output_string[get_number_of_chars_in_ascii_string(output_string)] = ' ';
  output_string[get_number_of_chars_in_ascii_string(output_string)] = 'M';
  output_string[get_number_of_chars_in_ascii_string(output_string)] = 'B';
  program_element_layout_calculate_floating_element_position(FIRST_AREA, 400, 20, ELEMENT_MIDDLE_ALIGNMENT, ELEMENT_CENTER_ALIGNMENT, get_number_of_chars_in_ascii_string(output_string)*8, 8);
  print(output_string, element_x, element_y, BLACK);
  program_element_move_horizontally(FIRST_AREA, 15);

  //print label at bottom
  add_label(FIRST_AREA, ELEMENT_MIDDLE_ALIGNMENT, "You can cancel action by holding ESC");
 }
 else {
  //print label to center of message window
  program_element_layout_calculate_floating_element_position(FIRST_AREA, 400, 23+20+23, ELEMENT_MIDDLE_ALIGNMENT, ELEMENT_CENTER_ALIGNMENT, (sizeof("Writing informations about file data...")-1)*8, 8);
  print("Writing informations about file data...", element_x, element_y, BLACK);
 }

 //show message window on screen
 redraw_message_window(430, 15+23+20+23+15);
}