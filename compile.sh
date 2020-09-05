#!/bin/bash

echo "Compiling BleskOS..."
nasm -f bin bleskos.asm -o bleskos.bin

echo "Compiling BleskOS bootloader..."
nasm -f bin bootloader/bootloader.asm -o bootloader/bootloader.bin

echo "Erasing image..."
dd if=/dev/zero of=bleskos.img bs=1024 count=1440

echo "Copying bootloader..."
dd if=bootloader/bootloader.bin of=bleskos.img conv=notrunc seek=0

echo "Copying BleskOS..."
dd if=bleskos.bin of=bleskos.img conv=notrunc seek=1

echo "Starting Qemu..."
qemu-system-i386 -soundhw pcspk -soundhw hda -net nic,model=rtl8139 -net user -hda bleskos.img

sleep 50
