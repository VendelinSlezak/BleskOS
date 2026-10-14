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
#include <kernel/hardware/controllers/pci/pci.h>
#include <kernel/hardware/groups/logging/logging.h>
#include <kernel/hardware/devices/timers/main.h>
#include <kernel/hardware/devices/memory/memory_allocators.h>
#include <kernel/hardware/main.h>
#include <kernel/hardware/devices/cpu/commands.h>
#include <kernel/hardware/devices/cpu/scheduler.h>
#include <kernel/hardware/devices/cpu/mutex.h>
#include <kernel/hardware/devices/cpu/interrupt.h>
#include <kernel/hardware/devices/hard_disk/parallel_ata.h>
#include <kernel/libc/string.h>

/* local variables */
uint32_t is_isa_compatibile_primary_controller_present = false;
uint32_t is_isa_compatibile_secondary_controller_present = false;
uint32_t number_of_controllers = 0;
hardware_t **ide_controllers = NULL;

/* functions */
void initialize_ide_entry_on_pci(hardware_t *device) {
    uint8_t progif = pci_inb(device, 0x9);
    uint32_t has_dma_capability = ((progif & 0x80) == 0x80);
    uint16_t dma_base_address = (has_dma_capability) ? pci_get_io(device, PCI_BAR5) : 0x0000;
    interrupt_description_t interrupt;

    // initialize PCI bus
    pci_enable_io_space(device);
    if(has_dma_capability) {
        pci_enable_busmastering(device);
    }

    // primary controller is ISA compatibile
    if(((progif & 0x01) == 0x00)) {
        if(is_isa_compatibile_primary_controller_present == false) {
            ide_controller_data_t *ide_controller_data = kalloc(sizeof(ide_controller_data_t));
            ide_controller_data->base = 0x1F0;
            ide_controller_data->control = 0x3F6;
            ide_controller_data->dma = dma_base_address;

            // check if bus is not floating
            if(inb(ide_controller_data->base + 7) != 0xFF) {
                hardware_t *primary_ide_controller = add_hardware(device, "Primary IDE controller", NULL, ide_controller_data, initialize_ide_controller, NULL);
                primary_ide_controller->data = ide_controller_data;
                ide_controllers = krealloc(ide_controllers, sizeof(hardware_t *) * (number_of_controllers + 1));
                ide_controllers[number_of_controllers] = primary_ide_controller;
                number_of_controllers++;
                interrupt = set_isa_interrupt_handler(14, ide_interrupt_handler, NULL, NULL);
                ide_controller_data->interrupt_processor_index = interrupt.processor_index;
                init_hardware(primary_ide_controller);
            }

            is_isa_compatibile_primary_controller_present = true;
        }
    }
    // primary controller is PCI native
    else {
        ide_controller_data_t *ide_controller_data = kalloc(sizeof(ide_controller_data_t));
        ide_controller_data->base = pci_get_io(device, PCI_BAR0);
        ide_controller_data->control = pci_get_io(device, PCI_BAR1);
        ide_controller_data->dma = dma_base_address;

        // check if bus is not floating
        if(inb(ide_controller_data->base + 7) != 0xFF) {
            hardware_t *primary_ide_controller = add_hardware(device, "Primary IDE controller", NULL, ide_controller_data, initialize_ide_controller, NULL);
            primary_ide_controller->data = ide_controller_data;
            ide_controllers = krealloc(ide_controllers, sizeof(hardware_t *) * (number_of_controllers + 1));
            ide_controllers[number_of_controllers] = primary_ide_controller;
            number_of_controllers++;
            interrupt = set_pci_interrupt_handler(device, ide_interrupt_handler, NULL, NULL);
            ide_controller_data->interrupt_processor_index = interrupt.processor_index;
            init_hardware(primary_ide_controller);
        }
    }

    // secondary controller is ISA compatibile
    if(((progif & 0x04) == 0x00)) {
        if(is_isa_compatibile_secondary_controller_present == false) {
            ide_controller_data_t *ide_controller_data = kalloc(sizeof(ide_controller_data_t));
            ide_controller_data->base = 0x170;
            ide_controller_data->control = 0x376;
            ide_controller_data->dma = dma_base_address;

            // check if bus is not floating
            if(inb(ide_controller_data->base + 7) != 0xFF) {
                hardware_t *secondary_ide_controller = add_hardware(device, "Secondary IDE controller", NULL, ide_controller_data, initialize_ide_controller, NULL);
                secondary_ide_controller->data = ide_controller_data;
                ide_controllers = krealloc(ide_controllers, sizeof(hardware_t *) * (number_of_controllers + 1));
                ide_controllers[number_of_controllers] = secondary_ide_controller;
                number_of_controllers++;
                interrupt = set_isa_interrupt_handler(15, ide_interrupt_handler, NULL, NULL);
                ide_controller_data->interrupt_processor_index = interrupt.processor_index;
                init_hardware(secondary_ide_controller);
            }

            is_isa_compatibile_secondary_controller_present = true;
        }
    }
    // secondary controller is PCI native
    else {
        ide_controller_data_t *ide_controller_data = kalloc(sizeof(ide_controller_data_t));
        ide_controller_data->base = pci_get_io(device, PCI_BAR2);
        ide_controller_data->control = pci_get_io(device, PCI_BAR3);
        ide_controller_data->dma = dma_base_address;

        // check if bus is not floating
        if(inb(ide_controller_data->base + 7) != 0xFF) {
            hardware_t *secondary_ide_controller = add_hardware(device, "Secondary IDE controller", NULL, ide_controller_data, initialize_ide_controller, NULL);
            secondary_ide_controller->data = ide_controller_data;
            ide_controllers = krealloc(ide_controllers, sizeof(hardware_t *) * (number_of_controllers + 1));
            ide_controllers[number_of_controllers] = secondary_ide_controller;
            number_of_controllers++;
            interrupt = set_pci_interrupt_handler(device, ide_interrupt_handler, NULL, NULL);
            ide_controller_data->interrupt_processor_index = interrupt.processor_index;
            init_hardware(secondary_ide_controller);
        }
    }

    device->is_initialized = true;
}

