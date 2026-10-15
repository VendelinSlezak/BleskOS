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
#include <kernel/hardware/devices/memory/memory_allocators.h>
#include <kernel/hardware/devices/cpu/scheduler.h>
#include <kernel/hardware/main.h>
#include <kernel/libc/string.h>

/* local variables */
storage_group_t *storage_group;

/* functions */
void initialize_storage_group(void) {
    storage_group = kalloc(sizeof(storage_group_t));
}

void add_storage_device(hardware_t *device, storage_group_device_functions_t *functions, uint32_t size_of_sector, uint64_t number_of_sectors, uint32_t max_number_of_parallel_transfers) {
    storage_device_t *storage_device = kalloc(sizeof(storage_device_t) + (sizeof(void *) * max_number_of_parallel_transfers));
    storage_device->device = device;
    storage_device->functions = functions;
    storage_device->size_of_sector = size_of_sector;
    storage_device->number_of_sectors = number_of_sectors;
    storage_device->first_nonacessible_byte = (number_of_sectors * size_of_sector); // TODO: check overflow
    storage_device->max_number_of_parallel_transfers = max_number_of_parallel_transfers;
    storage_device->is_transfer_slot_taken = kalloc(sizeof(uint8_t) * max_number_of_parallel_transfers);
    for(int i = 0; i < max_number_of_parallel_transfers; i++) {
        storage_device->is_transfer_slot_taken[i] = false;
        storage_device->slot_two_sectors_buffer[i] = kalloc(size_of_sector * 2);
    }

    storage_group->devices = krealloc(storage_group->devices, sizeof(storage_device_t *) * (storage_group->number_of_devices + 1));
    storage_group->devices[storage_group->number_of_devices] = storage_device;
    storage_group->number_of_devices++;

    // TODO: send signal to filesystem subsystem
}

void remove_storage_device(hardware_t *device) {
    for(int i = 0; i < storage_group->number_of_devices; i++) {
        if(storage_group->devices[i]->device == device) {
            // TODO: send signal to filesystem subsystem
            // TODO: stop all transfers

            for(int j = 0; j < storage_group->devices[i]->max_number_of_parallel_transfers; j++) {
                kfree(storage_group->devices[i]->slot_two_sectors_buffer[j]);
            }
            kfree(storage_group->devices[i]->is_transfer_slot_taken);
            kfree(storage_group->devices[i]);

            if(i < (storage_group->number_of_devices - 1)) {
                storage_group->devices[i] = storage_group->devices[storage_group->number_of_devices - 1];
            }
            storage_group->number_of_devices--;
            storage_group->devices = krealloc(storage_group->devices, sizeof(storage_device_t *) * storage_group->number_of_devices);
            break;
        }
    }
}

uint32_t get_number_of_storage_devices(void) {
    return storage_group->number_of_devices;
}

storage_device_t *get_storage_device(uint32_t index) {
    if(index < storage_group->number_of_devices) {
        return storage_group->devices[index];
    }
    return NULL;
}

storage_transfer_status_t *storage_prepare_transfer(storage_device_t *storage_device) {
    for(int i = 0; i < storage_device->max_number_of_parallel_transfers; i++) {
        // try to change value from false to true
        uint8_t expected = false;
        if(__atomic_compare_exchange_n(&storage_device->is_transfer_slot_taken[i], 
                                       &expected, 
                                       true, 
                                       false,
                                       __ATOMIC_SEQ_CST,
                                       __ATOMIC_RELAXED)) {
            storage_transfer_status_t *status = kalloc(sizeof(storage_transfer_status_t));
            if(very_unlikely(status == NULL)) {
                storage_device->is_transfer_slot_taken[i] = false; // if allocation fails, release the slot
                return NULL;
            }
            status->status = STORAGE_TRANSFER_WAITING;
            status->slot = i;
            return status;
        }
    }

    return NULL;
}

