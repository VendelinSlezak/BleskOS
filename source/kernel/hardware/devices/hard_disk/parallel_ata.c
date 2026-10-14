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
#include <kernel/hardware/groups/logging/logging.h>
#include <kernel/hardware/groups/storage/storage.h>
#include <kernel/hardware/main.h>
#include <kernel/hardware/controllers/ide/ide.h>
#include <kernel/hardware/devices/memory/memory_allocators.h>
#include <kernel/hardware/devices/cpu/scheduler.h>
#include <kernel/hardware/devices/cpu/commands.h>
#include <kernel/libc/string.h>
#include <kernel/hardware/groups/storage/storage.h>

/* local variables */
static storage_group_device_functions_t parallel_ata_functions = {
    .read = parallel_ata_read_wrapper,
    .write = parallel_ata_write_wrapper,
    .convert_lba_to_offset = parallel_ata_convert_lba_to_offset,
};

/* functions */
void initialize_parallel_ata(hardware_t *self) {
    ide_controller_device_data_t *data_from_controller = (ide_controller_device_data_t *) self->data_from_controller;
    ata_identify_command_device_info_t *identify_command_data = (ata_identify_command_device_info_t *) &data_from_controller->device_info;
    self->data = kalloc(sizeof(parallel_ata_data_t));
    parallel_ata_data_t *data = (parallel_ata_data_t *) self->data;

    // convert from big endian to little endian
    switch_16_endian(identify_command_data->model_number, 40);
    switch_16_endian(identify_command_data->serial_number, 20);
    switch_16_endian(identify_command_data->firmware_revision, 8);

    // TODO: find if LBA and DMA are supported
    // TODO: check if number of sectors and reported LBA support are not mismatch
    uint32_t is_lba48_supported = ((identify_command_data->command_sets_supported_2 & (1 << 10)) == (1 << 10));
    uint64_t number_of_sectors = identify_command_data->lba28_total_number_of_sectors;
    uint16_t rotation_rate = identify_command_data->nominal_media_rotation_rate;
    uint16_t udma_modes = identify_command_data->udma_modes;
    if(is_lba48_supported && (identify_command_data->lba48_total_number_of_sectors != 0)) {
        number_of_sectors = identify_command_data->lba48_total_number_of_sectors;
    }
    uint32_t size_of_sector = 512;
    if(    ((identify_command_data->physical_logical_sector_size & 0xC000) == 0x4000)
        && ((identify_command_data->physical_logical_sector_size & (1 << 12)) == (1 << 12))) {
        size_of_sector = (identify_command_data->words_per_logical_sector * 2);
    }

    uint8_t *highest_supported_udma_mode = "DMA is not supported";
    uint8_t *selected_udma_mode = "DMA is not supported";

    log("\n[PATA] Hard disk ATA %d.%d"
        "\n       Model number: %040s"
        "\n       Serial number: %020s"
        "\n       Firmware revision: %08s"
        "\n       LBA48 supported: %d"
        "\n       Number of sectors: %d (0x%x%x)"
        "\n       Sector size: %d"
        "\n       Rotation rate: %d"
        "\n       Highest supported UDMA mode: %s"
        "\n       Selected UDMA mode: %s"
        , identify_command_data->major_version_number
        , identify_command_data->minor_version_number
        , identify_command_data->model_number
        , identify_command_data->serial_number
        , identify_command_data->firmware_revision
        , is_lba48_supported
        , (uint32_t)number_of_sectors, (uint32_t)(number_of_sectors >> 32), (uint32_t)number_of_sectors
        , size_of_sector
        , rotation_rate
        , get_udma_mode_name(udma_modes & 0x3F)
        , get_udma_mode_name((udma_modes >> 8) & 0x3F)
    );

    // fill device data
    if(is_lba48_supported) {
        data->max_number_of_sectors_per_one_request = 65536;
    }
    else {
        data->max_number_of_sectors_per_one_request = 256;
    }
    data->number_of_sectors = number_of_sectors;
    data->size_of_sector = size_of_sector;
    data->does_support_lba48 = is_lba48_supported;

    // add device to group
    add_storage_device(self, &parallel_ata_functions, size_of_sector, number_of_sectors, 1);
    log("\n[PATA] Parallel ATA hard disk %d has been initialized", self->id);

    self->is_initialized = true;
}

