#!/bin/bash

echo "Compiling BleskOS..."
mkdir -p compile
rm -f compile/*.bin

# compile assembly code
nasm -f bin source/bootloader/bootloader.asm -o compile/bootloader.bin
nasm -f elf32 source/assembly/bleskos.asm -o compile/bleskos_asm.bin

# compile C code (possible arguments: - / fast / wall)
if $( echo $@ | grep -Fiq wall )
then
	gcc -m32 -c -fno-stack-protector -ffreestanding -fno-PIC -std=gnu99 source/bleskos.c -o compile/bleskos_c.bin -O0 -Wall -Wno-pointer-sign -Wno-unused-variable 
elif $( echo $@ | grep -Fiq fast )
then
	gcc -m32 -c -fno-stack-protector -ffreestanding -fno-PIC -std=gnu99 source/bleskos.c -o compile/bleskos_c.bin -O0
else
	gcc -m32 -c -fno-stack-protector -ffreestanding -fno-PIC -std=gnu99 source/bleskos.c -o compile/bleskos_c.bin -Os
fi

# link assembly code and C code together
ld -m elf_i386 --oformat=binary -T source/linker.ld -o compile/bleskos.bin compile/bleskos_asm.bin compile/bleskos_c.bin

# if compilation was successfull, create bleskos.img
if [ -f compile/bleskos.bin ] && [ -f compile/bootloader.bin ]
then
	echo "Creating image..."
    rm -f bleskos.img
	dd if=/dev/zero of=bleskos.img bs=1024 count=1440
	dd if=compile/bootloader.bin of=bleskos.img conv=notrunc seek=0
	dd if=compile/bleskos.bin of=bleskos.img conv=notrunc seek=10
	echo "Done"
fi

# if we want to compile new release, create iso and img files and zip them
if $( echo $@ | grep -Fiq release) 
then
	echo "Creating iso image..."
	mkdir -p iso
	cp bleskos.img iso/bleskos.img
	genisoimage -o bleskos.iso -b bleskos.img iso

    RELEASE=$(grep -E 'BLESKOS_VERSION|BLESKOS_UPDATE' source/bleskos.h | \
        awk -F ' ' '{print $3}' | \
        paste -sd ' ' - | \
        awk '{print "bleskos_v" $1 "u" $2}')
	echo Release: $RELEASE
	mv bleskos.img ${RELEASE}.img
    ln -s ${RELEASE}.img bleskos.img
	zip ${RELEASE}_img.zip ${RELEASE}.img
	mv bleskos.iso ${RELEASE}.iso
	zip ${RELEASE}_iso.zip ${RELEASE}.iso

    rm -r compile
    rm -r iso

    qemu-system-i386 -hda ${RELEASE}.img -accel kvm
    qemu-system-i386 -cdrom ${RELEASE}.iso -accel kvm
fi