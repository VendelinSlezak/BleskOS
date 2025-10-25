ASM_FILE  := ramdisk.asm
IMG_FILE  := ramdisk.img

FILES := $(filter kernel.bin,$(wildcard *)) $(filter-out makefile $(ASM_FILE) $(IMG_FILE) kernel.bin,$(wildcard *))

all: $(IMG_FILE)

$(IMG_FILE): $(ASM_FILE)
	@echo "[RAMDISK] Creating image..."
	@nasm -f bin $(ASM_FILE) -o $@
	@rm $(ASM_FILE)
	@echo "[RAMDISK] Image $(IMG_FILE) successfully created"

$(ASM_FILE): $(FILES)
	@echo "[RAMDISK] Generating content..."
	@echo "section .data" > $@
	@echo "org 0" >> $@
	@echo "start_of_image:" >> $@

	@echo "    dd end_of_image - start_of_image" >> $@

	@i=0; \
	for f in $(FILES); do \
	    name=$$(basename $$f); \
	    echo "    dd start_file$$i, end_file$$i - start_file$$i" >> $@; \
	    echo "    db \"$$name\",0" >> $@; \
	    i=$$((i+1)); \
	done

	@echo '    times 0x1000-($$-$$$$) db 0 ; align table to one page' >> $@

	@i=0; \
	for f in $(FILES); do \
	    echo "start_file$$i:" >> $@; \
	    echo "    incbin \"$$f\"" >> $@; \
	    echo "end_file$$i:" >> $@; \
	    i=$$((i+1)); \
	done

	@echo "end_of_image:" >> $@

clean:
	rm -f $(ASM_FILE) $(IMG_FILE)
