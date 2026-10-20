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
#include <kernel/hardware/devices/memory/virtual_memory.h>
#include <kernel/hardware/main.h>
#include <kernel/hardware/devices/cpu/commands.h>
#include <kernel/hardware/devices/cpu/scheduler.h>
#include <kernel/hardware/devices/cpu/mutex.h>
#include <kernel/hardware/devices/cpu/interrupt.h>
#include <kernel/hardware/devices/cpu/info.h>
#include <kernel/hardware/devices/hard_disk/parallel_ata.h>
#include <kernel/libc/string.h>

/* local variables */
uint32_t is_isa_compatibile_primary_controller_present = false;
uint32_t is_isa_compatibile_secondary_controller_present = false;
uint32_t number_of_controllers = 0;
hardware_t **ide_controllers = NULL;

/* functions for controller initialization */
void initialize_ide_entry_on_pci(hardware_t *device) {
    uint8_t progif = pci_inb(device, 0x9);
    uint32_t has_dma_capability = ((progif & 0x80) == 0x80);
    uint16_t dma_base_address = (has_dma_capability) ? pci_get_io(device, PCI_BAR4) : NULL; // TODO: it can be also memory mapped
    interrupt_description_t interrupt;

    // initialize PCI bus
    pci_enable_io_space(device);
    if(has_dma_capability) {
        pci_enable_busmastering(device);
    }

    // primary controller is ISA compatibile
    if(((progif & 0x01) == 0x00)) {
        if(is_isa_compatibile_primary_controller_present == false) {
            // check if bus is not floating
            if(inb(0x1F0 + 7) != 0xFF) {
                ide_controller_data_t *ide_controller_data = kalloc(sizeof(ide_controller_data_t));
                ide_controller_data->base = 0x1F0;
                ide_controller_data->control = 0x3F6;
                ide_controller_data->dma = dma_base_address;
                hardware_t *primary_ide_controller = add_hardware(device, "Primary IDE controller", NULL, ide_controller_data, initialize_ide_controller, NULL);
                primary_ide_controller->data = ide_controller_data;
                ide_controllers = krealloc(ide_controllers, sizeof(hardware_t *) * (number_of_controllers + 1));
                ide_controllers[number_of_controllers] = primary_ide_controller;
                number_of_controllers++;
                interrupt = set_isa_interrupt_handler(14, ide_interrupt_handler, (uint32_t)&ide_interrupt_handler_thread, NULL);
                ide_controller_data->interrupt_processor_index = interrupt.processor_index;
                init_hardware(primary_ide_controller);
            }

            is_isa_compatibile_primary_controller_present = true;
        }
    }
    // primary controller is PCI native
    else {
        // check if bus is not floating
        if(inb(pci_get_io(device, PCI_BAR0) + 7) != 0xFF) {
            ide_controller_data_t *ide_controller_data = kalloc(sizeof(ide_controller_data_t));
            ide_controller_data->base = pci_get_io(device, PCI_BAR0);
            ide_controller_data->control = (pci_get_io(device, PCI_BAR1) + 2);
            ide_controller_data->dma = dma_base_address;
            hardware_t *primary_ide_controller = add_hardware(device, "Primary IDE controller", NULL, ide_controller_data, initialize_ide_controller, NULL);
            primary_ide_controller->data = ide_controller_data;
            ide_controllers = krealloc(ide_controllers, sizeof(hardware_t *) * (number_of_controllers + 1));
            ide_controllers[number_of_controllers] = primary_ide_controller;
            number_of_controllers++;
            interrupt = set_pci_interrupt_handler(device, ide_interrupt_handler, (uint32_t)&ide_interrupt_handler_thread, NULL);
            ide_controller_data->interrupt_processor_index = interrupt.processor_index;
            init_hardware(primary_ide_controller);
        }
    }

    // secondary controller is ISA compatibile
    if(((progif & 0x04) == 0x00)) {
        if(is_isa_compatibile_secondary_controller_present == false) {
            // check if bus is not floating
            if(inb(0x170 + 7) != 0xFF) {
                ide_controller_data_t *ide_controller_data = kalloc(sizeof(ide_controller_data_t));
                ide_controller_data->base = 0x170;
                ide_controller_data->control = 0x376;
                ide_controller_data->dma = (dma_base_address + 8);
                hardware_t *secondary_ide_controller = add_hardware(device, "Secondary IDE controller", NULL, ide_controller_data, initialize_ide_controller, NULL);
                secondary_ide_controller->data = ide_controller_data;
                ide_controllers = krealloc(ide_controllers, sizeof(hardware_t *) * (number_of_controllers + 1));
                ide_controllers[number_of_controllers] = secondary_ide_controller;
                number_of_controllers++;
                interrupt = set_isa_interrupt_handler(15, ide_interrupt_handler, (uint32_t)&ide_interrupt_handler_thread, NULL);
                ide_controller_data->interrupt_processor_index = interrupt.processor_index;
                init_hardware(secondary_ide_controller);
            }

            is_isa_compatibile_secondary_controller_present = true;
        }
    }
    // secondary controller is PCI native
    else {
        // check if bus is not floating
        if(inb(pci_get_io(device, PCI_BAR2) + 7) != 0xFF) {
            ide_controller_data_t *ide_controller_data = kalloc(sizeof(ide_controller_data_t));
            ide_controller_data->base = pci_get_io(device, PCI_BAR2);
            ide_controller_data->control = (pci_get_io(device, PCI_BAR3) + 2);
            ide_controller_data->dma = (dma_base_address + 8);
            hardware_t *secondary_ide_controller = add_hardware(device, "Secondary IDE controller", NULL, ide_controller_data, initialize_ide_controller, NULL);
            secondary_ide_controller->data = ide_controller_data;
            ide_controllers = krealloc(ide_controllers, sizeof(hardware_t *) * (number_of_controllers + 1));
            ide_controllers[number_of_controllers] = secondary_ide_controller;
            number_of_controllers++;
            interrupt = set_pci_interrupt_handler(device, ide_interrupt_handler, (uint32_t)&ide_interrupt_handler_thread, NULL);
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

    // initialize DMA
    if(data->dma != NULL) {
        uint16_t dma = data->dma;
        ide_prd_t *prdt = aligned_kalloc(4096, 4096);
        data->prdt = prdt;
        uint32_t prdt_pm = (uint32_t) get_pm_of_vm_page(PAGE_MASK((uint32_t) prdt));

        // stop transfers
        outb(dma + 0, (0 << 0));

        // write PRDT address
        outd(dma + 4, prdt_pm);

        // clear status bits
        outb(dma + 2, (1 << 2) | (1 << 1));
    }

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
    uint32_t status = ide_send_read_command(device, drive, ide_send_identify_command, base, 100000, NULL, NULL);
    
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
                hard_disk_info->dma = data->dma;
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

/* functions for changing controller state */
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

    log("\n[IDE] Resetting controller 0x%04X", base);

    // we need to be sure that no transfer is in progress
    LOCK_MUTEX_WAIT_IN_SWITCHES(&data->mutex);

    // get actual selected drive
    uint8_t selected_drive = inb(base + 6);

    // reset controller with disabled interrupts
    outb(control, (1 << 2) | (1 << 1));
    sleep_current_thread(10); // specification requires at least 5 microseconds, a little more will not harm
    outb(control, (1 << 1)); // stop reset
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

    // now everything should work fine
    UNLOCK_MUTEX(&data->mutex);
}

/* functions for processing interrupts */
void ide_interrupt_handler(interrupt_stack_t *stack_of_interrupt) {
    // log("\n[IDE] Interrupt");
    for(int i = 0; i < number_of_controllers; i++) {
        ide_controller_data_t *data = (ide_controller_data_t *) ide_controllers[i]->data;
        uint16_t base = data->base;
        uint16_t control = data->control;
        uint16_t dma = data->dma;
        uint8_t status_register = inb(control + 0); // get status
        if((status_register & 0x80) == 0x80) { // disk is busy
            continue;
        }

        // acknowledge interrupt
        status_register = inb(base + 7);

        // check if this interrupt is for this controller
        if(data->interrupt_expected == false) {
            continue;
        }
        else if(dma != NULL && (inb(dma + 2) & (1 << 2)) == 0) {
            continue;
        }

        // check exact status
        if((status_register & 0x08) == 0x08) {
            data->transfer_status = IDE_DATA_READY;
        }
        else if((status_register & 0x01) == 0x01) {
            data->transfer_status = IDE_ERROR;
            data->number_of_iht_function_calls = 0;
            data->number_of_expected_iht_function_calls = 0;
            data->max_number_of_expected_iht_function_calls = 0;
        }
        else { // disk is ready
            data->transfer_status = IDE_COMMAND_COMPLETED;
        }

        // destroy timeout thread
        if(data->timeout_thread != NULL) {
            kill_other_kernel_thread(data->timeout_thread);
            data->timeout_thread = NULL;
        }

        // prepare call of iht function
        if(data->number_of_expected_iht_function_calls < data->max_number_of_expected_iht_function_calls) {
            data->number_of_expected_iht_function_calls++;
        }
        data->interrupt_expected = false;

        // unblock waiting thread
        if(data->max_number_of_expected_iht_function_calls == 0) {
            unblock_kernel_thread(data->waiting_thread_for_irq);
        }
    }
}

void ide_interrupt_handler_thread(void) {
    for(int i = 0; i < number_of_controllers; i++) {
        ide_controller_data_t *data = (ide_controller_data_t *) ide_controllers[i]->data;
        while(data->number_of_iht_function_calls < data->number_of_expected_iht_function_calls) {
            data->number_of_iht_function_calls++;
            if(data->number_of_iht_function_calls == data->max_number_of_expected_iht_function_calls) {
                data->iht_function(data->iht_function_arguments);
                data->transfer_status = IDE_COMMAND_COMPLETED;
                unblock_kernel_thread(data->waiting_thread_for_irq);
            }
            else {
                data->interrupt_expected = true;
                data->timeout_thread = create_sleeping_kernel_thread_on_processor("IDE timeout", data->interrupt_processor_index, (uint32_t)ide_timeout, (uint32_t []) { (uint32_t) ide_controllers[i] }, 1, 100000);
                data->iht_function(data->iht_function_arguments);
            }
        }
    }
}

void ide_command_processed(hardware_t *controller) {
    ide_controller_data_t *data = (ide_controller_data_t *) controller->data;
    UNLOCK_MUTEX(&data->mutex);
}

void ide_timeout(hardware_t *controller) {
    lock_core();
    logical_processor_t *lpdata = get_current_logical_processor_struct();
    ide_controller_data_t *data = (ide_controller_data_t *) controller->data;
    data->interrupt_expected = false;
    data->transfer_status = IDE_TIMEOUT;
    unblock_kernel_thread(data->waiting_thread_for_irq);
    unlock_core();
}

/* functions for sending commands to controller */
uint32_t ide_send_dma_command(hardware_t *controller, uint32_t drive, void *buffer_ptr, uint32_t size, uint32_t direction, void (*send_command)(uint32_t argument), uint32_t argument, uint32_t microseconds_timeout) {
    // be sure that controller is ours
    ide_controller_data_t *data = (ide_controller_data_t *) controller->data;
    LOCK_MUTEX_WAIT_IN_SWITCHES(&data->mutex); // TODO: replace mutex with waiting list where will be added all other waiting threads, which will be then unblocked

    // stop all DMA transfers
    uint16_t dma = data->dma;
    outb(dma + 0, (0 << 0));

    // select drive
    ide_select_drive(controller, drive);

    // sending of command needs specific sequence to prevent race conditions, now we will block thread and only then complete sending
    data->max_number_of_expected_iht_function_calls = 0;
    block_current_thread_but_keep_going();

    // current thread is blocked and will wait for command completion
    kernel_thread_t *current_thread = get_current_kernel_thread();
    data->waiting_thread_for_irq = current_thread;
    data->interrupt_expected = true;

    // fill PRDT table with PRD entries
    ide_prd_t *prdt = data->prdt;
    uint32_t buffer = (uint32_t) buffer_ptr;
    uint32_t bytes_to_skip_in_first_page = (buffer - (buffer / PAGE_SIZE * PAGE_SIZE));
    uint32_t bytes_to_read_in_first_page = (PAGE_SIZE - bytes_to_skip_in_first_page);
    if(bytes_to_read_in_first_page > size) {
        bytes_to_read_in_first_page = size;
    }
    if(bytes_to_read_in_first_page > 0) {
        prdt->physical_address_of_buffer = (uint32_t) (get_pm_of_vm_page(PAGE_MASK(buffer)) + bytes_to_skip_in_first_page);
        prdt->byte_count = bytes_to_read_in_first_page;
        size -= bytes_to_read_in_first_page;
        buffer += bytes_to_read_in_first_page;
        prdt->last_prd = (size == 0) ? 1 : 0;
        prdt++;
    }
    while(size > 0) {
        prdt->physical_address_of_buffer = (uint32_t) get_pm_of_vm_page(PAGE_MASK(buffer));
        prdt->byte_count = (size > PAGE_SIZE) ? PAGE_SIZE : size;
        size -= prdt->byte_count;
        buffer += prdt->byte_count;
        prdt->last_prd = (size == 0) ? 1 : 0;
        prdt++;
    }
    mfence(); // TODO: is this needed?

    // set direction of DMA transfer
    outb(dma + 0, (direction == IDE_DMA_READ) ? (1 << 3) : (0 << 3));

    // clear relevant status bits
    outb(dma + 2, (1 << 2) | (1 << 1));

    // create timeout thread, timeout MUST be always bigger than the time it takes to execute arguments->send_command()
    data->timeout_thread = create_sleeping_kernel_thread_on_processor("IDE timeout", data->interrupt_processor_index, (uint32_t)ide_timeout, (uint32_t []) { (uint32_t) controller }, 1, microseconds_timeout);

    // call functions which will send command to controller and trigger interrupt
    send_command(argument);

    // start DMA transfer
    outb(dma + 0, inb(dma + 0) | (1 << 0));

    switch_to_another_thread(); // now this thread is blocked, and will be unblocked either by arrival of interrupt, or timeout thread

    // thread was unblocked, that mean that either IRQ arrived, or timeout happened, in both cases, stop DMA transfer
    outb(dma + 0, (0 << 0));

    // check if DMA error happened
    if((inb(dma + 2) & (1 << 1)) == (1 << 1)) {
        data->transfer_status = IDE_ERROR;
        // TODO: check if this is CRC error, if so, retry sending command
        outb(dma + 2, (1 << 2)); // clear error bit
        UNLOCK_MUTEX(&data->mutex);
        ide_reset_controller(controller); // reset controller to be sure that device is out of error state
    }

    // return status to caller to process it
    return data->transfer_status;
}

uint32_t ide_send_read_command(hardware_t *controller, uint32_t drive, void (*send_command)(uint32_t argument), uint32_t argument, uint32_t microseconds_timeout, void (*iht_function)(uint32_t arguments_ptr), uint32_t expected_irqs) {
    // be sure that controller is ours
    ide_controller_data_t *data = (ide_controller_data_t *) controller->data;
    LOCK_MUTEX_WAIT_IN_SWITCHES(&data->mutex); // TODO: replace mutex with waiting list where will be added all other waiting threads, which will be then unblocked

    // select drive
    ide_select_drive(controller, drive);

    // prepare interrupt handler
    data->waiting_thread_for_irq = get_current_kernel_thread();
    data->interrupt_expected = true;
    data->iht_function = iht_function;
    data->iht_function_arguments = (uint32_t) argument;
    data->number_of_iht_function_calls = 0;
    data->number_of_expected_iht_function_calls = 0;
    data->max_number_of_expected_iht_function_calls = expected_irqs;

    // sending of command needs specific sequence to prevent race conditions, now we will block thread and only then complete sending
    block_current_thread_but_keep_going();

    // create timeout thread, timeout MUST be always bigger than the time it takes to execute arguments->send_command()
    data->timeout_thread = create_sleeping_kernel_thread_on_processor("IDE timeout", data->interrupt_processor_index, (uint32_t)ide_timeout, (uint32_t []) { (uint32_t) controller }, 1, microseconds_timeout);

    // call functions which will send command to controller and trigger interrupt
    send_command(argument);
    switch_to_another_thread(); // now this thread is blocked, and will be unblocked either by arrival of interrupt, or timeout thread

    // thread was unblocked, that mean that either IRQ arrived, or timeout happened
    // in both cases, send status to caller to process it
    return data->transfer_status;
}

uint32_t ide_send_write_command(hardware_t *controller, uint32_t drive, void (*send_command)(uint32_t argument), uint32_t argument, uint32_t microseconds_timeout, void (*write_data)(uint32_t argument), uint32_t expected_irqs) {
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

    // prepare interrupt handler
    data->waiting_thread_for_irq = get_current_kernel_thread();
    data->interrupt_expected = true;
    data->iht_function = write_data;
    data->iht_function_arguments = (uint32_t) argument;
    data->number_of_iht_function_calls = 0;
    data->number_of_expected_iht_function_calls = 0;
    data->max_number_of_expected_iht_function_calls = (expected_irqs - 1); // first interrupt will be just acknowledgement of already sended data

    // sending of data needs specific sequence to prevent race conditions, now we will block thread and only then complete sending
    block_current_thread_but_keep_going();

    // create timeout thread, timeout MUST be always bigger than the time it takes to execute arguments->send_command()
    data->timeout_thread = create_sleeping_kernel_thread_on_processor("IDE timeout", data->interrupt_processor_index, (uint32_t)ide_timeout, (uint32_t []) { (uint32_t) controller }, 1, microseconds_timeout);

    // call functions which will send data to controller and trigger interrupt
    write_data(argument);
    switch_to_another_thread(); // now this thread is blocked, and will be unblocked either by arrival of interrupt, or timeout thread

    // thread was unblocked, that mean that either IRQ arrived, or timeout happened
    // in both cases, send status to caller to process it
    return data->transfer_status;
}

/* functions for checking controller state */
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

/* functions for logging */
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