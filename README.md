# BleskOS

BleskOS is 32-bit operating system written from scratch. One of main features is alternative graphic user interface that do not use windows, as classic operating systems do, but switches between programs on full screen. BleskOS is designed for older computers. Operating systems of today have very high hardware requirements that make computers even 10 years old completely unusable. Goal of BleskOS is to provide system, that could be installed on such computers and make them usable again.

![Snímka obrazovky_2024-02-15_17-48-49](https://github.com/VendelinSlezak/BleskOS/assets/43180618/35b483be-73c6-411f-be70-b2866301bb9e)

## How to try BleskOS

You can download image of last version here on github from last Release. Then you have two options:

1. Run BleskOS on emulator - some of well-known are Virtualbox, QEMU, Bochs and so on. If you downloaded image .img, you should to connect it to emulator as floppy or hard disk, if you downloaded image .iso, you should connect it as optical disk. You can also connect disk.img from testing folder where are some files for testing.
2. Run BleskOS on real computer - If you want to run BleskOS on your computer, you will need to create bootable medium. You can either burn .iso image on optical disk, or write .img image on USB flash. Do not just copy file! You need to write image right from start of USB flash. In linux, you can use program dd for this: `sudo dd if=bleskos_(actual version).img of=pathToYourUSB` For example if you have USB on /dev/sdg then you can write for example `sudo dd if=bleskos_2024u8.img of=/dev/sdg` Then insert optical disk/USB flash to your computer and boot it from your bootable medium. If you do not see your bootable medium, make sure that you are booting in legacy mode, because BleskOS do not support UEFI.

If you copied BleskOS image on USB flash, it creates one partition, so you can format rest of USB and use it for data. If you have such USB and you want to update BleskOS image without destroying other partitions, you need to skip first sector of BleskOS image and you need to skip first sector of USB flash and start rewriting from second sector. You can do this by following command: `sudo dd if=bleskos_(actual version).img skip=1 of=pathToYourUSB seek=1`

## How to use BleskOS

During boot, you will see bootloader screen where you can select to boot BleskOS normally, or with special boot options.

![VirtualBox_BleskOS_28_09_2023_16_38_07](https://github.com/VendelinSlezak/BleskOS/assets/43180618/e611cd37-5f77-4a6f-acbd-0992fa992230)

On real computers you should select first option. However, in emulators is best graphic mode often too big for monitor, so you may want to select boot with special boot options, what select graphic mode 1024x768 in emulators.

After booting screen, you will see Main window. When you see Main window, BleskOS is fully booted and ready to be used. You do not need to wait, as in classic operating systems, where even if you see desktop, you need to wait for all processes to be started. From Main window you can go to programs by clicking on their button, or by appropriate key.

![VirtualBox_BleskOS_12_10_2023_18_38_32](https://github.com/VendelinSlezak/BleskOS/assets/43180618/8fb9b3c6-6b7d-44ed-9324-eff1e2bb0448)

In all programs you can use these keyboard shortcuts:

- ESC = Back to Main window
- F1 = Open file
- F2 = Save file
- F3 = New file
- F4 = Close file
- F5/F6 = Select another file on left/right

When you want to switch from one program to another, you can either use mouse, or you can just press ESC and then press key of program.

In dialog windows you can press ENTER for selecting Yes, or ESC for selecting No. If you see window only with text, you can close it by ENTER, ESC or just clicking somewhere on screen.

## Compiling BleskOS

BleskOS can be compiled under linux. For compiling you have to have installed gcc, nasm and dd. If you have them, you can run `./compile.sh` script, that will generate bleskos.img file. If you want to compile without any optimizations, you can run it as `./compile fast`. If you want to only test some features from drivers or libraries, you may want to uncomment NO_PROGRAMS macro from source/bleskos.c, what will remove all code related to programs, so it will compile faster.

If you have installed QEMU, you can then use `./testing.sh` script. It will run QEMU with bleskos.img connected as floppy with testing/disk.img connected as hard disk. If you want to run BleskOS with own image with files, you have to copy it to testing folder and then write `./testing.sh yourimagename.img`.

## BleskOS development state

Drivers

- Graphic: VBE
- Sound: AC97, Intel HD Audio
- Inbuilt hardware: PS/2 keyboard, PS/2 mouse, PATA hard disk, PATAPI optical disk drive, AHCI hard disk, AHCI optical disk drive
- Filesystems: FAT12/16/32, ext2 (read-only), ISO9660 (read-only), CDDA (read-only)
- USB controllers: UHCI, OHCI, EHCI
- USB devices: USB mouse, USB keyboard, USB mass storage device, USB hub
- Ethernet cards: Realtek 8139/8169, Intel e1000(e), AMD PC-net
- Network stack: IPv4, ARP, UDP, TCP, DHCP, DNS, HTTP

[List of working hardware](https://github.com/VendelinSlezak/BleskOS/wiki/List-of-working-hardware) contain list of real hardware that was already tested and is working.

[Changelog](https://github.com/VendelinSlezak/BleskOS/releases) are in the [Releases](https://github.com/VendelinSlezak/BleskOS/releases) pages.

Programs

- Document viewer
- Text editor
- Graphic editor
- Media viewer
- Internet browser
- File Manager

Supported file formats

- TXT and all UTF-8 encoded text files
- JPG, PNG (read-only), GIF, QOI, BMP
- MP3, WAV, CDDA
- HTML
- ODT, DOCX (read-only)

## I like this project, can I help you?

You are welcome 🙂

- Give us a star and watch repository to be notified about updates
- If you will find some bugs, report them in [Issues](https://github.com/VendelinSlezak/BleskOS/issues)
- Try BleskOS on some real computer and let us know how it worked in [Discussions](https://github.com/VendelinSlezak/BleskOS/discussions) We would greatly appreciate if you would also add content of System log (Main window>[F1] System board>[L] System log>[F2] Backup the log).
- If you really like this project, you can support it on buymeacoffee.com by buying me some coffee ☕️

[!["Buy Me A Coffee"](https://www.buymeacoffee.com/assets/img/custom_images/orange_img.png)](https://buymeacoffee.com/cnn4fyv7xh0)

Thank you very much for any support!
