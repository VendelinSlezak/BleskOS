#!/bin/bash

echo "Compiling BleskOS..."
mkdir compile
nasm -f bin source/bootloader/bootloader.asm -o compile/bootloader.bin
nasm -f elf32 source/assembly/bleskos.asm -o compile/bleskos_asm.bin
gcc -m32 -c -fno-stack-protector -ffreestanding -fno-PIC -std=gnu99 source/bleskos.c -o compile/bleskos_c.bin #-Wall -Wno-pointer-sign -Wno-unused-variable
ld -m elf_i386 --oformat=binary -T source/linker.ld -o compile/bleskos.bin compile/bleskos_asm.bin compile/bleskos_c.bin

echo "Creating image..."
dd if=/dev/zero of=bleskos.img bs=1024 count=1440
dd if=compile/bootloader.bin of=bleskos.img conv=notrunc seek=0
dd if=compile/bleskos.bin of=bleskos.img conv=notrunc seek=5
echo "Done"

#echo "Creating iso image..."
#mkdir iso
#cp bleskos.img iso/bleskos.img
#genisoimage -o bleskos.iso -b bleskos.img iso

#qemu-system-i386 -hda bleskos.img