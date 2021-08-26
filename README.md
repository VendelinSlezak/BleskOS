# BleskOS
Operation system working in protected mode with legacy boot. Main goal is develop operation system whose will be usable for normal user. Every version of BleskOS is tested on real computers. You do everything with BleskOS on your own risk!

### Drivers
Graphic:
* VESA

Sound:
* AC97
* Intel HD Audio (in progress)

Inbuilt hardware:
* PS/2 keyboard
* PS/2 mouse
* Paralel hard disk
* Paralel CDROM

Filesystems:
* JUS
* FAT32
* ISO9660 (in progress)

USB controllers:
* OHCI (in progress)
* UHCI
* EHCI

USB devices:
* USB keyboard
* USB mouse
* USB mass storage

### Programs
* File manager (in progress, usable version will be released 28/8/2021)
* Text editor (in progress, first version will be released 28/8/2021)

## How to build
BleskOS is completly written in NASM. You should use last version 2.1

compile.sh is written for linux. After download, you have to check in properties "Allow run this file as program" and you can compile BleskOS by running this file.

If you do not use linux, you can build BleskOS by this steps:
1. compile bootloader/bootloader.asm to bootloader.bin
2. compile source/bleskos.asm to bleskos.bin
3. create file bleskos.img
4. write bootloader.bin to first sector of bleskos.img
5. write bleskos.bin from second sector of bleskos.img
6. connect file bleskos.img as hard disk to emulator
7. and start emulator
