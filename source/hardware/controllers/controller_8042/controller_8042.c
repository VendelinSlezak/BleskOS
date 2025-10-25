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
#include <hardware/groups/logging/logging.h>
#include <kernel/x86/cpu/commands.h>
#include <kernel/x86/entities/entities.h>
#include <kernel/x86/interrupt_controllers/main.h>
#include <kernel/x86/scheduler/lock.h>
#include <kernel/x86/system_call.h>

#include <hardware/devices/human_input/ps2_keyboard.h>

/* local variables */
e_controller_8042_attr_t *controller_8042_attr;

/* functions for communication with controller */
dword_t read_ps2_data(void) {
    for(int i = 0; i < 100; i++) {
        if((inb(0x64) & 0x1) == 0x1) {
            return inb(0x60);
        }
    }

    for(int i = 0; i < 200; i++) {
        if((inb(0x64) & 0x1) == 0x1) {
            return inb(0x60);
        }
        SC_SLEEP(1000);
    }
    
    return INVALID;
}

dword_t write_ps2_data(byte_t value) {
    for(int i = 0; i < 100; i++) {
        if((inb(0x64) & 0x2) == 0x0) {
            outb(0x60, value);
            return SUCCESS;
        }
    }

    for(int i = 0; i < 200; i++) {
        if((inb(0x64) & 0x2) == 0x0) {
            outb(0x60, value);
            return SUCCESS;
        }
        SC_SLEEP(1000);
    }
    
    return ERROR;
}

dword_t write_ps2_command(byte_t value) {
    for(int i = 0; i < 100; i++) {
        if((inb(0x64) & 0x2) == 0x0) {
            outb(0x64, value);
            return SUCCESS;
        }
    }
    
    for(int i = 0; i < 200; i++) {
        if((inb(0x64) & 0x2) == 0x0) {
            outb(0x64, value);
            return SUCCESS;
        }
        SC_SLEEP(1000);
    }
    
    return ERROR;
}

/* functions for controller initialization */
dword_t is_controller_8042_present(void) {
    return TRUE; // TODO: add checking
}

void initialize_controller_8042_entity(void) {
    dword_t entity_number = create_entity("8042 Controller", E_TYPE_KERNEL_RING);
    controller_8042_attr = entity_get_attr_ptr(entity_number);
    create_command_without_ca_without_fin(entity_number, initialize_controller_8042);
}

