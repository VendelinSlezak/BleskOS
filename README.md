# BleskOS

BleskOS is 32-bit operating system written from scratch. One of main features is alternative graphic user interface that do not use windows, as classic operating systems do, but switches between programs on full screen. BleskOS is designed for older computers. Operating systems of today have very high hardware requirements that make computers even 10 years old completely unusable. Goal of BleskOS is to provide system, that could be installed on such computers and make them usable again.

![VirtualBox_BleskOS_28_09_2023_16_46_49](https://github.com/VendelinSlezak/BleskOS/assets/43180618/7a25508d-fbed-4af2-addc-0f79a15a9f7d)

## What is new

Here is info about most important improvements in several last updates.

**Update 6**
- Find/Find and replace/Go to line functions added to Text editor - from user perspective is now Text editor comparable to classic text editors such as Notepad

**Update 5**
- Functions to edit image that were in Media viewer are now also in Graphic editor
- Fixed bug about reading USB descriptor from low-speed devices on UHCI
- Improvement of HDA driver
- Better logging

**Update 4**
- Added function Undo and Redo for Text editor and text areas
- Text editor support light/dark mode

**Update 3**
- Several improvements on UHCI, OHCI and AHCI drivers
- Fixed showing of FAT entry with only short name

**Update 2**
- ext2 read-only driver
- Support for Extended Boot Record and Global Partition Table
- Graphic Editor shows dimensions of image
- System board and System log can be partially controlled by mouse

## How to try BleskOS

You can download image of last version here on github. Then you have two options:

1. Run BleskOS on emulator - some of well-known are Virtualbox, QEMU, Bochs and so on. If you downloaded image .img, you should to connect it to emulator as floppy or hard disk, if you downloaded image .iso, you should connect it as optical disk.
2. Run BleskOS on real computer - If you want to run BleskOS on your computer, you will need to create bootable medium. You can either burn .iso image on optical disk, or write .img image on USB flash. Do not just copy file! You need to write image right from start of USB flash. In linux, you can use program dd for this: `sudo dd if=bleskos_(actual version)_.img of=pathToYourUSB` For example if you have USB on /dev/sdg then you can write for example `sudo dd if=bleskos_2023_update_1.img of=/dev/sdg` Then insert optical disk/USB flash to your computer and boot it from your bootable medium. If you do not see your bootable medium, make sure that you are booting in legacy mode, because BleskOS do not support UEFI.

## How to use BleskOS

During boot, you will see bootloader screen where you can select to boot BleskOS normally, or with special boot options.

![VirtualBox_BleskOS_28_09_2023_16_38_07](https://github.com/VendelinSlezak/BleskOS/assets/43180618/e611cd37-5f77-4a6f-acbd-0992fa992230)

On real computers you should select first option. However, in emulators is best graphic mode often too big for monitor, so you may want to select boot with special boot options, what select graphic mode 1024x768 in emulators.

After booting screen, you will see Main window. When you see Main window, BleskOS is fully booted and ready to be used. You do not need to wait, as in classic operating systems, where even if you see desktop, you need to wait for all processes to be started. From Main window you can go to programs by clicking on their button, or by appropriate key.

![VirtualBox_BleskOS_03_10_2023_19_07_11](https://github.com/VendelinSlezak/BleskOS/assets/43180618/4cc598df-61e3-42fd-beb2-a8f62d96206b)

In all programs you can use these keyboard shortcuts:

- ESC = Back to Main window
- F1 = Open file
- F2 = Save file
- F3 = New file
- F4 = Close file
- F5/F6 = Select another file on left/right

When you want to switch from one program to another, you can either use mouse, or you can just press ESC and then press key of program.

In dialog windows you can press ENTER for selecting Yes, or ESC for selecting No. If you see window only with text, you can close it by ENTER, ESC or just clicking somewhere on screen.

## BleskOS development state

Drivers

- Graphic: VBE
- Sound: AC97, Intel HD Audio(works in QEMU)
- Inbuilt hardware: PS/2 keyboard, PS/2 mouse, PATA hard disk, PATAPI optical disk drive, AHCI hard disk, AHCI optical disk drive
- Filesystems: FAT12/16/32, ext2 (read-only), ISO9660 (read-only)
- USB controllers: UHCI, OHCI, EHCI
- USB devices: USB mouse, USB keyboard, USB mass storage device
- Ethernet cards: Realtek 8139/8169, Intel e1000(e), AMD PC-net
- Network stack: IPv4, ARP, UDP, TCP, DHCP, DNS, HTTP

Programs

- Text editor
- Graphic editor
- Media viewer
- Internet browser

Supported file formats

- TXT and all UTF-8 encoded files text files
- PNG, GIF, BMP
- WAV
- HTML

## I like this project, can I help you?

You are welcome 🙂

- Give us a star
- Watch repository to be notified about updates
- If you will find some bugs, report them in [Issues](https://github.com/VendelinSlezak/BleskOS/issues)
- Try BleskOS on some real computer and let us know how it worked in [Discussions](https://github.com/VendelinSlezak/BleskOS/discussions)
