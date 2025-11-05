# Tools
C_COMPILER = gcc
C_COMPILER_FLAGS =  -m32 -ffreestanding -fno-stack-protector -fno-PIC -std=gnu99 -O2 \
          			-Wall -Wno-pointer-sign -Wno-unused-variable

ASSEMBLY_COMPILER = nasm
ASSEMBLY_COMPILER_FLAGS = -f elf32

LINKER = ld
LINKER_FLAGS = -z noexecstack -m elf_i386 -T source/linker.ld

OBJCOPY = objcopy
OBJCOPY_FLAGS = -O binary

# Directories
SRC_DIR = source
BUILD_DIR = build
IMAGE = bleskos.img

# Sources
C_SRCS = $(shell find $(SRC_DIR) -name "*.c" ! -path "$(SRC_DIR)/bootloader/*")
ASM_SRCS = $(shell find $(SRC_DIR) -name "*.asm" ! -path "$(SRC_DIR)/bootloader/*")
BOOTLOADER_ASM_SRCS = $(shell find "$(SRC_DIR)/bootloader" -name "*.asm")

# Object files
C_OBJS = $(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.c.o,$(C_SRCS))
ASM_OBJS = $(patsubst $(SRC_DIR)/%.asm,$(BUILD_DIR)/%.asm.o,$(ASM_SRCS))

# Header files (to generate in build/)
C_GEN_HDRS = $(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.h,$(C_SRCS))

# All header files
HDR_SRCS = $(shell find $(SRC_DIR) -name "*.h")
# Those headers which have a matching .c
HDR_WITH_C = $(patsubst $(SRC_DIR)/%.c,$(SRC_DIR)/%.h,$(C_SRCS))
# Headers without matching .c (need to be copied)
HDR_ONLY = $(filter-out $(HDR_WITH_C),$(HDR_SRCS))
# Destination .h files in build/
HDR_ONLY_BUILD = $(patsubst $(SRC_DIR)/%,$(BUILD_DIR)/%,$(HDR_ONLY))

# Targets
TARGET_ELF = $(BUILD_DIR)/kernel.elf
TARGET_BIN = $(BUILD_DIR)/kernel.bin
TARGET_BOOTLOADER_LIVE = $(BUILD_DIR)/bootloader/bootloader_live.img

.PHONY: clean build_live run_qemu run_bochs

# Create build dir
$(BUILD_DIR)/%:
	@mkdir -p $(dir $@)

# Create live bootloader image
$(TARGET_BOOTLOADER_LIVE): $(BOOTLOADER_ASM_SRCS)
	@echo "[INFO] Compiling live bootloader..."
	@mkdir -p $(dir $@)
	@$(ASSEMBLY_COMPILER) -f bin $(SRC_DIR)/bootloader/bootloader_live_mbr.asm -o $(BUILD_DIR)/bootloader/bootloader_live_mbr.bin
	@$(ASSEMBLY_COMPILER) -f bin $(SRC_DIR)/bootloader/bootloader_live_partition.asm -o $(BUILD_DIR)/bootloader/bootloader_live_partition.bin
	@$(ASSEMBLY_COMPILER) -f bin $(SRC_DIR)/bootloader/bootloader_live_extended.asm -o $(BUILD_DIR)/bootloader/bootloader_live_extended.bin
	@echo "[INFO] Creating live bootloader image..."
	@dd if=/dev/zero of=$@ bs=1024 count=1440 status=none
	@dd if=$(BUILD_DIR)/bootloader/bootloader_live_mbr.bin of=$@ conv=notrunc seek=0 status=none
	@dd if=$(BUILD_DIR)/bootloader/bootloader_live_partition.bin of=$@ conv=notrunc seek=1 status=none
	@dd if=$(BUILD_DIR)/bootloader/bootloader_live_extended.bin of=$@ conv=notrunc seek=2 status=none
	@echo "[SUCCESS] Live bootloader image created"

# Compile ASM
$(BUILD_DIR)/%.asm.o: $(SRC_DIR)/%.asm
	@echo "[INFO] Compiling $<..."
	@mkdir -p $(dir $@)
	@$(ASSEMBLY_COMPILER) $(ASSEMBLY_COMPILER_FLAGS) $< -o $@

