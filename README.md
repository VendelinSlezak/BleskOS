# BleskOS
Operation system working in protected mode with legacy boot. Main goal is develop operation system whose will be usable for normal user. Every version of BleskOS is tested on real computers. You do everything with BleskOS on your own risk!

### Drivers
Graphic:
* VESA

Sound:
* AC97

Inbuilt hardware:
* PS/2 keyboard
* PS/2 mouse
* IDE hard disk
* IDE cdrom
* AHCI hard disk

Filesystems:
* JUS
* FAT32
* ISO9660

USB controllers:
* OHCI
* UHCI
* EHCI

USB devices:
* USB keyboard
* USB mouse
* USB mass storage

### Drivers in developing
- [x] Intel HD Audio
- [x] USB controller xHCI
- [x] AHCI cdrom
- [ ] Filesystems UDF and CDDA
- [ ] USB printer
- [x] Enternet card Intel e1000
- [ ] Ethernet card Realtek 8139
- [ ] TCP/IP stack

### Programs
* File manager (in progress, not available on github now)
* Text editor (in progress, not available on github now)

## How to build
BleskOS is completly written in NASM. You should use last version 2.1

compile.sh is written for linux. After download, you have to check in properties "Allow run this file as program" and you can compile BleskOS by running this file.

If you do not use linux, you can build BleskOS by this steps:
1. compile bootloader/bootloader.asm to bootloader.bin
2. compile source/bleskos.asm to bleskos.bin
3. create file bleskos.img
4. write bootloader.bin from first sector(LBA 0) to bleskos.img
5. write bleskos.bin from sixth sector(LBA 5) to bleskos.img
6. connect file bleskos.img to emulator as hard disk
7. and start emulator

## License
We do not have any responsibility for anything what will happen by using BleskOS or some part of BleskOS code.
You can learn from BleskOS source code, but if you want to copy some part of it to your project, you have to let us know by email klaykap /@/ yandex /./ com .
Anything you do with BleskOS, you do it on your own risk.
