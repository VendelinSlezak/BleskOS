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

; BOOTLOADER FUNCTION
read_geometry_of_boot_device:
    test byte [boot_device_number], 0x80
    jnz .if_chs_disk
        mov ah, 0x08
        mov dl, byte [boot_device_number]
        int 13h
        mov ax, 0
        mov es, ax ; set es to zero to be sure that int 13h did not messed it up

        BOOTLOADER_PANIC jc, 'can not read CHS disk geometry'

        ; compute parameters for LBA to CHS conversion
        movzx ax, cl
        and ax, 0x3F
        mov word [sectors_per_head], ax
        movzx bx, dh
        inc bx ; number of heads is 0-based
        mul bx
        mov word [sectors_per_cylinder], ax
    .if_chs_disk:

    ret

; BOOTLOADER FUNCTION
reset_boot_device:
    mov dl, byte [boot_device_number]
    mov ax, 0
    int 13h
    BOOTLOADER_PANIC jc, 'can not reset boot device'

    ret

; BOOTLOADER FUNCTION
load_ramdisk_to_physical_and_virtual_memory:
    ; GET SIZE OF RAMDISK THAT NEEDS TO BE LOADED INTO MEMORY
    mov bp, PAGE_OF_LOADED_DATA_START
    mov ax, 10
    call read_sector

    ; LOAD BLESKOS RAMDISK TO MEMORY
    mov edi, 0xFF000000 ; pointer to virtual memory
    mov dword [data_input], PAGE_OF_LOADED_DATA_START ; we will always copy data from here
    mov dword [data_size], PAGE_SIZE ; we will always copy one page of data
    mov eax, dword [PAGE_OF_LOADED_DATA_START] ; read size of ramdisk in bytes
    shr eax, 12
    inc eax
    mov word [ramdisk_number_of_pages], ax ; size was converted to number of pages
    mov ax, 10
    mov cx, word [ramdisk_number_of_pages]
    .load_page_of_data:
    push cx
        ; print how much data is loaded
        pusha
            mov ax, 100
            mov bx, word [ramdisk_number_of_pages]
            sub bx, cx
            mul bx
            mov bx, word [ramdisk_number_of_pages]
            mov dx, 0
            div bx
            call print_loaded_percent
        popa

        ; load one page of data from disk
        mov bp, PAGE_OF_LOADED_DATA_START
        call read_page_of_data

        ; save LBA number of sector to be readed
        add ax, 8
        push ax

        ; allocate page from physical memory
        mov eax, PAGE_SIZE
        call phy_alloc

        ; map this page to virtual memory
        mov eax, (1 << 1) | (1 << 0) ; flags: write-back, supervisor, read-write, present
        mov esi, ebp ; phy_alloc returned physical memory address in ebp
        call map_page_to_virtual_memory
        add edi, PAGE_SIZE ; move to next page in virtual memory

        ; copy disk data to this page
        mov dword [data_output], esi
        call copy_data

        ; restore LBA number of sector to be readed
        pop ax 
    pop cx
    loop .load_page_of_data

    PRINT_STRING 'Ramdisk successfully loaded'

    ret

; input: ax = LBA number of first sector, bp = offset where data will be loaded
; output: nothing
read_page_of_data:
    pusha

    ; test if we will use CHS method or LBA method
    test byte [boot_device_number], 0x80
    jnz .use_lba_read

    ; use CHS
    mov cx, 8
    mov bp, PAGE_OF_LOADED_DATA_START
    .load_sector_of_data:
        call read_sector
        inc ax ; next sector
        add bp, 512
    loop .load_sector_of_data

    popa
    ret

    .use_lba_read:
    ; set packet
    mov word [ah_0x42_packet.signature], 0x0010 ; set signature
    mov word [ah_0x42_packet.number_of_sectors], 8 ; we will read eight sectors
    mov word [ah_0x42_packet.offset], bp ; set physical memory pointer
    mov word [ah_0x42_packet.segment], 0x0000
    mov word [ah_0x42_packet.lba_lower_word], ax ; LBA value
    mov word [ah_0x42_packet.lba_upper_word], 0
    mov dword [ah_0x42_packet.lba_upper_dword], 0

    ; read sector
    mov si, ah_0x42_packet ; pointer to packet
    mov ah, 0x42 ; read LBA
    mov dl, byte [boot_device_number] ; drive number
    int 13h
    jc read_sector.error

    ; make sure that data segment was not destroyed
    mov ax, 0
    mov ds, ax

    popa
    ret

; input: ax = LBA number of sector, bp = offset where sector will be loaded
; output: nothing
read_sector:
    pusha

    ; test if we will use CHS method or LBA method
    test byte [boot_device_number], 0x80
    jnz .use_lba_read

    ; use CHS method, convert LBA to CHS
    ; calculate cylinder
    mov bx, word [sectors_per_cylinder]
    xor dx, dx
    div bx
    mov ch, al ; cylinder

    ; calculate head and sector
    mov ax, dx
    mov bx, word [sectors_per_head]
    xor dx, dx
    div bx
    mov dh, al ; head
    inc dl ; sectors are 1-based
    mov cl, dl ; sector

    ; read sector
    mov ax, 0
    mov es, ax
    mov ah, 0x2 ; read CHS
    mov al, 1 ; one sector
    ; ch, dh, cl were set above
    mov dl, byte [boot_device_number] ; drive number
    mov bx, bp ; memory offset
    int 13h
    jc .error
    cmp ah, 0
    jne .error

    popa
    ret

    ; read sector with LBA method
    .use_lba_read:
    ; set packet
    mov word [ah_0x42_packet.signature], 0x0010 ; set signature
    mov word [ah_0x42_packet.number_of_sectors], 1 ; we will read one sector
    mov word [ah_0x42_packet.offset], bp ; set physical memory pointer
    mov word [ah_0x42_packet.segment], 0x0000
    mov word [ah_0x42_packet.lba_lower_word], ax ; LBA value
    mov word [ah_0x42_packet.lba_upper_word], 0
    mov dword [ah_0x42_packet.lba_upper_dword], 0

    ; read sector
    mov si, ah_0x42_packet ; pointer to packet
    mov ah, 0x42 ; read LBA
    mov dl, byte [boot_device_number] ; drive number
    int 13h
    jc .error

    ; make sure that data segment was not destroyed
    mov ax, 0
    mov ds, ax

    popa
    ret

    ; there was error during reading sector
    .error:
    BOOTLOADER_PANIC jmp, 'can not read sector from disk'

sectors_per_cylinder dw 0
sectors_per_head dw 0

alignb 16 ; this packet needs to be aligned
ah_0x42_packet:
    .signature dw 0x0010
    .number_of_sectors dw 1
    .offset dw 0x0000
    .segment dw 0x0000
    .lba_lower_word dw 0
    .lba_upper_word dw 0
    .lba_upper_dword dd 0

loading_string db '% of BleskOS kernel is loaded...', 0

boot_device_number db 0

ramdisk_number_of_pages dw 0