# Generate .h file
$(BUILD_DIR)/%.h: $(SRC_DIR)/%.c
	@echo "[INFO] Generating header for $<..."
	@mkdir -p $(dir $@)
	@TMP_HDR="$@.tmp"; \
	GUARD_RAW="$@"; \
	GUARD_NAME=$$(echo "$$GUARD_RAW" | tr '/.' '_' | tr -cd 'A-Za-z0-9_'); \
	GUARD_NAME=$$(echo "$$GUARD_NAME" | tr '[:lower:]' '[:upper:]'); \
	if [ -f "$(SRC_DIR)/$*.h" ]; then cp "$(SRC_DIR)/$*.h" "$$TMP_HDR"; else : > "$$TMP_HDR"; fi; \
	echo "#ifndef $$GUARD_NAME" > "$$TMP_HDR.with_guard"; \
	echo "#define $$GUARD_NAME" >> "$$TMP_HDR.with_guard"; \
	echo "" >> "$$TMP_HDR.with_guard"; \
	cat "$$TMP_HDR" >> "$$TMP_HDR.with_guard"; \
	python3 extract_prototypes.py < $< >> "$$TMP_HDR.with_guard"; \
	echo "" >> "$$TMP_HDR.with_guard"; \
	echo "#endif /* $$GUARD_NAME */" >> "$$TMP_HDR.with_guard"; \
	if ! cmp -s "$$TMP_HDR.with_guard" "$@"; then \
		mv "$$TMP_HDR.with_guard" "$@"; \
	else \
		rm "$$TMP_HDR.with_guard"; \
	fi; \
	rm -f "$$TMP_HDR"

# Copy pure header files without .c
$(BUILD_DIR)/%.h: $(SRC_DIR)/%.h
	@echo "[INFO] Copying $<..."
	@mkdir -p $(dir $@)
	@TMP_HDR="$@.tmp"; \
	GUARD_RAW="$@"; \
	GUARD_NAME=$$(echo "$$GUARD_RAW" | tr '/.' '_' | tr -cd 'A-Za-z0-9_'); \
	GUARD_NAME=$$(echo "$$GUARD_NAME" | tr '[:lower:]' '[:upper:]'); \
	echo "#ifndef $$GUARD_NAME" > "$$TMP_HDR"; \
	echo "#define $$GUARD_NAME" >> "$$TMP_HDR"; \
	echo "" >> "$$TMP_HDR"; \
	cat "$<" >> "$$TMP_HDR"; \
	echo "" >> "$$TMP_HDR"; \
	echo "#endif /* $$GUARD_NAME */" >> "$$TMP_HDR"; \
	if ! cmp -s "$$TMP_HDR" "$@"; then \
		mv "$$TMP_HDR" "$@"; \
	else \
		rm "$$TMP_HDR"; \
	fi

# Compile .c file
$(BUILD_DIR)/%.c.o: $(SRC_DIR)/%.c $(BUILD_DIR)/%.h
	@echo "[INFO] Compiling $<..."
	@mkdir -p $(dir $@)
	@$(C_COMPILER) $(C_COMPILER_FLAGS) -Ibuild -include source/global_declarations.h -include $(@:.c.o=.h) -c $< -o $@

# Link
$(TARGET_ELF): $(ASM_OBJS) $(C_OBJS)
	@echo "[INFO] Linking..."
	@$(LINKER) $(LINKER_FLAGS) -o $@ $(ASM_OBJS) $(C_OBJS)

# Convert to BIN
$(TARGET_BIN): $(TARGET_ELF)
	@echo "[INFO] Converting ELF to BIN..."
	@$(OBJCOPY) $(OBJCOPY_FLAGS) $< $@

# Build live image
build_live: $(TARGET_BOOTLOADER_LIVE) $(C_GEN_HDRS) $(HDR_ONLY_BUILD) $(TARGET_BIN)
	@cp $(TARGET_BIN) ramdisk/kernel.bin
	@$(MAKE) -C ramdisk -f makefile
	@echo "[INFO] Creating bootable image..."
	@cp $(TARGET_BOOTLOADER_LIVE) $(IMAGE)
	@dd if=ramdisk/ramdisk.img of=$(IMAGE) conv=notrunc seek=10 status=none
	@echo "[SUCCESS] Image created: $(IMAGE)"

# Run qemu
run_qemu: $(IMAGE)
	@echo "[RUN] Starting QEMU..."
	@qemu-system-i386 -drive file=$(IMAGE),format=raw,if=floppy -debugcon stdio -no-reboot -smp 2
	@echo "\n\n[RUN] QEMU exited"

# Run bochs (you need to replace /home/user/bochs with your path to configuration file)
run_bochs: $(IMAGE)
	@echo "[RUN] Starting Bochs..."
	@bochs -f /home/user/bochs
	@echo "\n\n[RUN] Bochs exited"

# Clean
clean:
	@echo "[INFO] Removing build artifacts..."
	@rm -rf $(BUILD_DIR) $(IMAGE)
	@echo "[SUCCESS] Artifacts removed"