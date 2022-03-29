# BleskOS
Operation system working in protected mode with legacy boot. Main goal is develop operation system whose will be usable for normal user. Every version of BleskOS is tested on real computers.

![VirtualBox_BleskOS_01_03_2022_18_35_56](https://user-images.githubusercontent.com/43180618/158466244-77a4047c-7ae9-487b-895a-b49a4ebbcb99.png)


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

Ethernet cards:
* AMD PC-net II/III
* Intel e1000

Internet stack:
* DHCP
* ARP
* DNS
* IPv4
* TCP

### Drivers in developing
- [x] Ethernet card Realtek 8139
- [x] Ethernet card Realtek 8169
- [x] Intel HD Audio
- [ ] USB controller xHCI
- [ ] AHCI cdrom
- [ ] Filesystems UDF and CDDA
- [ ] USB printer

### Programs
* Text editor (TXT files)
* Graphic editor (BMP files)
* Media viewer (BMP and WAV files)
* Internet browser (only HTTP pages)
* Document editor (very simple, TD files only for BleskOS)

## How to build
BleskOS is completly written in assembly language and can be compiled using NASM. You should use the latest version (2.1)

compile.sh is written for linux systems. After download, you have to check in properties "Allow executing file as program" or you can enter the following in your terminal: ```chmod +x compile.sh``` and you can compile BleskOS by running this file.

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
