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

Filesystems:
* JUS
* FAT32 (not available on GitHub now)

USB controllers:
* UHCI
* EHCI

USB devices:
* USB mouse
* USB mass storage (in progress)

### Programs
Currently nothing, but text editor and graphic editor are on the way.

## How to build
BleskOS is completly written in NASM. You should use last version 2.1

## Booting
If BleskOS do not start, bootloader report what happens.
| Error | Description | Solution |
|-------|-------------|----------|
|E1| Error while reading from hard disk | Write BleskOS to USB drive one more time |
|E2| Usable graphic mode was not founded | Use another computer |
