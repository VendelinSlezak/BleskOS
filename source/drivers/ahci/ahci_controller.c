//BleskOS

/*
* MIT License
* Copyright (c) 2023-2025 BleskOS developers
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

void ahci_add_new_pci_device(struct pci_device_info_t device) {
    // check number of already connected cards
    if(components->n_ahci >= MAX_NUMBER_OF_AHCI_CONTROLLERS) {
        return;
    }

    // log driver
    logf("%s", __FILE__);

    // save basic device informations
    copy_memory((dword_t)&device, (dword_t)&components->ahci[components->n_ahci].pci, sizeof(struct pci_device_info_t));

    // read other device informations
    components->ahci[components->n_ahci].base = pci_get_mmio(device, PCI_BAR5);

    // configure PCI
    pci_set_bits(device, 0x04, PCI_STATUS_DISABLE_INTERRUPTS | PCI_STATUS_BUSMASTERING | PCI_STATUS_MMIO);

    // enable AHCI mode
    mmio_outd(components->ahci[components->n_ahci].base + 0x04, 0x80000000);

    // start BIOS handoff
    if(mmio_ind(components->ahci[components->n_ahci].base + 0x10) >= 0x00010200) { // check version
        // check if BIOS ownership is supported
        if((mmio_ind(components->ahci[components->n_ahci].base + 0x24) & 0x1)==0x1) {
            // send signal that we want to take ownership
            mmio_outd(components->ahci[components->n_ahci].base + 0x28, 0x2);
            
            // set timeout
            components->ahci[components->n_ahci].pci.bios_handoff_timeout = (time_of_system_running + 2000);
        }
    }

    // update number of devices
    components->n_ahci++;
}

void initalize_ahci_controller(byte_t number_of_controller) {
    // log
    logf("\n\nDriver: AHCI controller\nDevice: PCI bus %d:%d:%d:%d",
        components->ahci[number_of_controller].pci.segment,
        components->ahci[number_of_controller].pci.bus,
        components->ahci[number_of_controller].pci.device,
        components->ahci[number_of_controller].pci.function);
    dword_t base = components->ahci[number_of_controller].base;

    // disable interrupts
    mmio_outd(base + 0x04, 0x80000000);

    // log version
    logf("\nVersion: %d.", mmio_ind(base + 0x10)>>16);
    if((mmio_ind(base + 0x10) & 0xFFFF)==0x0905) {
        logf("95");
    }
    else {
        logf("%d.%d", (mmio_ind(base + 0x10)>>8) & 0xFF, mmio_ind(base + 0x10) & 0xFF);
    }

    // disable BIOS ownership
    if(mmio_ind(base + 0x10) >= 0x00010200) { // check version
        // check if BIOS ownership is supported
        if((mmio_ind(base + 0x24) & 0x1)==0x1) {
            // wait for BIOS handoff
            while(time_of_system_running < components->ahci[components->n_ahci].pci.bios_handoff_timeout) {
                asm("nop");

                if((mmio_ind(base + 0x28) & 0x3) == 0x2) {
                    logf("\nBIOS ownership released successfully");
                    break;
                }
            }
            if(((mmio_ind(base + 0x28) & 0x3) != 0x2) || time_of_system_running >= components->ahci[components->n_ahci].pci.bios_handoff_timeout) {
                logf("\nAHCI ERROR: BIOS did not released ownership");
                return;
            }
        }
    }

    // reset controller
    mmio_outd(base + 0x04, 0x80000001); // set reset bit
    if(ahci_wait(base + 0x04, 0x1, 0x0, 500)==STATUS_GOOD) {
        wait(100); // wait for ports to finish reset
        logf("\nAHCI: controller has been reset");
    }
    else {
        logf("\nAHCI ERROR: can not leave reset");
        return;
    }

    // enable AHCI mode if it was disabled by reset
    mmio_outd(base + 0x04, 0x80000000);

    // scan for ports
    dword_t ahci_port_base_address = 0;
    for(dword_t port_number = 0, mask = 1; port_number < 32; port_number++, mask <<= 1) {
        // test if port exist
        if((mmio_ind(base + 0x0C) & mask)==mask) {
            logf("\nPort %d ", port_number);

            // calculate base address of port
            ahci_port_base_address = (base + 0x100 + (port_number*0x80));

            // enable power for port
            if((mmio_ind(ahci_port_base_address + 0x18) & 0x4) != 0x4) {
                mmio_outd(ahci_port_base_address + 0x18, mmio_ind(ahci_port_base_address + 0x18) | 0x4);
                if(ahci_wait(ahci_port_base_address + 0x18, 0x4, 0x4, 100)==STATUS_ERROR) {
                    logf("can not be powered");
                    continue; //go to next port
                }
            }

            // spin-up device
            if((mmio_ind(ahci_port_base_address + 0x18) & 0x2) != 0x2) {
                // clear SERR register
                mmio_outd(ahci_port_base_address + 0x30, 0xFFFFFFFF);
                wait(10);

                // clear SCTL register to be sure that no action is requested here
                mmio_outd(ahci_port_base_address + 0x2C, 0);
                wait(10);

                // spin-up device - it will cause initalization sequence
                mmio_outd(ahci_port_base_address + 0x18, mmio_ind(ahci_port_base_address + 0x18) | 0x2);
                if(ahci_wait(ahci_port_base_address + 0x18, 0x2, 0x2, 100) == STATUS_ERROR) {
                    logf("can not be spinned-up");
                    continue; //go to next port
                }

                // wait for communication to be estabilished
                if(ahci_wait(ahci_port_base_address + 0x28, 0xF, 0x3, 200)==STATUS_ERROR) {
                    logf("can not estabilish communication");
                    continue; //go to next port
                }
            }

            // clear SERR register to acknowledge interrupt, otherwise device will be stuck in endless BSY mode
            mmio_outd(ahci_port_base_address + 0x30, 0xFFFFFFFF);
            wait(10);

            // test if port is in working state
            dword_t port_status = mmio_ind(ahci_port_base_address + 0x28);
            if((port_status & 0xF) == 3
                && ((port_status>>4) & 0xF) != 0
                && ((port_status>>8) & 0xF) == 1) {
                // after reset, device will send FIS with his signature, so enable receiving FIS to receive it
                mmio_outd(ahci_port_base_address + 0x18, mmio_ind(ahci_port_base_address + 0x18) | 0x10);
                wait(10);

                // move port to idle state
                if((mmio_ind(ahci_port_base_address + 0x18) & 0x11)!=0x00) {
                    mmio_outd(ahci_port_base_address + 0x18, mmio_ind(ahci_port_base_address + 0x18) & ~0x01); // clear start bit
                    if(ahci_wait(ahci_port_base_address + 0x18, 0x1, 0x0, 100)==STATUS_ERROR) {
                        logf("\nAHCI ERROR: port command list can not be stopped");
                        continue; //go to next port
                    }
                    mmio_outd(ahci_port_base_address + 0x18, mmio_ind(ahci_port_base_address + 0x18) & ~0x10); // clear FIS receive enable
                    if(ahci_wait(ahci_port_base_address + 0x18, 0x10, 0x0, 100)==STATUS_ERROR) {
                        logf("\nAHCI ERROR: port FIS receive can not be stopped");
                        continue; //go to next port
                    }
                }

                // check signature to see what type of device is connected
                if(mmio_ind(ahci_port_base_address + 0x24)==AHCI_NO_DEVICE_ATTACHED) {
                    logf("has no device attached");
                }
                else if(mmio_ind(ahci_port_base_address + 0x24)==AHCI_SATA_DEVICE) {
                    logf("has SATA device");
                    if(hard_disk_info.controller_type==NO_CONTROLLER) {
                        //enable port
                        hard_disk_info.base_2 = (dword_t) (ahci_enable_port(ahci_port_base_address));
                        if(hard_disk_info.base_2 == STATUS_ERROR) {
                            continue; //go to next port
                        }
                        hard_disk_info.controller_type = AHCI_CONTROLLER;
                        hard_disk_info.device_port = port_number;
                        hard_disk_info.base_1 = ahci_port_base_address;

                        //send IDENTIFY command to read hard disk size
                        if(ahci_send_identify_command(hard_disk_info.base_1, hard_disk_info.base_2)==STATUS_GOOD) {
                            //read useful values
                            if(device_info->lba48_total_number_of_sectors!=0) {
                                if(device_info->lba48_total_number_of_sectors>0xFFFFFFFF) {
                                    hard_disk_info.number_of_sectors = 0xFFFFFFFF;
                                }
                                else {
                                    hard_disk_info.number_of_sectors = device_info->lba48_total_number_of_sectors;
                                }
                            }
                            else {
                                hard_disk_info.number_of_sectors = device_info->lba28_total_number_of_sectors;
                            }
                        }
                        else {
                            logf("\nAHCI: can not get IDENTIFY command from hard disk");
                            hard_disk_info.number_of_sectors = 0;
                        }
                    }
                }
                else if(mmio_ind(ahci_port_base_address + 0x24)==AHCI_SATAPI_DEVICE) {
                    logf("has SATAPI device");
                    if(optical_drive_info.controller_type==NO_CONTROLLER) {
                        //enable port
                        optical_drive_info.base_2 = (dword_t) (ahci_enable_port(ahci_port_base_address));
                        if(optical_drive_info.base_2==STATUS_ERROR) {
                            continue; //go to next port
                        }
                        optical_drive_info.controller_type = AHCI_CONTROLLER;
                        optical_drive_info.device_port = port_number;
                        optical_drive_info.base_1 = ahci_port_base_address;
                        optical_drive_info.number_of_sectors = 0;
                    }
                }
                else {
                    logf("signature is 0x%x", mmio_ind(ahci_port_base_address + 0x28));
                }
            }
            else {
                logf("is not active");
            }
        }
    }
}

byte_t ahci_wait(dword_t base_address, dword_t mask, dword_t desired_result, dword_t time) {
    volatile dword_t timeout = (time_of_system_running+time);

    while(time_of_system_running < timeout) {
        asm("nop");

        if((mmio_ind(base_address) & mask)==desired_result) {
            return STATUS_GOOD;
        }
    }

    return STATUS_ERROR;
}

byte_t *ahci_enable_port(dword_t port_base_address) {
    byte_t *command_list_memory = (byte_t *) aligned_calloc(0x2000, 0xFFF);
    byte_t *receive_fis_memory = (byte_t *) aligned_calloc(0x2000, 0xFFF);

    //set memory structures
    mmio_outd(port_base_address + 0x00, (dword_t)command_list_memory);
    mmio_outd(port_base_address + 0x04, 0);
    mmio_outd(port_base_address + 0x08, (dword_t)receive_fis_memory);
    mmio_outd(port_base_address + 0x0C, 0);

    //start port
    mmio_outd(port_base_address + 0x18, mmio_ind(port_base_address + 0x18) | 0x01); //set start bit
    if(ahci_wait(port_base_address + 0x18, 0x1, 0x1, 100)==STATUS_ERROR) {
        logf("\nAHCI ERROR: port command list can not be started");
        return STATUS_ERROR;
    }
    mmio_outd(port_base_address + 0x18, mmio_ind(port_base_address + 0x18) | 0x10); //set FIS receive enable
    if(ahci_wait(port_base_address + 0x18, 0x10, 0x10, 100)==STATUS_ERROR) {
        logf("\nAHCI ERROR: port FIS receive can not be started");
        return STATUS_ERROR;
    }

    //return memory for command list, that will be used also for PRD tables
    //we do not need to worry about FIS memory, we will leave it for use of device
    return command_list_memory;
}

byte_t ahci_send_command(dword_t port_base_address, dword_t command_list_memory, byte_t command_type, byte_t command_direction, byte_t atapi_command[0x10], byte_t command, word_t sector_count, dword_t lba, dword_t memory, dword_t byte_count) {
    //wait for possibility to send command
    if(ahci_wait(port_base_address + 0x20, 0x88, 0x0, 200)==STATUS_ERROR) {
        logf("\nAHCI ERROR: BSY and DRQ are not clear %x %x %x", mmio_ind(port_base_address + 0x10), mmio_ind(port_base_address + 0x20), mmio_ind(port_base_address + 0x30));
        return STATUS_ERROR;
    }

    //clear all memory involved
    clear_memory(command_list_memory, 0x2000);

    //set command list entry 0
    struct ahci_command_list_entry_t *ahci_command_list_entry = (struct ahci_command_list_entry_t *) (command_list_memory);
    ahci_command_list_entry->command_fis_length_in_dwords = 5;
    ahci_command_list_entry->atapi = command_type;
    ahci_command_list_entry->write = command_direction;
    ahci_command_list_entry->number_of_command_table_entries = 1;
    ahci_command_list_entry->command_table_low_memory = (command_list_memory+0x1000);

    //set FIS command
    struct ahci_command_and_prd_table_t *ahci_command_and_prd_table = (struct ahci_command_and_prd_table_t *) (command_list_memory+0x1000);
    ahci_command_and_prd_table->fis_type = 0x27;
    ahci_command_and_prd_table->flags = 0x80;
    ahci_command_and_prd_table->command = command;
    ahci_command_and_prd_table->lba_0 = ((lba>>0) & 0xFF);
    ahci_command_and_prd_table->lba_1 = ((lba>>8) & 0xFF);
    ahci_command_and_prd_table->lba_2 = ((lba>>16) & 0xFF);
    ahci_command_and_prd_table->device_head = 0xE0;
    ahci_command_and_prd_table->lba_3 = ((lba>>24) & 0xFF);
    ahci_command_and_prd_table->sector_count_low = ((sector_count>>0) & 0xFF);
    ahci_command_and_prd_table->sector_count_high = ((sector_count>>8) & 0xFF);
    ahci_command_and_prd_table->control = 0x08;

    //set ATAPI command
    if(command_type == AHCI_ATAPI) {
        for(dword_t i = 0; i < 0x10; i++) {
            ahci_command_and_prd_table->atapi_command[i] = atapi_command[i];
        }
    }
    
    //set PRD table
    if(byte_count != 0) {
        ahci_command_and_prd_table->data_base_low_memory = memory;
        ahci_command_and_prd_table->data_byte_count = (byte_count-1);
    }

    //wait to be possible to send command list entry 0
    if(ahci_wait(port_base_address + 0x38, 0x01, 0x0, 100) == STATUS_ERROR) {
        logf("\nAHCI ERROR: command 0 is not free");
        return STATUS_ERROR;
    }

    //clear error register
    mmio_outd(port_base_address + 0x10, 0xFFFFFFFF);

    //send command list entry 0
    mmio_outd(port_base_address + 0x38, 0x01);

    //wait for transfer outcome
    volatile dword_t timeout = (time_of_system_running + 4000); // wait max 4 seconds
    while(time_of_system_running < timeout) {
        asm("nop");

        if((mmio_ind(port_base_address + 0x38) & 0x1)==0x0) { //command was processed
            if((mmio_ind(port_base_address + 0x10) & 0x40000000)==0x0) { //check for error
                return STATUS_GOOD;
            }
        }
        
        if((mmio_ind(port_base_address + 0x10) & 0x40000000)==0x40000000) { //error
            //we need to clear COMMAND_ISSUE register to be able to send commands again, to do so we will restart command list
            mmio_outd(port_base_address + 0x18, mmio_ind(port_base_address + 0x18) & ~0x01); //clear start bit
            if(ahci_wait(port_base_address + 0x18, 0x1, 0x0, 100)==STATUS_ERROR) {
                logf("\nAHCI ERROR: port command list can not be stopped");
            }
            mmio_outd(port_base_address + 0x18, mmio_ind(port_base_address + 0x18) | 0x01); //set start bit
            if(ahci_wait(port_base_address + 0x18, 0x1, 0x1, 100)==STATUS_ERROR) {
                logf("\nAHCI ERROR: port command list can not be started");
            }

            return STATUS_ERROR;
        }
    }
    
    //timeout
    logf("\nAHCI: command timeout");
    return STATUS_ERROR;
}

byte_t ahci_send_ata_command(dword_t port_base_address, dword_t command_list_memory, byte_t command_direction, byte_t command, word_t sector_count, dword_t lba, dword_t memory, dword_t byte_count) {
    return ahci_send_command(port_base_address, command_list_memory, AHCI_ATA, command_direction, 0, command, sector_count, lba, memory, byte_count);
}

byte_t ahci_send_atapi_command(dword_t port_base_address, dword_t command_list_memory, dword_t memory, word_t byte_count, byte_t atapi_command[0x10]) {
    return ahci_send_command(port_base_address, command_list_memory, AHCI_ATAPI, AHCI_READ, atapi_command, 0xA0, 0, (byte_count<<8), memory, byte_count);
}

byte_t ahci_send_identify_command(dword_t port_base_address, dword_t command_list_memory) {
    //clear device info memory
    clear_memory((dword_t)device_info, 512);

    //transfer
    return ahci_send_ata_command(port_base_address, command_list_memory, AHCI_READ, 0xEC, 0, 0, (dword_t)device_info, 512);
}