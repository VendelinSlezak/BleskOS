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

#include <kernel/x86/entities/entities.h>
#include <kernel/x86/memory/vm_allocator.h>
#include <kernel/x86/libc/stdio.h>
#include <kernel/x86/libc/string.h>

#include <software/ramdisk_programs/convBMPtoI/main.h>
#include <software/ramdisk_programs/internal_image/main.h>

/* local variables */
memch_program_list_entry_t memch_program_list[] = {
    { "convBMPtoI", conv_bmp_to_i, FALSE, MEMCH_OUTPUT_NEW_STREAM },
    { "drawIonScreen", display_internal_image, TRUE, MEMCH_OUTPUT_EXISTING_ALLOCATION }
};

/* functions */
// void memch_run(char *name, memch_program_info_t *state_info, memch_input_t *input, memch_output_t *output) {
//     for(int i = 0; i < sizeof(memch_program_list) / sizeof(memch_program_list_entry_t); i++) {
//         if(strcmp(memch_program_list[i].name, name) == 0) {
            // // reset
            // memch_set_state(state_info, MEMCH_STATE_PROCESSING, 0);
            // memch_input_seek(input, 0, SEEK_SET);
            // memch_output_seek(output, 0, SEEK_SET);

            // // run program
            // void *functions[] = { (void *)&memch_wrapper };
            // dword_t entity_number = create_entity(memch_program_list[i].name, E_TYPE_KERNEL_RING, 1, functions);
            // e_memch_program_attr_t *attr = entity_get_attr_ptr(entity_number);
            // attr->program = memch_program_list[i].program;
            // attr->state_info = state_info;
            // attr->input = input;
            // attr->output = output;
            // create_command_without_ca_with_fin(entity_number, 0, memch_finalizer);
//             return;
//         }
//     }

//     state_info->state = MEMCH_STATE_ERROR;
//     state_info->state_value = INVALID;
// }

// void memch_wrapper(void) {
//     command_info_t *command_info = (command_info_t *) P_MEM_COMMAND_INFO;
//     e_memch_program_attr_t *attr = command_info->entity_attributes;
//     attr->program(attr->state_info, attr->input, attr->output);
// }

void memch_finalizer(void) {
    // TODO: free all allocated structures
}

memch_program_output_t memch_run_and_complete(char *name, ...) {
    memch_program_output_t program_output;
    memch_program_state_t state;
    program_output.state = &state;

    for(int i = 0; i < sizeof(memch_program_list) / sizeof(memch_program_list_entry_t); i++) {
        if(strcmp(memch_program_list[i].name, name) == 0) {
            // create program structures
            va_list args;
            va_start(args, name);

            program_output.state->state = MEMCH_STATE_PROCESSING;
            program_output.state->state_value = INVALID;

            void *parameters = (void *) INVALID;
            if(memch_program_list[i].does_have_parameters == TRUE) {
                parameters = va_arg(args, void *);
            }
            dword_t input_type = va_arg(args, int);
            memch_input_t input;
            if(input_type == MEMCH_INPUT_ALLOCATION) {
                input.type = MEMCH_INPUT_TYPE_ALLOCATION;
                input.type_struct = va_arg(args, void *);
                input.type_struct_length = va_arg(args, dword_t);
                input.pointer = 0;
            }
            else if(input_type == MEMCH_INPUT_STREAM) {
                input.type = MEMCH_INPUT_TYPE_STREAM;
                input.s_ptr = va_arg(args, STREAM_t *);
                sseek(input.s_ptr, 0, SEEK_SET);
            }
            else if(input_type == MEMCH_INPUT_MEMCH_OUTPUT) {
                memch_program_output_t memch_output = va_arg(args, memch_program_output_t);
                if(memch_output.output->type == MEMCH_OUTPUT_TYPE_ALLOCATION) {
                    input.type = MEMCH_INPUT_TYPE_ALLOCATION;
                    input.type_struct = memch_output.output->type_struct;
                    input.type_struct_length = memch_output.output->type_struct_length;
                    input.pointer = 0;
                }
                else if(memch_output.output->type == MEMCH_OUTPUT_TYPE_STREAM) {
                    input.type = MEMCH_INPUT_TYPE_STREAM;
                    input.s_ptr = memch_output.output->s_ptr;
                    sseek(input.s_ptr, 0, SEEK_SET);
                }
            }
            memch_output_t output;
            if(memch_program_list[i].output_type == MEMCH_OUTPUT_EXISTING_ALLOCATION) {
                output.type = MEMCH_OUTPUT_TYPE_ALLOCATION;
                output.type_struct = va_arg(args, void *);
                output.type_struct_length = va_arg(args, dword_t);
                output.pointer = 0;
            }
            else if(memch_program_list[i].output_type == MEMCH_OUTPUT_EXISTING_STREAM) {
                output.type = MEMCH_OUTPUT_TYPE_STREAM;
                output.s_ptr = (STREAM_t *) &output.s;
                memcpy(&output.s, va_arg(args, STREAM_t *), sizeof(STREAM_t));
            }
            else if(memch_program_list[i].output_type == MEMCH_OUTPUT_NEW_ALLOCATION) {
                output.type = MEMCH_OUTPUT_TYPE_ALLOCATION;
                output.type_struct_length = va_arg(args, dword_t);
                output.type_struct = kcalloc(output.type_struct_length);
                output.pointer = 0;
            }
            else if(memch_program_list[i].output_type == MEMCH_OUTPUT_NEW_STREAM) {
                output.type = MEMCH_OUTPUT_TYPE_STREAM;
                output.s_ptr = (STREAM_t *) &output.s;
                output.s = open_rwstream();
            }
            else if(memch_program_list[i].output_type == MEMCH_OUTPUT_FROM_PARAMETERS) {
                // TODO:
            }

            // create program data
            memch_program_data_t program_data;
            program_data.state = &state;
            program_data.parameters = parameters;
            program_data.input = &input;
            program_data.output = &output;

            // load program structures to output
            program_output.output = &output;

            // run program
            memch_program_list[i].program(&program_data);

            va_end(args);
            return program_output;
        }
    }

    program_output.state->state = MEMCH_STATE_ERROR;
    program_output.state->state_value = INVALID;
    return program_output;
}

