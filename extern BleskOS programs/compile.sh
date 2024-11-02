#!/bin/bash

echo "Compiling C code..."
gcc -m32 -c -fno-pic -ffreestanding -std=gnu99 $1.c -o $1.o
ld -Ttext 0x10000 -m elf_i386 --oformat binary -e main $1.o -o $1.bin

# you can change these commands to automatically copy compiled file to image
# echo "Copying .bin file..."
# sudo mount ... ...
# sudo cp $1.bin ...
# sudo umount ...