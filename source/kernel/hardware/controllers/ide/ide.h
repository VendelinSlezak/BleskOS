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
#include <kernel/hardware/devices/cpu/interrupt.h>
#include <kernel/hardware/devices/cpu/mutex.h>
#include <kernel/hardware/devices/cpu/scheduler.h>

#define PATA_MASTER 0xE0
#define PATA_SLAVE 0xF0

#define PATA_HARD_DISK_SIGNATURE 0x0000
#define PATA_OPTICAL_DRIVE_SIGNATURE 0xEB14

typedef struct {
    uint16_t general_configuration;             // Word 0
    uint16_t logical_cylinders;                 // Word 1 (obsolete/legacy)
    uint16_t specific_configuration;            // Word 2
    uint16_t logical_heads;                     // Word 3 (obsolete/legacy)
    uint16_t retired_1[2];                      // Words 4-5
    uint16_t logical_sectors_per_track;         // Word 6 (obsolete/legacy)
    uint16_t reserved_for_assignment_1[3];      // Words 7-9
    char     serial_number[20];                 // Words 10-19
    uint16_t retired_2[2];                      // Words 20-21
    uint16_t obsolete_1;                        // Word 22
    char     firmware_revision[8];              // Words 23-26
    char     model_number[40];                  // Words 27-46
    uint8_t  maximum_block_transfer;            // Word 47 (lower byte)
    uint8_t  vendor_specific_1;                 // Word 47 (upper byte)
    uint16_t trusted_computing_options;         // Word 48
    uint16_t capabilities_1;                    // Word 49 (DMA, LBA support)
    uint16_t capabilities_2;                    // Word 50
    uint16_t obsolete_2[2];                     // Words 51-52
    uint16_t field_validity;                    // Word 53
    uint16_t current_numbers[5];                // Words 54-58 (obsolete CHS translation)
    uint16_t multi_sector_setting;              // Word 59
    uint32_t lba28_total_number_of_sectors;     // Words 60-61 (28-bit LBA)
    uint16_t singleword_dma_transfer;           // Word 62 (obsolete)
    uint16_t multiword_dma_transfer;            // Word 63
    uint16_t flow_control_pio_transfer;         // Word 64
    uint16_t minimum_mwdma_cycle_time;          // Word 65
    uint16_t recommended_mwdma_cycle_time;      // Word 66
    uint16_t minimum_pio_cycle_time;            // Word 67
    uint16_t minimum_pio_cycle_time_iordy;      // Word 68
    uint16_t reserved_for_assignment_2[11];     // Words 69-79
    uint16_t major_version_number;              // Word 80 (ATA standard versions)
    uint16_t minor_version_number;              // Word 81
    uint16_t command_sets_supported_1;          // Word 82
    uint16_t command_sets_supported_2;          // Word 83
    uint16_t command_set_feature_extension;     // Word 84
    uint16_t command_sets_enabled_1;            // Word 85
    uint16_t command_sets_enabled_2;            // Word 86
    uint16_t command_set_feature_default;       // Word 87
    uint16_t udma_modes;                        // Word 88 (Ultra DMA modes)
    uint16_t security_erase_time;               // Word 89
    uint16_t enhanced_security_erase_time;      // Word 90
    uint16_t current_advanced_power_management; // Word 91
    uint16_t master_password_identifier;        // Word 92
    uint16_t hardware_reset_results;            // Word 93
    uint16_t current_acoustic_management;       // Word 94
    uint16_t stream_min_request_size;           // Word 95
    uint16_t streaming_transfer_time_dma;       // Word 96
    uint16_t streaming_access_latency;          // Word 97
    uint32_t streaming_perf_granularity;        // Words 98-99
    uint64_t lba48_total_number_of_sectors;     // Words 100-103 (64-bit integer hold 48-bit value)
    uint16_t streaming_transfer_time_pio;       // Word 104
    uint16_t max_blocks_per_data_set_management;// Word 105
    uint16_t physical_logical_sector_size;      // Word 106 (Physical/Logical sector architecture)
    uint16_t inter_seek_delay;                  // Word 107
    uint16_t world_wide_name[4];                // Words 108-111 (WWN identifier)
    uint16_t reserved_for_world_wide_name_ext[4];// Words 112-115
    uint16_t reserved_for_incits_tru10;         // Word 116
    uint32_t words_per_logical_sector;       // Words 117-118 (if sector > 512B)
    uint16_t command_sets_supported_3;          // Word 119
    uint16_t command_sets_enabled_3;            // Word 120
    uint16_t reserved_for_assignment_3[6];      // Words 121-126
    uint16_t obsolete_3;                        // Word 127
    uint16_t security_status;                   // Word 128
    uint16_t vendor_specific_2[31];             // Words 129-159
    uint16_t cfa_power_mode_1;                  // Word 160
    uint16_t reserved_for_compactflash[7];      // Words 161-167
    uint16_t device_nominal_form_factor;        // Word 168 (2.5", 3.5", M.2 ...)
    uint16_t data_set_management_support;       // Word 169 (TRIM support)
    uint16_t additional_product_identifier[4];  // Words 170-173
    uint16_t reserved_for_assignment_4[2];      // Words 174-175
    char     media_serial_number[60];           // Words 176-205
    uint16_t sct_command_transport;             // Word 206
    uint16_t reserved_for_assignment_5[2];      // Words 207-208
    uint16_t alignment_of_logical_blocks;       // Word 209
    uint32_t write_read_verify_sector_count;    // Words 210-211
    uint32_t verify_sector_count_mode_2;        // Words 212-213
    uint16_t nv_cache_capabilities;             // Word 214
    uint16_t nv_cache_size_in_logical_blocks[2];// Words 215-216
    uint16_t nominal_media_rotation_rate;       // Word 217 (RPM, 0x0001 = SSD)
    uint16_t reserved_for_assignment_6;         // Word 218
    uint16_t nv_cache_options;                  // Word 219
    uint16_t write_read_verify_mode;            // Word 220
    uint16_t reserved_for_assignment_7;         // Word 221
    uint16_t transport_major_version_number;    // Word 222
    uint16_t transport_minor_version_number;    // Word 223
    uint16_t reserved_for_assignment_8[6];      // Words 224-229
    uint64_t extended_number_of_user_addressable_sectors; // Words 230-233
    uint16_t min_blocks_per_download_microcode; // Word 234
    uint16_t max_blocks_per_download_microcode; // Word 235
    uint16_t reserved_for_assignment_9[19];     // Words 236-254
    uint8_t  signature;                         // Word 255 (lower byte: 0xA5)
    uint8_t  checksum;                          // Word 255 (upper byte: Checksum)
} __attribute__((packed)) ata_identify_command_device_info_t;