void set_storage_transfer_status(storage_transfer_status_t *status, uint32_t new_status, int increase_in_bytes) {
    status->status = new_status;
    status->transferred_bytes += increase_in_bytes;
    if(status->up != NULL) {
        set_storage_transfer_status(status->up, new_status, increase_in_bytes);
    }
}

void storage_read(storage_device_t *storage_device, uint64_t offset, uint32_t size, void *buffer, storage_transfer_status_t *status) {
    hardware_t *device = storage_device->device;

    uint64_t first_nonacessible_byte = storage_device->first_nonacessible_byte;
    uint64_t end = (offset + size);
    if(offset >= first_nonacessible_byte || end > first_nonacessible_byte || end < offset) {
        status->status = STORAGE_TRANSFER_ERROR;
        return;
    }

    uint64_t first_sector = offset / storage_device->size_of_sector;
    uint32_t skip_bytes_in_first_sector = (uint32_t)(offset - (first_sector * storage_device->size_of_sector));
    uint64_t last_sector = ((offset + size - 1) / storage_device->size_of_sector);
    uint32_t read_bytes_in_last_sector = (uint32_t)((offset + size) - (last_sector * storage_device->size_of_sector));
    uint32_t number_of_sectors = (uint32_t)(last_sector - first_sector + 1);

    status->status = STORAGE_TRANSFER_RUNNING;
    void *two_sectors_buffer = storage_device->slot_two_sectors_buffer[status->slot];

    if(number_of_sectors == 1) {
        // request for exactly one sector
        if(skip_bytes_in_first_sector == 0 && size == storage_device->size_of_sector) {
            storage_device->functions->read(device, first_sector, 1, buffer, status);
        }
        // request for less than one sector
        else {
            storage_device->functions->read(device, first_sector, 1, two_sectors_buffer, status);
            memcpy(buffer, two_sectors_buffer + skip_bytes_in_first_sector, size);
        }
    }
    else if(number_of_sectors == 2) {
        // request for exactly two sectors
        if(skip_bytes_in_first_sector == 0 && read_bytes_in_last_sector == storage_device->size_of_sector) {
            storage_device->functions->read(device, first_sector, 2, buffer, status);
        }
        // request for less than two sectors but more than one
        else {
            storage_device->functions->read(device, first_sector, 2, two_sectors_buffer, status);
            memcpy(buffer, two_sectors_buffer + skip_bytes_in_first_sector, size);
        }
    }
    else {
        // first sector is not aligned to buffer
        if(skip_bytes_in_first_sector > 0) {
            storage_device->functions->read(device, first_sector, 1, two_sectors_buffer, status);
            memcpy(buffer, two_sectors_buffer + skip_bytes_in_first_sector, size);
        }

        // load all sectors that are aligned to buffer
        uint64_t first_aligned_sector = (first_sector + 1);
        void *aligned_buffer = (buffer - skip_bytes_in_first_sector + storage_device->size_of_sector);
        uint32_t number_of_aligned_sectors = (number_of_sectors - 2);
        if(skip_bytes_in_first_sector == 0) {
            first_aligned_sector--;
            number_of_aligned_sectors++;
            aligned_buffer -= storage_device->size_of_sector;
        }
        if(read_bytes_in_last_sector == storage_device->size_of_sector) {
            number_of_aligned_sectors++;
        }
        storage_device->functions->read(device, first_aligned_sector, number_of_aligned_sectors, aligned_buffer, status);

        // last sector is not aligned to buffer
        if(read_bytes_in_last_sector < storage_device->size_of_sector) {
            storage_device->functions->read(device, last_sector, 1, two_sectors_buffer, status);
            memcpy(buffer + size - read_bytes_in_last_sector, two_sectors_buffer, read_bytes_in_last_sector);
        }
    }

    // release the slot
    storage_device->is_transfer_slot_taken[status->slot] = false;
}

