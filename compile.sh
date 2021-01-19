#!/bin/bash

mkdir images

nasm -f bin bootloader/bootloader.asm -o bootloader/bootloader.bin
nasm -f bin source/bleskos.asm -o images/bleskos.bin

dd if=/dev/zero of=images/bleskos.hdd bs=1024 count=1000
dd if=bootloader/bootloader.bin of=images/bleskos.hdd conv=notrunc seek=0
dd if=images/bleskos.bin of=images/bleskos.hdd conv=notrunc seek=1

qemu-system-i386 -m 64 -usbdevice mouse -soundhw hda -hda images/bleskos.hdd

sleep 60
