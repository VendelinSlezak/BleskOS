//BleskOS

/*
* MIT License
* Copyright (c) 2023-2025 BleskOS developers
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include <stdarg.h>

#define NUM_OF_CHARACTERS_IN_DDBG 1024*100

#define DDBG_IS_COMMAND(position, name_of_command) (command_section_type[position] == 's' && is_memory_equal_with_memory(command_sections[position], name_of_command, sizeof(name_of_command)-1) == STATUS_TRUE)

struct ddbg_value_parsing_info_t {
 byte_t number_of_taken_bits;
 byte_t *label;
 char type_of_output;
};

byte_t *deep_debugger_memory;

dword_t first_showed_character_offset, first_showed_line, num_of_characters, num_of_lines;

void deep_debugger(void);
void deep_debugger_redraw(void);
void clear_ddbg(void);
void ddbg_printf(byte_t *string, ...);
void ddbg_print_parsed_mem(byte_t *value_label, struct ddbg_value_parsing_info_t *parsing_info, dword_t value);