void storage_write(storage_device_t *storage_device, uint64_t offset, uint32_t size, void *buffer, storage_transfer_status_t *status) {
    hardware_t *device = storage_device->device;

    uint64_t first_nonacessible_byte = storage_device->first_nonacessible_byte;
    uint64_t end = (offset + size);
    if(offset >= first_nonacessible_byte || end > first_nonacessible_byte || end < offset) {
        status->status = STORAGE_TRANSFER_ERROR;
        return;
    }

    uint64_t first_sector = offset / storage_device->size_of_sector;
    uint32_t skip_bytes_in_first_sector = (uint32_t)(offset - (first_sector * storage_device->size_of_sector));
    uint64_t last_sector = ((offset + size - 1) / storage_device->size_of_sector);
    uint32_t write_bytes_in_last_sector = (uint32_t)((offset + size) - (last_sector * storage_device->size_of_sector));
    uint32_t number_of_sectors = (uint32_t)(last_sector - first_sector + 1);

    status->status = STORAGE_TRANSFER_RUNNING;
    void *two_sectors_buffer = storage_device->slot_two_sectors_buffer[status->slot];

    if(number_of_sectors == 1) {
        // request for exactly one sector
        if(skip_bytes_in_first_sector == 0 && size == storage_device->size_of_sector) {
            storage_device->functions->write(device, first_sector, 1, buffer, status);
        }
        // request for less than one sector
        else {
            storage_device->functions->read(device, first_sector, 1, two_sectors_buffer, status);
            memcpy(two_sectors_buffer + skip_bytes_in_first_sector, buffer, size);
            storage_device->functions->write(device, first_sector, 1, two_sectors_buffer, status);
        }
    }
    else if(number_of_sectors == 2) {
        // request for exactly two sectors
        if(skip_bytes_in_first_sector == 0 && write_bytes_in_last_sector == storage_device->size_of_sector) {
            storage_device->functions->write(device, first_sector, 2, buffer, status);
        }
        // request for less than two sectors but more than one
        else {
            storage_device->functions->read(device, first_sector, 2, two_sectors_buffer, status);
            memcpy(two_sectors_buffer + skip_bytes_in_first_sector, buffer, size);
            storage_device->functions->write(device, first_sector, 2, two_sectors_buffer, status);
        }
    }
    else {
        // first sector is not aligned to buffer
        if(skip_bytes_in_first_sector > 0) {
            storage_device->functions->read(device, first_sector, 1, two_sectors_buffer, status);
            memcpy(two_sectors_buffer + skip_bytes_in_first_sector, buffer, size);
            storage_device->functions->write(device, first_sector, 1, two_sectors_buffer, status);
        }

        // load all sectors that are aligned to buffer
        uint64_t first_aligned_sector = (first_sector + 1);
        void *aligned_buffer = (buffer - skip_bytes_in_first_sector + storage_device->size_of_sector);
        uint32_t number_of_aligned_sectors = (number_of_sectors - 2);
        if(skip_bytes_in_first_sector == 0) {
            first_aligned_sector--;
            number_of_aligned_sectors++;
            aligned_buffer -= storage_device->size_of_sector;
        }
        if(write_bytes_in_last_sector == storage_device->size_of_sector) {
            number_of_aligned_sectors++;
        }
        storage_device->functions->write(device, first_aligned_sector, number_of_aligned_sectors, aligned_buffer, status);

        // last sector is not aligned to buffer
        if(write_bytes_in_last_sector < storage_device->size_of_sector) {
            storage_device->functions->read(device, last_sector, 1, two_sectors_buffer, status);
            memcpy(buffer + size - write_bytes_in_last_sector, two_sectors_buffer, write_bytes_in_last_sector);
            storage_device->functions->write(device, last_sector, 1, two_sectors_buffer, status);
        }
    }

    // release the slot
    storage_device->is_transfer_slot_taken[status->slot] = false;
}

uint64_t storage_convert_lba_to_offset(storage_device_t *storage_device, uint64_t lba) {
    return storage_device->functions->convert_lba_to_offset(storage_device->device, lba);
}

