# x86 kernel

Files

* `kernel_32.asm` - This file is executed after bootloader jumps to kernel code. It calls 
* `kernel.c / kernel.h` - This file contains highest layer of kernel. Function `void initialize_kernel(void)` will boot kernel and all connected drivers and software interfaces and then move execution to main_window.
* `system_call.c / system_call.h` - BleskOS system calls can be invoked through `int 0xA0`. This file processes this interrupt.

Folders

* `acpi` - Code for parsing ACPI tables.
* `cmos` - Code for reading data in CMOS.
* `cpu` - Code for CPU commands, and CPU specific structures (IDT, GDT, TSS)
* `entities` - Code for dealing with entities and commands
* `hardware` - Code that connects kernel and drivers
* `interrupt_controllers` - Code for initializing interrupt controllers (now only PIC)
* `libc` - Code for BleskOS implementation of standard C library with some extensions. Currently only small subset of functions is implemented.
* `memory` - Code for dealing with physical and virtual memory
* `ramdisk` - Code for getting files from loaded ramdisk
* `scheduler` - Code for preempetive scheduling of commands
* `timers` - Code for working with timers (now only PIT)