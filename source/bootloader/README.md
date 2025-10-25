# BleskOS bootloader

BleskOS uses custom bootloader.

Now there is only one version of bootloader for legacy live booting. This bootloader consists from three files:

* `bootloader_live_mbr.asm` - This file converts to 512 bytes output, which is MBR sector in resulting image. It loads sector 1 to 0x7C00.
* `bootloader_live_partition.asm` - This file converts to 512 bytes output, which is partition bootloader sector. It loads 8 sectors from sector 2 to 0x1000.
* `bootloader_live_extended.asm` - This file contains extended part of bootloader that loads BleskOS ramdisk, prepares environment for kernel and starts it.

Folder `modules` contains files that are included by `bootloader_live_extended.asm`

## Bootloader output

Result from compiling bootloader is `bootloader_live.img`. It's layout is:

| Sector | Description |
|:---|:---|
| 0 | MBR |
| 1 | Partition bootloader |
| 2 - 9 | Extended bootloader |

Ramdisk needs to be written from sector 10.

## BleskOS kernel environment

BleskOS kernel expects those conditions to be met before execution:

* Loaded informations about physical memory and current graphic output
* A20 line enabled
* Paging enabled
* Protected mode

This is final memory layout before kernel.bin execution:

| Virtual memory | Description |
|:---|:---|
| 0x1C000 - 0x1CFFF | Physical memory map |
| 0x1D000 - 0x1DFFF | 32 bit free physical memory map |
| 0x1E000 - 0x1EFFF | Current graphic output info |
| 0x1F000 - 0x1FFFF | Kernel stack |
| 0xFF000000 - 0xFF100000 | Ramdisk |
| 0xFF001000 - ? | kernel.bin |

Execution starts by `jmp 0xFF001000`.

Job of creating this environment is on `bootloader_live_extended.asm`.