void initialize_ide_controller(hardware_t *self) {
    ide_controller_data_t *data = (ide_controller_data_t *) self->data;

    // log controller info
    log("\n[IDE] Controller found at 0x%04X and 0x%04X with %s transfer mode", data->base, data->control, (data->dma != 0) ? "DMA" : "PIO");

    // reset controller
    ide_reset_controller(self);

    // check drives
    initialize_ide_drive(self, PATA_MASTER);
    initialize_ide_drive(self, PATA_SLAVE);
    self->is_initialized = true;
}

void initialize_ide_drive(hardware_t *device, uint8_t drive) {
    ide_controller_data_t *data = (ide_controller_data_t *) device->data;
    uint16_t base = data->base;

    // check if bus is floating
    ide_select_drive(device, drive);
    if(inb(base + 7) == 0xFF) {
        return; // this drive does not exist
    }

    // send IDENTIFY command
    uint32_t status = ide_send_read_command(device, drive, ide_send_identify_command, base, 100000);
    
    // process response
    if(status == IDE_TIMEOUT) {
        log("\n[IDE] Controller 0x%04X %s drive is not present", data->base, (drive == PATA_MASTER) ? "Master" : "Slave");
        ide_command_processed(device);
    }
    else {
        // read drive type
        uint16_t drive_type = (inb(base + 4) | (inb(base + 5) << 8));

        // read data, if available
        ata_identify_command_device_info_t device_info;
        if(status == IDE_DATA_READY) {
            uint16_t *device_info_ptr = (uint16_t *) &device_info;
            for(int i = 0; i < 256; i++) {
                device_info_ptr[i] = inw(base + 0);
            }
        }
        ide_command_processed(device);

        // recognize device
        if(drive_type == PATA_HARD_DISK_SIGNATURE) {
            if(status == IDE_DATA_READY) {
                log("\n[IDE] Drive %s is a hard disk", (drive == PATA_MASTER) ? "Master" : "Slave");
                ide_controller_device_data_t *hard_disk_info = kalloc(sizeof(ide_controller_device_data_t));
                hard_disk_info->drive = drive;
                memcpy(&hard_disk_info->device_info, &device_info, sizeof(ata_identify_command_device_info_t));
                hardware_t *hard_disk = add_hardware(device, "PATA hard disk", NULL, hard_disk_info, initialize_parallel_ata, NULL);
                init_hardware(hard_disk);
            }
            // else drive is not present
        }
        else if(drive_type == PATA_OPTICAL_DRIVE_SIGNATURE) {
            log("\n[IDE] Drive %s is a optical disk", (drive == PATA_MASTER) ? "Master" : "Slave");
        }
        else {
            log("\n[IDE] Drive is unknown 0x%04X", drive_type);
        }
    }
}

