# main.c

This is layer above drivers that directly communicate with some storage medium.

## initalize_storage_controllers()

Method `initalize_storage_controllers()` is called during booting to initalize all IDE and AHCI controllers, and find out and prepare for use one hard disk and one optical disk drive. If there are multiple drives, in only first one is used. Informations about hard disk are saved to structure hard_disk_info and informations about optical drive are saved to structure optical_drive_info.

## Using storage devices

Before reading or writing to any storage device you have to call `select_storage_medium(medium_type, medium_number)`. You can use these values:

* Hard disk: `select_storage_medium(MEDIUM_HARD_DISK, DEFAULT_MEDIUM_NUMBER);`
* Optical drive: `select_storage_medium(MEDIUM_OPTICAL_DRIVE, DEFAULT_MEDIUM_NUMBER);`
* USB mass storage device: `select_storage_medium(MEDIUM_USB_MSD, number from 0 to 9 given by position in usb_mass_storage_devices[] array);`

After calling this method, you can use `read_storage_medium()` and `write_storage_medium()` to work with selected medium.

You can detect if particular drive is present by method `does_storage_medium_exist()` with values same as in `select_storage_medium()`. Method will return STATUS_TRUE or STATUS_FALSE.

## Using optical drive

Rest of methods are used only with optical drive. Actual code allow to work only with one optical drive, so these methods could be called even if optical drive is not selected by `select_storage_medium()`.

### detect_optical_disk()

This method returns STATUS_TRUE if optical drive has some disk inserted, or STATUS_FALSE if it do not. If there is disk inserted, variable `optical_disk_sector_size` contain size of one sector (should be always 2048), and variable `optical_disk_size` contain number of sectors in disk.

### read_optical_disk_toc()

This method reads Table Of Content of disk and saves it to structure optical_disk_table_of_content.

### read_audio_cd()

This method read sectors from Audio CD.

### eject_optical_disk()

This method ejects optical drive.

### spin_down_optical_drive()

This method stops spinning of disk in optical drive. This method should be always called after reading larger file, otherwise even if system do not read any data, drive can be quite loud by spinning disk after reading for some time.

# TODO

* More connected hard disks or optical drives at once