// TODO: function for stopping transfers

transfer_packet_t *create_transfer_packet(void *buffer) {
    transfer_packet_t *transfer_packet = kalloc(sizeof(transfer_packet_t));
    transfer_packet->buffer = buffer;
    transfer_packet->status.status = STORAGE_TRANSFER_WAITING;
    return transfer_packet;
}

void add_transfer_to_packet(transfer_packet_t **transfer_packet_ptr, uint64_t offset, uint32_t size) {
    transfer_packet_t *transfer_packet = *transfer_packet_ptr;
    if(transfer_packet->number_of_entries == 0) {
        *transfer_packet_ptr = krealloc(transfer_packet, sizeof(transfer_packet_t) + (sizeof(transfer_packet_entry_t) * 1));
        transfer_packet = *transfer_packet_ptr;

        transfer_packet_entry_t *entry = &transfer_packet->entries[0];
        entry->offset = offset;
        entry->size = size;
        entry->buffer = transfer_packet->buffer;
        transfer_packet->number_of_entries++;
        transfer_packet->buffer += size;
        transfer_packet->size += size;
    }
    else {
        transfer_packet_entry_t *last_entry = &transfer_packet->entries[transfer_packet->number_of_entries - 1];
        if(last_entry->offset + last_entry->size == offset) {
            last_entry->size += size;
        }
        else {
            // TODO: go through all entries, and check if offset should be inserted between existing entries, so transfer will go only from lower to higher sectors

            *transfer_packet_ptr = krealloc(transfer_packet, sizeof(transfer_packet_t) + (sizeof(transfer_packet_entry_t) * (transfer_packet->number_of_entries + 1)));
            transfer_packet = *transfer_packet_ptr;

            transfer_packet_entry_t *entry = &transfer_packet->entries[transfer_packet->number_of_entries];
            entry->offset = offset;
            entry->size = size;
            entry->buffer = transfer_packet->buffer;
            transfer_packet->number_of_entries++;
        }
        transfer_packet->buffer += size;
        transfer_packet->size += size;
    }
}

void read_transfer_packet(storage_device_t *storage_device, transfer_packet_t *transfer_packet) {
    transfer_packet->status.number_of_all_bytes = transfer_packet->size;
    for(uint32_t i = 0; i < transfer_packet->number_of_entries; i++) {
        transfer_packet_entry_t *entry = &transfer_packet->entries[i];
        transfer_packet->actual_transfer_status = storage_prepare_transfer(storage_device);
        transfer_packet->actual_transfer_status->up = &transfer_packet->status;
        storage_read(storage_device, entry->offset, entry->size, entry->buffer, transfer_packet->actual_transfer_status);
        kfree(transfer_packet->actual_transfer_status);
        transfer_packet->actual_transfer_status = NULL;
        if(transfer_packet->status.status == STORAGE_TRANSFER_ERROR) {
            break;
        }
    }
}

void write_transfer_packet(storage_device_t *storage_device, transfer_packet_t *transfer_packet) {
    transfer_packet->status.number_of_all_bytes = transfer_packet->size;
    for(uint32_t i = 0; i < transfer_packet->number_of_entries; i++) {
        transfer_packet_entry_t *entry = &transfer_packet->entries[i];
        transfer_packet->actual_transfer_status = storage_prepare_transfer(storage_device);
        transfer_packet->actual_transfer_status->up = &transfer_packet->status;
        storage_write(storage_device, entry->offset, entry->size, entry->buffer, transfer_packet->actual_transfer_status);
        kfree(transfer_packet->actual_transfer_status);
        transfer_packet->actual_transfer_status = NULL;
        if(transfer_packet->status.status == STORAGE_TRANSFER_ERROR) {
            break;
        }
    }
}

void free_transfer_packet(transfer_packet_t *transfer_packet) {
    kfree(transfer_packet);
}