#!/bin/bash

mkdir images

nasm -f bin bootloader/bootloader.asm -o bootloader/bootloader.bin
nasm -f bin source/bleskos.asm -o images/bleskos.bin -Wfoo

dd if=/dev/zero of=images/bleskos.hdd bs=1024 count=10000
dd if=bootloader/bootloader.bin of=images/bleskos.hdd conv=notrunc seek=0
dd if=images/bleskos.bin of=images/bleskos.hdd conv=notrunc seek=5

qemu-system-i386 -hda images/bleskos.hdd
