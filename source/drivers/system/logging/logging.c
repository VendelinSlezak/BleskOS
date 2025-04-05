//BleskOS

/*
* MIT License
* Copyright (c) 2023-2025 BleskOS developers
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

void initalize_logging(void) {
    logging_memory = (word_t *) calloc(LOGGING_MAX_NUMBER_OF_CHARS*2+2);
    logging_memory_chars = 0;
    logging_memory_first_showed_char = 0;

    logf("BleskOS %d update %d\n\nPress F2 to save System log as TXT file", BLESKOS_VERSION, BLESKOS_UPDATE);
}

void developer_program_log(void) {
    redraw:
    show_log();

    while(1) {
        wait_for_user_input();
        move_mouse_cursor();
    
        if(keyboard_code_of_pressed_key==KEY_ESC || mouse_click_button_state==MOUSE_CLICK) {
            return;
        }
        else if(keyboard_code_of_pressed_key==KEY_F2) {
            // count number of chars
            dword_t number_of_chars = 0;
            while(logging_memory[number_of_chars] != 0) {
                number_of_chars++;
            }

            // convert text area content to utf-8
            convert_unicode_to_utf_8((dword_t)logging_memory, number_of_chars);

            // save file
            file_dialog_save((byte_t *)converted_file_memory, converted_file_size-1, "txt"); //size is without zero char ending
            free((void *)converted_file_memory);

            goto redraw;
        }
        else if(keyboard_code_of_pressed_key==KEY_HOME) {
            logging_memory_first_showed_char = 0;
            goto redraw;
        }
        else if(keyboard_code_of_pressed_key==KEY_END) {
            logging_memory_first_showed_char = logging_memory_chars;
            goto redraw;
        }
        else if(keyboard_code_of_pressed_key==KEY_UP || (mouse_wheel_movement!=0 && mouse_wheel_movement<0x80000000)) {
            if(logging_memory_first_showed_char == 0) {
                continue; //we are at start of logging memory
            }

            logging_memory_first_showed_char--;
            if(logging_memory_first_showed_char == 0) {
                goto redraw; //we were one char ahead from start of logging memory
            }

            logging_memory_first_showed_char--;
            if(logging_memory[logging_memory_first_showed_char] == 0xA && logging_memory[logging_memory_first_showed_char+1] == 0xA) {
                logging_memory_first_showed_char++; // last two chars were enters, so we need to go back just one char - one line
                goto redraw;
            }

            // skip all characters in line
            while(logging_memory[logging_memory_first_showed_char] != 0xA && logging_memory_first_showed_char != 0) {
                logging_memory_first_showed_char--;
            }

            // skip enter
            if(logging_memory[logging_memory_first_showed_char] == 0xA) {
                logging_memory_first_showed_char++;
            }
            
            goto redraw;
        }
        else if(keyboard_code_of_pressed_key==KEY_DOWN || mouse_wheel_movement>0x80000000) {
            // if there are two empty lines, skip one
            if(logging_memory[logging_memory_first_showed_char] == 0xA && logging_memory[logging_memory_first_showed_char+1] == 0xA) {
                logging_memory_first_showed_char++;
                goto redraw;
            }
            
            // skip all characters in line
            while(logging_memory[logging_memory_first_showed_char] != 0xA && logging_memory[logging_memory_first_showed_char] != 0) {
                logging_memory_first_showed_char++;
            }

            //skip enter
            if(logging_memory[logging_memory_first_showed_char] == 0xA) {
                logging_memory_first_showed_char++;
            }

            goto redraw;
        }
        else if(keyboard_code_of_pressed_key==KEY_PAGE_UP) {
            for(dword_t i = 0; i < 10; i++) {
                if(logging_memory_first_showed_char == 0) {
                    continue; //we are at start of logging memory
                }

                logging_memory_first_showed_char--;
                if(logging_memory_first_showed_char == 0) {
                    goto redraw; //we were one char ahead from start of logging memory
                }

                logging_memory_first_showed_char--;
                if(logging_memory[logging_memory_first_showed_char] == 0xA && logging_memory[logging_memory_first_showed_char+1] == 0xA) {
                    logging_memory_first_showed_char++; // last two chars were enters, so we need to go back just one char - one line
                    continue;
                }

                // skip all characters in line
                while(logging_memory[logging_memory_first_showed_char] != 0xA && logging_memory_first_showed_char != 0) {
                    logging_memory_first_showed_char--;
                }

                // skip enter
                if(logging_memory[logging_memory_first_showed_char] == 0xA) {
                    logging_memory_first_showed_char++;
                }
            }

            goto redraw;
        }
        else if(keyboard_code_of_pressed_key==KEY_PAGE_DOWN) {
            for(dword_t i = 0; i < 10; i++) {
                // if there are two empty lines, skip one
                if(logging_memory[logging_memory_first_showed_char] == 0xA && logging_memory[logging_memory_first_showed_char+1] == 0xA) {
                    logging_memory_first_showed_char++;
                    continue;
                }
                
                // skip all characters in line
                while(logging_memory[logging_memory_first_showed_char] != 0xA && logging_memory[logging_memory_first_showed_char] != 0) {
                    logging_memory_first_showed_char++;
                }

                //skip enter
                if(logging_memory[logging_memory_first_showed_char] == 0xA) {
                    logging_memory_first_showed_char++;
                }
            }

            goto redraw;
        }
    }
}

void skip_logs(void) {
    logging_memory_first_showed_char = logging_memory_chars;
}

void show_log(void) {
    // draw grey background
    clear_screen(0x444444);

    // draw characters
    dword_t pointer = logging_memory_first_showed_char;
    dword_t x = 10, y = 10;
    while(y < (monitors[0].height-20)) {
        if(logging_memory[pointer] == 0) {
            break; // end of log
        }
        else if(logging_memory[pointer] == 0xA) { // end of line
            x = 10;
            y += 10;
        }
        else { // draw char on screen
            if(x < monitors[0].width) {
                draw_char(logging_memory[pointer], x, y, WHITE);
            }
            x += 8;
        }
        pointer++;
    }
    
    // show log
    redraw_screen();
}

void log_char(word_t character) {
    // check if this is valid character
    if(character == 0) {
        return;
    }

    // send character to first available debug device
    if(character < 0x100) {
        if(components->p_e9_debug_device == STATUS_TRUE) {
            e9_debug_device_send_char((byte_t)character);
        }
        else {
            for(dword_t i = 0; i < components->n_serial_port; i++) {
                if(components->serial_port[i].device_type == SERIAL_PORT_DEBUG_DEVICE) {
                    if(character == 0xA) {
                        serial_port_send_byte(i, 0xD);
                    }
                    serial_port_send_byte(i, (byte_t)character);
                    break;
                }
            }
        }
        
    }

    // save character to memory
    if(logging_memory_chars < LOGGING_MAX_NUMBER_OF_CHARS) {
        logging_memory[logging_memory_chars] = character;
        logging_memory_chars++;
    }
}

void logf(byte_t *string, ...) {
    va_list args;
    va_start(args, string);

    while(*string != 0) {
        // log character from string
        if(*string != '%') {
            log_char(*string);
            string++;
        }
        else {
            // skip '%' char
            string++;

            // log % character
            if(*string == '%') {
                log_char('%');
                string++;
                continue;
            }
            // log character from argument
            else if(*string == 'c') {
                // load character
                word_t character = va_arg(args, dword_t);
                string++;

                // log character
                log_char(character);
                continue;
            }

            // get number of characters to print
            dword_t number_of_chars_in_parameter = 0;
            if(*string == '0') {
                string++;
                number_of_chars_in_parameter = convert_byte_string_to_number((dword_t)string);

                // skip numbers in string
                while(is_number(*string) == STATUS_TRUE) {
                    string++;
                }
            }

            // log string from argument
            if(*string == 's') {
                // load argument
                byte_t *parameter_string = va_arg(args, byte_t *);

                // log full string
                if(number_of_chars_in_parameter == 0) {
                    while(*parameter_string != 0) {
                        log_char(*parameter_string);
                        parameter_string++;
                    }
                }
                // log part of string
                else {
                    for(dword_t i = 0; i < number_of_chars_in_parameter; i++) {
                        log_char(*parameter_string);
                        parameter_string++;
                    }
                }

                // skip 's' char
                string++;
            }
            // log unicode string from argument
            else if(*string == 'S') {
                // load argument
                word_t *parameter_string = va_arg(args, word_t *);

                // log full string
                if(number_of_chars_in_parameter == 0) {
                    while(*parameter_string != 0) {
                        log_char(*parameter_string);
                        parameter_string++;
                    }
                }
                // log part of string
                else {
                    for(dword_t i = 0; i < number_of_chars_in_parameter; i++) {
                        log_char(*parameter_string);
                        parameter_string++;
                    }
                }

                // skip 'S' char
                string++;
            }
            // log decadic number from argument
            else if(*string == 'd') {
                // load argument
                dword_t number = va_arg(args, dword_t);

                // convert number to string
                byte_t number_string[11];
                byte_t number_of_digits = 0;
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

                // log digits
                byte_t *number_string_pointer = (byte_t *) (((dword_t)&number_string)+(number_of_digits-1));
                for(dword_t i = 0; i < number_of_digits; i++) {
                    log_char(*number_string_pointer);
                    number_string_pointer--;
                }

                // skip 'd' char
                string++;
            }
            // log hexadecimal number from argument
            else if(*string == 'x') {
                // load argument
                dword_t number = va_arg(args, dword_t);

                // set number of digits on output
                if(number_of_chars_in_parameter == 0) {
                    number_of_chars_in_parameter = 8;
                }

                // log digits
                for(dword_t i=0, digit=0, shift=(number_of_chars_in_parameter*4-4); i<number_of_chars_in_parameter; i++, shift-=4) {
                    digit = ((number>>shift) & 0xF);
                    log_char((digit<10) ? (digit+'0') : (digit+'A'-10));
                }

                // skip 'x' char
                string++;
            }
        }
    }
}

void serious_kernel_error(byte_t *type) {
    cmem(logging_memory, LOGGING_MAX_NUMBER_OF_CHARS*2);
    logging_memory_chars = 0;
    logging_memory_first_showed_char = 0;
    logf("SERIOUS ERROR OCCURED type: %s ", type);
}