void set_memch_input_allocation(memch_input_t *input, dword_t size_of_allocation, void *allocation) {
    input->type = MEMCH_INPUT_TYPE_ALLOCATION;
    input->type_struct = allocation;
    input->type_struct_length = size_of_allocation;
    input->pointer = 0;
}

void set_memch_input_stream(memch_input_t *input, STREAM_t *stream) {
    input->type = MEMCH_INPUT_TYPE_STREAM;
    input->type_struct = stream;
}

void *get_memch_input_ptr(memch_program_data_t *data) {
    memch_input_t *input = data->input;

    switch(input->type) {
        case(MEMCH_INPUT_TYPE_ALLOCATION):
            return input->type_struct + input->pointer;
        case(MEMCH_INPUT_TYPE_STREAM):
            return (void *) stell(input->s_ptr);
    }
    
    return (void *) INVALID;
}

void set_memch_output_allocation(memch_output_t *output, dword_t size_of_allocation, void *allocation) {
    output->type = MEMCH_OUTPUT_TYPE_ALLOCATION;
    output->type_struct = allocation;
    output->type_struct_length = size_of_allocation;
    output->pointer = 0;
}

void set_memch_output_stream(memch_output_t *output) {
    output->type = MEMCH_OUTPUT_TYPE_STREAM;
    output->s = open_rwstream();
}

void *get_memch_program_output_ptr(memch_program_output_t *output) {
    switch(output->output->type) {
        case(MEMCH_OUTPUT_TYPE_ALLOCATION):
            return output->output->type_struct;
        case(MEMCH_OUTPUT_TYPE_STREAM):
            return output->output->s_ptr->buffer;
    }
    
    return (void *) INVALID;
}

// TODO: save info for finalizer
void *memch_alloc(dword_t size) {
    return kcalloc(size);
}
void memch_free(void *ptr) {
    free(ptr);
}

void memch_set_state(memch_program_data_t *data, dword_t state, dword_t state_value) {
    memch_program_state_t *state_info = data->state;
    state_info->state = state;
    state_info->state_value = state_value;
}

