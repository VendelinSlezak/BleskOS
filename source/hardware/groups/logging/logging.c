/* 
* BleskOS
*
* MIT License
* Copyright (c) 2023-2025 BleskOS developers
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

/* includes */
#include <stdarg.h>
#include <kernel/x86/libc/stdio.h>
#include <kernel/x86/libc/ctype.h>
#include <kernel/x86/scheduler/lock.h>
#include <kernel/x86/entities/entities.h>
#include <kernel/x86/libc/stdlib.h>

#include <hardware/devices/e9_hack/e9_hack.h>
#include <hardware/devices/vga_text_mode/vga.h>

/* local variables */
dword_t logging_entity;
e_logging_group_t *logging_attr;

/* functions */
void initialize_logging_group(void) {
    logging_entity = create_entity("Logging group", E_TYPE_KERNEL_RING);
    logging_attr = entity_get_attr_ptr(logging_entity);

    logging_attr->number_of_devices = 0;
    if(is_e9_hack_present() == TRUE) {
        logging_attr->send_character[logging_attr->number_of_devices++] = e9_hack_send_char;
    }
    // if(is_vga_text_mode_present() == TRUE) {
    //     initialize_vga_text_mode();
    //     logging_attr->send_character[logging_attr->number_of_devices++] = vga_text_mode_send_character;
    // }

    logging_attr->logs = open_rwstream();
}

void *get_start_of_log_stream(void) {
    return logging_attr->logs.buffer;
}

void log_char(dword_t character) {
    if(character == 0 || character > 0xFF) {
        return;
    }

    sputc(character, (STREAM_t *)&logging_attr->logs);

    for(int i = 0; i < logging_attr->number_of_devices; i++) {
        logging_attr->send_character[i](character);
    }
}

void log(byte_t *string, ...) {   
    LOCK_MUTEX(&logging_attr->logging_to_output);
    lock_core();

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
                number_of_chars_in_parameter = atoi(string);

                // skip numbers in string
                while(isdigit(*string) != FALSE) {
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

                // skip 's' / 'S' char
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
            else if(*string == 'd' || *string == 'u') {
                // buffer na číslo ako string
                char number_string[12]; // big enough for sign + whole number + zero ending

                // convert number to string
                itoa(va_arg(args, dword_t), number_string, 11);

                // print number string
                char *p = number_string;
                while(*p != '\0') {
                    log_char(*p);
                    p++;
                }

                // skip 'd' in string
                string++;
            }
            // log hexadecimal number from argument
            else if(*string == 'x' || *string == 'X') {
                // load argument
                dword_t number = va_arg(args, dword_t);

                // set number of digits on output
                if(number_of_chars_in_parameter == 0) {
                    number_of_chars_in_parameter = 8;
                }

                // log digits
                for(dword_t i = 0, digit = 0, shift = (number_of_chars_in_parameter*4 - 4); i < number_of_chars_in_parameter; i++, shift -= 4) {
                    digit = ((number >> shift) & 0xF);
                    log_char((digit < 10) ? (digit + '0') : (digit + 'A' - 10));
                }

                // skip 'x' char
                string++;
            }
        }
    }

    unlock_core();
    UNLOCK_MUTEX(&logging_attr->logging_to_output);
}