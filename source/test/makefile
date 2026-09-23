C_COMPILER = gcc
C_COMPILER_FLAGS = -m32 -march=i686 -std=gnu99 -O2 -g

LINKER = ld
LINKER_FLAGS = -m elf_i386

SRC_DIR = $(ROOT_DIR)/source
BUILD_DIR = $(ROOT_DIR)/build

TEST_SRC_DIR = $(SRC_DIR)/test
C_SRCS = $(shell find $(TEST_SRC_DIR) -name "*.c")
OBJS = $(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.c.o,$(C_SRCS))

TARGET_ELF = $(BUILD_DIR)/test.elf

.PHONY: all clean

all: $(TARGET_ELF)

$(BUILD_DIR)/%.c.o: $(SRC_DIR)/%.c
	@echo "[INFO] Compiling $<..."
	@mkdir -p $(dir $@)
	@$(C_COMPILER) $(C_COMPILER_FLAGS) -I$(BUILD_DIR) -include $(SRC_DIR)/global_declarations.h -include $(@:.c.o=.h) -c $< -o $@

$(TARGET_ELF): $(OBJS)
	@echo "[INFO] Linking Test ELF ($@)..."
	@$(LINKER) $(LINKER_FLAGS) -o $@ $(OBJS)
	@cp $(TARGET_ELF) $(ROOT_DIR)/ramdisk/test.elf

clean:
	@rm -f $(TARGET_ELF) $(OBJS) $(ROOT_DIR)/ramdisk/test.elf