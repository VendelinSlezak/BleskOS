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

%define STANDARDIZED_GRAPHIC_OUTPUT_TYPE_TEXT_MODE 1
%define STANDARDIZED_GRAPHIC_OUTPUT_TYPE_GRAPHIC_MODE 2
%define VESA_SIGNATURE 0x41534556

; BOOTLOADER FUNCTION
find_and_set_best_graphic_mode:
    ; FIND BEST GRAPHIC MODE THAT BOOTLOADER CAN OFFER, SET IT AND DESCRIBE IT IN STANDARDIZED FORM
    ; dw type (0 = text mode, 1 = graphic mode)
    ; dw width (number of columns / pixels)
    ; dw height (number of rows / pixels)
    ; dw bits_per_pixel
    ; dw linear_frame_buffer
    ; dw bytes_per_line

    ; clear standardized graphic info table
    mov di, STANDARDIZED_GRAPHIC_OUTPUT_INFO_START
    call clear_page

    ; check if BGA is present
    mov dx, 0x01CE
    mov ax, 0x0000
    out dx, ax
    mov dx, 0x01CF
    in ax, dx
    
    cmp ax, 0xB0C2
    jb .bga_is_not_present
    cmp ax, 0xB0C5
    ja .bga_is_not_present
    jmp .vga_mode ; BGA is present, BleskOS kernel will set graphic mode

    .bga_is_not_present:

    ; load VBE info
    mov dword [vbe_info+vbe_info_block.signature], 'VBE3' ; we accept VBE3 structure
    mov di, vbe_info
    mov ax, 0x4F00
    int 10h
    cmp ax, 0x004F
    jne .vga_mode ; error - VBE is not supported by BIOS
    cmp dword [vbe_info+vbe_info_block.signature], VESA_SIGNATURE
    jne .vga_mode ; error - wrong signature

    ; check all available video modes
    mov ax, word [vbe_info+vbe_info_block.video_mode_list_segment]
    mov gs, ax
    mov si, word [vbe_info+vbe_info_block.video_mode_list_offset]
    .check_vbe_mode:
        ; check if this is last mode in list
        cmp word [gs:si], 0xFFFF ; official ending
        je .all_vbe_modes_checked

        ; read mode info
        pusha
        mov di, vbe_mode_info
        mov cx, word [gs:si]
        mov ax, 0x4F01
        int 10h
        mov word [vbe_return_code], ax
        popa

        ; check response
        cmp word [vbe_return_code], 0x004F
        jne .check_next_vbe_mode ; error during obtaining informations about mode
        mov al, byte [vbe_mode_info+vbe_mode_info_block.mode_attributes]
        and al, ((1 << 7) | (1 << 4) | (1 << 3) | (1 << 0)) ; parse bits: linear frame buffer available / graphic mode / color mode / mode is supported on hardware
        cmp al, ((1 << 7) | (1 << 4) | (1 << 3) | (1 << 0))
        jne .check_next_vbe_mode ; this mode is not good for us

        mov al, byte [STANDARDIZED_GRAPHIC_OUTPUT_INFO_START+standardized_graphic_output.bpp]
        cmp al, byte [vbe_mode_info+vbe_mode_info_block.bits_per_pixel]
        ja .check_next_vbe_mode ; we already have mode with better BPP
        mov ax, word [STANDARDIZED_GRAPHIC_OUTPUT_INFO_START+standardized_graphic_output.width]
        cmp ax, word [vbe_mode_info+vbe_mode_info_block.screen_width]
        ja .check_next_vbe_mode ; we already have mode with better screen width
        mov ax, word [STANDARDIZED_GRAPHIC_OUTPUT_INFO_START+standardized_graphic_output.height]
        cmp ax, word [vbe_mode_info+vbe_mode_info_block.screen_height]
        ja .check_next_vbe_mode ; we already have mode with better screen height

        ; TODO: check pixel color mask

        ; save this mode
        mov ax, word [gs:si]
        mov word [best_vbe_mode_number], ax
        mov dword [STANDARDIZED_GRAPHIC_OUTPUT_INFO_START+standardized_graphic_output.type], STANDARDIZED_GRAPHIC_OUTPUT_TYPE_GRAPHIC_MODE
        mov ax, word [vbe_mode_info+vbe_mode_info_block.screen_width]
        mov word [STANDARDIZED_GRAPHIC_OUTPUT_INFO_START+standardized_graphic_output.width], ax
        mov ax, word [vbe_mode_info+vbe_mode_info_block.screen_height]
        mov word [STANDARDIZED_GRAPHIC_OUTPUT_INFO_START+standardized_graphic_output.height], ax
        mov al, byte [vbe_mode_info+vbe_mode_info_block.bits_per_pixel]
        mov byte [STANDARDIZED_GRAPHIC_OUTPUT_INFO_START+standardized_graphic_output.bpp], al
        mov eax, dword [vbe_mode_info+vbe_mode_info_block.phys_base_ptr]
        mov dword [STANDARDIZED_GRAPHIC_OUTPUT_INFO_START+standardized_graphic_output.linear_frame_buffer], eax
        mov ax, word [vbe_mode_info+vbe_mode_info_block.bytes_per_line] ; bytes per line in VBE1 and VBE2
        cmp byte [vbe_info+vbe_info_block.major_version], 3
        jne .if_vbe3
            mov ax, word [vbe_mode_info+vbe_mode_info_block.bytes_per_line_vbe3] ; bytes per line in VBE3
        .if_vbe3:
        mov word [STANDARDIZED_GRAPHIC_OUTPUT_INFO_START+standardized_graphic_output.bytes_per_line], ax
    .check_next_vbe_mode:
    add si, 2
    cmp si, 0x0000 ; zero value would mean that si rolled over whole segment, and because there can not be more data about mode numbers than in one segment, we surely checked all modes
    jne .check_vbe_mode

    .all_vbe_modes_checked:
    cmp dword [STANDARDIZED_GRAPHIC_OUTPUT_INFO_START+0], STANDARDIZED_GRAPHIC_OUTPUT_TYPE_GRAPHIC_MODE
    jne .vga_mode ; VBE do not have any suitable graphic mode

    ; set VBE graphic mode
    mov ax, 0x4F02
    mov bx, word [best_vbe_mode_number]
    or bx, 0x4000 ; use linear frame buffer
    int 10h
    cmp ax, 0x004F
    jne .vga_mode ; there was error during setting graphic mode, we are still in vga mode
    jmp .graphic_mode_end

    ; graphic mode was not set, so save info about actual text mode 80x25
    .vga_mode:
    mov dword [STANDARDIZED_GRAPHIC_OUTPUT_INFO_START+standardized_graphic_output.type], STANDARDIZED_GRAPHIC_OUTPUT_TYPE_TEXT_MODE
    mov dword [STANDARDIZED_GRAPHIC_OUTPUT_INFO_START+standardized_graphic_output.width], 80
    mov dword [STANDARDIZED_GRAPHIC_OUTPUT_INFO_START+standardized_graphic_output.height], 25
    mov dword [STANDARDIZED_GRAPHIC_OUTPUT_INFO_START+standardized_graphic_output.bpp], 4
    mov dword [STANDARDIZED_GRAPHIC_OUTPUT_INFO_START+standardized_graphic_output.bytes_per_line], 80*2
    mov dword [STANDARDIZED_GRAPHIC_OUTPUT_INFO_START+standardized_graphic_output.linear_frame_buffer], 0xB8000

    .graphic_mode_end:

    ret

