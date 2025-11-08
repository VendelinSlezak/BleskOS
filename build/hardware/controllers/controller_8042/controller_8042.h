#ifndef BUILD_HARDWARE_CONTROLLERS_CONTROLLER_8042_CONTROLLER_8042_H
#define BUILD_HARDWARE_CONTROLLERS_CONTROLLER_8042_CONTROLLER_8042_H

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
dword_t read_ps2_data(void);
dword_t write_ps2_data(byte_t value);
dword_t write_ps2_command(byte_t value);
dword_t is_controller_8042_present(void);
void initialize_controller_8042_entity(void);
void initialize_controller_8042(void);
void controller_8042_initialize_channel(dword_t (*send)(byte_t), void (**receive_ptr)(dword_t, byte_t *, dword_t), size_t controller_8042_functions_for_device[]);
dword_t controller_8042_channel_send_command(dword_t (*send)(byte_t), byte_t *command_name, byte_t command);
void controller_8042_channel_process_data(dword_t entity, byte_t *buffer, dword_t size);
dword_t controller_8042_channel_1_write_command(byte_t command);
dword_t controller_8042_channel_1_write_command_with_payload(byte_t command, byte_t payload);
dword_t controller_8042_channel_2_write_command(byte_t command);
dword_t controller_8042_channel_2_write_command_with_payload(byte_t command, byte_t payload);
dword_t controller_8042_channel_1_send_command_universal(byte_t command, dword_t is_payload, byte_t payload, dword_t is_response);
void controller_8042_channel_1_command_response(dword_t entity, byte_t *buffer, dword_t size);
void controller_8042_channel_1_set_receive_function(dword_t entity, void receive(dword_t, byte_t *, dword_t));
dword_t controller_8042_channel_1_send_command(byte_t command);
dword_t controller_8042_channel_1_send_command_with_payload(byte_t command, byte_t payload);
dword_t controller_8042_channel_1_send_command_with_return(byte_t command);
dword_t controller_8042_channel_1_send_command_with_payload_and_return(byte_t command, byte_t payload);
dword_t controller_8042_channel_2_send_command_universal(byte_t command, dword_t is_payload, byte_t payload, dword_t is_response);
void controller_8042_channel_2_command_response(dword_t entity, byte_t *buffer, dword_t size);
void controller_8042_channel_2_set_receive_function(dword_t entity, void receive(dword_t, byte_t *, dword_t));
dword_t controller_8042_channel_2_send_command(byte_t command);
dword_t controller_8042_channel_2_send_command_with_payload(byte_t command, byte_t payload);
dword_t controller_8042_channel_2_send_command_with_return(byte_t command);
dword_t controller_8042_channel_2_send_command_with_payload_and_return(byte_t command, byte_t payload);
void controller_8042_irq(void);

#endif /* BUILD_HARDWARE_CONTROLLERS_CONTROLLER_8042_CONTROLLER_8042_H */
