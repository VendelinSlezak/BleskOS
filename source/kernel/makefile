SRC_DIR = $(ROOT_DIR)/source
BUILD_DIR = $(ROOT_DIR)/build

C_COMPILER = gcc
C_COMPILER_FLAGS = -m32 -ffreestanding -fno-stack-protector -fno-PIC -march=i686 -std=gnu99 -O2 \
                    -Wall -Wno-pointer-sign -Wno-unused-variable -g

ASSEMBLY_COMPILER = nasm
ASSEMBLY_COMPILER_FLAGS = -f elf32

LINKER = ld
LINKER_FLAGS = -z noexecstack -m elf_i386 -T $(SRC_DIR)/kernel/linker.ld

OBJCOPY = objcopy
OBJCOPY_FLAGS = -O binary

TARGET_ELF = $(BUILD_DIR)/kernel.elf
TARGET_BIN = $(BUILD_DIR)/kernel.bin

KERNEL_SRC_DIR = $(SRC_DIR)/kernel
C_SRCS = $(shell find $(KERNEL_SRC_DIR) -name "*.c")
ASM_SRCS = $(shell find $(KERNEL_SRC_DIR) -name "*.asm")
KERNEL_OBJS = $(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.c.o,$(C_SRCS)) \
              $(patsubst $(SRC_DIR)/%.asm,$(BUILD_DIR)/%.asm.o,$(ASM_SRCS))

.PHONY: all clean

all: $(TARGET_BIN)

# compiling .c files
$(BUILD_DIR)/%.c.o: $(SRC_DIR)/%.c
	@echo "[INFO] Compiling $<..."
	@mkdir -p $(dir $@)
	@$(C_COMPILER) $(C_COMPILER_FLAGS) -I$(BUILD_DIR) -include $(SRC_DIR)/global_declarations.h -include $(@:.c.o=.h) -c $< -o $@

# compiling .asm files
$(BUILD_DIR)/%.asm.o: $(SRC_DIR)/%.asm
	@echo "[INFO] Compiling $<..."
	@mkdir -p $(dir $@)
	@$(ASSEMBLY_COMPILER) $(ASSEMBLY_COMPILER_FLAGS) $< -o $@

# linking .elf file
$(TARGET_ELF): $(KERNEL_OBJS)
	@echo "[INFO] Linking Kernel..."
	@$(LINKER) $(LINKER_FLAGS) -o $@ $(KERNEL_OBJS)

# converting .elf file to final .bin file
$(TARGET_BIN): $(TARGET_ELF)
	@echo "[INFO] Converting kernel.elf to kernel.bin..."
	@$(OBJCOPY) $(OBJCOPY_FLAGS) $< $@
	@cp $(TARGET_BIN) $(ROOT_DIR)/ramdisk/kernel.bin

clean:
	@rm -f $(TARGET_ELF) $(TARGET_BIN) $(KERNEL_OBJS) $(ROOT_DIR)/ramdisk/kernel.bin