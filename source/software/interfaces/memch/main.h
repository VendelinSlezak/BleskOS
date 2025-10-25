/* 
* BleskOS
*
* MIT License
* Copyright (c) 2023-2025 BleskOS developers
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#ifndef MEMCH_MAIN_H
#define MEMCH_MAIN_H

#include <kernel/x86/libc/stdio.h>

enum {
    MEMCH_STATE_PROCESSING,
    MEMCH_STATE_SUCCESS,
    MEMCH_STATE_ERROR
};
typedef struct {
    dword_t state;
    dword_t state_value;
} memch_program_state_t;

enum {
    MEMCH_INPUT_TYPE_ALLOCATION,
    MEMCH_INPUT_TYPE_STREAM
};
typedef struct {
    dword_t type;

    void *type_struct;
    dword_t type_struct_length;
    dword_t pointer;

    STREAM_t *s_ptr;
} memch_input_t;

enum {
    MEMCH_OUTPUT_TYPE_ALLOCATION,
    MEMCH_OUTPUT_TYPE_STREAM
};
typedef struct {
    dword_t type;
    void *type_struct;
    dword_t type_struct_length;
    dword_t pointer;

    STREAM_t *s_ptr;
    STREAM_t s;
} memch_output_t;

typedef struct {
    memch_program_state_t *state;
    memch_output_t *output;
} memch_program_output_t;

enum {
    MEMCH_INPUT_ALLOCATION,
    MEMCH_INPUT_STREAM,
    MEMCH_INPUT_MEMCH_OUTPUT,
};
enum {
    MEMCH_OUTPUT_EXISTING_ALLOCATION,
    MEMCH_OUTPUT_EXISTING_STREAM,
    MEMCH_OUTPUT_NEW_ALLOCATION,
    MEMCH_OUTPUT_NEW_STREAM,
    MEMCH_OUTPUT_FROM_PARAMETERS
};
typedef struct {
    memch_program_state_t *state;
    void *parameters;
    memch_input_t *input;
    memch_output_t *output;
} memch_program_data_t;

typedef struct {
    char *name;
    void (*program)(memch_program_data_t *);
    dword_t does_have_parameters;
    dword_t output_type;
} memch_program_list_entry_t;

typedef struct {
    void (*program)(memch_program_data_t *, memch_input_t *, memch_output_t *);
    memch_program_state_t *state_info;
    memch_input_t *input;
    memch_output_t *output;
} e_memch_program_attr_t;

#endif