# Hardware

Hardware drivers are separated to following four categories:

* controllers = devices to which other devices are connected. E.g. PCI bus, USB controller
* devices = end devices that work by itself. E.g. PS/2 keyboard, hard disk, ethernet card
* groups = group of devices that are controlled in same way have its own group that enables controlling them through same functions. E.g. Human Input group connects all devices that are providing input from human, as keyboards, mouses, HID devices...
* subsystems = extension of group, if this group is usually controlled through another layer. E.g. Storage Devices are usually controlled through Filesystem.

Software works only with groups and subsystems.

Example how devices can be connected:

```
PS/2 controller (controller) -> PS/2 keyboard (device) -> Human input (group)
PCI bus (controller) -> AHCI (controller) -> SATA hard disk (device) -> Storage (group) -> Filesystems (subsystem)
```