dword_t memch_input_read(memch_program_data_t *data, void *ptr, dword_t size) {
    memch_input_t *input = data->input;

    if(input->type == MEMCH_INPUT_TYPE_ALLOCATION) {
        if(input->pointer + size > input->type_struct_length) {
            return ERROR;
        }
        memcpy(ptr, input->type_struct + input->pointer, size);
        input->pointer += size;
        return SUCCESS;
    }
    else if(input->type == MEMCH_INPUT_TYPE_STREAM) {

    }

    return ERROR;
}

dword_t memch_input_seek(memch_program_data_t *data, int offset, dword_t type) {
    memch_input_t *input = data->input;

    if(input->type == MEMCH_INPUT_TYPE_ALLOCATION) {
        if(type == SEEK_CUR) {
            if(input->pointer + offset > input->type_struct_length) {
                return ERROR;
            }
            input->pointer += offset;
            return SUCCESS;
        }
        else if(type == SEEK_SET) {
            if(offset > input->type_struct_length) {
                return ERROR;
            }
            input->pointer = offset;
            return SUCCESS;
        }

        return ERROR;
    }
    else if(input->type == MEMCH_INPUT_TYPE_STREAM) {

    }

    return ERROR;
}

dword_t memch_input_has_size(memch_program_data_t *data, dword_t size) {
    memch_input_t *input = data->input;

    if(input->type == MEMCH_INPUT_TYPE_ALLOCATION) {
        if(input->pointer + size < input->type_struct_length) {
            return SUCCESS;
        }
        else {
            return ERROR;
        }
    }
    else if(input->type == MEMCH_INPUT_TYPE_STREAM) {

    }

    return ERROR;
}

void *memch_input_get_ptr(memch_program_data_t *data) {
    memch_input_t *input = data->input;

    if(input->type == MEMCH_INPUT_TYPE_ALLOCATION) {
        return input->type_struct;
    }
    else if(input->type == MEMCH_INPUT_TYPE_STREAM) {
        return input->s_ptr->buffer;
    }

    return (void *) INVALID;
}

dword_t memch_output_write(memch_program_data_t *data, void *ptr, dword_t size) {
    memch_output_t *output = data->output;

    if(output->type == MEMCH_OUTPUT_TYPE_ALLOCATION) {
        if(output->pointer + size > output->type_struct_length) {
            return ERROR;
        }
        memcpy(output->type_struct + output->pointer, ptr, size);
        output->pointer += size;
        return SUCCESS;
    }
    else if(output->type == MEMCH_OUTPUT_TYPE_STREAM) {
        swrite(ptr, size, 1, &output->s);
        return SUCCESS;
    }

    return ERROR;
}

void *memch_output_get_ptr(memch_program_data_t *data) {
    memch_output_t *output = data->output;

    if(output->type == MEMCH_OUTPUT_TYPE_ALLOCATION) {
        return output->type_struct;
    }
    else if(output->type == MEMCH_OUTPUT_TYPE_STREAM) {
        return output->s.buffer;
    }

    return (void *) INVALID;
}

dword_t memch_output_seek(memch_program_data_t *data, int offset, dword_t type) {
    memch_output_t *output = data->output;

    if(output->type == MEMCH_OUTPUT_TYPE_ALLOCATION) {
        if(type == SEEK_CUR) {
            if(output->pointer + offset > output->type_struct_length) {
                return ERROR;
            }
            output->pointer += offset;
            return SUCCESS;
        }
        else if(type == SEEK_SET) {
            if(offset > output->type_struct_length) {
                return ERROR;
            }
            output->pointer = offset;
            return SUCCESS;
        }

        return ERROR;
    }
    else if(output->type == MEMCH_OUTPUT_TYPE_STREAM) {
        dword_t state = sseek(&output->s, offset, type);
        if(state == 0) {
            return SUCCESS;
        }
        else {
            return ERROR;
        }
    }

    return ERROR;
}

dword_t memch_output_prepare_size(memch_program_data_t *data, dword_t size) {
    memch_output_t *output = data->output;

    if(output->type == MEMCH_OUTPUT_TYPE_ALLOCATION) {
        if(output->pointer + size > output->type_struct_length) {
            return ERROR;
        }
        else {
            return SUCCESS;
        }
    }
    else if(output->type == MEMCH_OUTPUT_TYPE_STREAM) {
        s_grow(&output->s, size);
        return SUCCESS;
    }

    return ERROR;
}