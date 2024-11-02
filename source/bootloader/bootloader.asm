;BleskOS bootloader v04/05/2024

;;;;;
;; MIT License
;; Copyright (c) 2023-2024 Vendelín Slezák
;; Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
;; The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
;; THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
;;;;;

org 0x7C00

bits 16

start:
 jmp .code
 times 0x3E db 0 ;space for BPB, some BIOSes will overwrite this area
 .code:
 xor ax, ax
 mov ds, ax
 mov ss, ax
 mov es, ax
 mov sp, 0x7C00

 ;READ BOOTLOADER OF EXTENDED BOOTLOADER
 mov ah, 0x2 ;read
 mov al, 1 ;one sector
 mov ch, 0
 mov dh, 0
 mov cl, 2
 mov bx, 0x7E00 ;memory offset
 int 13h
 jnc bootloader_of_extended_bootloader
 int 13h ;try second time
 jnc bootloader_of_extended_bootloader
 int 13h ;try third time
 jnc bootloader_of_extended_bootloader
 
 ;error during loading second stage of bootloader, print error message independently from BIOS
 .error:
 mov ax, 0xB800
 mov es, ax
 mov si, 0
 mov cx, 80*25
 .clear_screen:
  mov word [es:si], 0x4020 ;clear screen with red background
  add si, 2
 loop .clear_screen

 mov byte [es:0], 'E'
 mov byte [es:2], 'r'
 mov byte [es:4], 'r'
 mov byte [es:6], 'o'
 mov byte [es:8], 'r'
 mov byte [es:12], '0'

 ;halt forever
 cli
 .halt:
  hlt
 jmp .halt
 
 ;MBR
 times 0x1B4-($-$$) db 0
 
 times 10 db 0 ;Disk ID
 db 0x80 ;bootable partition
 db 0, 2, 0 ;first sector in CHS (head/sector/cylinder)
 db 0x01 ;partition type FAT12
 db 1, 18, 80 ;last sector in CHS (head/sector/cylinder)
 dd 1 ;LBA of first sector
 dd 2880 ;sectors of this partition
 times 16 db 0 ;second entry
 times 16 db 0 ;thrid entry
 times 16 db 0 ;fourth entry
dw 0xAA55 ;bootable disk

bootloader_of_extended_bootloader:
 xor ax, ax
 mov ds, ax
 mov ss, ax
 mov es, ax
 mov sp, 0x7C00
 
 ;ENABLE A20
 in al, 0x92
 or al, 0x2
 out 0x92, al
 
 ;READ EXTENDED BOOTLOADER
 mov ah, 0x2 ;read
 mov al, 8 ;eigth sectors
 mov ch, 0
 mov dh, 0
 mov cl, 3 ;first sector is for MBR and second is bootloader of extended bootloader, extended bootloader itself starts from third sector
 mov bx, 0x8000 ;memory offset
 int 13h
 jnc extended_bootloader
 int 13h ;try second time
 jnc extended_bootloader
 int 13h ;try third time
 jnc extended_bootloader
 
 ;error during loading second stage of bootloader, print error message independently from BIOS
 .error:
 mov ax, 0xB800
 mov es, ax
 mov si, 0
 mov cx, 80*25
 .clear_screen:
  mov word [es:si], 0x4020 ;clear screen with red background
  add si, 2
 loop .clear_screen

 mov byte [es:0], 'E'
 mov byte [es:2], 'r'
 mov byte [es:4], 'r'
 mov byte [es:6], 'o'
 mov byte [es:8], 'r'
 mov byte [es:12], '0'

 ;halt forever
 cli
 .halt:
  hlt
 jmp .halt
 
 ;signature of BleskOS code partition
 times 1000-($-$$) db 0
 db 'BleskOS boot partition'
dw 0xAA55 ;bootable partition

extended_bootloader:
 mov byte [boot_drive], dl
