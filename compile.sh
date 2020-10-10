#!/bin/bash

nasm -f bin bleskos.asm -o bleskos_images/bleskos.bin
nasm -f bin bootloader/bootloader.asm -o bootloader/bootloader.bin

dd if=/dev/zero of=bleskos_images/bleskos.img bs=1024 count=1440
dd if=bootloader/bootloader.bin of=bleskos_images/bleskos.img conv=notrunc seek=0
dd if=bleskos_images/bleskos.bin of=bleskos_images/bleskos.img conv=notrunc seek=1

qemu-system-i386 -soundhw pcspk -hda bleskos_images/bleskos.img

sleep 50
