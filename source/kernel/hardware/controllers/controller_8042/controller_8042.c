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
#include <kernel/hardware/groups/logging/logging.h>
#include <kernel/hardware/devices/memory/memory_allocators.h>
#include <kernel/hardware/devices/cpu/commands.h>
#include <kernel/hardware/devices/cpu/interrupt.h>
#include <kernel/hardware/devices/cpu/scheduler.h>
#include <kernel/hardware/devices/timers/main.h>
#include <kernel/hardware/main.h>
#include <kernel/hardware/devices/human_input/ps2_keyboard.h>
#include <kernel/hardware/devices/human_input/ps2_mouse.h>
#include <kernel/hardware/devices/cpu/mutex.h>

/* local variables */
hardware_t *controller_8042;
static controller_8042_communication_functions_t controller_8042_communication_functions_channel_1 = {
    controller_8042_channel_1_set_receive_function,
    controller_8042_channel_1_send_command,
    controller_8042_channel_1_send_command_with_payload,
    controller_8042_channel_1_send_command_with_return,
    controller_8042_channel_1_send_command_with_payload_and_return
};
static controller_8042_communication_functions_t controller_8042_communication_functions_channel_2 = {
    controller_8042_channel_2_set_receive_function,
    controller_8042_channel_2_send_command,
    controller_8042_channel_2_send_command_with_payload,
    controller_8042_channel_2_send_command_with_return,
    controller_8042_channel_2_send_command_with_payload_and_return
};

/* functions for communication with controller */
uint32_t read_ps2_data(void) {
    for(int i = 0; i < 100; i++) {
        if((inb(0x64) & 0x1) == 0x1) {
            return inb(0x60);
        }
    }

    uint64_t time = get_time_in_microseconds();
    while((int)(get_time_in_microseconds() - time) < 200000) { // wait max 200ms
        if((inb(0x64) & 0x1) == 0x1) {
            return inb(0x60);
        }
    }

    return INVALID;
}

uint32_t write_ps2_data(uint8_t value) {
    for(int i = 0; i < 100; i++) {
        if((inb(0x64) & 0x2) == 0x0) {
            outb(0x60, value);
            return SUCCESS;
        }
    }

    uint64_t time = get_time_in_microseconds();
    while((int)(get_time_in_microseconds() - time) < 200000) { // wait max 200ms
        if((inb(0x64) & 0x2) == 0x0) {
            outb(0x60, value);
            return SUCCESS;
        }
    }
    
    return ERROR;
}

uint32_t write_ps2_command(uint8_t value) {
    for(int i = 0; i < 100; i++) {
        if((inb(0x64) & 0x2) == 0x0) {
            outb(0x64, value);
            return SUCCESS;
        }
    }
    
    uint64_t time = get_time_in_microseconds();
    while((int)(get_time_in_microseconds() - time) < 200000) { // wait max 200ms
        if((inb(0x64) & 0x2) == 0x0) {
            outb(0x64, value);
            return SUCCESS;
        }
    }

    return ERROR;
}

/* functions for controller initialization */
uint32_t is_controller_8042_present(void) {
    return true; // TODO: add checking
}

