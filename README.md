# BleskOS

IMPORTANT NOTE: I started developing new multitasking kernel, with bunch of improvements of overall code. Actual source code you are looking at is code in progress. If you want to get last usable version of BleskOS, download [v2025u10](https://github.com/VendelinSlezak/BleskOS/releases/tag/v2025u10). Detailed informations about development of new kernel and new phase of BleskOS, you can visit this topic: [New phase of BleskOS (entity-based multitasking kernel)](https://github.com/VendelinSlezak/BleskOS/discussions/104)

## Build

You need to have installed gcc, nasm, ld, objcopy and python3.

Then you can build BleskOS from source code by:

```
make build_live
```

This will generate bleskos.img, which can be connected to emulator as floppy or hard disk, or it can be written to USB flash. If you have installed qemu or bochs, you can directly run them from makefile by `make run_qemu` or `make run_bochs`.

You can remove generated files by `make clean`.

## File organization

```
BleskOS <- you are here
├── source
│   ├── bootloader_legacy
|   |   ├── modules
|   |   ├── bootloader_live_mbr.asm
|   |   ├── bootloader_live_partition.asm
|   |   └── bootloader_live_extended.asm
│   ├── kernel
|   |   ├── firmware
|   |   ├── hardware
|   |   ├── software
|   |   ├── libc
|   |   ├── kernel.asm
|   |   ├── kernel.c
|   |   ├── kernel.h
|   |   └── linker.ld
│   ├── userspace_library
│   └── global_declarations.h
├── ramdisk
├── extract_prototypes.py
├── makefile
├── LICENSE
└── CODING_GUIDELINES.md
```

Folder `source` contains all source code. It has several folders. Every folder contains source code that will be compiled to one result file, and then they will be all baked together in ramdisk.

Folder `ramdisk` contains all files that will be transformed to one result image which will be loaded by bootloader.

File `extract_prototypes.py` contains code for extracting prototypes of C functions so we do not need to do that manually.