void initialize_controller_8042(void) {
    controller_8042_attr->channel_1 = NOT_KNOWN;
    controller_8042_attr->channel_2 = NOT_KNOWN;
    log("\n[8042C] Initializing...");

    // disable PS/2 channels
    write_ps2_command(0xAD);
    write_ps2_command(0xA7);

    // clear buffer of data port
    for(dword_t i = 0; i < 1000; i++) {
        if((inb(0x64) & 0x1) == 0x1) { // there are data on data port
            inb(0x60);
        }
        else {
            break;
        }
    }
    if((inb(0x64) & 0x1) == 0x1) { // error: data port was not cleared
        log("\n[8042C] ERROR: locked data port");
        controller_8042_attr->channel_1 = NOT_PRESENT;
        controller_8042_attr->channel_2 = NOT_PRESENT;
        return;
    }

    // disable interrupts on both channels
    write_ps2_command(0x20);
    byte_t configuration_byte = read_ps2_data();
    write_ps2_command(0x60);
    write_ps2_data(configuration_byte & ~((1 << 0) | (1 << 1)));

    // check presence of channels
    if((configuration_byte & (1 << 4)) == 0) { // first channel should be disabled, if it is not, it can not exist
        controller_8042_attr->channel_1 = NOT_PRESENT;
    }
    if((configuration_byte & (1 << 5)) == 0) { // second channel should be disabled, if it is not, it can not exist
        controller_8042_attr->channel_2 = NOT_PRESENT;
    }

    // perform controller self-test
    write_ps2_command(0xAA);
    if(read_ps2_data() == 0x55) {
        // self-test succesfull
        log("\n[8042C] Self-test successfull");
    }
    else {
        // self-test failed
        log("\n[8042C] ERROR: Self-test failed");
        controller_8042_attr->channel_1 = NOT_PRESENT;
        controller_8042_attr->channel_2 = NOT_PRESENT;
        return;
    }

    // enable PS/2 channels
    write_ps2_command(0xAE);
    write_ps2_command(0xA8);

    // check presence of channels
    write_ps2_command(0x20);
    configuration_byte = read_ps2_data();
    if(controller_8042_attr->channel_1 == NOT_KNOWN) { // we do not know first channel presence yet
        if((configuration_byte & (1 << 4)) == 0) {
            controller_8042_attr->channel_1 = PRESENT; // first channel is enabled, so it exist
        }
        else {
            controller_8042_attr->channel_1 = NOT_PRESENT; // first channel is disabled, so it do not exist
        }
    }
    if(controller_8042_attr->channel_2 == NOT_KNOWN) { // we do not know second channel presence yet
        if((configuration_byte & (1 << 5)) == 0) {
            controller_8042_attr->channel_2 = PRESENT; // second channel is enabled, so it exist
        }
        else {
            controller_8042_attr->channel_2 = NOT_PRESENT; // second channel is disabled, so it do not exist
        }
    }

    // disable PS/2 channels
    write_ps2_command(0xAD);
    write_ps2_command(0xA7);

    // test first channel
    if(controller_8042_attr->channel_1 == PRESENT) {
        write_ps2_command(0xAB);
        if(read_ps2_data() == 0x00) { // test successfull
            log("\n[8042C] First channel test successfull");
        }
        else { // test failed
            log("\n[8042C] First channel test failed");
            controller_8042_attr->channel_1 = UNKNOWN_DEVICE;
        }
    }

    // test second channel
    if(controller_8042_attr->channel_2 == PRESENT) {
        write_ps2_command(0xA9);
        if(read_ps2_data() == 0x00) { // test successfull
            log("\n[8042C] Second channel test successfull");
        }
        else { // test failed
            log("\n[8042C] Second channel test failed");
            controller_8042_attr->channel_2 = UNKNOWN_DEVICE;
        }
    }

    // enable interrupts
    write_ps2_command(0x20);
    configuration_byte = read_ps2_data();
    if(controller_8042_attr->channel_1 == PRESENT) {
        connect_legacy_interrupt_to_handler(1, controller_8042_irq);
        configuration_byte |= (1 << 0); // enable interrupt 1
        configuration_byte &= ~(1 << 6); // disable translation to scan code set 1
    }
    if(controller_8042_attr->channel_2 == PRESENT) {
        connect_legacy_interrupt_to_handler(12, controller_8042_irq);
        configuration_byte |= (1 << 1); // enable interrupt 12
    }
    write_ps2_command(0x60);
    write_ps2_data(configuration_byte);

    // initialize first channel
    if(controller_8042_attr->channel_1 == PRESENT) {
        log("\n[8042C] Initializing device on first channel");
        write_ps2_command(0xAE);
        size_t controller_8042_functions_for_device_on_channel_1[] = {
            (size_t)controller_8042_channel_1_set_receive_function,
            (size_t)controller_8042_channel_1_send_command,
            (size_t)controller_8042_channel_1_send_command_with_payload,
            (size_t)controller_8042_channel_1_send_command_with_return,
            (size_t)controller_8042_channel_1_send_command_with_payload_and_return
        };
        controller_8042_initialize_channel(controller_8042_channel_1_write_command, &controller_8042_attr->channel_1_process_data, controller_8042_functions_for_device_on_channel_1);
    }

    // initialize second channel
    if(controller_8042_attr->channel_2 == PRESENT) {
        log("\n[8042C] Initializing device on second channel");
        write_ps2_command(0xA8);
        size_t controller_8042_functions_for_device_on_channel_2[] = {
            (size_t)controller_8042_channel_2_set_receive_function,
            (size_t)controller_8042_channel_2_send_command,
            (size_t)controller_8042_channel_2_send_command_with_payload,
            (size_t)controller_8042_channel_2_send_command_with_return,
            (size_t)controller_8042_channel_2_send_command_with_payload_and_return
        };
        controller_8042_initialize_channel(controller_8042_channel_2_write_command, &controller_8042_attr->channel_2_process_data, controller_8042_functions_for_device_on_channel_2);
    }
}