void initialize_controller_8042(hardware_t *self) {
    if(is_controller_8042_present() == false) {
        return;
    }
    controller_8042 = self;
    controller_8042->data = kalloc(sizeof(controller_8042_data_t));
    controller_8042_data_t *data = controller_8042->data;
    data->channel_1 = NOT_KNOWN;
    data->channel_2 = NOT_KNOWN;
    log("\n[8042C] Initializing...");

    // disable PS/2 channels
    write_ps2_command(0xAD);
    write_ps2_command(0xA7);

    // clear buffer of data port
    for(uint32_t i = 0; i < 1000; i++) {
        if((inb(0x64) & 0x1) == 0x1) { // there are data on data port
            inb(0x60);
        }
        else {
            break;
        }
    }
    if((inb(0x64) & 0x1) == 0x1) { // error: data port was not cleared
        log("\n[8042C] ERROR: locked data port");
        data->channel_1 = NOT_PRESENT;
        data->channel_2 = NOT_PRESENT;
        self->is_initialized = true;
        return;
    }

    // disable interrupts on both channels
    write_ps2_command(0x20);
    uint8_t configuration_byte = read_ps2_data();
    write_ps2_command(0x60);
    write_ps2_data(configuration_byte & ~((1 << 0) | (1 << 1)));

    // check presence of channels
    if((configuration_byte & (1 << 4)) == 0) { // first channel should be disabled, if it is not, it can not exist
        data->channel_1 = NOT_PRESENT;
    }
    if((configuration_byte & (1 << 5)) == 0) { // second channel should be disabled, if it is not, it can not exist
        data->channel_2 = NOT_PRESENT;
    }

    // perform controller self-test
    write_ps2_command(0xAA);
    if(read_ps2_data() == 0x55) { // self-test succesfull
        log("\n[8042C] Self-test successfull");
    }
    else { // self-test failed
        log("\n[8042C] ERROR: Self-test failed");
        data->channel_1 = NOT_PRESENT;
        data->channel_2 = NOT_PRESENT;
        self->is_initialized = true;
        return;
    }

    // enable PS/2 channels
    write_ps2_command(0xAE);
    write_ps2_command(0xA8);

    // check presence of channels
    write_ps2_command(0x20);
    configuration_byte = read_ps2_data();
    if(data->channel_1 == NOT_KNOWN) { // we do not know first channel presence yet
        if((configuration_byte & (1 << 4)) == 0) {
            data->channel_1 = PRESENT; // first channel is enabled, so it exist
        }
        else {
            data->channel_1 = NOT_PRESENT; // first channel is disabled, so it do not exist
        }
    }
    if(data->channel_2 == NOT_KNOWN) { // we do not know second channel presence yet
        if((configuration_byte & (1 << 5)) == 0) {
            data->channel_2 = PRESENT; // second channel is enabled, so it exist
        }
        else {
            data->channel_2 = NOT_PRESENT; // second channel is disabled, so it do not exist
        }
    }

    // disable PS/2 channels
    write_ps2_command(0xAD);
    write_ps2_command(0xA7);

    // test first channel
    if(data->channel_1 == PRESENT) {
        write_ps2_command(0xAB);
        if(read_ps2_data() == 0x00) { // test successfull
            log("\n[8042C] First channel test successfull");
        }
        else { // test failed
            log("\n[8042C] First channel test failed");
            data->channel_1 = UNKNOWN_DEVICE;
        }
    }

    // test second channel
    if(data->channel_2 == PRESENT) {
        write_ps2_command(0xA9);
        if(read_ps2_data() == 0x00) { // test successfull
            log("\n[8042C] Second channel test successfull");
        }
        else { // test failed
            log("\n[8042C] Second channel test failed");
            data->channel_2 = UNKNOWN_DEVICE;
        }
    }

    // enable interrupts
    write_ps2_command(0x20);
    configuration_byte = read_ps2_data();
    if(data->channel_1 == PRESENT) {
        set_isa_interrupt_handler(1, controller_8042_irq);
        configuration_byte |= (1 << 0); // enable interrupt 1
        configuration_byte &= ~(1 << 6); // disable translation to scan code set 1
    }
    if(data->channel_2 == PRESENT) {
        set_isa_interrupt_handler(12, controller_8042_irq);
        configuration_byte |= (1 << 1); // enable interrupt 12
    }
    write_ps2_command(0x60);
    write_ps2_data(configuration_byte);

    // initialize first channel
    if(data->channel_1 == PRESENT) {
        log("\n[8042C] Initializing device on first channel");
        write_ps2_command(0xAE);
        controller_8042_initialize_channel(
            &data->channel_1_device_structure,
            controller_8042_channel_1_write_command,
            &data->channel_1_process_data,
            &controller_8042_communication_functions_channel_1
        );
    }

    // initialize second channel
    if(data->channel_2 == PRESENT) {
        log("\n[8042C] Initializing device on second channel");
        write_ps2_command(0xA8);
        controller_8042_initialize_channel(
            &data->channel_2_device_structure,
            controller_8042_channel_2_write_command,
            &data->channel_2_process_data,
            &controller_8042_communication_functions_channel_2
        );
    }

    self->is_initialized = true;
}