extended_bootloader_redraw:
 ;clear screen
 mov ah, 0x2
 mov bh, 0
 mov dx, 0
 int 10h
 mov ah, 0x9
 mov al, ' '
 mov bh, 0
 mov bl, 0x20
 mov cx, 2000
 int 10h
 
 ;draw line
 mov ah, 0x2
 mov bh, 0
 mov dl, 1
 mov dh, byte [selected_entry]
 add dh, 3
 int 10h
 mov ah, 0x9
 mov al, ' '
 mov bh, 0
 mov bl, 0x40
 mov cx, 78
 int 10h
 
 ;print strings
 mov dx, 0x0101
 mov si, boot_up_str
 call print
 
 mov dx, 0x0301
 mov si, boot_bleskos_str
 call print
 
 mov dx, 0x0401
 mov si, boot_bleskos_boot_options_str
 call print
 
 mov dx, 0x1501
 mov si, boot_options_str
 call print
 
 mov dx, 0x1701
 mov si, boot_down_str
 call print
 
	; preserve original registers
	push	cx
	push	dx

	; get current uptime
	xor	ah,	ah
	int	0x1A

	; wait no more than 8 seconds
	add	dx,	146	; 18.2 * 8 = 145.6 ~= 146
	jnc	.no_overflow

	; register overflow
	inc	cx

.no_overflow:
	; store in local variable
	mov	word [.uptime],	cx
	mov	word [.uptime + 0x02],		dx

 .wait_for_key:
	; get current uptime
	xor	ah,	ah
	int	0x1A

	; choose selected option?
	cmp	dx,	word [.uptime + 0x02]
	jb	.no
	cmp	cx,	word [.uptime]
	jb	.no

	; restore original registers
	pop	dx
	pop	cx

	; choose selected option
	jmp	.enter

.no:
	; check if there is key to retrieve
	mov	ah,	0x01
	int	0x16

	; nothing on keyboard cache
	jz	.wait_for_key

  mov ah, 0
  int 16h
  
	; restore original registers
	pop	dx
	pop	cx

  cmp ah, 0x1C
  je .enter
  
  cmp ah, 0x48
  je .key_up
  
  cmp ah, 0x50
  je .key_down
  
  cmp al, 'a'
  je options
  
  cmp al, 'A'
  je options
 jmp .wait_for_key
 
.uptime:
	dw	0x0000
	dw	0x0000
 
 .key_up:
  cmp word [selected_entry], 0
  je .wait_for_key
  dec word [selected_entry]
 jmp extended_bootloader_redraw
 
 .key_down:
  cmp word [selected_entry], 1
  je .wait_for_key
  inc word [selected_entry]
 jmp extended_bootloader_redraw
 
 .enter:
  cmp word [selected_entry], 1
  je .special_boot
  mov byte [boot_options], 0 ;classic boot without any boot options
  .find_highest_graphic_mode_and_load_bleskos:
  call highest_graphic_mode
  jmp load_bleskos
  
 .special_boot:
  cmp byte [manual_selecting_of_graphic_mode], 0
  je .find_highest_graphic_mode_and_load_bleskos ;mode is not manually selected, it is highest possible mode, but we boot with boot options
  cmp word [selected_graphic_mode], 0
  jne .if_no_mode
   mov word [selected_graphic_mode], 0x118 ;default mode
  .if_no_mode:
  jmp load_bleskos
 
options:
 ;clear screen
 mov ah, 0x2
 mov bh, 0
 mov dx, 0
 int 10h
 mov ah, 0x9
 mov al, ' '
 mov bh, 0
 mov bl, 0x20
 mov cx, 2000
 int 10h
 
 ;print strings
 mov dx, 0x0101
 mov si, options_manual_selecting_of_graphic_mode_str
 call print
 mov dx, 0x0128
 mov si, options_on
 call print
 cmp byte [manual_selecting_of_graphic_mode], 1
 je .if_manual_selecting_of_graphic_mode_off
  mov dx, 0x0128
  mov si, options_off
  call print
 .if_manual_selecting_of_graphic_mode_off:

 mov dx, 0x0301
 mov si, options_graphic_mode_str
 call print

 mov dx, 0x0501
 mov si, options_debug_messages_str
 call print
 mov dx, 0x0528
 mov si, options_on
 call print
 mov al, byte [boot_options]
 test al, 0x1
 jnz .if_debug_messages_off
  mov dx, 0x0528
  mov si, options_off
  call print
 .if_debug_messages_off:
 
 .halt:
  mov ah, 0
  int 16h
  
  cmp ah, 0x1
  je extended_bootloader_redraw
  
  cmp al, 'g'
  je select_graphic_mode
  
  cmp al, 'G'
  je select_graphic_mode

  cmp al, 'm'
  je change_manual_selecting_of_graphic_mode
  
  cmp al, 'M'
  je change_manual_selecting_of_graphic_mode

  cmp al, 'd'
  je change_showing_of_debug_messages_during_boot
  
  cmp al, 'D'
  je change_showing_of_debug_messages_during_boot
 jmp .halt

