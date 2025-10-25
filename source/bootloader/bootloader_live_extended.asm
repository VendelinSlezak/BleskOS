;;;;;
;;
;; BleskOS
;;
;; MIT License
;; Copyright (c) 2023-2025 BleskOS developers
;; Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
;; The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
;; THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
;;
;;;;;

%include "source/bootloader/modules/global_declarations.asm"

; bootloader of partition will load this code here
org 0x1000

; after start processor is in 16 bit mode
bits 16

; input: dl = number of boot device
start:
    ; segment registers are already set to zero by bootloader of partition

    ; save boot device number
    mov byte [boot_device_number], dl

    ; set stack to grow below executed code
    mov sp, 0x1000

    ; read info about physical memory
    PRINT_STRING 'Reading physical memory informations...'
    call read_physical_memory_map_from_bios
    call create_standardized_phy_memory_map
    call create_standardized_32_bit_free_memory_map

    ; enable A20 line
    PRINT_STRING 'Enabling A20 line...'
    call enable_a20_line

    ; initialize paging structures
    PRINT_STRING 'Initializing paging structures...'
    call initialize_paging_memory_structures

    ; load BleskOS ramdisk to memory
    PRINT_STRING 'Preparing to read ramdisk...'
    call reset_boot_device
    call read_geometry_of_boot_device
    call load_ramdisk_to_physical_and_virtual_memory

    ; print string to VGA mode, it will be visible if no graphic mode will be set
    PRINT_STRING 'Starting BleskOS kernel...'

    ; set best graphic mode possible
    call find_and_set_best_graphic_mode

    ; switch to protected mode
    cli ; we do not want to be interrupted

    lgdt [protected_mode_gdt_wrap] ; load GDT table that allows using 4 GB of memory

    mov eax, cr0
    or eax, 0x00000001 ; set protected mode bit
    mov cr0, eax

    jmp 0x0008:.protected_mode_code ; set cs register to GDT code segment 0x0008
    bits 32
    .protected_mode_code:
    mov ax, 0x0010 ; set all other segment registers to GDT data segment 0x0010
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax

    ; enable paging
    mov eax, PAGE_DIRECTORY_START
    mov cr3, eax

    mov eax, cr0
    or eax, 0x80000000 ; paging bit
    mov cr0, eax

    ; set stack
    mov esp, 0x20000

    ; start execution of BleskOS kernel
    jmp 0xFF001000

; FUNCTIONS

bits 16

%include "source/bootloader/modules/screen_output.asm"
%include "source/bootloader/modules/a20.asm"
%include "source/bootloader/modules/8042_controller.asm"
%include "source/bootloader/modules/memory.asm"
%include "source/bootloader/modules/storage.asm"
%include "source/bootloader/modules/vbe.asm"
%include "source/bootloader/modules/gdt.asm"