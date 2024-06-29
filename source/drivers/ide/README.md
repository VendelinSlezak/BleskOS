# Drivers for IDE interface

During boot in method `bleskos()`, method `scan_pci()` from drivers/system/pci.c finds all IDE controllers and saves their base adresses into structure `storage_controllers[]` that can be founded in drivers/storage/main.h. Entry base_1 contain base address of main ports (e.g. 0x1F0), and entry base_2 contain base address of additional ports (e.g. 0x3F4).

Boot process later calls method `initalize_storage_controllers()`. When this method founds IDE controller in `storage_controllers[]`, it will call method `initalize_ide_controller()`. This method will try to find out what devices are connected into Master and Slave of that controller. It is worth noting, that it is normal, that PCI reports bus that has nothing connected in it, or bus that do not even exist. owever if there is some hard disk or optical disk drive, this method will save informations about them into structures `hard_disk_info` and `optical_drive_info`.

If you want to use some finded device, you need to call method `ide_select_drive(base_1, PATA_MASTER/PATA_SLAVE)` to select device and then you can use appropriate methods from ide_parallel_ata to work with hard disk, or ide_parallel_atapi to work with optical drive. But you do not need to worry about it, normal interface for communicating with devices in drivers/storage/main.c will do this for you.

## ide_controller.c

### void initalize_ide_controller(byte_t number_of_controller)

Initalizes controller by informations from `storage_controllers[]` structure from drivers/storage/main.h

### void initalize_ide_controller_drive(byte_t number_of_controller, byte_t number_of_drive)

Initalizes Master or Slave drive. number_of_drive must be PATA_MASTER or PATA_SLAVE.

### byte_t ide_send_identify_drive(word_t base_port)

This sends Identify command (0xEC) into controller with ports on base_port. You have to select Master/Slave drive by `ide_select_drive()` before calling this method. Method will return STATUS_GOOD or STATUS_ERROR, in global variable `ide_drive_type` there will be signature from registers and in `ide_drive_size` there will be number of sectors of device.

```
ide_select_drive(0x1F0, PATA_MASTER);
ide_send_identify_drive(0x1F0);
```

If you connected Hard disk with 10000 sectors into primary controller Master, this method should return STATUS_GOOD, in ide_drive_type there will be PATA_HARD_DISK_SIGNATURE and in ide_drive_size there will be 10000.

### void ide_reset_controller(word_t base_port, word_t alt_base_port)

This resets whole IDE controller. It is usually used with optical drives, if they are stuck in some unknown condition. After reset, controller should automatically select Master drive.

### void ide_select_drive(word_t base_port, byte_t drive)

This method selects Master or Slave drive on controller. drive must be PATA_MASTER or PATA_SLAVE.

## ide_parallel_ata.c

### byte_t pata_read(word_t base_port, dword_t sector, byte_t number_of_sectors, byte_t *memory)

This method reads sectors from hard disk. You have to select Master/Slave drive by `ide_select_drive()` before calling this method. Method will return STATUS_GOOD or STATUS_ERROR.

```
byte_t *data = (byte_t *) malloc(512*5);
ide_select_drive(hard_disk_info.base_1, hard_disk_info.device_port);
pata_read(hard_disk_info.base_1, 10, 5, data);
```

### byte_t pata_write(word_t base_port, dword_t sector, byte_t number_of_sectors, byte_t *memory)

This method writer sectors to hard disk. You have to select Master/Slave drive by `ide_select_drive()` before calling this method. Method will return STATUS_GOOD or STATUS_ERROR.

```
ide_select_drive(hard_disk_info.base_1, hard_disk_info.device_port);
pata_write(hard_disk_info.base_1, 0, 1, mbr_sector);
```

## ide_parallel_atapi.c

### byte_t patapi_test_unit_ready(word_t base_port, word_t alt_base_port)

This method sends TEST_UNIT_READY command to optical disk drive. You have to select Master/Slave drive by `ide_select_drive()` before calling this method. Method will return STATUS_GOOD if it is ready (it means that optical disk is inside) and STATUS_ERROR otherwise.

### byte_t patapi_detect_disk(word_t base_port, word_t alt_base_port)

First calling of TEST_UNIT_READY after disk is loaded usually fails, so we need to send this command twice, what is what this method does. It returns STATUS_GOOD if there is optical disk and STATUS_ERROR if there is not.

```
ide_select_drive(optical_drive_info.base_1, optical_drive_info.device_port);
if(patapi_detect_disk(hard_disk_info.base_1, optical_drive_info.base_2)==STATUS_GOOD) {
 //there is optical disk inserted
}
```

### byte_t patapi_read(word_t base_port, word_t alt_base_port, dword_t sector, byte_t number_of_sectors, byte_t *memory)

This method reads sectors from optical disk. You have to select Master/Slave drive by `ide_select_drive()` before calling this method. Method will return STATUS_GOOD or STATUS_ERROR.

### byte_t patapi_read_audio_cd(word_t base_port, word_t alt_base_port, dword_t sector, byte_t number_of_sectors, byte_t *memory)

This method reads sectors from Audio optical disk, what means that every sector has size 2352 bytes. You have to select Master/Slave drive by `ide_select_drive()` before calling this method. Method will return STATUS_GOOD or STATUS_ERROR.

## TODO

* ide_parallel_ata: If there is support for lba48, add support for reading more than 255 sectors per one request
* patapi_read_cd_toc(): How to set ATAPI packet for reading more than 31 TOC entries?
* Methods for writing on optical disk
* Support for other devices like SD card readers if they are connected through IDE