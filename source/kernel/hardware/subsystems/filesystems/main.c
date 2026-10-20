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
#include <kernel/hardware/devices/memory/memory_allocators.h>
#include <kernel/hardware/devices/cpu/mutex.h>
#include <kernel/hardware/devices/cpu/scheduler.h>

/* local variables */
mutex_t connected_devices_mutex;
uint32_t number_of_connected_devices;
filesystem_device_info_t **connected_devices;
mutex_t filesystems_mutex;
uint32_t number_of_filesystems;
partition_info_t **filesystems;

uint8_t *partition_type_str[256] = {
    [0x00] = "Empty",
    [0x01] = "FAT12",
    [0x02] = "XENIX root",
    [0x03] = "XENIX usr",
    [0x04] = "FAT16 <32M",
    [0x05] = "Extended",
    [0x06] = "FAT16",
    [0x07] = "HPFS/NTFS/exFAT",
    [0x08] = "AIX",
    [0x09] = "AIX bootable",
    [0x0A] = "OS/2 Boot Manager",
    [0x0B] = "W95 FAT32",
    [0x0C] = "W95 FAT32 (LBA)",
    [0x0E] = "W95 FAT16 (LBA)",
    [0x0F] = "W95 Ext'd (LBA)",
    [0x10] = "OPUS",
    [0x11] = "Hidden FAT12",
    [0x12] = "Compaq diagnostics",
    [0x14] = "Hidden FAT16 <32M",
    [0x16] = "Hidden FAT16",
    [0x17] = "Hidden HPFS/NTFS",
    [0x18] = "AST SmartSleep",
    [0x1B] = "Hidden W95 FAT32",
    [0x1C] = "Hidden W95 FAT32 (LBA)",
    [0x1E] = "Hidden W95 FAT16 (LBA)",
    [0x27] = "Hidden NTFS WinRE",
    [0x39] = "Plan 9",
    [0x3C] = "PartitionMagic recovery",
    [0x40] = "Venix 80286",
    [0x41] = "PPC PReP Boot",
    [0x42] = "SFS",
    [0x4D] = "QNX4.x",
    [0x4E] = "QNX4.x 2nd part",
    [0x4F] = "QNX4.x 3rd part",
    [0x50] = "OnTrack DM",
    [0x51] = "OnTrack DM6 Aux",
    [0x52] = "CP/M",
    [0x53] = "OnTrack DM6 Aux",
    [0x54] = "OnTrackDM6",
    [0x55] = "EZ-Drive",
    [0x56] = "Golden Bow",
    [0x5C] = "Priam Edisk",
    [0x61] = "SpeedStor",
    [0x63] = "GNU HURD or SysV",
    [0x64] = "Novell Netware 286",
    [0x65] = "Novell Netware 386",
    [0x70] = "DiskSecure Multi-Boot",
    [0x75] = "PC/IX",
    [0x80] = "Old Minix",
    [0x81] = "Minix / old Linux",
    [0x82] = "Linux swap / Solaris",
    [0x83] = "Linux",
    [0x84] = "OS/2 hidden or hidden C: drive",
    [0x85] = "Linux extended",
    [0x86] = "NTFS volume set",
    [0x87] = "NTFS volume set",
    [0x88] = "Linux plaintext",
    [0x8E] = "Linux LVM",
    [0x93] = "Amoeba",
    [0x94] = "Amoeba BBT",
    [0x9F] = "BSD/OS",
    [0xA0] = "IBM Thinkpad hibernation",
    [0xA5] = "FreeBSD",
    [0xA6] = "OpenBSD",
    [0xA7] = "NeXTSTEP",
    [0xA8] = "Darwin UFS",
    [0xA9] = "NetBSD",
    [0xAB] = "Darwin boot",
    [0xAF] = "HFS / HFS+",
    [0xB7] = "BSDI fs",
    [0xB8] = "BSDI swap",
    [0xBB] = "Boot Wizard hidden",
    [0xBC] = "Acronis FAT32 LBA",
    [0xBE] = "Solaris boot",
    [0xBF] = "Solaris",
    [0xC1] = "DRDOS/sec (FAT-12)",
    [0xC4] = "DRDOS/sec (FAT-16 < 32M)",
    [0xC6] = "DRDOS/sec (FAT-16)",
    [0xC7] = "Syrinx",
    [0xDA] = "Non-FS data",
    [0xDB] = "CP/M / CTOS / ...",
    [0xDE] = "Dell Utility",
    [0xDF] = "BootIt",
    [0xE1] = "DOS access",
    [0xE3] = "DOS R/O",
    [0xE4] = "SpeedStor",
    [0xEA] = "Linux extended",
    [0xEB] = "BeOS fs",
    [0xEE] = "GPT",
    [0xEF] = "EFI (FAT-12/16/32)",
    [0xF0] = "Linux/PA-RISC boot loader",
    [0xF1] = "SpeedStor",
    [0xF2] = "DOS secondary",
    [0xF4] = "SpeedStor",
    [0xF8] = "EBBR protective partition",
    [0xFB] = "VMware VMFS",
    [0xFC] = "VMware VMKCORE",
    [0xFD] = "Linux raid auto",
    [0xFE] = "LANstep",
    [0xFF] = "BBT"
};