void ide_send_identify_command(uint32_t base) {
    outb(base + 2, 0);
    outb(base + 3, 0);
    outb(base + 4, 0);
    outb(base + 5, 0);
    outb(base + 7, 0xEC);
}

uint32_t ide_send_read_command(hardware_t *controller, uint32_t drive, void (*send_command)(uint32_t argument), uint32_t argument, uint32_t microseconds_timeout) {
    // be sure that controller is ours
    ide_controller_data_t *data = (ide_controller_data_t *) controller->data;
    LOCK_MUTEX_WAIT_IN_SWITCHES(&data->mutex); // TODO: replace mutex with waiting list where will be added all other waiting threads, which will be then unblocked

    // select drive
    ide_select_drive(controller, drive);

    // sending of command needs specific sequence to prevent race conditions, now we will block thread and only then complete sending
    ide_send_read_command_arguments_t arguments = {
        .controller = controller,
        .microseconds_timeout = microseconds_timeout,
        .send_command = send_command,
        .argument = argument,
    };
    block_current_thread_with_sequence_causing_block(ide_send_read_command_after_block_sequence, (uint32_t) &arguments);

    // thread was unblocked, that mean that either IRQ arrived, or timeout happened
    // in both cases, send status to caller to process it
    return data->transfer_status;
}

void ide_send_read_command_after_block_sequence(uint32_t arguments_ptr) {
    ide_send_read_command_arguments_t *arguments = (ide_send_read_command_arguments_t *) arguments_ptr;
    ide_controller_data_t *data = (ide_controller_data_t *) arguments->controller->data;

    // current thread is blocked and will wait for command completion
    kernel_thread_t *current_thread = get_current_kernel_thread();
    data->waiting_thread_for_irq = current_thread;
    data->interrupt_expected = true;

    // create timeout thread, timeout MUST be always bigger than the time it takes to execute arguments->send_command()
    data->timeout_thread = create_sleeping_kernel_thread_on_processor("IDE timeout", data->interrupt_processor_index, (uint32_t)ide_timeout, (uint32_t []) { (uint32_t) arguments->controller }, 1, arguments->microseconds_timeout);

    // call functions which will send command to controller and trigger interrupt
    arguments->send_command(arguments->argument);

    // now this thread is blocked, and will be unblocked either by arrival of interrupt, or timeout thread
}

uint32_t ide_renew_read_command(hardware_t *controller, void (*renew_command)(uint32_t argument), uint32_t argument, uint32_t microseconds_timeout) {
    ide_send_read_command_arguments_t arguments = {
        .controller = controller,
        .microseconds_timeout = microseconds_timeout,
        .send_command = renew_command,
        .argument = argument,
    };
    block_current_thread_with_sequence_causing_block(ide_send_read_command_after_block_sequence, (uint32_t) &arguments);

    // thread was unblocked, that mean that either IRQ arrived, or timeout happened
    // in both cases, send status to caller to process it
    ide_controller_data_t *data = (ide_controller_data_t *) controller->data;
    return data->transfer_status;
}

uint32_t ide_send_write_command(hardware_t *controller, uint32_t drive, void (*send_command)(uint32_t argument), uint32_t argument, void (*write_data)(uint32_t argument), uint32_t write_argument, uint32_t microseconds_timeout) {
    // be sure that controller is ours
    ide_controller_data_t *data = (ide_controller_data_t *) controller->data;
    LOCK_MUTEX_WAIT_IN_SWITCHES(&data->mutex); // TODO: replace mutex with waiting list where will be added all other waiting threads, which will be then unblocked

    // select drive
    ide_select_drive(controller, drive);

    // wait until drive is not busy
    if(ide_wait_drive_not_busy_with_switches(controller, microseconds_timeout) == ERROR) {
        return ERROR;
    }

    // send command
    send_command(argument);

    // wait until drive accepts data
    if(ide_wait_for_data(controller, microseconds_timeout) == ERROR) {
        return ERROR;
    }

    // sending of data needs specific sequence to prevent race conditions, now we will block thread and only then complete sending
    ide_send_write_command_arguments_t arguments = {
        .controller = controller,
        .microseconds_timeout = microseconds_timeout,
        .write_data = write_data,
        .argument = write_argument,
    };
    block_current_thread_with_sequence_causing_block(ide_send_write_command_after_block_sequence, (uint32_t) &arguments);

    // thread was unblocked, that mean that either IRQ arrived, or timeout happened
    // in both cases, send status to caller to process it
    return data->transfer_status;
}

