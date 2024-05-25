#!/bin/bash

if [ $1 ];
then
 qemu-system-i386 -fda ./bleskos.img -hda ./testing/$1 -boot order=a -soundhw hda -accel kvm
else
 qemu-system-i386 -fda ./bleskos.img -hda ./testing/disk.img -boot order=a -soundhw hda -accel kvm
fi