change_manual_selecting_of_graphic_mode:
 not byte [manual_selecting_of_graphic_mode]
 and byte [manual_selecting_of_graphic_mode], 0x1
 jmp options

change_showing_of_debug_messages_during_boot:
 mov al, byte [boot_options]
 not al
 and byte [boot_options], 0xFE
 and al, 0x1
 or byte [boot_options], al
 jmp options
 
select_graphic_mode:
 mov ax, 0x0E00
 mov es, ax
 mov word [es:0x00], 0
 mov word [es:0x12], 0
 mov word [es:0x14], 0
 mov word [es:0x19], 0
 mov di, 0
 mov cx, word [graphic_mode]
 mov ax, 0x4F01
 int 10h ;graphic mode info
 
 ;clear screen
 mov ah, 0x2
 mov bh, 0
 mov dx, 0
 int 10h
 mov ah, 0x9
 mov al, ' '
 mov bh, 0
 mov bl, 0x20
 mov cx, 2000
 int 10h
 
 ;print mode value
 mov ah, 0x2
 mov dx, 0x0101
 int 10h
 
 mov ah, 0xE
 mov al, '0'
 int 10h
 mov al, 'x'
 int 10h
 mov al, '1'
 int 10h
 
 mov al, byte [graphic_mode]
 call print_hex
 
 ;select mode
 cmp byte [es:0x19], 16
 je .select_mode
 cmp byte [es:0x19], 24
 je .select_mode
 cmp byte [es:0x19], 32
 jne .skip_select_mode
 .select_mode:
  mov ax, word [graphic_mode]
  mov word [selected_graphic_mode], ax
 .skip_select_mode:
 
 ;print mode parameters
 mov ah, 0x2
 mov bh, 0
 mov dx, 0x0301
 int 10h
 mov al, byte [es:0x00]
 call print_hex ;mode attributes

 mov ah, 0x2
 mov bh, 0
 mov dx, 0x0501
 int 10h
 mov ax, word [es:0x12]
 call print_var ;width
 
 mov ah, 0x2
 mov dx, 0x0701
 int 10h
 mov ax, word [es:0x14]
 call print_var ;height
 
 mov ah, 0x2
 mov dx, 0x0901
 int 10h
 mov ax, 0
 mov al, byte [es:0x19]
 call print_var ;bpp

 .halt:
  mov ah, 0
  int 16h
  
  cmp ah, 0x1
  je options
  
  cmp ah, 0x48
  je .key_up
  
  cmp ah, 0x50
  je .key_down
 jmp .halt
 
 .key_up:
  cmp word [graphic_mode], 0x101
  jl .halt
  dec word [graphic_mode]
 jmp select_graphic_mode
 
 .key_down:
  cmp word [graphic_mode], 0x1FE
  jg .halt
  inc word [graphic_mode]
 jmp select_graphic_mode
 
