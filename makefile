MAKEFLAGS += --no-print-directory

export ROOT_DIR := $(CURDIR)
SRC_DIR = source
BUILD_DIR = build

C_SRCS = $(shell find $(SRC_DIR) -name "*.c")
HDR_SRCS = $(shell find $(SRC_DIR) -name "*.h")
C_SRCS_WITH_HDR := $(foreach f,$(C_SRCS),$(if $(wildcard $(patsubst %.c,%.h,$(f))),$(f)))

C_GEN_HDRS = $(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.h,$(C_SRCS))
C_GEN_HDRS_WITH_HDR := $(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.h,$(C_SRCS_WITH_HDR))
HDR_WITH_C = $(patsubst $(SRC_DIR)/%.c,$(SRC_DIR)/%.h,$(C_SRCS))
HDR_ONLY = $(filter-out $(HDR_WITH_C),$(HDR_SRCS))
HDR_ONLY_BUILD := $(patsubst $(SRC_DIR)/%,$(BUILD_DIR)/%,$(HDR_ONLY))

IMAGE = bleskos.img
TARGET_BOOTLOADER_LIVE = $(BUILD_DIR)/bootloader_legacy/bootloader_live.img

# those folders have their own makefiles
SUBDIRS = source/bootloader_legacy source/kernel source/userspace_library source/test

.PHONY: all headers build_subdirs build_live clean run_qemu run_bochs $(SUBDIRS)

all: build_live

# GENERATING ALL HEADERS
headers: $(C_GEN_HDRS) $(HDR_ONLY_BUILD)

# generate guard definition name from file path
define get_guard
$(shell echo "$1" | tr '/.' '_' | tr -cd 'A-Za-z0-9_' | tr '[:lower:]' '[:upper:]')
endef

# update file only if content has changed
define update_if_changed
	@if ! cmp -s "$1" "$2"; then \
		mv "$1" "$2"; \
	else \
		rm -f "$1"; \
	fi
endef

# generate .h files from .c + .h files
$(C_GEN_HDRS_WITH_HDR): $(BUILD_DIR)/%.h: $(SRC_DIR)/%.h $(SRC_DIR)/%.c
	@echo "[INFO] Generating header for $< and $(SRC_DIR)/$*.c..."
	@mkdir -p $(dir $@)
	$(eval GUARD := $(call get_guard,$@))
	@TMP="$@.tmp"; \
	{ \
		echo "#ifndef $(GUARD)"; \
		echo "#define $(GUARD)"; \
		echo ""; \
		cat "$(SRC_DIR)/$*.h"; \
		python3 extract_prototypes.py < "$(SRC_DIR)/$*.c"; \
		echo ""; \
		echo "#endif /* $(GUARD) */"; \
	} > "$$TMP"
	$(call update_if_changed,"$@.tmp","$@")

# generate .h files from .c files
$(BUILD_DIR)/%.h: $(SRC_DIR)/%.c
	@echo "[INFO] Generating header for $<..."
	@mkdir -p $(dir $@)
	$(eval GUARD := $(call get_guard,$@))
	@TMP="$@.tmp"; \
	{ \
		echo "#ifndef $(GUARD)"; \
		echo "#define $(GUARD)"; \
		echo ""; \
		python3 extract_prototypes.py < $<; \
		echo ""; \
		echo "#endif /* $(GUARD) */"; \
	} > "$$TMP"
	$(call update_if_changed,"$@.tmp","$@")

# copy pure .h files
$(BUILD_DIR)/%.h: $(SRC_DIR)/%.h
	@echo "[INFO] Copying $<..."
	@mkdir -p $(dir $@)
	$(eval GUARD := $(call get_guard,$@))
	@TMP="$@.tmp"; \
	{ \
		echo "#ifndef $(GUARD)"; \
		echo "#define $(GUARD)"; \
		echo ""; \
		cat "$<"; \
		echo ""; \
		echo "#endif /* $(GUARD) */"; \
	} > "$$TMP"
	$(call update_if_changed,"$@.tmp","$@")

# COMPILING ALL SUBDIRS
$(SUBDIRS):
	@echo "[INFO] Compiling folder $@..."
	$(MAKE) -C $@

build_subdirs: headers $(SUBDIRS)

# CREATING FINAL IMAGE FOR LIVE BOOT
build_live: build_subdirs
	@$(MAKE) -C ramdisk -f makefile
	@cp $(TARGET_BOOTLOADER_LIVE) $(IMAGE)
	@dd if=ramdisk/ramdisk.img of=$(IMAGE) conv=notrunc seek=10 status=none
	@echo "[SUCCESS] Image created: $(IMAGE)"

# RUN IMAGE IN QEMU
run_qemu: $(IMAGE)
	@echo "[RUN] Starting QEMU..."
	@qemu-system-i386 -drive file=$(IMAGE),format=raw,if=floppy \
                      -debugcon stdio \
                      -no-reboot -d cpu_reset,guest_errors \
                      -smp 1
	@echo "\n\n[RUN] QEMU exited"

# RUN IMAGE IN BOCHS
run_bochs: $(IMAGE)
	@echo "[RUN] Starting Bochs..."
	@bochs -f /home/user/bochs -debugger
	@echo "\n\n[RUN] Bochs exited"

clean:
	@for dir in $(SUBDIRS); do \
		$(MAKE) -C $$dir clean; \
	done
	@$(MAKE) -C ramdisk -f makefile clean
	@rm -rf $(BUILD_DIR) $(IMAGE)
