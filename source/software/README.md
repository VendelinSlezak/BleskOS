# Software

This folder contains all code that is not working with hardware. There are programs that are usually called libraries and applications.

* `interfaces` = This folder contains code of interfaces. Every program needs to be registered through some interface, which provides specific functions for it to work.
* `ramdisk_programs` = This folder contains code of all programs that are on BleskOS ramdisk, so they are in memory immediately after exit of bootloader.

Code of this section is now a little messy, and do not fully implement model described above.