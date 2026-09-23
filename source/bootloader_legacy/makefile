BUILD_DIR = $(ROOT_DIR)/build
SRC_DIR = $(ROOT_DIR)/source/bootloader_legacy

ASSEMBLY_COMPILER = nasm
ASSEMBLY_COMPILER_FLAGS = -f bin

TARGET_BOOTLOADER_LIVE = $(BUILD_DIR)/bootloader_legacy/bootloader_live.img
BOOTLOADER_ASM_SRCS = $(shell find $(SRC_DIR) -name "*.asm")

.PHONY: all clean

all: $(TARGET_BOOTLOADER_LIVE)

$(TARGET_BOOTLOADER_LIVE): $(BOOTLOADER_ASM_SRCS)
	@echo "[INFO] Compiling live bootloader..."
	@mkdir -p $(dir $@)
	@$(ASSEMBLY_COMPILER) $(ASSEMBLY_COMPILER_FLAGS) $(SRC_DIR)/bootloader_live_mbr.asm -o $(BUILD_DIR)/bootloader_legacy/bootloader_live_mbr.bin
	@$(ASSEMBLY_COMPILER) $(ASSEMBLY_COMPILER_FLAGS) $(SRC_DIR)/bootloader_live_partition.asm -o $(BUILD_DIR)/bootloader_legacy/bootloader_live_partition.bin
	@$(ASSEMBLY_COMPILER) $(ASSEMBLY_COMPILER_FLAGS) $(SRC_DIR)/bootloader_live_extended.asm -o $(BUILD_DIR)/bootloader_legacy/bootloader_live_extended.bin
	@echo "[INFO] Creating live bootloader image..."
	@dd if=/dev/zero of=$@ bs=1024 count=1440 status=none
	@dd if=$(BUILD_DIR)/bootloader_legacy/bootloader_live_mbr.bin of=$@ conv=notrunc seek=0 status=none
	@dd if=$(BUILD_DIR)/bootloader_legacy/bootloader_live_partition.bin of=$@ conv=notrunc seek=1 status=none
	@dd if=$(BUILD_DIR)/bootloader_legacy/bootloader_live_extended.bin of=$@ conv=notrunc seek=2 status=none
	@echo "[SUCCESS] Live bootloader image created"

clean:
	@rm -rf $(BUILD_DIR)/bootloader_legacy