void controller_8042_initialize_channel(dword_t (*send)(byte_t), void (**receive_ptr)(dword_t, byte_t *, dword_t), size_t controller_8042_functions_for_device[]) {
    // set receive method
    *receive_ptr = controller_8042_channel_process_data;

    // reset device
    controller_8042_channel_send_command(send, "Reset", 0xFF);
    if(controller_8042_attr->buffer[1] != 0xAA) {
        log("\n[8042C] ERROR: Reset was not successfull");
        return;
    }
    log("\n[8042C] Device was successfully resetted");

    // get device ID
    controller_8042_channel_send_command(send, "Get Device ID", 0xF2);
    SC_SLEEP(10000); // wait for all bytes to arrive
    log("\n[8042C] Device ID:");
    for(int i = 1; i < controller_8042_attr->buffer_ptr; i++) {
        log(" 0x%02x", controller_8042_attr->buffer[i]);
    }

    // connect device to driver
    log("\n[8042C] Device type: ");
    if(    controller_8042_attr->buffer[1] == 0xAB
        || controller_8042_attr->buffer[1] == 0xAC) {
        log("Keyboard");
        dword_t entity_number = create_ps2_keyboard_device_entity(command_info->entity_number, controller_8042_functions_for_device);
        dword_t command_number = create_command_without_ca_without_fin(entity_number, initialize_ps2_keyboard);
        SC_WAIT_FOR_COMMAND(command_number);
    }
    else if(   controller_8042_attr->buffer[1] == 0x00
            || controller_8042_attr->buffer[1] == 0x03
            || controller_8042_attr->buffer[1] == 0x04) {
        log("Mouse");
        // TODO:
    }
    else {
        log("Unknown");
    }
}

dword_t controller_8042_channel_send_command(dword_t (*send)(byte_t), byte_t *command_name, byte_t command) {
    // send command
    controller_8042_attr->buffer_ptr = 0;
    send(command);

    // wait for response
    qword_t timeout = (kernel_attr->kernel_time + 1000000); // 1s
    while(kernel_attr->kernel_time < timeout) {
        asm volatile("pause");
        if(((volatile dword_t)controller_8042_attr->buffer_ptr) < 2) {
            SC_SWITCH();
            continue;
        }
        else {
            break;
        }
    }

    // check errors
    if(((volatile dword_t)controller_8042_attr->buffer_ptr) < 2) {
        log("\n[8042C] ERROR: Timeout");
        return ERROR;
    }
    if(controller_8042_attr->buffer[0] != 0xFA) {
        log("\n[8042C] ERROR: %s not acknowledged", command_name);
        return ERROR;
    }
    if(controller_8042_attr->buffer_ptr < 2) {
        log("\n[8042C] ERROR: %s did return too little data", command_name);
        return ERROR;
    }

    return SUCCESS;
}

void controller_8042_channel_process_data(dword_t entity, byte_t *buffer, dword_t size) {
    if(controller_8042_attr->buffer_ptr >= CONTROLLER_8042_BUFFER_SIZE) {
        return;
    }

    while(size--) {
        controller_8042_attr->buffer[controller_8042_attr->buffer_ptr++] = *buffer++;
    }
}

dword_t controller_8042_channel_1_write_command(byte_t command) {
    LOCK_MUTEX(&controller_8042_attr->sending_command);
    dword_t state = write_ps2_data(command);
    UNLOCK_MUTEX(&controller_8042_attr->sending_command);
    return state;
}

dword_t controller_8042_channel_1_write_command_with_payload(byte_t command, byte_t payload) {
    LOCK_MUTEX(&controller_8042_attr->sending_command);
    if(write_ps2_data(command) == ERROR) {
        UNLOCK_MUTEX(&controller_8042_attr->sending_command);
        return ERROR;
    }
    dword_t state = write_ps2_data(payload);
    UNLOCK_MUTEX(&controller_8042_attr->sending_command);
    return state;
}

dword_t controller_8042_channel_2_write_command(byte_t command) {
    LOCK_MUTEX(&controller_8042_attr->sending_command);
    if(write_ps2_command(0xD4) == ERROR) {
        UNLOCK_MUTEX(&controller_8042_attr->sending_command);
        return ERROR;
    }
    dword_t state = write_ps2_data(command);
    UNLOCK_MUTEX(&controller_8042_attr->sending_command);
    return state;
}

dword_t controller_8042_channel_2_write_command_with_payload(byte_t command, byte_t payload) {
    if(controller_8042_channel_2_write_command(command) == ERROR) {
        return ERROR;
    }
    return controller_8042_channel_2_write_command(payload);
}