void controller_8042_initialize_channel(hardware_t **device_structure, uint32_t (*send)(uint8_t), void (**receive_ptr)(hardware_t *, uint8_t *, uint32_t), controller_8042_communication_functions_t *communication_functions) {
    controller_8042_data_t *data = controller_8042->data;
    
    // set receive method
    *receive_ptr = controller_8042_channel_process_data;

    // reset device
    controller_8042_channel_send_command(send, "Reset", 0xFF);
    if(data->buffer[1] != 0xAA) {
        log("\n[8042C] ERROR: Reset was not successfull");
        return;
    }
    log("\n[8042C] Device was successfully resetted");

    // get device ID
    controller_8042_channel_send_command(send, "Get Device ID", 0xF2);
    sleep_current_thread(10); // wait for all bytes to arrive
    log("\n[8042C] Device ID:");
    for(int i = 1; i < data->buffer_ptr; i++) {
        log(" 0x%02x", data->buffer[i]);
    }

    // connect device to driver
    log("\n[8042C] Device type: ");
    if(    data->buffer[1] == 0xAB
        || data->buffer[1] == 0xAC) {
        log("Keyboard");
        *device_structure = add_hardware(
            controller_8042,
            "PS/2 keyboard",
            communication_functions,
            NULL,
            initialize_ps2_keyboard,
            NULL
        );
        synchronous_init_hardware(*device_structure);
    }
    else if(   data->buffer[1] == 0x00
            || data->buffer[1] == 0x03
            || data->buffer[1] == 0x04) {
        log("Mouse");
        *device_structure = add_hardware(
            controller_8042,
            "PS/2 mouse",
            communication_functions,
            NULL,
            initialize_ps2_mouse,
            NULL
        );
        synchronous_init_hardware(*device_structure);
    }
    else {
        log("Unknown");
    }
}

uint32_t controller_8042_channel_send_command(uint32_t (*send)(uint8_t), uint8_t *command_name, uint8_t command) {
    controller_8042_data_t *data = controller_8042->data;

    // send command
    data->buffer_ptr = 0;
    send(command);

    // wait for response
    uint64_t time = get_time_in_microseconds();
    while((int)(get_time_in_microseconds() - time) < 1000000) { // 1s
        asm volatile("pause");
        if(((volatile uint32_t)data->buffer_ptr) < 2) {
            switch_to_another_thread();
            continue;
        }
        else {
            break;
        }
    }

    // check errors
    if(((volatile uint32_t)data->buffer_ptr) < 2) {
        log("\n[8042C] ERROR: Timeout");
        return ERROR;
    }
    if(data->buffer[0] != 0xFA) {
        log("\n[8042C] ERROR: %s not acknowledged", command_name);
        return ERROR;
    }
    if(data->buffer_ptr < 2) {
        log("\n[8042C] ERROR: %s did return too little data", command_name);
        return ERROR;
    }

    return SUCCESS;
}

void controller_8042_channel_process_data(hardware_t *structure, uint8_t *buffer, uint32_t size) {
    controller_8042_data_t *data = controller_8042->data;
    
    if(data->buffer_ptr >= CONTROLLER_8042_BUFFER_SIZE) {
        return;
    }

    while(size--) {
        data->buffer[data->buffer_ptr++] = *buffer++;
    }
}

uint32_t controller_8042_channel_1_write_command(uint8_t command) {
    controller_8042_data_t *data = controller_8042->data;
    LOCK_MUTEX(&data->sending_command);
    uint32_t state = write_ps2_data(command);
    UNLOCK_MUTEX(&data->sending_command);
    return state;
}

uint32_t controller_8042_channel_1_write_command_with_payload(uint8_t command, uint8_t payload) {
    controller_8042_data_t *data = controller_8042->data;
    LOCK_MUTEX(&data->sending_command);
    if(write_ps2_data(command) == ERROR) {
        UNLOCK_MUTEX(&data->sending_command);
        return ERROR;
    }
    uint32_t state = write_ps2_data(payload);
    UNLOCK_MUTEX(&data->sending_command);
    return state;
}