void ide_send_write_command_after_block_sequence(uint32_t arguments_ptr) {
    ide_send_write_command_arguments_t *arguments = (ide_send_write_command_arguments_t *) arguments_ptr;
    ide_controller_data_t *data = (ide_controller_data_t *) arguments->controller->data;

    // current thread is blocked and will wait for command completion
    kernel_thread_t *current_thread = get_current_kernel_thread();
    data->waiting_thread_for_irq = current_thread;
    data->interrupt_expected = true;

    // create timeout thread, timeout MUST be always bigger than the time it takes to execute arguments->send_command()
    data->timeout_thread = create_sleeping_kernel_thread_on_processor("IDE timeout", data->interrupt_processor_index, (uint32_t)ide_timeout, (uint32_t []) { (uint32_t) arguments->controller }, 1, arguments->microseconds_timeout);

    // call functions which will send data to controller and trigger interrupt
    arguments->write_data(arguments->argument);

    // now this thread is blocked, and will be unblocked either by arrival of interrupt, or timeout thread
}

uint32_t ide_renew_write_command(hardware_t *controller, void (*write_data)(uint32_t argument), uint32_t write_argument, uint32_t microseconds_timeout) {
    ide_send_write_command_arguments_t arguments = {
        .controller = controller,
        .microseconds_timeout = microseconds_timeout,
        .write_data = write_data,
        .argument = write_argument,
    };
    block_current_thread_with_sequence_causing_block(ide_send_write_command_after_block_sequence, (uint32_t) &arguments);

    // thread was unblocked, that mean that either IRQ arrived, or timeout happened
    // in both cases, send status to caller to process it
    ide_controller_data_t *data = (ide_controller_data_t *) controller->data;
    return data->transfer_status;
}

void ide_interrupt_handler(interrupt_stack_t *stack_of_interrupt) {
    log("\n[IDE] Interrupt");
    for(int i = 0; i < number_of_controllers; i++) {
        ide_controller_data_t *data = (ide_controller_data_t *) ide_controllers[i]->data;
        uint8_t status_register = inb(data->control + 7); // get status
        if((status_register & 0x80) == 0x80) { // disk is busy
            continue;
        }

        // acknowledge interrupt
        status_register = inb(data->base + 7);

        // check if this interrupt is for this controller
        if(data->interrupt_expected == false) {
            continue;
        }

        // check exact status
        if((status_register & 0x08) == 0x08) {
            data->transfer_status = IDE_DATA_READY;
        }
        else if((status_register & 0x01) == 0x01) {
            data->transfer_status = IDE_ERROR;
        }
        else { // disk is ready
            data->transfer_status = IDE_COMMAND_COMPLETED;
        }

        // destroy timeout thread
        kill_other_kernel_thread(data->timeout_thread);
        data->timeout_thread = NULL;

        // unblock waiting thread
        data->interrupt_expected = false;
        unblock_kernel_thread(data->waiting_thread_for_irq);
    }
}

void ide_command_processed(hardware_t *controller) {
    ide_controller_data_t *data = (ide_controller_data_t *) controller->data;
    UNLOCK_MUTEX(&data->mutex);
}

void ide_timeout(hardware_t *controller) {
    lock_core();
    ide_controller_data_t *data = (ide_controller_data_t *) controller->data;
    data->interrupt_expected = false;
    data->transfer_status = IDE_TIMEOUT;
    unblock_kernel_thread(data->waiting_thread_for_irq);
    unlock_core();
}

uint32_t ide_wait_drive_not_busy(hardware_t *device, uint32_t microseconds_to_wait) {
    ide_controller_data_t *data = (ide_controller_data_t *) device->data;
    uint16_t base = data->base;
    volatile uint64_t timeout = (get_time_in_microseconds() + microseconds_to_wait);
    while((int)(timeout - get_time_in_microseconds()) > 0) {
        asm("pause");
        if((inb(base + 7) & 0x80) == 0x00) {
            return SUCCESS;  // drive is not busy
        }
    }
    return ERROR;
}