uint8_t *get_udma_mode_name(uint8_t udma_mode) {
    if((udma_mode & (1 << 5)) == (1 << 5)) {
        return "UDMA5";
    }
    else if((udma_mode & (1 << 4)) == (1 << 4)) {
        return "UDMA4";
    }
    else if((udma_mode & (1 << 3)) == (1 << 3)) {
        return "UDMA3";
    }
    else if((udma_mode & (1 << 2)) == (1 << 2)) {
        return "UDMA2";
    }
    else if((udma_mode & (1 << 1)) == (1 << 1)) {
        return "UDMA1";
    }
    else if((udma_mode & (1 << 0)) == (1 << 0)) {
        return "UDMA0";
    }
    else {
        return "DMA is not supported";
    }
}

uint32_t parallel_ata_read_wrapper(hardware_t *self, uint64_t lba, uint32_t number_of_sectors, void *buffer, storage_transfer_status_t *status) {
    parallel_ata_data_t *data = (parallel_ata_data_t *) self->data;
    uint32_t size_of_sector = data->size_of_sector;
    uint32_t max_number_of_sectors_per_one_request = data->max_number_of_sectors_per_one_request;

    while(number_of_sectors > 0) {
        uint32_t sectors_per_read = max_number_of_sectors_per_one_request;
        if(sectors_per_read > number_of_sectors) {
            sectors_per_read = number_of_sectors;
        }
        if(parallel_ata_read(self, lba, sectors_per_read, buffer, status) == ERROR) {
            return ERROR;
        }
        lba += sectors_per_read;
        number_of_sectors -= sectors_per_read;
        buffer += (sectors_per_read * size_of_sector);
    }

    return SUCCESS;
}

// this function assumes that lba is valid, and number of sectors is also valid with respect to lba and max number of sectors per request and buffer is as big as size of sectors
uint32_t parallel_ata_read(hardware_t *self, uint64_t lba, uint32_t number_of_sectors, void *buffer, storage_transfer_status_t *status) {
    parallel_ata_data_t *data = (parallel_ata_data_t *) self->data;
    ide_controller_device_data_t *data_from_controller = (ide_controller_device_data_t *) self->data_from_controller;
    hardware_t *controller = self->controller;
    ide_controller_data_t *controller_data = (ide_controller_data_t *) controller->data;
    uint32_t size_of_sector = data->size_of_sector;

    // send command
    parallel_ata_command_arguments_t arguments = {
        .base = controller_data->base,
        .number_of_sectors = (number_of_sectors == data->max_number_of_sectors_per_one_request) ? 0 : number_of_sectors,
        .sector = lba,
    };
    parallel_ata_read_sector_arguments_t read_sector_arguments = {
        .base = controller_data->base,
        .words_of_sector = (size_of_sector / 2),
        .buffer = buffer,
    };
    uint32_t transfer_status = ide_send_read_command(controller, data_from_controller->drive, ide_send_pio_read_command, (uint32_t) &arguments, 1000000);
    while(true) {
        if(transfer_status == IDE_TIMEOUT) {
            log("\n[PATA] Read timeout");
            ide_command_processed(controller);
            return ERROR;
        }
        else if(transfer_status == IDE_ERROR) {
            log("\n[PATA] Read error");
            ide_dump_status(controller);
            ide_command_processed(controller);
            return ERROR;
        }
        else if(number_of_sectors > 1) {
            log("\n[PATA] Sector arrived");
            transfer_status = ide_renew_read_command(controller, parallel_ata_read_sector, (uint32_t) &read_sector_arguments, 100000); // when sector is readed, it will immediately trigger reading and interrupt for next sector
            number_of_sectors--;
            continue;
        }
        else {
            log("\n[PATA] Last sector arrived");
            parallel_ata_read_sector((uint32_t) &read_sector_arguments);
            ide_command_processed(controller);
            return SUCCESS;
        }
    }
}

