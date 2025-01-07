#!/bin/bash

if [ $1 ];
then
 qemu-system-i386 -fda ./bleskos.img -hda ./testing/$1 -boot order=a -accel kvm
else
 qemu-system-i386 -fda ./bleskos.img -hda ./testing/disk.img -boot order=a -accel kvm -netdev user,id=net0 -device e1000,netdev=net0 -object filter-dump,id=f1,netdev=net0,file=/tmp/dump.dat
fi