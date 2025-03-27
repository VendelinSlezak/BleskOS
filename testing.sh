#!/bin/bash

if [ $1 ];
then
    qemu-system-i386 -fda ./bleskos.img -hda ./testing/$1 -boot order=a -accel kvm
else
    qemu-system-i386 -fda ./bleskos.img -hda ./testing/disk.img -boot order=a -accel kvm -debugcon stdio \
    
fi