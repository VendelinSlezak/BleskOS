# BleskOS
Operation system working in protected mode with legacy boot. Main goal is develop operation system whose will be usable for normal user. Every version of BleskOS is tested on real computers.

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
* Serial hard disk (in progress)

Filesystems:
* JUS
* FAT32 (not available on GitHub now)

USB controllers:
* UHCI through legacy USB support
* EHCI

USB devices:
* USB keyboard
* USB mouse
* USB mass storage (in progress)

### Programs
Currently nothing, but text editor and graphic editor are on the way.

## How to build
BleskOS is completly written in NASM. You should use last version 2.1
