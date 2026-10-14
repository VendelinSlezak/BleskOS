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
#include <kernel/hardware/devices/cpu/scheduler.h>
#include <kernel/hardware/groups/storage/storage.h>

typedef struct {
    uint64_t number_of_sectors;
    uint32_t size_of_sector;
    uint32_t max_number_of_sectors_per_one_request;
    uint32_t does_support_lba48;
} parallel_ata_data_t;

typedef struct {
    uint16_t base;
    uint16_t number_of_sectors;
    uint64_t sector;
} parallel_ata_command_arguments_t;

typedef struct {
    uint16_t base;
    uint16_t words_of_sector;
    uint8_t *buffer;
} parallel_ata_read_sector_arguments_t;

typedef struct {
    uint16_t base;
    uint16_t words_of_sector;
    uint8_t *buffer;
} parallel_ata_write_sector_arguments_t;