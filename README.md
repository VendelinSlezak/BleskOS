# BleskOS

IMPORTANT NOTE: I started developing new multitasking kernel, with bunch of improvements of overall code. Actual source code you are looking at is code in progress. If you want to get last usable version of BleskOS, download [v2025u10](https://github.com/VendelinSlezak/BleskOS/releases/tag/v2025u10). Detailed informations about development of new kernel and new phase of BleskOS, you can visit this topic: [New phase of BleskOS (entity-based multitasking kernel)](https://github.com/VendelinSlezak/BleskOS/discussions/104)

## Build

You need to have installed gcc, nasm, ld, objcopy and python3.

Then you can build BleskOS from source code by:

```
make build_live
```

This will generate bleskos.img, which can be connected to emulator as floppy or hard disk, or it can be written to USB flash. If you have installed qemu or bochs, you can directly run them from makefile by `run_qemu` or `run_bochs`.

You can remove generated files by `clean`.
