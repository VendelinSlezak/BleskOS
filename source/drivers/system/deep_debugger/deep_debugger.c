//BleskOS

/*
* MIT License
* Copyright (c) 2023-2025 Vendelín Slezák
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

void deep_debugger(void) {
 deep_debugger_memory = (byte_t *) calloc(NUM_OF_CHARACTERS_IN_DDBG);

 vga_text_mode_clear_screen(0x07);
 vga_text_mode_draw_square(24, 0, 80, 1, 0x70);
 vga_text_mode_move_cursor(24, 0);

 clear_ddbg();
 ddbg_printf("Welcome to BleskOS v%du%d Deep Debugger", BLESKOS_VERSION, BLESKOS_UPDATE);
 ddbg_printf("\n\nIf you do not know what to do, type \"help\"");

 deep_debugger_redraw();

 byte_t command[80];
 byte_t cursor_column = 0;

 while(1) {
  wait_for_user_input();

  if(keyboard_code_of_pressed_key == KEY_ENTER) {
   clear_ddbg();

   //parse command line
   byte_t *command_sections[80];
   dword_t command_section_number[80];
   byte_t command_section_type[80];
   byte_t number_of_command_sections = 0;
   for(dword_t i=0; i<80; i++) {
    //end of command
    if(command[i] == 0) {
     break;
    }
    //hexadecimal
    else if(command[i] == '0' && command[i+1] == 'x') {
     i += 2;
     command_section_type[number_of_command_sections] = 'x';
     command_section_number[number_of_command_sections] = convert_hex_string_to_number(&command[i]);
     number_of_command_sections++;

     while(is_number(command[i]) == STATUS_TRUE) {
      i++;
     }
    }
    //decimal
    else if(is_number(command[i]) == STATUS_TRUE) {
     command_section_type[number_of_command_sections] = 'd';
     command_section_number[number_of_command_sections] = convert_byte_string_to_number((dword_t)&command[i]);
     number_of_command_sections++;

     while(is_number(command[i]) == STATUS_TRUE) {
      i++;
     }
    }
    //string
    else {
     command_section_type[number_of_command_sections] = 's';
     command_sections[number_of_command_sections] = &command[i];
     number_of_command_sections++;

     while(is_char(command[i]) == STATUS_TRUE) {
      i++;
     }
    }
   }

   // help
   if(DDBG_IS_COMMAND(0, "help")) {
    ddbg_printf("Type commands and execute them with enter\nYou can scroll result with arrows\n\nList of commands:");
    ddbg_printf("\n\ninb 0xport");
    ddbg_printf("\ninw 0xport");
    ddbg_printf("\nind 0xport");
    ddbg_printf("\nmem 0xaddress");
    ddbg_printf("\nshutdown");
    ddbg_printf("\nshow devregs");
   }

   // inb 0xport
   else if(DDBG_IS_COMMAND(0, "inb") && command_section_type[1] == 'x') {
    word_t port = command_section_number[1];
    ddbg_printf("Port 0x%04x inb: 0x%02x", port, inb(port));
   }
   // inw 0xport
   else if(DDBG_IS_COMMAND(0, "inw") && command_section_type[1] == 'x') {
    word_t port = command_section_number[1];
    ddbg_printf("Port 0x%04x inw: 0x%04x", port, inw(port));
   }
   // ind 0xport
   else if(DDBG_IS_COMMAND(0, "ind") && command_section_type[1] == 'x') {
    word_t port = command_section_number[1];
    ddbg_printf("Port 0x%04x ind: 0x%x", port, ind(port));
   }

   // mem 0xaddress
   else if(DDBG_IS_COMMAND(0, "mem") && command_section_type[1] == 'x') {
    dword_t memory = command_section_number[1];
    ddbg_printf("Memory 0x%x\n Byte: %02x %d %c\n Word: %04x %d\n Dword: %x %d", memory, mmio_inb(memory), mmio_inb(memory), mmio_inb(memory), mmio_inw(memory), mmio_inw(memory), mmio_ind(memory), mmio_ind(memory));
   }

   // show
   else if(DDBG_IS_COMMAND(0, "show")) {
    // show memb 0xaddress bytes
    if(DDBG_IS_COMMAND(1, "memb")) {
     byte_t *memory = (byte_t *) (command_section_number[2]);
     for(dword_t i=0; i<command_section_number[3]; i++) {
      ddbg_printf("\nMem: 0x%x Data: 0x%02x %c", (dword_t)memory, *memory, *memory);
      memory++;
     }
    }

    // show memw 0xaddress bytes
    else if(DDBG_IS_COMMAND(1, "memw")) {
     word_t *memory = (word_t *) (command_section_number[2]);
     for(dword_t i=0; i<command_section_number[3]; i++) {
      ddbg_printf("\nMem: 0x%x Data: 0x%04x %c", (dword_t)memory, *memory, *memory);
      memory++;
     }
    }

    // show memd 0xaddress bytes
    else if(DDBG_IS_COMMAND(1, "memd")) {
     word_t *memory = (word_t *) (command_section_number[2]);
     for(dword_t i=0; i<command_section_number[3]; i++) {
      ddbg_printf("\nMem: 0x%x Data: 0x%x %c", (dword_t)memory, *memory, *memory);
      memory++;
     }
    }

    // show devregs
    else if(DDBG_IS_COMMAND(1, "devregs")) {
     // show devregs uhci NumOfController
     if(DDBG_IS_COMMAND(2, "uhci") && command_section_type[3] == 'd') {
      ddbg_show_devregs_uhci(command_section_number[3]);
     }

    }

    //unknown
    else {
     ddbg_printf("Unknown command:\n%s", command);
    }
   }

   // shutdown
   else if(DDBG_IS_COMMAND(0, "shutdown")) {
    shutdown();
   }

   //unknown
   else {
    ddbg_printf("Unknown command:\n%s", command);
   }

   //delete command
   clear_memory((dword_t)&command, 80);
   cursor_column = 0;
   vga_text_mode_clear_screen(0x07);
   vga_text_mode_draw_square(24, 0, 80, 1, 0x70);
   vga_text_mode_move_cursor(24, 0);

   deep_debugger_redraw();
   continue;
  }
  else if(keyboard_code_of_pressed_key == KEY_UP && first_showed_line > 0) {
   first_showed_line--;
   first_showed_character_offset -= 2;
   while(deep_debugger_memory[first_showed_character_offset] != '\n') {
    first_showed_character_offset--;
   }
   first_showed_character_offset++;
   deep_debugger_redraw();
   continue;
  }
  else if(keyboard_code_of_pressed_key == KEY_PAGE_UP) {
   for(dword_t i=0; i<24; i++) {
    if(first_showed_line > 0) {
     first_showed_line--;
     first_showed_character_offset -= 2;
     while(deep_debugger_memory[first_showed_character_offset] != '\n') {
      first_showed_character_offset--;
     }
     first_showed_character_offset++;
    }
    else {
     break;
    }
   }
   deep_debugger_redraw();
   continue;
  }
  else if(keyboard_code_of_pressed_key == KEY_DOWN && (first_showed_line+24) < num_of_lines) {
   first_showed_line++;
   while(deep_debugger_memory[first_showed_character_offset] != '\n') {
    first_showed_character_offset++;
   }
   first_showed_character_offset++;
   deep_debugger_redraw();
   continue;
  }
  else if(keyboard_code_of_pressed_key == KEY_PAGE_DOWN) {
   for(dword_t i=0; i<24; i++) {
    if((first_showed_line+24) < num_of_lines) {
     first_showed_line++;
     while(deep_debugger_memory[first_showed_character_offset] != '\n') {
      first_showed_character_offset++;
     }
     first_showed_character_offset++;
    }
    else {
     break;
    }
   }
   deep_debugger_redraw();
   continue;
  }
  else if(keyboard_code_of_pressed_key == KEY_BACKSPACE && cursor_column>0) {
   cursor_column--;
   command[cursor_column] = 0;
   vga_text_mode_put_char(24, cursor_column, 0);
   vga_text_mode_move_cursor(24, cursor_column);
   continue;
  }

  if(keyboard_unicode_value_of_pressed_key != 0 && cursor_column<80) {
   command[cursor_column] = keyboard_unicode_value_of_pressed_key;
   vga_text_mode_put_char(24, cursor_column, keyboard_unicode_value_of_pressed_key);
   cursor_column++;
   vga_text_mode_move_cursor(24, cursor_column);
  }
 }
}

void deep_debugger_redraw(void) {
 vga_text_mode_draw_square(0, 0, 80, 24, 0x07);

 byte_t line = 0, column = 0;
 dword_t pointer = first_showed_character_offset;
 while(deep_debugger_memory[pointer] != 0 && line < 24) {
  if(deep_debugger_memory[pointer] == 0xA) {
   line++;
   column = 0;
  }
  else if(column < 80) {
   vga_text_mode_put_char(line, column, deep_debugger_memory[pointer]);
   column++;
  }
  pointer++;
 }
}

void clear_ddbg(void) {
 clear_memory((dword_t)deep_debugger_memory, NUM_OF_CHARACTERS_IN_DDBG);
 deep_debugger_memory[0] = '\n';
 first_showed_character_offset = 1;
 num_of_characters = 1;
 num_of_lines = 1;
 first_showed_line = 0;
}

void ddbg_printf(byte_t *string, ...) {
 va_list args;
 va_start(args, string);

 while(*string != 0) {
  if(*string != '%') {
   if(*string == '\n') {
    num_of_lines++;
   }

   //add character
   deep_debugger_memory[num_of_characters] = *string;
   num_of_characters++;
   string++;
  }
  else {
   string++;

   //add % character
   if(*string == '%') {
    deep_debugger_memory[num_of_characters] = '%';
    num_of_characters++;
    string++;
    continue;
   }
   else if(*string == 'c') {
    byte_t character = va_arg(args, dword_t);
    string++;
    if(character == 0) {
     continue;
    }
    deep_debugger_memory[num_of_characters] = character;
    if(character == '\n') {
     num_of_lines++;
    }
    num_of_characters++;
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
      deep_debugger_memory[num_of_characters] = *parameter_string;
      if(deep_debugger_memory[num_of_characters] == '\n') {
       num_of_lines++;
      }
      num_of_characters++;
      parameter_string++;
     }
    }
    else {
     for(dword_t i=0; i<number_of_chars_in_parameter; i++) {
      deep_debugger_memory[num_of_characters] = *parameter_string;
      if(deep_debugger_memory[num_of_characters] == '\n') {
       num_of_lines++;
      }
      num_of_characters++;
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
     deep_debugger_memory[num_of_characters] = *number_string_pointer;
     num_of_characters++;
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
     deep_debugger_memory[num_of_characters] = ((digit<10) ? (digit+'0') : (digit+'A'-10));
     num_of_characters++;
    }

    string++;
   }
  }
 }
}

void ddbg_print_parsed_mem(byte_t *value_label, struct ddbg_value_parsing_info_t *parsing_info, dword_t value) {
 // parsing_info = { number_of_taken_bits, "Label of bits", 'd'/'x' }
 // output = ("\nLabel of bits: %d/%x", value & number_of_taken_bits)
 // "Label of bits" == 0 mean "Reserved" and is printed only if nonzero
 if(value_label != 0) {
  ddbg_printf("\n\n%s", value_label);
 }
 while(parsing_info->number_of_taken_bits != 0) {
  byte_t *print_string = "\n %s: 0x%00x";
  if(parsing_info->type_of_output == 'd') {
   print_string[6] = '%';
   print_string[7] = 'd';
   print_string[8] = 0;
  }
  else if(parsing_info->type_of_output == 'x') {
   print_string[6] = '0';
   print_string[7] = 'x';
   print_string[8] = '%';
   print_string[9] = '0';
   print_string[10] = ((parsing_info->number_of_taken_bits % 4 == 0) ? (parsing_info->number_of_taken_bits / 4) : ((parsing_info->number_of_taken_bits / 4)+1))+'0';
  }
  if(parsing_info->label != 0) {
   ddbg_printf(print_string, parsing_info->label, value & ~(0xFFFFFFFF << parsing_info->number_of_taken_bits));
  }
  else if((value & ~(0xFFFFFFFF << parsing_info->number_of_taken_bits)) != 0) {
   ddbg_printf(print_string, "Reserved", value & ~(0xFFFFFFFF << parsing_info->number_of_taken_bits));
  }
  parsing_info++;
  value >>= parsing_info->number_of_taken_bits;
 }
}