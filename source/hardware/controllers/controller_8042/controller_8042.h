/* 
* BleskOS
*
* MIT License
* Copyright (c) 2023-2025 BleskOS developers
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include <kernel/x86/scheduler/lock.h>

#define CONTROLLER_8042_CHANNEL_INTERRUPT_MAX_DATA 10
#define CONTROLLER_8042_BUFFER_SIZE 100
typedef enum {
    NOT_KNOWN,
    NOT_PRESENT,
    PRESENT,
    PS2_KEYBOARD,
    PS2_MOUSE,
    UNKNOWN_DEVICE
} ps2_channel_state_t;
typedef struct {
    mutex_t sending_command;

    ps2_channel_state_t channel_1;
    dword_t channel_1_entity;
    dword_t channel_1_buffer_num_of_bytes;
    byte_t channel_1_buffer[CONTROLLER_8042_CHANNEL_INTERRUPT_MAX_DATA];
    void (*channel_1_process_data)(dword_t entity, byte_t *buffer, dword_t size);
    dword_t channel_1_device_response_state;
    dword_t channel_1_device_response_data;

    ps2_channel_state_t channel_2;
    dword_t channel_2_entity;
    dword_t channel_2_buffer_num_of_bytes;
    byte_t channel_2_buffer[CONTROLLER_8042_CHANNEL_INTERRUPT_MAX_DATA];
    void (*channel_2_process_data)(dword_t entity, byte_t *buffer, dword_t size);
    dword_t channel_2_device_response_state;
    dword_t channel_2_device_response_data;

    dword_t buffer_ptr;
    byte_t buffer[CONTROLLER_8042_BUFFER_SIZE];
} e_controller_8042_attr_t;