best_vbe_mode_number dw 0
vbe_return_code dw 0
vbe_info times 512 db 0
vbe_mode_info times 256 db 0

; STRUCTURES
struc standardized_graphic_output
    .type                   resd 1
    .width                  resd 1
    .height                 resd 1
    .bpp                    resd 1
    .bytes_per_line         resd 1
    .linear_frame_buffer    resd 1
endstruc

struc vbe_info_block
    .signature                      resd 1
    .minor_version                  resb 1
    .major_version                  resb 1
    .oem_string_offset              resw 1
    .oem_string_segment             resw 1
    .capabilities                   resd 1
    .video_mode_list_offset         resw 1
    .video_mode_list_segment        resw 1
    .total_memory_in_64kb_blocks    resw 1
    .software_revision              resw 1
    .vendor_name_offset             resw 1
    .vendor_name_segment            resw 1
    .product_name_offset            resw 1
    .product_name_segment           resw 1
    .product_revision_offset        resw 1
    .product_revision_segment       resw 1
    .reserved                       resb 222
    .oem_data_area                  resb 256
endstruc

struc vbe_mode_info_block
    .mode_attributes                        resw 1
    .window_a_attributes                    resb 1
    .window_b_attributes                    resb 1
    .window_granularity                     resw 1
    .window_size                            resw 1
    .window_a_segment                       resw 1
    .window_b_segment                       resw 1
    .window_function_offset                 resw 1
    .window_function_segment                resw 1
    .bytes_per_line                         resw 1

    .screen_width                           resw 1
    .screen_height                          resw 1
    .char_cell_width                        resb 1
    .char_cell_height                       resb 1
    .number_of_planes                       resb 1
    .bits_per_pixel                         resb 1
    .number_of_banks                        resb 1
    .memory_model_type                      resb 1
    .bank_size_kb                           resb 1
    .number_of_images                       resb 1
    .reserved1                              resb 1

    .red_mask_size                          resb 1
    .red_mask_position                      resb 1
    .green_mask_size                        resb 1
    .green_mask_position                    resb 1
    .blue_mask_size                         resb 1
    .blue_mask_position                     resb 1
    .reserved_mask_size                     resb 1
    .reserved_mask_position                 resb 1
    .direct_color_attributes                resb 1

    .phys_base_ptr                          resd 1
    .reserved2                              resd 1
    .reserved3                              resw 1

    .bytes_per_line_vbe3                    resw 1
    .images_banked                          resb 1
    .images_linear                          resb 1
    .linear_red_mask_size                   resb 1
    .linear_red_mask_position               resb 1
    .linear_green_mask_size                 resb 1
    .linear_green_mask_position             resb 1
    .linear_blue_mask_size                  resb 1
    .linear_blue_mask_position              resb 1
    .linear_reserved_mask_size              resb 1
    .linear_reserved_mask_position          resb 1
    .max_pixel_clock                        resd 1

    .reserved4                              resb 189
endstruc