#!/bin/bash

if [ $1 ];
then
 qemu-system-i386 -fda ./bleskos.img -hda ./testing/$1.img -accel kvm -boot order=a
else
 qemu-system-i386 -fda ./bleskos.img -hda ./testing/disk.img -accel kvm -boot order=a
fi