uint32_t controller_8042_channel_2_write_command(uint8_t command) {
    controller_8042_data_t *data = controller_8042->data;
    LOCK_MUTEX(&data->sending_command);
    if(write_ps2_command(0xD4) == ERROR) {
        UNLOCK_MUTEX(&data->sending_command);
        return ERROR;
    }
    uint32_t state = write_ps2_data(command);
    UNLOCK_MUTEX(&data->sending_command);
    return state;
}

uint32_t controller_8042_channel_2_write_command_with_payload(uint8_t command, uint8_t payload) {
    if(controller_8042_channel_2_write_command(command) == ERROR) {
        return ERROR;
    }
    return controller_8042_channel_2_write_command(payload);
}

/* functions for communication with devices */
uint32_t controller_8042_channel_1_send_command_universal(uint8_t command, uint32_t is_payload, uint8_t payload, uint32_t is_response) {
    controller_8042_data_t *data = controller_8042->data;
    data->channel_1_process_data = controller_8042_channel_1_command_response;

    // retry sending command max 3 times
    for(int i = 0; i < 3; i++) {
        // send command
        data->channel_1_device_response_state = INVALID;
        data->channel_1_device_response_data = INVALID;
        if(is_payload == false) {
            if(controller_8042_channel_1_write_command(command) == ERROR) {
                log("\n[8042C] ERROR: Controller is busy");
                return INVALID;
            }
        }
        else {
            if(controller_8042_channel_1_write_command_with_payload(command, payload) == ERROR) {
                log("\n[8042C] ERROR: Controller is busy");
                return INVALID;
            }
        }

        // wait for response
        uint64_t time = get_time_in_microseconds() ;
        while((int)(get_time_in_microseconds() - time) < 100000) { // 100ms
            asm volatile("pause");
            if(((volatile uint32_t)data->channel_1_device_response_state) == INVALID) {
                switch_to_another_thread();
                continue;
            }
            else {
                if(is_response == true && data->channel_1_device_response_data == INVALID) {
                    switch_to_another_thread();
                    continue;
                }
                break;
            }
        }
        if(((volatile uint32_t)data->channel_1_device_response_state) == INVALID) {
            log("\n[8042C] ERROR: Timeout on channel 1 command");
            return INVALID;
        }

        // process response
        if(data->channel_1_device_response_state == 0xFA) {
            if(is_response == false) {
                return SUCCESS;
            }
            else {
                return data->channel_1_device_response_data;
            }
        }
        else if(data->channel_1_device_response_state != 0xFE) {
            log("\n[8042C] ERROR: Channel 1 command returned 0x%02x", data->channel_1_device_response_state);
            return INVALID;
        }
    }

    log("\n[8042C] ERROR: Channel 1 request command resend more than 3 times");
    return INVALID;
}

void controller_8042_channel_1_command_response(hardware_t *structure, uint8_t *buffer, uint32_t size) {
    controller_8042_data_t *data = controller_8042->data;
    while(size--) {
        if(*buffer == 0xFA) { // byte accepted, one interrupt can send multiple of those
            data->channel_1_device_response_state = 0xFA;
        }
        else {
            if(*buffer > 0xFA) { // error code
                data->channel_1_device_response_state = *buffer; // return what error code happened
                data->channel_1_device_response_data = INVALID;
            }
            else { // return value
                data->channel_1_device_response_data = *buffer;
            }
            return;
        }

        buffer++;
    }
}

void controller_8042_channel_1_set_receive_function(hardware_t *structure, void receive(hardware_t *, uint8_t *, uint32_t)) {
    controller_8042_data_t *data = controller_8042->data;
    data->channel_1_device_structure = structure;
    data->channel_1_process_data = receive;
}

uint32_t controller_8042_channel_1_send_command(uint8_t command) {
    controller_8042_data_t *data = controller_8042->data;
    void *structure = data->channel_1_device_structure;
    void *receive_function = data->channel_1_process_data;

    uint32_t state = controller_8042_channel_1_send_command_universal(command, false, 0, false);

    data->channel_1_device_structure = structure;
    data->channel_1_process_data = receive_function;

    if(state == INVALID) {
        return ERROR;
    }
    else {
        return SUCCESS;
    }
}