highest_graphic_mode:
 ;get EDID info to read best resoultion of monitor
 mov ax, 0x0100
 mov es, ax
 mov di, 0
 mov al, 0
 mov cx, 0x1000
 rep stosb ;clear memory for EDID

 mov di, 0
 mov ax, 0x4F15
 mov bl, 0x1
 mov cx, 0
 mov dx, 0
 int 10h ;load EDID

 mov word [edid_best_x], 0
 cmp word [es:0], 0xFF00 ;check signature
 jne .if_edid_present
 cmp word [es:2], 0xFFFF ;check signature
 jne .if_edid_present
 cmp word [es:4], 0xFFFF ;check signature
 jne .if_edid_present
 cmp word [es:6], 0x00FF ;check signature
 jne .if_edid_present
  mov al, byte [es:54+2]
  mov ah, byte [es:54+4]
  shr ah, 4
  mov word [edid_best_x], ax ;read X monitor resoultion
 .if_edid_present:

 ;get array with numbers of VESA modes
 mov ax, 0x0100
 mov es, ax
 mov di, 0
 mov ax, 0x4F00
 int 10h
 cmp ax, 0x004F
 jne .done ;error - VBE is not supported on BIOS
 mov ax, word [es:16]
 mov gs, ax ;segment of array
 mov si, word [es:14] ;offset of array

 ;initalize variables
 mov word [vesa_last_24_mode_x], 0
 mov word [vesa_last_32_mode_x], 0

 ;here we will load info about VESA mode
 mov ax, 0x0F00
 mov es, ax
 mov di, 0
 .find_mode:
  ;clear values
  mov word [es:0], 0
  mov word [es:0x12], 0
  mov word [es:0x14], 0
  mov byte [es:0x19], 0

  ;load mode info
  mov cx, word [gs:si] ;read mode number from array
  cmp cx, 0xFFFF
  je .get_value_of_best_mode ;end of array
  cmp cx, 0x0000
  je .get_value_of_best_mode ;end of array
  mov ax, 0x4F01
  push gs
  push si
  int 10h
  pop si
  pop gs

  ;error
  cmp ax, 0x004F
  jne .next_mode

  ;test mode attributes
  mov bx, word [es:0x00]
  and bx, ((1<<7) | (1<<4) | (1<<3) | (1<<0)) ;linear frame buffer available/graphic mode/color mode/mode is supported on hardware
  cmp bx, ((1<<7) | (1<<4) | (1<<3) | (1<<0))
  jne .next_mode

  ;test if it is good mode for this monitor
  cmp word [edid_best_x], 0
  je .if_edid_best_x_present
   mov bx, word [es:0x12]
   mov dx, word [edid_best_x]
   cmp bx, dx
   ja .next_mode
  .if_edid_best_x_present:

  ;we support 16/24/32 bit color modes
  cmp byte [es:0x19], 16
  je .mode_16_bpp
  cmp byte [es:0x19], 24
  je .mode_24_bpp
  cmp byte [es:0x19], 32
  je .mode_32_bpp

  jmp .next_mode

  .mode_16_bpp:
  mov bx, word [vesa_last_16_mode_x]
  mov dx, word [es:0x12]
  cmp bx, dx
  jg .next_mode ;we already have bigger mode
  mov word [vesa_16_mode_number], cx
  mov bx, word [es:0x12]
  mov word [vesa_last_16_mode_x], bx
  jmp .next_mode

  .mode_24_bpp:
  mov bx, word [vesa_last_24_mode_x]
  mov dx, word [es:0x12]
  cmp bx, dx
  jg .next_mode ;we already have bigger mode
  mov word [vesa_24_mode_number], cx
  mov bx, word [es:0x12]
  mov word [vesa_last_24_mode_x], bx
  jmp .next_mode

  .mode_32_bpp:
  mov bx, word [vesa_last_32_mode_x]
  mov dx, word [es:0x12]
  cmp bx, dx
  jg .next_mode ;we already have bigger mode
  mov word [vesa_32_mode_number], cx
  mov bx, word [es:0x12]
  mov word [vesa_last_32_mode_x], bx

 .next_mode:
 add si, 2
 jmp .find_mode

 .get_value_of_best_mode:
 mov ax, word [vesa_32_mode_number]
 cmp word [vesa_32_mode_number], 0
 jne .select_graphic_mode

 mov ax, word [vesa_24_mode_number]
 cmp word [vesa_24_mode_number], 0
 jne .select_graphic_mode

 mov ax, word [vesa_16_mode_number]
 cmp word [vesa_16_mode_number], 0
 jne .select_graphic_mode

 mov ax, 0 ;no usable graphic mode founded

 .select_graphic_mode:
 mov word [selected_graphic_mode], ax
 
 .done:
 ret
 
error_background:
 mov ah, 0x2
 mov bh, 0
 mov dx, 0x0
 int 10h
 
 mov ah, 0x9
 mov al, ' '
 mov bl, 0x40
 mov cx, 2000
 int 10h
 
 mov dx, 0x0101 ;there will be printed error message
 
 ret
 
