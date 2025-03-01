//BleskOS

/*
* MIT License
* Copyright (c) 2023-2025 Vendelín Slezák
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

void initalize_logging(void) {
 logging_mem = calloc(1024*1024);
 if(logging_mem==0) {
  memory_error_debug(0x0000FF);
 }
 logging_mem_end = (logging_mem+(1024*1024)-2); //this is pointer to lend of log memory, we need to left zero ending
 logging_mem_pointer = logging_mem;
 logging_mem_draw_pointer = logging_mem; //this is pointer to first value that is drawed on screen
}

void developer_program_log(void) {
 word_t *log = (word_t *) (logging_mem_draw_pointer);

 redraw:
 show_log();

 while(1) {
  wait_for_user_input();
  move_mouse_cursor();
  
  if(keyboard_code_of_pressed_key==KEY_ESC || mouse_click_button_state==MOUSE_CLICK) {
   return;
  }
  else if(keyboard_code_of_pressed_key==KEY_F2) {
   //count number of chars
   word_t *logging_area = (word_t *) (logging_mem);
   dword_t number_of_chars = 0;
   while(*logging_area!=0) {
    number_of_chars++;
    logging_area++;
   }

   //convert text area content to utf-8
   convert_unicode_to_utf_8(logging_mem, number_of_chars);

   //save file
   file_dialog_save((byte_t *)converted_file_memory, converted_file_size-1, "txt"); //size is without zero char ending
   free(converted_file_memory);
   goto redraw;
  }
  else if(keyboard_code_of_pressed_key==KEY_HOME) {
   logging_mem_draw_pointer = logging_mem; //go to start of logging memory
   log = (word_t *) (logging_mem_draw_pointer);

   goto redraw;
  }
  else if(keyboard_code_of_pressed_key==KEY_END) {
   //go to end of logging memory
   while(*log!=0) {
    log++;
   }
   logging_mem_draw_pointer = ((dword_t)log);

   goto redraw;
  }
  else if(keyboard_code_of_pressed_key==KEY_UP || (mouse_wheel_movement!=0 && mouse_wheel_movement<0x80000000)) {
   if((dword_t)log==logging_mem) {
    continue; //we are at start of logging memory
   }

   log--;
   logging_mem_draw_pointer-=2;
   if((dword_t)log==logging_mem) {
    goto redraw; //we were one char ahead from start of logging memory
   }

   log--;
   logging_mem_draw_pointer-=2;
   if(log[0]==0xA && log[1]==0xA) {
    //last two chars were enters, so we need to go back just one char - one line
    log++;
    logging_mem_draw_pointer+=2;
    goto redraw;
   }

   //skip all characters in line
   while(*log!=0xA && (dword_t)log!=logging_mem) {
    log--;
    logging_mem_draw_pointer-=2;
   }

   //skip enter
   if(*log==0xA) {
    log++;
    logging_mem_draw_pointer+=2;
   }
   
   goto redraw;
  }
  else if(keyboard_code_of_pressed_key==KEY_DOWN || mouse_wheel_movement>0x80000000) {
   //skip all characters in line
   while(*log!=0xA && *log!=0) {
    log++;
    logging_mem_draw_pointer+=2;
   }

   //skip enter
   if(*log==0xA) {
    log++;
    logging_mem_draw_pointer+=2;
   }

   goto redraw;
  }
  else if(keyboard_code_of_pressed_key==KEY_PAGE_UP) {
   for(dword_t i=0; i<10; i++) {
    if((dword_t)log==logging_mem) {
     continue; //we are at start of logging memory
    }

    log--;
    logging_mem_draw_pointer-=2;
    if((dword_t)log==logging_mem) {
     goto redraw; //we were one char ahead from start of logging memory
    }

    log--;
    logging_mem_draw_pointer-=2;
    if(log[0]==0xA && log[1]==0xA) {
     //last two chars were enters, so we need to go back just one char - one line
     log++;
     logging_mem_draw_pointer+=2;
     goto redraw;
    }

    //skip all characters in line
    while(*log!=0xA && (dword_t)log!=logging_mem) {
     log--;
     logging_mem_draw_pointer-=2;
    }

    //skip enter
    if(*log==0xA) {
     log++;
     logging_mem_draw_pointer+=2;
    }
   }

   goto redraw;
  }
  else if(keyboard_code_of_pressed_key==KEY_PAGE_DOWN) {
   for(dword_t i=0; i<10; i++) {
    //skip all characters in line
    while(*log!=0xA && *log!=0) {
     log++;
     logging_mem_draw_pointer+=2;
    }

    //skip enter
    if(*log==0xA) {
     log++;
     logging_mem_draw_pointer+=2;
    }
   }

   goto redraw;
  }
 }
}

void skip_logs(void) {
 word_t *log = (word_t *) (logging_mem_draw_pointer);

 while(*log!=0) {
  log++;
  logging_mem_draw_pointer+=2;
 }
}

void show_log(void) {
 word_t *log = (word_t *) (logging_mem_draw_pointer);

 clear_screen(0x444444);

 draw_x = 10;
 draw_y = 10;
 for(dword_t num_of_lines = ((screen_height-20)/10); num_of_lines>0; log++) {
  if(*log==0) {
   break; //we are at end of log
  }
  if(*log==0xA) { //we are at end of line
   draw_x = 10;
   draw_y += 10;
   num_of_lines--;
  }
  else { //draw char on screen
   if(draw_x < screen_width) {
    draw_char(*log, draw_x, draw_y, WHITE);
   }
   draw_x += 8;
  }
 }
 
 //redraw
 redraw_screen();
}

void log_char(word_t character) {
 word_t *log = (word_t *) (logging_mem_pointer);

 outb(0xE9, character);
 
 *log = character;
 log++;
 logging_mem_pointer += 2;
 
 //if we reach end of logging memory, start again from start
 if(logging_mem_pointer >= logging_mem_end) {
  logging_mem_pointer = logging_mem;
 }
}

void log(char *string) {
 word_t *log = (word_t *) (logging_mem_pointer);
 
 while(*string!=0) {
  outb(0xE9, *string);
  
  *log=(word_t)*string;
  log++;
  string++;
  logging_mem_pointer+=2;
  
  //if we reach end of logging memory, start again from start
  if(logging_mem_pointer>=logging_mem_end) {
   logging_mem_pointer = logging_mem;
   log = (word_t *) (logging_mem);
  }
 }
}

void log_unicode(word_t *string) {
 word_t *log = (word_t *) (logging_mem_pointer);
 
 while(*string!=0) {
  *log=(word_t)*string;
  log++;
  string++;
  logging_mem_pointer+=2;
  
  //if we reach end of logging memory, start again from start
  if(logging_mem_pointer>=logging_mem_end) {
   logging_mem_pointer = logging_mem;
   log = (word_t *) (logging_mem);
  }
 }
}

void log_var(dword_t value) {
 byte_t number_string[11];
 dword_t number_digits = 0;
 
 for(dword_t i=0; i<11; i++) {
  number_string[i]=0;
 }
 
 for(dword_t i=9; i>0; i--) {
  number_string[i]=(value%10);
  value/=10;
  number_digits++;
  if(value==0) {
   break;
  }
 }
 
 for(int i=0; i<10; i++) {
  number_string[i]+='0';
 }
 
 log((char *) (number_string+(10-number_digits)));
}

void log_var_with_space(dword_t value) {
 log_var(value);
 log(" ");
}

void log_hex(dword_t value) {
 byte_t number_string[11];
 
 number_string[0]='0';
 number_string[1]='x';
 number_string[10]=0;
 
 for(dword_t i=2, digit=0, shift=28; i<10; i++, shift-=4) {
  digit = ((value>>shift) & 0xF);
  number_string[i] = ((digit<10) ? (digit+'0') : (digit+'A'-10));
 }
 
 log((char *)number_string);
}

void log_hex_with_space(dword_t value) {
 log_hex(value);
 log(" ");
}

void log_hex_specific_size(dword_t value, dword_t chars) {
 byte_t number_string[11];
 
 number_string[0]='0';
 number_string[1]='x';
 number_string[chars+2]=0;
 
 for(dword_t i=2, digit=0, shift=(chars*4-4); i<(chars+2); i++, shift-=4) {
  digit = ((value>>shift) & 0xF);
  number_string[i] = ((digit<10) ? (digit+'0') : (digit+'A'-10));
 }
 
 log((char *)number_string);
}

void log_hex_specific_size_with_space(dword_t value, dword_t chars) {
 log_hex_specific_size(value, chars);
 log(" ");
}

void logf(byte_t *string, ...) {
 va_list args;
 va_start(args, string);

 while(*string != 0) {
  if(*string != '%') {
   //add character
   log_char(*string);
   string++;
  }
  else {
   string++;

   //add % character
   if(*string == '%') {
    log_char('%');
    string++;
    continue;
   }
   else if(*string == 'c') {
    byte_t character = va_arg(args, dword_t);
    string++;
    if(character == 0) {
     continue;
    }
    log_char(character);
    continue;
   }

   //get number of characters to print
   dword_t number_of_chars_in_parameter = 0;
   if(*string == '0') {
    string++;
    number_of_chars_in_parameter = convert_byte_string_to_number((dword_t)string);

    //skip all numbers
    while(is_number(*string) == STATUS_TRUE) {
     string++;
    }
   }

   //add string
   if(*string == 's') {
    byte_t *parameter_string = va_arg(args, byte_t *);

    if(number_of_chars_in_parameter == 0) {
     while(*parameter_string != 0) {
      log_char(*parameter_string);
      parameter_string++;
     }
    }
    else {
     for(dword_t i=0; i<number_of_chars_in_parameter; i++) {
      log_char(*parameter_string);
      parameter_string++;
     }
    }

    string++;
   }
   //add decadic number
   else if(*string == 'd') {
    dword_t number = va_arg(args, dword_t);
    byte_t number_string[11];
    byte_t number_of_digits = 0;

    //parse digits from number
    if(number == 0) {
     number_string[0] = '0';
     number_of_digits = 1;
    }
    else {
     while(number != 0) {
      number_string[number_of_digits] = ((number % 10)+'0');
      number_of_digits++;
      number /= 10;
     }
    }

    //add digits
    byte_t *number_string_pointer = (byte_t *) (((dword_t)&number_string)+(number_of_digits-1));
    for(dword_t i=0; i<number_of_digits; i++) {
     log_char(*number_string_pointer);
     number_string_pointer--;
    }

    string++;
   }
   //add hexadecimal number
   else if(*string == 'x') {
    dword_t number = va_arg(args, dword_t);

    if(number_of_chars_in_parameter == 0) {
     number_of_chars_in_parameter = 8;
    }

    for(dword_t i=0, digit=0, shift=(number_of_chars_in_parameter*4-4); i<number_of_chars_in_parameter; i++, shift-=4) {
     digit = ((number>>shift) & 0xF);
     log_char((digit<10) ? (digit+'0') : (digit+'A'-10));
    }

    string++;
   }
  }
 }
}