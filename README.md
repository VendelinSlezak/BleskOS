# BleskOS

IMPORTANT NOTE: I started developing new multitasking kernel, with bunch of improvements of overall code. Actual source code you are looking at is code in progress. If you want to get last usable version of BleskOS, download [v2025u10](https://github.com/VendelinSlezak/BleskOS/releases/tag/v2025u10). Detailed informations about development of new kernel and new phase of BleskOS, you can visit this topic: [New phase of BleskOS (entity-based multitasking kernel)](https://github.com/VendelinSlezak/BleskOS/discussions/104)

## Build

### Prerequisites

* gcc
* nasm
* ld
* objcopy
* python3
* (optional) qemu / bochs / virtualbox

Latest version of everything is the best, but code should be able to compile even under older versions.

```bash
sudo apt install gcc nasm binutils python3 qemu-system-x86
```

### Clone repository

```bash
git clone https://github.com/VendelinSlezak/BleskOS
cd BleskOS
```

### Compile source code

```bash
make build_live
```

`build_live` will produce image that is immediately runnable from everywhere as fully functional operating system.

### Run image

Command `make build_live` generated bleskos.img, which can be connected to emulator as floppy or hard disk, or it can be written to USB flash.

If you have installed qemu or bochs, you can directly run them from makefile with: 

```bash
make run_qemu
```

Or if you have installed Bochs, you can edit path to your bochsrc file in `makefile` and run it with:

```bash
make run_bochs
```

Usually during development those commands work best together as:

```bash
make build_live run_qemu
```

### Remove build files

```bash
make clean
```

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