uint32_t controller_8042_channel_1_send_command_with_payload(uint8_t command, uint8_t payload) {
    controller_8042_data_t *data = controller_8042->data;
    void *structure = data->channel_1_device_structure;
    void *receive_function = data->channel_1_process_data;

    uint32_t state = controller_8042_channel_1_send_command_universal(command, true, payload, false);

    data->channel_1_device_structure = structure;
    data->channel_1_process_data = receive_function;

    if(state == INVALID) {
        return ERROR;
    }
    else {
        return SUCCESS;
    }
}

uint32_t controller_8042_channel_1_send_command_with_return(uint8_t command) {
    controller_8042_data_t *data = controller_8042->data;
    void *structure = data->channel_1_device_structure;
    void *receive_function = data->channel_1_process_data;

    uint32_t state = controller_8042_channel_1_send_command_universal(command, false, 0, true);

    data->channel_1_device_structure = structure;
    data->channel_1_process_data = receive_function;

    return state;
}

uint32_t controller_8042_channel_1_send_command_with_payload_and_return(uint8_t command, uint8_t payload) {
    controller_8042_data_t *data = controller_8042->data;
    void *structure = data->channel_1_device_structure;
    void *receive_function = data->channel_1_process_data;

    uint32_t state = controller_8042_channel_1_send_command_universal(command, true, payload, true);

    data->channel_1_device_structure = structure;
    data->channel_1_process_data = receive_function;

    return state;
}

uint32_t controller_8042_channel_2_send_command_universal(uint8_t command, uint32_t is_payload, uint8_t payload, uint32_t is_response) {
    controller_8042_data_t *data = controller_8042->data;
    data->channel_2_process_data = controller_8042_channel_2_command_response;

    // retry sending command max 3 times
    for(int i = 0; i < 3; i++) {
        // send command
        data->channel_2_device_response_state = INVALID;
        data->channel_2_device_response_data = INVALID;
        if(is_payload == false) {
            if(controller_8042_channel_2_write_command(command) == ERROR) {
                log("\n[8042C] ERROR: Controller is busy");
                return INVALID;
            }
        }
        else {
            if(controller_8042_channel_2_write_command_with_payload(command, payload) == ERROR) {
                log("\n[8042C] ERROR: Controller is busy");
                return INVALID;
            }
        }

        // wait for response
        uint64_t time = get_time_in_microseconds();
        while((int)(get_time_in_microseconds() - time) < 100000) { // 100ms
            asm volatile("pause");
            if(((volatile uint32_t)data->channel_2_device_response_state) == INVALID) {
                switch_to_another_thread();
                continue;
            }
            else {
                if(is_response == true && data->channel_2_device_response_data == INVALID) {
                    switch_to_another_thread();
                    continue;
                }
                break;
            }
        }
        if(((volatile uint32_t)data->channel_2_device_response_state) == INVALID) {
            log("\n[8042C] ERROR: Timeout on channel 2 command");
            return INVALID;
        }

        // process response
        if(data->channel_2_device_response_state == 0xFA) {
            if(is_response == false) {
                return SUCCESS;
            }
            else {
                return data->channel_2_device_response_data;
            }
        }
        else if(data->channel_2_device_response_state != 0xFE) {
            log("\n[8042C] ERROR: Channel 2 command returned 0x%02x", data->channel_2_device_response_state);
            return INVALID;
        }
    }

    log("\n[8042C] ERROR: Channel 2 request command resend more than 3 times");
    return INVALID;
}

void controller_8042_channel_2_command_response(hardware_t *structure, uint8_t *buffer, uint32_t size) {
    controller_8042_data_t *data = controller_8042->data;
    while(size--) {
        if(*buffer == 0xFA) { // byte accepted, one interrupt can send multiple of those
            data->channel_2_device_response_state = 0xFA;
        }
        else {
            if(*buffer > 0xFA) { // error code
                data->channel_2_device_response_state = *buffer; // return what error code happened
                data->channel_2_device_response_data = INVALID;
            }
            else { // return value
                data->channel_2_device_response_data = *buffer;
            }
            return;
        }

        buffer++;
    }
}

void controller_8042_channel_2_set_receive_function(hardware_t *structure, void receive(hardware_t *, uint8_t *, uint32_t)) {
    controller_8042_data_t *data = controller_8042->data;
    data->channel_2_device_structure = structure;
    data->channel_2_process_data = receive;
}