error_memory:
 call error_background
 mov si, boot_error_memory
 call print
 jmp error_halt
 
error_loading:
 call error_background
 mov si, boot_error_loading
 call print
 jmp error_halt
 
error_graphic_info:
 call error_background
 mov si, boot_error_graphic_info
 call print
 jmp error_halt
 
error_graphic_mode:
 call error_background
 mov si, boot_error_graphic_mode
 call print
 jmp error_halt
 
error_0x100000_not_available:
 call error_background
 mov si, boot_error_0x100000_not_available
 call print
 
error_halt:
 hlt
 jmp error_halt
 
print:
 ;move cursor
 mov ah, 0x2
 ;dx is already set
 int 10h

 .after_cursor_is_moved:
 mov al, byte [si]
 cmp al, 0
 je .done
 
 mov ah, 0xE
 mov bh, 0
 int 10h
 inc si
 jmp .after_cursor_is_moved
 
 .done:
 ret
 
print_var:
 push ax
 
 mov bx, 10
 mov dx, 0
 div bx
 add dl, '0'
 mov byte [print_var_str+3], dl
 mov dx, 0
 div bx
 add dl, '0'
 mov byte [print_var_str+2], dl
 mov dx, 0
 div bx
 add dl, '0'
 mov byte [print_var_str+1], dl
 mov dx, 0
 div bx
 add dl, '0'
 mov byte [print_var_str+0], dl
 
 pop ax
 mov si, print_var_str+3
 cmp ax, 10
 jl .print
 mov si, print_var_str+2
 cmp ax, 100
 jl .print
 mov si, print_var_str+1
 cmp ax, 1000
 jl .print
 mov si, print_var_str
 
 .print:
 call print.after_cursor_is_moved
 
 ret

print_hex:
 mov bl, al
 and bl, 0xF
 add bl, '0'
 cmp bl, '9'+1
 jl .if_first_char_alphabet
  add bl, 7
 .if_first_char_alphabet:

 mov bh, al
 shr bh, 4
 add bh, '0'
 cmp bh, '9'+1
 jl .if_second_char_alphabet
  add bh, 7
 .if_second_char_alphabet:

 mov ah, 0xE
 mov al, bh
 int 10h
 mov al, bl
 int 10h

 ret

 boot_up_str db 'Please choose system you want to boot:', 0
 boot_bleskos_str db 'BleskOS', 0
 boot_bleskos_boot_options_str db 'BleskOS special boot options', 0
 boot_options_str db '[a] Boot options', 0
 boot_down_str db 'BleskOS live bootloader', 0
 boot_loading_str db 'Loading BleskOS', 0
 
 boot_error_memory db 'Size of RAM memory can not be readed', 0
 boot_error_loading db 'Error during loading BleskOS', 0
 boot_error_graphic_info db 'Informations about graphic mode can not be readed', 0
 boot_error_graphic_mode db 'Error during setting graphic mode', 0
 boot_error_0x100000_not_available db 'Memory at 0x100000 is not free', 0

 options_manual_selecting_of_graphic_mode_str db '[m] Manual selecting of graphic mode', 0
 options_graphic_mode_str db '[g] Select graphic mode', 0
 options_debug_messages_str db '[d] Debug messages during boot', 0
 options_on db 'On', 0
 options_off db 'Off', 0
 
 print_var_str db 0, 0, 0, 0, 0

 boot_drive db 0
 selected_entry dw 0
 selected_graphic_mode dw 0x118
 graphic_mode dw 0x118
 vesa_mode_number dw 0
 vesa_16_mode_number dw 0
 vesa_24_mode_number dw 0
 vesa_32_mode_number dw 0
 vesa_last_16_mode_x dw 0
 vesa_last_24_mode_x dw 0
 vesa_last_32_mode_x dw 0
 edid_best_x dw 0
 manual_selecting_of_graphic_mode db 1
 boot_options dd 0

;edi = destination, ecx = how many bytes
copy_loaded_data:
 mov esi, 0x10000

 .copy_byte:
 mov al, byte [esi]
 mov byte [edi], al
 inc esi
 inc edi
 loop .copy_byte

 ret