/* functions */
void add_device_to_filesystems_subsystem(storage_device_t *device) {
    filesystem_device_info_t *filesystem_device_info = kalloc(sizeof(filesystem_device_info_t));
    filesystem_device_info->device = device;
    filesystem_device_info->type = device->type;
    filesystem_device_info->number_of_partitions = 0;
    create_kernel_thread("Find Filesystems on Device", (uint32_t) find_filesystems_on_device, (uint32_t []) { (uint32_t) filesystem_device_info }, 1);
}

void find_filesystems_on_device(filesystem_device_info_t *device) {
    storage_device_t *storage_device = device->device;

    // load MBR
    mbr_sector_t mbr;
    if(storage_simple_read(storage_device, 0, sizeof(mbr_sector_t), &mbr) == ERROR) {
        log("\n[FILESYSTEM] Failed to read MBR");
        kfree(device);
        return;
    }

    // check if MBR is valid
    if(mbr.boot_signature != 0xAA55) {
        log("\n[FILESYSTEM] MBR is not valid");
        kfree(device);
        return;
    }

    // find valid partitions
    log("\n[FILESYSTEM] PARTITIONS:");
    for(int i = 0; i < 4; i++) {
        if(mbr.partition[i].type == 0x00) {
            continue;
        }

        uint64_t first_sector = mbr.partition[i].lba_address_of_first_sector;
        uint64_t number_of_sectors = mbr.partition[i].number_of_sectors;
        uint64_t size_in_bytes = number_of_sectors * storage_device->size_of_sector;
        log("\n             Primary partition %d %s from %d sectors %d, size in bytes 0x%x%x",
            (i + 1),
            partition_type_str[mbr.partition[i].type],
            (uint32_t)first_sector,
            (uint32_t)number_of_sectors,
            (uint32_t)(size_in_bytes >> 32),
            (uint32_t)size_in_bytes
        );
        check_partition(storage_device, first_sector, number_of_sectors);

        if(mbr.partition[i].type == 0x05) {
            uint32_t extended_partition_first_sector = mbr.partition[i].lba_address_of_first_sector;
            uint32_t extended_partition_start_sector = mbr.partition[i].lba_address_of_first_sector;
            uint32_t logical_partition_number = 1;
            while(true) {
                ebr_sector_t ebr;
                if(storage_simple_read(storage_device, storage_convert_lba_to_offset(storage_device, extended_partition_start_sector), sizeof(ebr_sector_t), &ebr) == ERROR) {
                    log("\n[FILESYSTEM] Failed to read EBR at sector %d", (uint32_t)extended_partition_start_sector);
                    break;
                }

                if(ebr.partition[0].type == 0x00) {
                    break;
                }

                first_sector = (extended_partition_start_sector + ebr.partition[0].lba_address_of_first_sector);
                number_of_sectors = ebr.partition[0].number_of_sectors;
                size_in_bytes = number_of_sectors * storage_device->size_of_sector;
                log("\n                 Logical partition %d %s from %d sectors %d, size in bytes 0x%x%x",
                    logical_partition_number,
                    partition_type_str[ebr.partition[0].type],
                    (uint32_t)first_sector,
                    (uint32_t)number_of_sectors,
                    (uint32_t)(size_in_bytes >> 32),
                    (uint32_t)size_in_bytes
                );
                check_partition(storage_device, first_sector, number_of_sectors);

                logical_partition_number++;
                if(ebr.partition[1].type == 0x00) {
                    break;
                }
                else {
                    extended_partition_start_sector = extended_partition_first_sector + ebr.partition[1].lba_address_of_first_sector;
                }
            }
        }
    }
}

void check_partition(storage_device_t *storage_device, uint64_t first_sector, uint64_t number_of_sectors) {
    if(first_sector >= storage_device->number_of_sectors || first_sector + number_of_sectors - 1 >= storage_device->number_of_sectors) {
        log("\n[FILESYSTEM] Partition is not valid");
        return;
    }
}