enum {
    IDE_DMA_READ,
    IDE_DMA_WRITE,
};
typedef struct {
    uint32_t physical_address_of_buffer;
    uint16_t byte_count;
    uint16_t : 15;
    uint16_t last_prd : 1;
}__attribute__((packed)) ide_prd_t;

enum {
    IDE_DATA_READY = 0,
    IDE_ERROR,
    IDE_TIMEOUT,
    IDE_COMMAND_COMPLETED,
};
typedef struct {
    uint16_t base;
    uint16_t control;
    uint16_t dma;

    uint32_t interrupt_processor_index;

    hardware_t *master_device;
    hardware_t *slave_device;

    mutex_t mutex;
    uint32_t interrupt_expected;
    kernel_thread_t *waiting_thread_for_irq;
    void *timeout_thread;
    uint32_t transfer_status;

    uint32_t number_of_iht_function_calls;
    uint32_t number_of_expected_iht_function_calls;
    uint32_t max_number_of_expected_iht_function_calls;
    void (*iht_function)(uint32_t arguments_ptr);
    uint32_t iht_function_arguments;

    ide_prd_t *prdt;
} ide_controller_data_t;

typedef struct {
    uint32_t drive;
    uint16_t dma;
    ata_identify_command_device_info_t device_info;
} ide_controller_device_data_t;