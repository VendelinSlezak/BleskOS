# Kernel

The most basic principle of BleskOS kernel is that everything is entity. Entity is thing that has attributes and can be operated with some functions. It is similar to object in OOP, but it does not have any more OOP characteristics.

If you want entity to do something, you give it a command. Anything that runs independently in BleskOS is command. There are not processes or threads, only commands, but those can be runned in way, that they act like processes or threads.

Source code is open for migration to other architectures, but for now there is support only for x86 32 bit.