/* functions for communication with devices */
dword_t controller_8042_channel_1_send_command_universal(byte_t command, dword_t is_payload, byte_t payload, dword_t is_response) {
    controller_8042_attr->channel_1_process_data = controller_8042_channel_1_command_response;

    // retry sending command max 3 times
    for(int i = 0; i < 3; i++) {
        // send command
        controller_8042_attr->channel_1_device_response_state = INVALID;
        controller_8042_attr->channel_1_device_response_data = INVALID;
        if(is_payload == FALSE) {
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
        qword_t timeout = (kernel_attr->kernel_time + 100000); // 100ms
        while(kernel_attr->kernel_time < timeout) {
            asm volatile("pause");
            if(((volatile dword_t)controller_8042_attr->channel_1_device_response_state) == INVALID) {
                SC_SWITCH();
                continue;
            }
            else {
                if(is_response == TRUE && controller_8042_attr->channel_1_device_response_data == INVALID) {
                    SC_SWITCH();
                    continue;
                }
                break;
            }
        }
        if(((volatile dword_t)controller_8042_attr->channel_1_device_response_state) == INVALID) {
            log("\n[8042C] ERROR: Timeout on channel 1 command");
            return INVALID;
        }

        // process response
        if(controller_8042_attr->channel_1_device_response_state == 0xFA) {
            if(is_response == FALSE) {
                return SUCCESS;
            }
            else {
                return controller_8042_attr->channel_1_device_response_data;
            }
        }
        else if(controller_8042_attr->channel_1_device_response_state != 0xFE) {
            log("\n[8042C] ERROR: Channel 1 command returned 0x%02x", controller_8042_attr->channel_1_device_response_state);
            return INVALID;
        }
    }

    log("\n[8042C] ERROR: Channel 1 request command resend more than 3 times");
    return INVALID;
}

void controller_8042_channel_1_command_response(dword_t entity, byte_t *buffer, dword_t size) {
    while(size--) {
        if(*buffer == 0xFA) { // byte accepted, one interrupt can send multiple of those
            controller_8042_attr->channel_1_device_response_state = 0xFA;
        }
        else {
            if(*buffer > 0xFA) { // error code
                controller_8042_attr->channel_1_device_response_state = *buffer; // return what error code happened
                controller_8042_attr->channel_1_device_response_data = INVALID;
            }
            else { // return value
                controller_8042_attr->channel_1_device_response_data = *buffer;
            }
            return;
        }

        buffer++;
    }
}

void controller_8042_channel_1_set_receive_function(dword_t entity, void receive(dword_t, byte_t *, dword_t)) {
    controller_8042_attr->channel_1_entity = entity;
    controller_8042_attr->channel_1_process_data = receive;
}

dword_t controller_8042_channel_1_send_command(byte_t command) {
    dword_t state = controller_8042_channel_1_send_command_universal(command, FALSE, 0, FALSE);
    if(state == INVALID) {
        return ERROR;
    }
    else {
        return SUCCESS;
    }
}

dword_t controller_8042_channel_1_send_command_with_payload(byte_t command, byte_t payload) {
    dword_t state = controller_8042_channel_1_send_command_universal(command, TRUE, payload, FALSE);
    if(state == INVALID) {
        return ERROR;
    }
    else {
        return SUCCESS;
    }
}

dword_t controller_8042_channel_1_send_command_with_return(byte_t command) {
    return controller_8042_channel_1_send_command_universal(command, FALSE, 0, TRUE);
}

dword_t controller_8042_channel_1_send_command_with_payload_and_return(byte_t command, byte_t payload) {
    return controller_8042_channel_1_send_command_universal(command, TRUE, payload, TRUE);
}

dword_t controller_8042_channel_2_send_command_universal(byte_t command, dword_t is_payload, byte_t payload, dword_t is_response) {
    controller_8042_attr->channel_2_process_data = controller_8042_channel_2_command_response;

    // retry sending command max 3 times
    for(int i = 0; i < 3; i++) {
        // send command
        controller_8042_attr->channel_2_device_response_state = INVALID;
        controller_8042_attr->channel_2_device_response_data = INVALID;
        if(is_payload == FALSE) {
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
        qword_t timeout = (kernel_attr->kernel_time + 100000); // 100ms
        while(kernel_attr->kernel_time < timeout) {
            asm volatile("pause");
            if(((volatile dword_t)controller_8042_attr->channel_2_device_response_state) == INVALID) {
                SC_SWITCH();
                continue;
            }
            else {
                if(is_response == TRUE && controller_8042_attr->channel_2_device_response_data == INVALID) {
                    SC_SWITCH();
                    continue;
                }
                break;
            }
        }
        if(((volatile dword_t)controller_8042_attr->channel_2_device_response_state) == INVALID) {
            log("\n[8042C] ERROR: Timeout on channel 2 command");
            return INVALID;
        }

        // process response
        if(controller_8042_attr->channel_2_device_response_state == 0xFA) {
            if(is_response == FALSE) {
                return SUCCESS;
            }
            else {
                return controller_8042_attr->channel_2_device_response_data;
            }
        }
        else if(controller_8042_attr->channel_2_device_response_state != 0xFE) {
            log("\n[8042C] ERROR: Channel 2 command returned 0x%02x", controller_8042_attr->channel_2_device_response_state);
            return INVALID;
        }
    }

    log("\n[8042C] ERROR: Channel 2 request command resend more than 3 times");
    return INVALID;
}

void controller_8042_channel_2_command_response(dword_t entity, byte_t *buffer, dword_t size) {
    while(size--) {
        if(*buffer == 0xFA) { // byte accepted, one interrupt can send multiple of those
            controller_8042_attr->channel_2_device_response_state = 0xFA;
        }
        else {
            if(*buffer > 0xFA) { // error code
                controller_8042_attr->channel_2_device_response_state = *buffer; // return what error code happened
                controller_8042_attr->channel_2_device_response_data = INVALID;
            }
            else { // return value
                controller_8042_attr->channel_2_device_response_data = *buffer;
            }
            return;
        }

        buffer++;
    }
}

void controller_8042_channel_2_set_receive_function(dword_t entity, void receive(dword_t, byte_t *, dword_t)) {
    controller_8042_attr->channel_2_entity = entity;
    controller_8042_attr->channel_2_process_data = receive;
}

dword_t controller_8042_channel_2_send_command(byte_t command) {
    dword_t state = controller_8042_channel_2_send_command_universal(command, FALSE, 0, FALSE);
    if(state == INVALID) {
        return ERROR;
    }
    else {
        return SUCCESS;
    }
}

dword_t controller_8042_channel_2_send_command_with_payload(byte_t command, byte_t payload) {
    dword_t state = controller_8042_channel_2_send_command_universal(command, TRUE, payload, FALSE);
    if(state == INVALID) {
        return ERROR;
    }
    else {
        return SUCCESS;
    }
}

dword_t controller_8042_channel_2_send_command_with_return(byte_t command) {
    return controller_8042_channel_2_send_command_universal(command, FALSE, 0, TRUE);
}

dword_t controller_8042_channel_2_send_command_with_payload_and_return(byte_t command, byte_t payload) {
    return controller_8042_channel_2_send_command_universal(command, TRUE, payload, TRUE);
}

/* interrupt handler */
void controller_8042_irq(void) {
    for(int i = 0; i < 1000; i++) { // this prevents infinite loop
        while((inb(0x64) & 0x1) == 0x1) { // read all possible data
            // check if buffers of channels are full
            if(controller_8042_attr->channel_1_buffer_num_of_bytes >= CONTROLLER_8042_CHANNEL_INTERRUPT_MAX_DATA) {
                controller_8042_attr->channel_1_process_data(controller_8042_attr->channel_1_entity, controller_8042_attr->channel_1_buffer, controller_8042_attr->channel_1_buffer_num_of_bytes);
                controller_8042_attr->channel_1_buffer_num_of_bytes = 0;
            }
            else if(controller_8042_attr->channel_2_buffer_num_of_bytes >= CONTROLLER_8042_CHANNEL_INTERRUPT_MAX_DATA) {
                controller_8042_attr->channel_2_process_data(controller_8042_attr->channel_2_entity, controller_8042_attr->channel_2_buffer, controller_8042_attr->channel_2_buffer_num_of_bytes);
                controller_8042_attr->channel_2_buffer_num_of_bytes = 0;
            }

            // read informations about data
            byte_t status = inb(0x64);
            byte_t data = inb(0x60);
            byte_t data_source = (status & (1 << 5));

            // insert data to buffer
            if(data_source == 0) {
                controller_8042_attr->channel_1_buffer[controller_8042_attr->channel_1_buffer_num_of_bytes++] = data;
            }
            else {
                controller_8042_attr->channel_2_buffer[controller_8042_attr->channel_2_buffer_num_of_bytes++] = data;
            }
        }
    }

    // process data from first channel
    if(controller_8042_attr->channel_1_buffer_num_of_bytes != 0) {
        controller_8042_attr->channel_1_process_data(controller_8042_attr->channel_1_entity, controller_8042_attr->channel_1_buffer, controller_8042_attr->channel_1_buffer_num_of_bytes);
        controller_8042_attr->channel_1_buffer_num_of_bytes = 0;
    }

    // process data from second channel
    if(controller_8042_attr->channel_2_buffer_num_of_bytes != 0) {
        controller_8042_attr->channel_2_process_data(controller_8042_attr->channel_2_entity, controller_8042_attr->channel_2_buffer, controller_8042_attr->channel_2_buffer_num_of_bytes);
        controller_8042_attr->channel_2_buffer_num_of_bytes = 0;
    }
}