load_bleskos:
 ;print message
 mov ah, 0x2
 mov bh, 0
 mov dx, 0x0
 int 10h
 
 mov ah, 0x9
 mov al, ' '
 mov bl, 0x20
 mov cx, 2000
 int 10h
 
 mov dx, 0x0101
 mov si, boot_loading_str
 call print

 ;clear memory where we will load informations for BleskOS
 mov ax, 0
 mov es, ax
 mov di, 0x1000
 mov al, 0
 mov cx, 0x3000
 rep stosb

 ;get memory map
 mov ax, 0x0100
 mov es, ax
 mov di, 0
 
 mov ebx, 0
 mov cx, 50
 .get_memory_entry: 
 push cx
  mov eax, 0xE820
  mov ecx, 24
  mov edx, 0x534D4150 ;signature
  int 15h
  
  jc error_memory
 
  add di, 24
 pop cx
 cmp ebx, 0
 je .last_entry
 loop .get_memory_entry
 .last_entry:

 ;found if there is enough free memory at 0x100000
 mov di, 0
 mov cx, 50
 .test_one_memory_block:
  ;free entry
  cmp dword [es:di+16], 1
  jne .test_one_memory_block_next_loop

  ;free entry at 0x100000
  cmp dword [es:di+4], 0
  jne .test_one_memory_block_next_loop
  cmp dword [es:di+0], 0x100000
  jne .test_one_memory_block_next_loop

  ;length more than 1 MB
  cmp dword [es:di+12], 0
  ja .0x100000_is_available
  cmp dword [es:di+8], 0x100000
  ja .0x100000_is_available
 .test_one_memory_block_next_loop:
 add di, 24
 loop .test_one_memory_block
 jmp error_0x100000_not_available

 ;1 MB of memory is for BleskOS code
 .0x100000_is_available:
 add dword [es:di+0], 0x100000 ;start of memory entry
 sub dword [es:di+8], 0x100000 ;length of memory entry - if it is bigger than 4GB this may cause ovewflow, but we do not need to worry about it, because BleskOS will read this as 4 GB long entry because of higher bits, not caring about these lesser bits
 
 ;get EDID
 mov ax, 0x0200
 mov es, ax
 mov di, 0
 mov dword [di], 0
 mov ax, 0x4F15
 mov bl, 0x1
 mov cx, 0
 mov dx, 0
 int 10h

 ;jump to unreal mode to gain access to higher memory
 cli
 push ds
 lgdt [unreal_mode_gdt_wrap]

 mov  eax, cr0
 or eax, 1
 mov  cr0, eax
 jmp 0x0008:protected_mode

 protected_mode:
 mov bx, 0x0010
 mov ds, bx

 and al, 0xFE
 mov  cr0, eax
 jmp 0x0000:unreal_mode

 unreal_mode:
 pop ds
 sti
 
 ;load bleskos
 mov edi, 0x100000 ;we will copy loaded data here
 cmp byte [boot_drive], 0x80
 jae .hard_disk_boot

 ;load first 8 sectors to align reading from floppy
 mov ch, 0 ;cylinder
 mov dh, 0 ;head
 mov cl, 11 ;sector
 mov dl, byte [boot_drive]
 mov bx, 0x0000
 mov ax, 0x1000
 mov es, ax
 mov ah, 0x2 ;read
 mov al, 8 ;8 sectors
 pusha
 int 13h
 popa
 jc error_loading

 ;copy loaded data
 mov ecx, 8*512
 call copy_loaded_data
 
 ;set CHS for next reading
 mov ch, 0 ;cylinder
 mov dh, 1 ;head
 mov cl, 1 ;sector
 
 .floppy_load_cylinder:
  ;print . to show that loading is progressing
  pusha
  mov al, '.'
  mov ah, 0xE
  int 10h
  popa

  cmp ch, 50 ;load 50 sectors = 900 KB
  ja .select_graphic_mode

  ;we need to read heads separately, because reading more might cause issues on some hardware (e.g. Virtualbox)
  
  ;load head 1
  mov bx, 0x0000
  mov ax, 0x1000
  mov es, ax
  mov ah, 0x2 ;read
  mov al, 18 ;18 sectors
  mov dh, 1 ;head 1
  pusha ;some hardware change values in registers during int 13h so we have to save everything
  int 13h
  popa
  jc error_loading
  
  ;copy loaded data
  push ecx
  mov ecx, 18*512
  call copy_loaded_data
  pop ecx

  ;next cylinder
  inc ch

  ;load head 0
  mov bx, 0x0000
  mov ax, 0x1000
  mov es, ax
  mov ah, 0x2 ;read
  mov al, 18 ;18 sectors
  mov dh, 0 ;head 0
  pusha ;some hardware change values in registers during int 13h so we have to save everything
  int 13h
  popa
  jc error_loading
  
  ;copy loaded data
  push ecx
  mov ecx, 18*512
  call copy_loaded_data
  pop ecx
 jmp .floppy_load_cylinder
 
 .hard_disk_boot:
 mov dword [0xF008], 10 ;first sector of BleskOS code
 mov edx, 10 ;we will store sector number here
 mov cx, 30 ;load 30x64 sectors = 960 KB
 .load_bleskos_from_hard_disk:
  ;create descriptor of transfer
  mov word [0xF000], 0x0010 ;signature
  mov word [0xF002], 64
  mov word [0xF004], 0x0000 ;memory offset
  mov word [0xF006], 0x1000 ;segment
  mov dword [0xF00C], 0

  ;read
  pusha ;some hardware change values in registers during int 13h so we have to save everything
  mov ah, 0x42
  mov si, 0xF000
  mov dl, byte [boot_drive]
  int 13h
  popa
  jc error_loading
  
  ;sector for next transfer
  add edx, 64
  mov dword [0xF008], edx

  ;copy loaded data
  push ecx
  mov ecx, 64*512
  call copy_loaded_data
  pop ecx

  ;print . to show that loading is progressing
  pusha
  mov al, '.'
  mov ah, 0xE
  int 10h
  popa
 loop .load_bleskos_from_hard_disk
 
 ;VESA
 .select_graphic_mode:
 cmp word [selected_graphic_mode], 0
 je error_graphic_mode

 mov ax, 0x0300
 mov es, ax
 mov di, 0
 mov ax, 0x4F00
 int 10h ;load VESA information block
 cmp ax, 0x004F
 jne error_graphic_info ;error during loading VESA information block

 mov ax, 0x0380
 mov es, ax
 mov di, 0
 mov cx, word [selected_graphic_mode]
 mov ax, 0x4F01
 int 10h ;load graphic mode info
 cmp ax, 0x004F
 jne error_graphic_info ;error during loading graphic mode informations
 
 mov ax, 0x4F02
 mov bx, word [selected_graphic_mode]
 or bx, 0x4000 ;use linear frame buffer
 int 10h ;set graphic mode
 cmp ax, 0x004F
 jne error_graphic_mode ;error during setting graphic mode
 
 ;enter to protected mode
 cli
 lgdt [gdt_wrap]
 mov eax, cr0
 or eax, 1
 mov cr0, eax
 jmp 0x0008:clear_pipe
 bits 32
 clear_pipe:
 mov ax, 0x0010
 mov ds, ax
 mov es, ax
 mov fs, ax
 mov gs, ax
 mov ss, ax

 mov esp, 0x80000 ;set stack pointer
 mov eax, dword [boot_options]
 push eax ;push value of boot option to stack for bleskos() method
 jmp 0x100000 ;execute BleskOS

 unreal_mode_gdt:
  dq 0 ;first item is null
  ;code
  dw 0xFFFF, 0
  db 0, 10011010b, 00000000b, 0
  ;data
  dw 0xFFFF, 0
  db 0, 10010010b, 11001111b, 0
 unreal_mode_gdt_end:

 unreal_mode_gdt_wrap:
   dw unreal_mode_gdt_end - unreal_mode_gdt - 1
   dd unreal_mode_gdt

 gdt:
  dq 0 ;first item is null
  ;code
  dw 0xFFFF, 0
  db 0, 10011010b, 11001111b, 0
  ;data
  dw 0xFFFF, 0
  db 0, 10010010b, 11001111b, 0
 gdt_end:

 gdt_wrap:
  dw gdt_end - gdt - 1
  dd gdt