void ide_send_pio_read_command(uint32_t arguments_ptr) {
    parallel_ata_command_arguments_t *arguments = (parallel_ata_command_arguments_t *) arguments_ptr;
    uint16_t base = arguments->base;
    uint16_t number_of_sectors = arguments->number_of_sectors;
    uint64_t sector = arguments->sector;
    if(sector < 0x01000000) {
        outb(base + 2, number_of_sectors);
        outb(base + 3, (sector & 0xFF));
        outb(base + 4, ((sector >> 8) & 0xFF));
        outb(base + 5, ((sector >> 16) & 0xFF));
        outb(base + 7, 0x20); // LBA28 PIO READ command
    }
    else {
        outb(base + 2, ((number_of_sectors >> 8) & 0xFF));
        outb(base + 3, ((sector >> 24) & 0xFF));
        outb(base + 4, ((sector >> 32) & 0xFF));
        outb(base + 5, ((sector >> 36) & 0xFF));
        outb(base + 2, number_of_sectors);
        outb(base + 3, (sector & 0xFF));
        outb(base + 4, ((sector >> 8) & 0xFF));
        outb(base + 5, ((sector >> 16) & 0xFF));
        outb(base + 7, 0x24); // LBA48 PIO READ command
    }
}

void parallel_ata_read_sector(uint32_t arguments_ptr) {
    parallel_ata_read_sector_arguments_t *arguments = (parallel_ata_read_sector_arguments_t *) arguments_ptr;
    uint32_t words_of_sector = arguments->words_of_sector;
    uint16_t base = arguments->base;
    for(int i = 0; i < words_of_sector; i++) {
        uint16_t value = inw(base + 0);
        *arguments->buffer++ = (value & 0xFF);
        *arguments->buffer++ = ((value >> 8) & 0xFF);
    }
}

uint32_t parallel_ata_write_wrapper(hardware_t *self, uint64_t lba, uint32_t number_of_sectors, void *buffer, storage_transfer_status_t *status) {
    parallel_ata_data_t *data = (parallel_ata_data_t *) self->data;
    uint32_t size_of_sector = data->size_of_sector;
    uint32_t max_number_of_sectors_per_one_request = data->max_number_of_sectors_per_one_request;

    while(number_of_sectors > 0) {
        uint32_t sectors_per_write = max_number_of_sectors_per_one_request;
        if(sectors_per_write > number_of_sectors) {
            sectors_per_write = number_of_sectors;
        }
        if(parallel_ata_write(self, lba, sectors_per_write, buffer, status) == ERROR) {
            return ERROR;
        }
        lba += sectors_per_write;
        number_of_sectors -= sectors_per_write;
        buffer += (sectors_per_write * size_of_sector);
    }

    return SUCCESS;
}

