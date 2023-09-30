//BleskOS

/*
* MIT License
* Copyright (c) 2023 Vendelín Slezák
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

void file_dialog_draw_background(void) {
 clear_screen(0x884E10); //brown
 draw_full_square(0, 0, graphic_screen_x, 24, 0x60BD00); //green
 draw_full_square(0, graphic_screen_y-24, graphic_screen_x, 24, 0x60BD00); //green
}

void file_dialog_draw_devices(void) {
 byte_t *mem = (byte_t *) file_dialog_devices_mem;
 
 //print label
 print("Devices:", 8, 32, BLACK);
 
 //clear background
 draw_full_square(0, 48, 150, graphic_screen_y-72, 0x884E10);

 //selected device
 draw_full_square(0, 48+file_dialog_selected_device*25, 150, 24, 0xFF0000);
 
 for(int i=0, draw_line=48; i<FD_NUMBER_OF_DEVICE_ENTRIES; i++, mem+=FD_DEVICE_ENTRY_LENGTH) {
  if(mem[0]==FD_NO_DEVICE) {
   return;
  }
 
  //print label
  print((byte_t *)((dword_t)mem+20), 8, draw_line+8, WHITE);
  draw_line+=25;
 }
}

void file_dialog_draw_files(void) {
 byte_t *folder8 = (byte_t *) (file_dialog_folder_mem+(file_dialog_folder_first_displayed_file_num*256));
 word_t *folder16 = (word_t *) (file_dialog_folder_mem+(file_dialog_folder_first_displayed_file_num*256));
 dword_t *folder32 = (dword_t *) (file_dialog_folder_mem+(file_dialog_folder_first_displayed_file_num*256));
 
 word_t file_name_print[9];
 
 //clear
 draw_full_square(158, 47, (graphic_screen_x-158), (graphic_screen_y-48-32), 0x884E10);
 
 //if no folder is loaded, return
 if(file_dialog_folder_mem==0) {
  return;
 }
 
 if(file_dialog_type_of_displaying_files==FD_DISPLAY_FILES_LIST) {
  if(file_dialog_highlighted_file!=NO_FILE_SELECTED) {
   draw_full_square(158, ((48+file_dialog_highlighted_file*10)-1), (graphic_screen_x-158-20), 10, 0xFF0000);
  }
  
  //TODO: add scrollbar

  print("Name", 158, 32, BLACK);
  print("Size", (graphic_screen_x-12-72-80-8), 32, BLACK);
  print("Date", (graphic_screen_x-12-80-8), 32, BLACK);

  for(int i=0, draw_line = 48; i<file_dialog_list_num_of_files_on_screen; i++) {  
   if(folder32[(64*i)]==0) {
    return;
   }
   
   //print name
   for(int j=0, draw_column = 158; j<file_dialog_list_num_of_chars_of_name; j++) {
    if(folder16[(128*i)+16+j]==0) {
     break;
    }
    draw_char(folder16[(128*i)+16+j], draw_column, draw_line, BLACK);
    draw_column += 8;
   }
   
   //print size
   if(folder8[(256*i)+11]!=0x10) { //do not write size of folders
    if(folder32[(64*i)+7]<1024) {
     print_var(folder32[(64*i)+7], (graphic_screen_x-12-72-80-8), draw_line, BLACK);
     print("B", (graphic_screen_x-12-24-80-8), draw_line, BLACK);
    }
    else if(folder32[(64*i)+7]<1024*1024) {
     print_var((folder32[(64*i)+7]/1024), (graphic_screen_x-12-72-80-8), draw_line, BLACK);
     print("KB", (graphic_screen_x-12-24-80-8), draw_line, BLACK);
    }
    else if(folder32[(64*i)+7]<1024*1024*1024) {
     print_var((folder32[(64*i)+7]/1024/1024), (graphic_screen_x-12-72-80-8), draw_line, BLACK);
     print("MB", (graphic_screen_x-12-24-80-8), draw_line, BLACK);
    }
   }
   
   //print date
   print_var((folder16[(128*i)+8] >> 9)+1980, (graphic_screen_x-12-80-8), draw_line, BLACK);
   draw_char('/', (graphic_screen_x-12-48-8), draw_line, BLACK);
   if(((folder16[(128*i)+8] >> 5) & 0xF)<10) {
    draw_char('0', (graphic_screen_x-12-40-8), draw_line, BLACK);
    print_var(((folder16[(128*i)+8] >> 5) & 0xF), (graphic_screen_x-12-32-8), draw_line, BLACK);
   }
   else {
    print_var(((folder16[(128*i)+8] >> 5) & 0xF), (graphic_screen_x-12-40-8), draw_line, BLACK);
   }
   draw_char('/', (graphic_screen_x-12-24-8), draw_line, BLACK);
   if((folder16[(128*i)+8] & 0x1F)<10) {
    draw_char('0', (graphic_screen_x-12-16-8), draw_line, BLACK);
    print_var((folder16[(128*i)+8] & 0x1F), (graphic_screen_x-12-8-8), draw_line, BLACK);
   }
   else {
    print_var((folder16[(128*i)+8] & 0x1F), (graphic_screen_x-12-16-8), draw_line, BLACK);
   }
   
   draw_line+=10;
  }
 }
 else if(file_dialog_type_of_displaying_files==FD_DISPLAY_FILES_ICONS) { //TODO:
  dword_t actual_file=0;
  for(int line=0; line<file_dialog_icons_files_lines; line++) {
   for(int column=0; column<file_dialog_icons_files_columns; column++) {
    if(folder32[(64*actual_file)]==0) {
     return;
    }
    
    //draw if selected
    if((file_dialog_folder_first_displayed_file_num+actual_file)==file_dialog_selected_file) {
     draw_full_square(154+column*80, 35+line*110, 72, 110, 0xFF0000); //red
    }
    
    //draw icon
    if(folder8[(256*actual_file)+11]==0x10) {
     draw_full_square(158+column*80, 40+line*110, 64, 64, 0xFFED00); //yellow
     print("folder", 158+column*80+8, 40+line*110+28, BLACK);
    }
    else {
     get_file_extension(file_dialog_folder_mem, (file_dialog_folder_first_displayed_file_num+actual_file));
     
     //draw icon background
     if(file_extension[0]=='t' && file_extension[1]=='x' && file_extension[2]=='t') { //TXT
      draw_full_square(158+column*80, 40+line*110, 64, 64, 0x909090); //light grey
     }
     else if(file_extension[0]=='d' && file_extension[1]=='o' && file_extension[2]=='c') { //DOC/DOCX
      draw_full_square(158+column*80, 40+line*110, 64, 64, 0x009BFF); //blue
     }
     else if(file_extension[0]=='p' && file_extension[1]=='n' && file_extension[2]=='g') { //PNG
      draw_full_square(158+column*80, 40+line*110, 64, 64, 0xCE8000); //orange
     }
     else if(file_extension[0]=='j' && file_extension[1]=='p' && file_extension[2]=='g') { //JPG
      draw_full_square(158+column*80, 40+line*110, 64, 64, 0x65CE00); //light green
     }
     else if(file_extension[0]=='g' && file_extension[1]=='i' && file_extension[2]=='f') { //GIF
      draw_full_square(158+column*80, 40+line*110, 64, 64, 0xE33D00); //orange
     }
     else if(file_extension[0]=='b' && file_extension[1]=='m' && file_extension[2]=='p') { //BMP
      draw_full_square(158+column*80, 40+line*110, 64, 64, 0x5B4400); //brown
     }
     else if(file_extension[0]=='h' && file_extension[1]=='t' && file_extension[2]=='m') { //HTM/HTML
      draw_full_square(158+column*80, 40+line*110, 64, 64, 0x00FF9B); //light blue
     }
     else {
      draw_full_square(158+column*80, 40+line*110, 64, 64, WHITE);
     }
     
     //print extension name
     dword_t extension_name=0;
     for(int i=0; i<8; i++) {
      if(file_extension[i]==0) {
       break;
      }
      extension_name++;
     }
     file_extension[8]=0;
     print_ascii((byte_t *)file_extension, 158+column*80+32-(extension_name*4), 40+line*110+28, BLACK);
    }
    
    //print name
    dword_t name_length=0;
    for(int i=0; i<24; i++) {
     if(folder16[(128*actual_file)+16+i]==0) {
      break;
     }
     name_length++;
    }
    if(name_length<8) {
     print_unicode((word_t *)(file_dialog_folder_mem+(file_dialog_folder_first_displayed_file_num+actual_file)*256+32), 158+column*80+32-(name_length*4), 40+line*110+64+8, BLACK);
    }
    else if(name_length<16) {
     for(int i=0; i<8; i++) {
      file_name_print[i]=folder16[(128*actual_file)+16+i];
     }
     file_name_print[8]=0;
     print_unicode((word_t *)file_name_print, 158+column*80, 40+line*110+64+8, BLACK);
     
     print_unicode((word_t *)(file_dialog_folder_mem+(file_dialog_folder_first_displayed_file_num+actual_file)*256+32+16), 158+column*80+32-((name_length-8)*4), 40+line*110+64+18, BLACK);
    }
    else {
     for(int i=0; i<8; i++) {
      file_name_print[i]=folder16[(128*actual_file)+16+i];
     }
     file_name_print[8]=0;
     print_unicode((word_t *)file_name_print, 158+column*80, 40+line*110+64+8, BLACK);
     
     for(int i=0; i<8; i++) {
      file_name_print[i]=folder16[(128*actual_file)+16+8+i];
     }
     file_name_print[8]=0;
     print_unicode((word_t *)file_name_print, 158+column*80, 40+line*110+64+18, BLACK);
     
     for(int i=0; i<8; i++) {
      file_name_print[i]=folder16[(128*actual_file)+16+8+8+i];
     }
     file_name_print[8]=0;
     print_unicode((word_t *)file_name_print, 158+column*80+32-((name_length-16)*4), 40+line*110+64+28, BLACK);
    }
    
    actual_file++;
   }
  }
  
  //TODO: add scrollbar
 }
}

void draw_file_dialog(void) { 
 file_dialog_draw_background();
 if(file_dialog_window_type==FD_TYPE_OPEN) {
  print("Open file", 8, 8, BLACK);
 }
 else if(file_dialog_window_type==FD_TYPE_SAVE) {
  print("File type:", graphic_screen_x-120-get_number_of_chars_in_ascii_string((byte_t *)file_dialog_file_extension)*8-96, graphic_screen_y-16, BLACK);
  print((byte_t *)file_dialog_file_extension, graphic_screen_x-120-get_number_of_chars_in_ascii_string((byte_t *)file_dialog_file_extension)*8-8, graphic_screen_y-16, BLACK);
  print("Save file", 8, 8, BLACK);
  draw_button("[S] Save as", graphic_screen_x-120, graphic_screen_y-24, 120, 24);
 }
 print("[F12] Refresh devices [Page up/down] Select another device up/down", 8, graphic_screen_y-16, BLACK);
 
 if(file_dialog_state==FD_STATE_NO_DEVICE) {
  print("No storage device detected", 8, 32, BLACK);
  return;
 }
 
 file_dialog_draw_devices();
 if(file_dialog_state==FD_STATE_FOLDER_LOADED && file_dialog_folder_num_of_files>0) {
  file_dialog_draw_files();
  
  //if we are not in root folder, add button for go back
  dword_t *devices_mem32 = (dword_t *) (file_dialog_devices_mem+file_dialog_selected_device*FD_DEVICE_ENTRY_LENGTH);
  if(devices_mem32[3]!=0) {
   draw_button("[b] Go back", 0, graphic_screen_y-24-32, 150, 24);
  }
 }
 else if(file_dialog_state==FD_STATE_FOLDER_LOADED && file_dialog_folder_num_of_files==0) {
  print("There are no files in this folder", 158, 48, BLACK);
  
  //if we are not in root folder, add button for go back
  dword_t *devices_mem32 = (dword_t *) (file_dialog_devices_mem+file_dialog_selected_device*FD_DEVICE_ENTRY_LENGTH);
  if(devices_mem32[3]!=0) {
   draw_button("[b] Go back", 0, graphic_screen_y-24-32, 150, 24);
  }
 }
 else if(file_dialog_state==FD_STATE_FOLDER_LOADING) {
  print("Loading folder...", 158, 48, BLACK);
 }
 else if(file_dialog_state==FD_STATE_FOLDER_LOADING_ERROR) {
  print("Error during loading folder", 158, 48, BLACK);
 }
 else if(file_dialog_state==FD_STATE_FILE_LOADING) {
  print("Loading file...", 158, 48, BLACK);
 }
 else if(file_dialog_state==FD_STATE_FOLDER_WRITING) {
  print("Writing folder...", 158, 48, BLACK);
 }
 else if(file_dialog_state==FD_STATE_FILE_WRITING) {
  print("Writing file...", 158, 48, BLACK);
 }
 else if(file_dialog_state==FD_STATE_FILESYSTEM_UNKNOWN) {
  print("Unknown filesystem", 158, 48, BLACK);
 }
 else if(file_dialog_state==FD_STATE_FILESYSTEM_ERROR) {
  print("Error in filesystem, you can try remove and insert device", 158, 48, BLACK);
 }
}

void redraw_file_dialog(void) {
 draw_file_dialog();
 mouse_cursor_save_background(mouse_cursor_x, mouse_cursor_y);
 draw_mouse_cursor(mouse_cursor_x, mouse_cursor_y);
 redraw_screen();
}
