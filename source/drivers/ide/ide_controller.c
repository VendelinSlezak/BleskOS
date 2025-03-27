//BleskOS

/*
* MIT License
* Copyright (c) 2023-2025 BleskOS developers
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

void ide_add_new_pci_device(struct pci_device_info_t device) {
    // check number of already connected cards
    if(components->n_ide >= MAX_NUMBER_OF_IDE_CONTROLLERS) {
        return;
    }

    // log driver
    logf("%s", __FILE__);

    // read device informations
    byte_t progif = pci_inb(device, 0x09);
    if(progif == 0x00
       || progif == 0x0A
       || progif == 0x80
       || progif == 0x8A) {
        if(ide_is_bus_floating(0x1F0) == STATUS_FALSE) {
            copy_memory((dword_t)&device, (dword_t)&components->ide[components->n_ide].pci, sizeof(struct pci_device_info_t));
            components->ide[components->n_ide].io_base = 0x1F0;
            components->ide[components->n_ide].control_base = 0x3F4;
            components->n_ide++;
        }
        if(components->n_ide >= MAX_NUMBER_OF_IDE_CONTROLLERS) {
            return;
        }
        if(ide_is_bus_floating(0x170) == STATUS_FALSE) {
            copy_memory((dword_t)&device, (dword_t)&components->ide[components->n_ide].pci, sizeof(struct pci_device_info_t));
            components->ide[components->n_ide].io_base = 0x170;
            components->ide[components->n_ide].control_base = 0x374;
            components->n_ide++;
        }
    }
    else {
        if(ide_is_bus_floating(pci_get_io(device, PCI_BAR0)) == STATUS_FALSE) {
            copy_memory((dword_t)&device, (dword_t)&components->ide[components->n_ide].pci, sizeof(struct pci_device_info_t));
            components->ide[components->n_ide].io_base = pci_get_io(device, PCI_BAR0);
            components->ide[components->n_ide].control_base = pci_get_io(device, PCI_BAR1);
            components->n_ide++;
        }
        if(components->n_ide >= MAX_NUMBER_OF_IDE_CONTROLLERS) {
            return;
        }
        if(ide_is_bus_floating(pci_get_io(device, PCI_BAR2)) == STATUS_FALSE) {
            copy_memory((dword_t)&device, (dword_t)&components->ide[components->n_ide].pci, sizeof(struct pci_device_info_t));
            components->ide[components->n_ide].io_base = pci_get_io(device, PCI_BAR2);
            components->ide[components->n_ide].control_base = pci_get_io(device, PCI_BAR3);
            components->n_ide++;
        }
    }

    // configure PCI
    pci_set_bits(device, 0x04, PCI_STATUS_DISABLE_INTERRUPTS | PCI_STATUS_BUSMASTERING | PCI_STATUS_IO);
}

void initalize_ide_controller(byte_t number_of_controller) {
    // log
    logf("\n\nDriver: IDE controller\nDevice: PCI bus %d:%d:%d:%d",
        components->ide[number_of_controller].pci.segment,
        components->ide[number_of_controller].pci.bus,
        components->ide[number_of_controller].pci.device,
        components->ide[number_of_controller].pci.function);

    // reset controller
    ide_reset_controller(components->ide[number_of_controller].io_base, components->ide[number_of_controller].control_base);

    // initalize master drive
    initalize_ide_controller_drive(number_of_controller, PATA_MASTER);

    // initalize slave drive
    initalize_ide_controller_drive(number_of_controller, PATA_SLAVE);
}

void initalize_ide_controller_drive(byte_t number_of_controller, byte_t number_of_drive) {
    // select drive
    ide_select_drive(components->ide[number_of_controller].io_base, number_of_drive);

    // if there is floating bus, it mean that this drive do not exist
    if(ide_is_bus_floating(components->ide[number_of_controller].io_base) == STATUS_TRUE) {
        return;
    }

    // send IDENTIFY command that will detect what type of device is connected
    struct ide_drive_info_t drive_info = ide_send_identify_drive(components->ide[number_of_controller].io_base);

    // save info about connected device
    if(number_of_drive == PATA_MASTER) {
        components->ide[number_of_controller].master_drive_present = STATUS_TRUE;
        components->ide[number_of_controller].master_drive_type = drive_info.type;
        components->ide[number_of_controller].master_drive_size = drive_info.size;
    }
    else if(number_of_drive == PATA_SLAVE) {
        components->ide[number_of_controller].slave_drive_present = STATUS_TRUE;
        components->ide[number_of_controller].slave_drive_type = drive_info.type;
        components->ide[number_of_controller].slave_drive_size = drive_info.size;
    }

    // add device to global lists
    if(hard_disk_info.controller_type == NO_CONTROLLER
        && drive_info.type == PATA_HARD_DISK_SIGNATURE
        && drive_info.size != 0) {
        hard_disk_info.controller_type = IDE_CONTROLLER;
        hard_disk_info.device_port = number_of_drive;
        hard_disk_info.base_1 = components->ide[number_of_controller].io_base;
        hard_disk_info.base_2 = components->ide[number_of_controller].control_base;
        hard_disk_info.number_of_sectors = drive_info.size;
    }
    else if(optical_drive_info.controller_type == NO_CONTROLLER
        && drive_info.type == PATA_OPTICAL_DRIVE_SIGNATURE) {
        optical_drive_info.controller_type = IDE_CONTROLLER;
        optical_drive_info.device_port = number_of_drive;
        optical_drive_info.base_1 = components->ide[number_of_controller].io_base;
        optical_drive_info.base_2 = components->ide[number_of_controller].control_base;
        optical_drive_info.number_of_sectors = 0;
    }

    // log info about drive
    logf("\n Drive: ");
    if(number_of_drive == PATA_MASTER) {
        logf("MASTER");
    }
    else if(number_of_drive == PATA_SLAVE) {
        logf("SLAVE");
    }
    logf(" type 0x%04x size %d", drive_info.type, drive_info.size);
}

struct ide_drive_info_t ide_send_identify_drive(word_t base_port) {
    struct ide_drive_info_t info;
    info.type = 0;
    info.size = 0;

    // send IDENTIFY command
    outb(base_port + 2, 0);
    outb(base_port + 3, 0);
    outb(base_port + 4, 0);
    outb(base_port + 5, 0);
    outb(base_port + 7, 0xEC);

    // wait
    byte_t status = ide_wait_for_data(base_port, 50);

    // read drive type
    info.type = (inb(base_port + 4) | (inb(base_port + 5) << 8));

    // if there are no data, return
    if(status == STATUS_ERROR) {
        return info;
    }

    // read device info
    dword_t device_info_memory = ((dword_t)device_info);
    clear_memory(device_info_memory, 512);
    word_t *device_info_pointer = (word_t *) (device_info_memory);
    for(dword_t i = 0; i < 256; i++) {
        device_info_pointer[i] = inw(base_port + 0);
    }

    // read useful values
    if(device_info->lba48_total_number_of_sectors!=0) {
        if(device_info->lba48_total_number_of_sectors>0xFFFFFFFF) {
            info.size = 0xFFFFFFFF;
        }
        else {
            info.size = device_info->lba48_total_number_of_sectors;
        }
    }
    else {
        info.size = device_info->lba28_total_number_of_sectors;
    }

    return info;
}

byte_t ide_is_bus_floating(word_t base_port) {
    // check invalid state of status register
    if(inb(base_port + 7)==0xFF || inb(base_port + 7)==0x7F) {
        return STATUS_TRUE;
    }
    else {
        return STATUS_FALSE;
    }
}

byte_t ide_wait_drive_not_busy(word_t base_port, dword_t wait_ticks) {
    volatile dword_t timeout = (time_of_system_running+wait_ticks);
    
    while(time_of_system_running < timeout) {
        asm("nop");

        if((inb(base_port + 7) & 0x80) == 0x00) { //drive is not busy
            return STATUS_GOOD;
        }
    }
    
    return STATUS_ERROR;
}

byte_t ide_wait_drive_not_busy_with_error_status(word_t base_port, dword_t wait_ticks) {
    volatile dword_t timeout = (time_of_system_running+wait_ticks);
    
    while(time_of_system_running < timeout) {
        asm("nop");

        byte_t status_register = inb(base_port + 7);
        if((status_register & 0x01) == 0x01) { //error
            return STATUS_ERROR;
        }
        else if((status_register & 0x80) == 0x00) { //drive is not busy
            return STATUS_GOOD;
        }
    }
    
    return STATUS_ERROR;
}

byte_t ide_wait_for_data(word_t base_port, dword_t wait_ticks) {
    volatile dword_t timeout = (time_of_system_running+wait_ticks);

    while(time_of_system_running < timeout) {
        asm("nop");
        
        byte_t status_register = inb(base_port + 7);
        if((status_register & 0x88)==0x08) { //data are ready
            return STATUS_GOOD;
        }
        else if((status_register & 0x81)==0x01) { //error
            return STATUS_ERROR;
        }
    }
    
    return STATUS_ERROR;
}

void ide_clear_device_output(word_t base_port) {
    for(dword_t i=0; i<(2048*0xFFFF); i++) {
        if((inb(base_port+7) & 0x08) == 0x08) { //Data Ready bit is set
            inw(base_port+0);
        }
        else {
            return;
        }
    }
}

void ide_reset_controller(word_t base_port, word_t alt_base_port) {
    // get actual selected drive
    byte_t selected_drive = inb(base_port+6);

    // reset controller
    outb(alt_base_port+2, (1<<2) | (1<<1));
    wait(1);
    outb(alt_base_port+2, (1<<1));
    wait(1);
    if(inb(base_port + 7) != 0xFF) {
        ide_wait_drive_not_busy(base_port, 100);
    }

    // restore previously selected drive
    ide_select_drive(base_port, selected_drive);
}

void ide_select_drive(word_t base_port, byte_t drive) {
    // check if drive is already selected
    if(inb(base_port + 6) == drive) {
        return;
    }

    // select drive
    ide_wait_drive_not_busy(base_port, 100);
    outb(base_port + 6, drive);
    wait(1);
}