// this function assumes that lba is valid, and number of sectors is also valid with respect to lba and max number of sectors per request and buffer is as big as size of sectors
uint32_t parallel_ata_write(hardware_t *self, uint64_t lba, uint32_t number_of_sectors, void *buffer, storage_transfer_status_t *status) {
    parallel_ata_data_t *data = (parallel_ata_data_t *) self->data;
    ide_controller_device_data_t *data_from_controller = (ide_controller_device_data_t *) self->data_from_controller;
    hardware_t *controller = self->controller;
    ide_controller_data_t *controller_data = (ide_controller_data_t *) controller->data;
    uint32_t size_of_sector = data->size_of_sector;

    // send command
    parallel_ata_command_arguments_t arguments = {
        .base = controller_data->base,
        .number_of_sectors = (number_of_sectors == data->max_number_of_sectors_per_one_request) ? 0 : number_of_sectors,
        .sector = lba,
    };
    parallel_ata_write_sector_arguments_t write_sector_arguments = {
        .base = controller_data->base,
        .words_of_sector = (size_of_sector / 2),
        .buffer = buffer,
    };
    uint32_t transfer_status = ide_send_write_command(controller, data_from_controller->drive, ide_send_pio_write_command, (uint32_t) &arguments, parallel_ata_write_sector, (uint32_t) &write_sector_arguments, 1000000);
    number_of_sectors--;
    while(true) {
        if(transfer_status == IDE_TIMEOUT) {
            log("\n[PATA] Write timeout");
            ide_command_processed(controller);
            return ERROR;
        }
        else if(transfer_status == IDE_ERROR) {
            log("\n[PATA] Write error");
            ide_dump_status(controller);
            ide_command_processed(controller);
            return ERROR;
        }
        else if(number_of_sectors > 0) {
            log("\n[PATA] Sector is written");

            transfer_status = ide_renew_write_command(controller, parallel_ata_write_sector, (uint32_t) &write_sector_arguments, 100000);
            number_of_sectors--;
        }
        else { // number_of_sectors == 0
            log("\n[PATA] Last sector is written");
            ide_command_processed(controller);
            break;
        }
    }

    // flush cache
    if(data->does_support_lba48) {
        transfer_status = ide_send_read_command(controller, data_from_controller->drive, ide_send_lba48_flush_command, controller_data->base, 1000000);
    }
    else {
        transfer_status = ide_send_read_command(controller, data_from_controller->drive, ide_send_lba28_flush_command, controller_data->base, 1000000);
    }
    ide_command_processed(controller);
    return (transfer_status == IDE_TIMEOUT || transfer_status == IDE_ERROR) ? ERROR : SUCCESS;
}

void ide_send_pio_write_command(uint32_t arguments_ptr) {
    parallel_ata_command_arguments_t *arguments = (parallel_ata_command_arguments_t *) arguments_ptr;
    uint16_t base = arguments->base;
    uint16_t number_of_sectors = arguments->number_of_sectors;
    uint64_t sector = arguments->sector;
    if(sector < 0x01000000) {
        outb(base + 2, number_of_sectors);
        outb(base + 3, (sector & 0xFF));
        outb(base + 4, ((sector >> 8) & 0xFF));
        outb(base + 5, ((sector >> 16) & 0xFF));
        outb(base + 7, 0x30); // LBA28 PIO WRITE command
    }
    else {
        outb(base + 2, ((number_of_sectors >> 8) & 0xFF));
        outb(base + 3, ((sector >> 24) & 0xFF));
        outb(base + 4, ((sector >> 32) & 0xFF));
        outb(base + 5, ((sector >> 36) & 0xFF));
        outb(base + 2, number_of_sectors);
        outb(base + 3, (sector & 0xFF));
        outb(base + 4, ((sector >> 8) & 0xFF));
        outb(base + 5, ((sector >> 16) & 0xFF));
        outb(base + 7, 0x34); // LBA48 PIO WRITE command
    }
}

void parallel_ata_write_sector(uint32_t arguments_ptr) {
    parallel_ata_write_sector_arguments_t *arguments = (parallel_ata_write_sector_arguments_t *) arguments_ptr;
    uint32_t words_of_sector = arguments->words_of_sector;
    uint16_t base = arguments->base;
    for(int i = 0; i < words_of_sector; i++) {
        outw(base + 0, (arguments->buffer[1] << 8) | (arguments->buffer[0]));
        arguments->buffer += 2;
    }
}

void ide_send_lba28_flush_command(uint32_t base) {
    outb(base + 2, 0);
    outb(base + 3, 0);
    outb(base + 4, 0);
    outb(base + 5, 0);
    outb(base + 7, 0xE7);
}

void ide_send_lba48_flush_command(uint32_t base) {
    outb(base + 2, 0);
    outb(base + 3, 0);
    outb(base + 4, 0);
    outb(base + 5, 0);
    outb(base + 7, 0xEA);
}

uint64_t parallel_ata_convert_lba_to_offset(hardware_t *device, uint64_t lba) {
    parallel_ata_data_t *data = (parallel_ata_data_t *) device->data;
    if(lba >= data->number_of_sectors) {
        return -1;
    }
    uint64_t offset = lba * data->size_of_sector;
    return offset;
}