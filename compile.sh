#!/bin/bash

echo "Compiling BleskOS..."
mkdir -p compile
rm -f compile/*.bin

nasm -f bin source/bootloader/bootloader.asm -o compile/bootloader.bin
nasm -f elf32 source/assembly/bleskos.asm -o compile/bleskos_asm.bin

if $( echo $@ | grep -Fiq wall )
then
	gcc -m32 -c -fno-stack-protector -ffreestanding -fno-PIC -std=gnu99 source/bleskos.c -o compile/bleskos_c.bin -Wall -Wno-pointer-sign -Wno-unused-variable 
else
	gcc -m32 -c -fno-stack-protector -ffreestanding -fno-PIC -std=gnu99 source/bleskos.c -o compile/bleskos_c.bin
fi

ld -m elf_i386 --oformat=binary -T source/linker.ld -o compile/bleskos.bin compile/bleskos_asm.bin compile/bleskos_c.bin

if [ -f compile/bleskos.bin ] && [ -f compile/bootloader.bin ]
then
	echo "Creating image..."
	dd if=/dev/zero of=bleskos.img bs=1024 count=1440
	dd if=compile/bootloader.bin of=bleskos.img conv=notrunc seek=0
	dd if=compile/bleskos.bin of=bleskos.img conv=notrunc seek=10
	echo "Done"
fi

if $( echo $@ | grep -Fiq iso) 
then
	echo "Creating iso image..."
	mkdir -p iso
	cp bleskos.img iso/bleskos.img
	genisoimage -o bleskos.iso -b bleskos.img iso

	RELEASE=$(grep -F 'print_to_message_window("Version' source/bleskos.c  | while IFS=' "' read A B C D E F; do echo bleskos_${C}u${E} ; done)
	echo Release: $RELEASE
	mv bleskos.img ${RELEASE}.img
	zip ${RELEASE}_img.zip ${RELEASE}.img
	mv bleskos.iso ${RELEASE}.iso
	zip ${RELEASE}_iso.zip ${RELEASE}.iso
fi