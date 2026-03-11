/* 
* BleskOS
*
* MIT License
* Copyright (c) 2023-2026 BleskOS developers
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

/* includes */
#include <stdarg.h>
#include <libc/ctype.h>
#include <libc/stdlib.h>
#include <kernel/cpu/commands.h>
#include <kernel/cpu/mutex.h>
#include <kernel/cpu/interrupt.h>
#include <kernel/memory/memory_allocators.h>

#include <kernel/software/syscall.h>
#include <kernel/hardware/main.h>
#include <kernel/hardware/devices/logging/e9_device.h>

/* local variables */
int logging_enabled = false;
logging_group_t *logging_group;
uint32_t logging_device_id;
uint32_t *logging_data;
uint32_t logging_data_size;
uint32_t logging_data_pointer;
mutex_t logging_mutex;

/* functions */
void initialize_logging_group(void) {
    logging_group = (logging_group_t *) kalloc(sizeof(logging_group_t));
    logging_device_id = get_unique_hardware_id();
    add_device_to_hardware_list(logging_device_id, HARDWARE_TYPE_LOGGER);

    logging_data = (uint32_t *) kalloc(sizeof(uint32_t) * 1024 * 10);
    logging_data_size = 1024 * 10 - 1;
    logging_data_pointer = 0;
}

void add_logging_device(void (*send_character)(uint32_t character)) {
    if(logging_group->number_of_devices >= MAX_NUMBER_OF_LOGGING_GROUP_DEVICES) {
        return;
    }
    logging_group->devices[logging_group->number_of_devices].id = get_unique_hardware_id();
    logging_group->devices[logging_group->number_of_devices].send_character = send_character;
    logging_group->number_of_devices++;
}

void log_char(uint32_t character) {
    if(logging_enabled == false) {
        return;
    }

    if(logging_data_pointer < 1024 * 1024 * 5) {
        if(logging_data_pointer >= logging_data_size) {
            logging_data = (uint32_t *) krealloc(logging_data, sizeof(uint32_t) * (logging_data_size + 1024 * 10));
            logging_data_size += 1024 * 10;
        }
        logging_data[logging_data_pointer++] = character;
    }

    for(int i = 0; i < logging_group->number_of_devices; i++) {
        logging_group->devices[i].send_character(character);
    }
}