uint32_t controller_8042_channel_2_send_command(uint8_t command) {
    controller_8042_data_t *data = controller_8042->data;
    void *structure = data->channel_2_device_structure;
    void *receive_function = data->channel_2_process_data;

    uint32_t state = controller_8042_channel_2_send_command_universal(command, false, 0, false);

    data->channel_2_device_structure = structure;
    data->channel_2_process_data = receive_function;

    if(state == INVALID) {
        return ERROR;
    }
    else {
        return SUCCESS;
    }
}

uint32_t controller_8042_channel_2_send_command_with_payload(uint8_t command, uint8_t payload) {
    controller_8042_data_t *data = controller_8042->data;
    void *structure = data->channel_2_device_structure;
    void *receive_function = data->channel_2_process_data;

    uint32_t state = controller_8042_channel_2_send_command_universal(command, true, payload, false);

    data->channel_2_device_structure = structure;
    data->channel_2_process_data = receive_function;

    if(state == INVALID) {
        return ERROR;
    }
    else {
        return SUCCESS;
    }
}

uint32_t controller_8042_channel_2_send_command_with_return(uint8_t command) {
    controller_8042_data_t *data = controller_8042->data;
    void *structure = data->channel_2_device_structure;
    void *receive_function = data->channel_2_process_data;

    uint32_t state = controller_8042_channel_2_send_command_universal(command, false, 0, true);

    data->channel_2_device_structure = structure;
    data->channel_2_process_data = receive_function;

    return state;
}

uint32_t controller_8042_channel_2_send_command_with_payload_and_return(uint8_t command, uint8_t payload) {
    controller_8042_data_t *data = controller_8042->data;
    void *structure = data->channel_2_device_structure;
    void *receive_function = data->channel_2_process_data;

    uint32_t state = controller_8042_channel_2_send_command_universal(command, true, payload, true);

    data->channel_2_device_structure = structure;
    data->channel_2_process_data = receive_function;

    return state;
}

/* interrupt handler */
void controller_8042_irq(interrupt_stack_t *stack_of_interrupt) {
    controller_8042_data_t *data = controller_8042->data;
    for(int i = 0; i < 1000; i++) { // this prevents infinite loop
        while((inb(0x64) & 0x1) == 0x1) { // read all possible data
            // check if buffers of channels are full
            if(data->channel_1_buffer_num_of_bytes >= CONTROLLER_8042_CHANNEL_INTERRUPT_MAX_DATA) {
                data->channel_1_process_data(data->channel_1_device_structure, data->channel_1_buffer, data->channel_1_buffer_num_of_bytes);
                data->channel_1_buffer_num_of_bytes = 0;
            }
            else if(data->channel_2_buffer_num_of_bytes >= CONTROLLER_8042_CHANNEL_INTERRUPT_MAX_DATA) {
                data->channel_2_process_data(data->channel_2_device_structure, data->channel_2_buffer, data->channel_2_buffer_num_of_bytes);
                data->channel_2_buffer_num_of_bytes = 0;
            }

            // read informations about data
            uint8_t status = inb(0x64);
            uint8_t received_data = inb(0x60);
            uint8_t data_source = (status & (1 << 5));

            // insert data to buffer
            if(data_source == 0) {
                data->channel_1_buffer[data->channel_1_buffer_num_of_bytes++] = received_data;
            }
            else {
                data->channel_2_buffer[data->channel_2_buffer_num_of_bytes++] = received_data;
            }
        }
    }

    // process data from first channel
    if(data->channel_1_buffer_num_of_bytes != 0) {
        data->channel_1_process_data(data->channel_1_device_structure, data->channel_1_buffer, data->channel_1_buffer_num_of_bytes);
        data->channel_1_buffer_num_of_bytes = 0;
    }

    // process data from second channel
    if(data->channel_2_buffer_num_of_bytes != 0) {
        data->channel_2_process_data(data->channel_2_device_structure, data->channel_2_buffer, data->channel_2_buffer_num_of_bytes);
        data->channel_2_buffer_num_of_bytes = 0;
    }
}