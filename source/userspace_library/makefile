SRC_DIR = $(ROOT_DIR)/source
BUILD_DIR = $(ROOT_DIR)/build

C_COMPILER = gcc
C_COMPILER_FLAGS = -m32 -ffreestanding -fno-stack-protector -fno-PIC -march=i686 -std=gnu99 -O2 \
                    -Wall -Wno-pointer-sign -Wno-unused-variable -g

ASSEMBLY_COMPILER = nasm
ASSEMBLY_COMPILER_FLAGS = -f elf32

LINKER = ld
LIB_LINKER_FLAGS = -z noexecstack -m elf_i386 -Ttext 0xA0000000

TARGET_LIBS_ELF = $(BUILD_DIR)/userspace_library.elf

LIB_SRC_DIR = $(SRC_DIR)/userspace_library
LIB_C_SRCS = $(shell find $(LIB_SRC_DIR) -name "*.c")
LIB_ASM_SRCS = $(shell find $(LIB_SRC_DIR) -name "*.asm")
LIB_OBJS = $(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.c.o,$(LIB_C_SRCS)) \
           $(patsubst $(SRC_DIR)/%.asm,$(BUILD_DIR)/%.asm.o,$(LIB_ASM_SRCS))

.PHONY: all clean

all: $(TARGET_LIBS_ELF)

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

# linking
$(TARGET_LIBS_ELF): $(LIB_OBJS)
	@echo "[INFO] Linking Libraries ELF..."
	@$(LINKER) $(LIB_LINKER_FLAGS) -o $@ $(LIB_OBJS)
	@cp $(TARGET_LIBS_ELF) $(ROOT_DIR)/ramdisk/userspace_library.elf

clean:
	@rm -f $(TARGET_LIBS_ELF) $(LIB_OBJS) $(ROOT_DIR)/ramdisk/userspace_library.elf