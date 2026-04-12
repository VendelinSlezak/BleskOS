CC      = gcc
CFLAGS  = -m32 -static -ffreestanding -nostdlib -fno-pie -no-pie -O2
LDFLAGS = -Wl,-e,main
TARGET  = ../ramdisk/dump_log.elf
SRC     = dump_log.c

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) $(LDFLAGS) $(SRC) -o $(TARGET)

clean:
	rm -f $(TARGET)

.PHONY: all clean