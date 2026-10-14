/* 
* BleskOS
*
* MIT License
* Copyright (c) 2023-2026 BleskOS developers
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include <kernel/hardware/main.h>

enum {
    STORAGE_TRANSFER_WAITING,
    STORAGE_TRANSFER_RUNNING,
    STORAGE_TRANSFER_ERROR,
    STORAGE_TRANSFER_FINISHED,
};
typedef struct storage_transfer_status_t {
    struct storage_transfer_status_t *up;
    uint32_t status;
    uint32_t slot;
    uint32_t number_of_loaded_bytes;
    uint32_t number_of_all_bytes;
} storage_transfer_status_t;

typedef struct {
    uint32_t (*read)(hardware_t *device, uint64_t lba, uint32_t number_of_sectors, void *buffer, storage_transfer_status_t *status);
    uint32_t (*write)(hardware_t *device, uint64_t lba, uint32_t number_of_sectors, void *buffer, storage_transfer_status_t *status);
    uint64_t (*convert_lba_to_offset)(hardware_t *device, uint64_t lba);
} storage_group_device_functions_t;
typedef struct {
    hardware_t *device;
    storage_group_device_functions_t *functions;
    uint32_t size_of_sector;
    uint64_t number_of_sectors;
    uint64_t first_nonacessible_byte;

    uint32_t max_number_of_parallel_transfers;
    uint8_t *is_transfer_slot_taken;
    void *slot_two_sectors_buffer[];
} storage_device_t;
typedef struct {
    uint32_t number_of_devices;
    storage_device_t **devices;
} storage_group_t;

typedef struct {
    uint64_t offset;
    uint32_t size;
    void *buffer;
} transfer_packet_entry_t;
typedef struct {
    storage_transfer_status_t status;
    void *buffer;
    uint32_t size;
    storage_transfer_status_t *actual_transfer_status;
    uint32_t number_of_entries;
    transfer_packet_entry_t entries[];
} transfer_packet_t;