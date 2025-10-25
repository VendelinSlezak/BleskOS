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

%define PAGE_SIZE 4096

%define BIOS_PHYSICAL_MEMORY_MAP_START 0x3000
%define BIOS_PHYSICAL_MEMORY_MAP_ENTRY_SIZE 24

%define STANDARDIZED_PHY_MEM_MAP_START 0x2000
%define STANDARDIZED_PHY_MEM_MAP_FREE_ENTRY 1
%define STANDARDIZED_PHY_MEM_MAP_USED_ENTRY 2
%define STANDARDIZED_PHY_MEM_MAP_ENTRY_SIZE 20

%define STANDARDIZED_FREE_PHY_MEM_MAP_START 0x3000
%define STANDARDIZED_FREE_PHY_MEM_MAP_ENTRY_SIZE 8

%define PAGE_DIRECTORY_START 0x4000
%define PAGE_TABLE_FIRST_MB 0x5000
%define PAGE_TABLE_COMMAND 0x6000
%define PAGE_TABLE_KERNEL 0x7000
%define PAGE_TABLE_SHARED 0x8000
%define PAGE_TABLE_PM_STACK 0x9000

%define STANDARDIZED_GRAPHIC_OUTPUT_INFO_START 0xA000

%define PAGE_OF_LOADED_DATA_START 0xB000

%define PAGE_LOGS 0xC000

%macro BOOTLOADER_PANIC 2+
    section .rodata
    %%string:
        db %2, 0
    section .text
        mov si, %%string
        %1 bootloader_panic
%endmacro

%macro PRINT_STRING 1
    section .rodata
    %%string:
        db %1, 0
    section .text
        lea si, [%%string]
        call print_first_line_string
%endmacro

%macro CONTROLLER_8042_SEND_CMD 1
    mov  al, %1
    call controller_8042_send_command
%endmacro

%macro CONTROLLER_8042_SEND_DATA 1
    mov  al, %1
    call controller_8042_send_data
%endmacro

%macro CONTROLLER_8042_READ_DATA 1
    call controller_8042_read_data
    mov  %1, al
%endmacro