void log(uint8_t *string, ...) {
    LOCK_MUTEX(&logging_mutex);
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
                uint16_t character = va_arg(args, uint32_t);
                string++;

                // log character
                log_char(character);
                continue;
            }
            // get number of characters to print
            uint32_t number_of_chars_in_parameter = 0;
            if(*string == '0') {
                string++;
                number_of_chars_in_parameter = atoi(string);

                // skip numbers in string
                while(isdigit(*string) != false) {
                    string++;
                }
            }

            // log string from argument
            if(*string == 's') {
                // load argument
                uint8_t *parameter_string = va_arg(args, uint8_t *);

                // log full string
                if(number_of_chars_in_parameter == 0) {
                    while(*parameter_string != 0) {
                        log_char(*parameter_string);
                        parameter_string++;
                    }
                }
                // log part of string
                else {
                    for(uint32_t i = 0; i < number_of_chars_in_parameter; i++) {
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
                uint16_t *parameter_string = va_arg(args, uint16_t *);

                // log full string
                if(number_of_chars_in_parameter == 0) {
                    while(*parameter_string != 0) {
                        log_char(*parameter_string);
                        parameter_string++;
                    }
                }
                // log part of string
                else {
                    for(uint32_t i = 0; i < number_of_chars_in_parameter; i++) {
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
                itoa(va_arg(args, uint32_t), number_string, 11);

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
                uint32_t number = va_arg(args, uint32_t);

                // set number of digits on output
                if(number_of_chars_in_parameter == 0) {
                    number_of_chars_in_parameter = 8;
                }

                // log digits
                for(uint32_t i = 0, digit = 0, shift = (number_of_chars_in_parameter*4 - 4); i < number_of_chars_in_parameter; i++, shift -= 4) {
                    digit = ((number >> shift) & 0xF);
                    log_char((digit < 10) ? (digit + '0') : (digit + 'A' - 10));
                }

                // skip 'x' char
                string++;
            }
        }
    }

    va_end(args);
    UNLOCK_MUTEX(&logging_mutex);
}

/* userspace functions */
void logging_group_process_userspace_command(uint32_t device_id, logging_device_command_t *command) {
    if(device_id != logging_device_id) {
        return;
    }
    if(return_validated_pointer(command, sizeof(logging_device_command_t) + sizeof(uint32_t)) == NULL) {
        return;
    }

    switch(command->type) {
        case LOGGING_GROUP_COMMAND_SEND_CHARACTER: {
            uint32_t character = command->argument[0];
            log_char(character);
            break;
        }
        case LOGGING_GROUP_COMMAND_SEND_STRING: {
            uint8_t *string = (uint8_t *) command->argument[0];
            for(int i = 0; i < 10000; i++) { // max 10000 chars per string
                if(return_validated_pointer(&string[i], 1) == NULL || string[i] == 0) {
                    break;
                }
                log_char(string[i]);
            }
            break;
        }
        case LOGGING_GROUP_COMMAND_SEND_FORMATTED_STRING: {
            uint8_t *string = (uint8_t *) command->argument[0];
            uint32_t *argument = (uint32_t *) &command->argument[1];
            for(int i = 0; i < 10000; i++) { // max 10000 chars per string
                if(return_validated_pointer(&string[i], 1) == NULL || string[i] == 0) {
                    break;
                }
                uint8_t ch = string[i];

                if(ch == '%') {
                    if(return_validated_pointer(&string[i+1], 1) == NULL) {
                        break;
                    }
                    switch(string[i+1]) {
                        case '%': {
                            log_char('%');
                            i++;
                            break;
                        }
                        case 'c': {
                            if(return_validated_pointer(argument, sizeof(uint32_t)) == NULL) {
                                break;
                            }
                            log_char(*argument);
                            i++;
                            argument++;
                            break;
                        }
                        case 'd': {
                            if(return_validated_pointer(argument, sizeof(uint32_t)) == NULL) {
                                break;
                            }
                            char number_string[12]; // big enough for sign + whole number + zero ending
                            itoa(*argument, number_string, 11);
                            char *p = number_string;
                            while(*p != '\0') {
                                log_char(*p);
                                p++;
                            }
                            i++;
                            argument++;
                            break;
                        }
                        case 's': {
                            if(return_validated_pointer(argument, sizeof(uint32_t)) == NULL) {
                                break;
                            }
                            uint8_t *string = (uint8_t *) *argument;
                            for(int j = 0; j < 10000; j++) { // max 10000 chars per string
                                if(return_validated_pointer(&string[j], 1) == NULL || string[j] == 0) {
                                    break;
                                }
                                log_char(string[j]);
                            }
                            i++;
                            argument++;
                            break;
                        }
                        case 'x': {
                            if(return_validated_pointer(argument, sizeof(uint32_t)) == NULL) {
                                break;
                            }
                            uint32_t number = *argument;
                            for(uint32_t i = 0, digit = 0, shift = (8*4 - 4); i < 8; i++, shift -= 4) {
                                digit = ((number >> shift) & 0xF);
                                log_char((digit < 10) ? (digit + '0') : (digit + 'A' - 10));
                            }
                            i++;
                            argument++;
                            break;
                        }
                        default: {
                            break;
                        }
                    }
                }
                else {
                    log_char(ch);
                }
            }
            break;
        }
    }
}

void copy_log_to_userspace(uint32_t *buffer) {
    for(int i = 0; i < logging_data_pointer; i++) {
        buffer[i] = logging_data[i];
    }
    buffer[logging_data_pointer] = 0;
}