uint32_t ide_wait_drive_not_busy_with_switches(hardware_t *device, uint32_t microseconds_to_wait) {
    ide_controller_data_t *data = (ide_controller_data_t *) device->data;
    uint16_t base = data->base;
    volatile uint64_t timeout = (get_time_in_microseconds() + microseconds_to_wait);
    while((int)(timeout - get_time_in_microseconds()) > 0) {
        if((inb(base + 7) & 0x80) == 0x00) {
            return SUCCESS;  // drive is not busy
        }
        switch_to_another_thread();
    }
    return ERROR;
}

uint32_t ide_wait_for_data(hardware_t *device, uint32_t microseconds_to_wait) {
    ide_controller_data_t *data = (ide_controller_data_t *) device->data;
    uint16_t base = data->base;
    volatile uint64_t timeout = (get_time_in_microseconds() + microseconds_to_wait);
    while((int)(timeout - get_time_in_microseconds()) > 0) {
        uint8_t status_register = inb(base + 7);
        if((status_register & 0x88) == 0x08) { // data are ready
            return SUCCESS;
        }
        else if((status_register & 0x81) == 0x01) { // error
            return ERROR;
        }
        switch_to_another_thread();
    }
    return ERROR;
}

void ide_select_drive(hardware_t *device, uint8_t drive) {
    ide_controller_data_t *data = (ide_controller_data_t *) device->data;
    uint16_t base = data->base;

    // check if drive is already selected
    if(inb(base + 6) == drive) {
        return;
    }

    // select drive
    ide_wait_drive_not_busy(device, 100000);
    outb(base + 6, drive);
    wait_microseconds(1); // specification requires at least 400 nanoseconds, a little more will not harm
}

void ide_reset_controller(hardware_t *device) {
    ide_controller_data_t *data = (ide_controller_data_t *) device->data;
    uint16_t base = data->base;
    uint16_t control = data->control;

    // get actual selected drive
    uint8_t selected_drive = inb(base + 6);

    // reset controller with disabled interrupts
    outb(control, (1 << 2) | (1 << 1));
    sleep_current_thread(10); // specification requires at least 5 microseconds, a little more will not harm
    outb(control, (1 << 1));
    sleep_current_thread(10); // specification requires at least 5 microseconds, a little more will not harm
    if(inb(base + 7) != 0xFF) {
        ide_wait_drive_not_busy(device, 100000);
    }

    // acknowledge interrupt status register
    inb(base + 7);

    // enable interrupts
    outb(control, 0);

    // restore previously selected drive
    ide_select_drive(device, selected_drive);
}

void ide_dump_status(hardware_t *device) {
    ide_controller_data_t *data = (ide_controller_data_t *) device->data;
    uint16_t base = data->base;
    uint8_t status = inb(base + 7);
    log("\n[IDE] 0x%04X Status 0x%02X:", base + 7, status);
    if((status & 0x80) == 0x80) {
        log(" BSY");
    }
    if((status & 0x40) == 0x40) {
        log(" DRDY");
    }
    if((status & 0x20) == 0x20) {
        log(" DF");
    }
    if((status & 0x10) == 0x10) {
        log(" DSC");
    }
    if((status & 0x08) == 0x08) {
        log(" DRQ");
    }
    if((status & 0x04) == 0x04) {
        log(" CORR");
    }
    if((status & 0x02) == 0x02) {
        log(" IDX");
    }
    if((status & 0x01) == 0x01) {
        log(" ERR (");
        uint8_t error = inb(base + 1);
        if((error & 0x80) == 0x80) {
            log("bad block");
        }
        if((error & 0x40) == 0x40) {
            log("uncorrectable data");
        }
        if((error & 0x20) == 0x20) {
            log("media changed");
        }
        if((error & 0x10) == 0x10) {
            log("ID mark not found");
        }
        if((error & 0x08) == 0x08) {
            log("media change request");
        }
        if((error & 0x04) == 0x04) {
            log("command aborted");
        }
        if((error & 0x02) == 0x02) {
            log("track 0 not found");
        }
        if((error & 0x01) == 0x01) {
            log("no address mark");
        }
        log(")");
    }
}