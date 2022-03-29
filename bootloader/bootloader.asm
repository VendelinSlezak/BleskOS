;BleskOS live bootloader v29/03/2022

org 0x7C00

bits 16

start:
 jmp .code
 times 0x3E db 0
 .code:
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
 mov al, 4 ;four sectors
 mov ch, 0
 mov dh, 0
 mov cl, 2
 mov bx, 0x7E00 ;memory offset
 int 13h
 jnc extended_bootloader
 int 13h
 jnc extended_bootloader
 int 13h
 jnc extended_bootloader
 
 .error:
 mov ah, 0x02
 mov bh, 0
 mov dx, 0x0
 int 10h ;move cursor
 mov ah, 0xE
 mov al, 'e'
 int 10h
 mov al, 'r'
 int 10h
 mov al, 'r'
 int 10h
 
 cli
 .halt:
  hlt
 jmp .halt
 
times 510-($-$$) db 0
dw 0xAA55

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
 mov ah, 0x2
 mov dx, 0x0101
 int 10h
 mov si, boot_up_str
 call print
 
 mov ah, 0x2
 mov dx, 0x0301
 int 10h
 mov si, boot_bleskos_str
 call print
 
 mov ah, 0x2
 mov dx, 0x0401
 int 10h
 mov si, boot_bleskos_boot_options_str
 call print
 
 mov ah, 0x2
 mov dx, 0x1501
 int 10h
 mov si, boot_options_str
 call print
 
 mov ah, 0x2
 mov dx, 0x1701
 int 10h
 mov si, boot_down_str
 call print
 
 .halt:
  mov ah, 0
  int 16h
  
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
 jmp .halt
 
 .key_up:
  cmp word [selected_entry], 0
  je .halt
  dec word [selected_entry]
 jmp extended_bootloader_redraw
 
 .key_down:
  cmp word [selected_entry], 1
  je .halt
  inc word [selected_entry]
 jmp extended_bootloader_redraw
 
 .enter:
  cmp word [selected_entry], 1
  je .special_boot
 .start_bleskos:
  call highest_graphic_mode
  jmp load_bleskos
  
 .special_boot:
  cmp word [selected_graphic_mode], 0
  jne .if_no_mode
   mov word [selected_graphic_mode], 0x118
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
 mov ah, 0x2
 mov dx, 0x0101
 int 10h
 mov si, options_graphic_mode_str
 call print
 
 .halt:
  mov ah, 0
  int 16h
  
  cmp ah, 0x1
  je extended_bootloader_redraw
  
  cmp al, 'g'
  je select_graphic_mode
  
  cmp al, 'G'
  je select_graphic_mode
 jmp .halt
 
select_graphic_mode:
 mov ax, 0x7000
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
 
 mov bl, byte [graphic_mode]
 and bl, 0xF
 mov bh, byte [graphic_mode]
 shr bh, 4
 and bh, 0xF
 
 add bh, '0'
 cmp bh, '9'+1
 jl .if_first_char
  add bh, 7
 .if_first_char:
 
 add bl, '0'
 cmp bl, '9'+1
 jl .if_second_char
  add bl, 7
 .if_second_char:
 
 mov al, bh
 int 10h
 mov al, bl
 int 10h
 
 ;select mode
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
 mov ax, word [es:0x12]
 call print_var
 
 mov ah, 0x2
 mov dx, 0x0501
 int 10h
 mov ax, word [es:0x14]
 call print_var
 
 mov ah, 0x2
 mov dx, 0x0701
 int 10h
 mov ax, 0
 mov al, byte [es:0x19]
 call print_var

 .halt:
  mov ah, 0
  int 16h
  
  cmp ah, 0x1
  je options
  
  cmp ah, 0x49
  je .key_up
  
  cmp ah, 0x51
  je .key_down
 jmp .halt
 
 .key_up:
  cmp word [graphic_mode], 0x101
  jl .halt
  dec word [graphic_mode]
 jmp select_graphic_mode
 
 .key_down:
  cmp word [graphic_mode], 0x190
  jg .halt
  inc word [graphic_mode]
 jmp select_graphic_mode
 
highest_graphic_mode:
 mov word [vesa_last_24_mode_x], 0
 mov word [vesa_last_32_mode_x], 0
 mov ax, 0x7000
 mov es, ax
 mov di, 0
 mov cx, 0x100
 .find_mode:
  mov word [es:0], 0
  mov word [es:0x12], 0
  mov word [es:0x14], 0
  mov byte [es:0x19], 0
  mov ax, 0x4F01 ;mode info
  int 10h

  mov bx, word [es:0x00]
  and bx, 0x91
  cmp bx, 0x91
  jne .next_mode
  cmp byte [es:0x19], 24
  je .mode_24_bpp
  cmp byte [es:0x19], 32
  je .mode_32_bpp
  jmp .next_mode

  .mode_24_bpp:
  mov bx, word [vesa_last_24_mode_x]
  mov dx, word [es:0x12]
  cmp bx, dx
  jg .next_mode
  mov word [vesa_24_mode_number], cx
  mov bx, word [es:0x12]
  mov word [vesa_last_24_mode_x], bx
  jmp .next_mode

  .mode_32_bpp:
  mov bx, word [vesa_last_32_mode_x]
  mov dx, word [es:0x12]
  cmp bx, dx
  jg .next_mode
  mov word [vesa_32_mode_number], cx
  mov bx, word [es:0x12]
  mov word [vesa_last_32_mode_x], bx

 .next_mode:
 inc cx
 cmp cx, 0x150
 jne .find_mode

 mov ax, word [vesa_32_mode_number]
 mov word [selected_graphic_mode], ax
 cmp word [vesa_32_mode_number], 0
 jne .done

 mov ax, word [vesa_24_mode_number]
 mov word [selected_graphic_mode], ax
 cmp word [vesa_24_mode_number], 0
 jne .done
 
 mov word [selected_graphic_mode], 0
 
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
 
 mov ah, 0x2
 mov dx, 0x0101
 int 10h
 
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
 
error_floppy_boot:
 call error_background
 mov si, boot_error_floppy_boot
 call print
 jmp error_halt
 
error_halt:
 hlt
 jmp error_halt
 
print:
 mov al, byte [si]
 cmp al, 0
 je .done
 
 mov ah, 0xE
 mov bh, 0
 int 10h
 inc si
 jmp print
 
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
 call print
 
 ret

 boot_up_str db 'Please choose system you want to boot:', 0
 boot_bleskos_str db 'BleskOS', 0
 boot_bleskos_boot_options_str db 'BleskOS special boot options', 0
 boot_options_str db '[a] Boot options', 0
 boot_down_str db 'BleskOS live bootloader v1.0', 0
 
 boot_error_memory db 'Size of RAM memory can not be readed', 0
 boot_error_loading db 'Error during loading BleskOS', 0
 boot_error_graphic_info db 'Informations about graphic mode can not be readed', 0
 boot_error_graphic_mode db 'Error during setting graphic mode', 0
 boot_error_floppy_boot db 'Floppy can not be readed by this version of bootloader', 0

 options_graphic_mode_str db '[g] Select graphic mode', 0
 
 print_var_str db 0, 0, 0, 0, 0

 boot_drive db 0
 selected_entry dw 0
 selected_graphic_mode dw 0x118
 graphic_mode dw 0x118
 vesa_mode_number dw 0
 vesa_24_mode_number dw 0
 vesa_32_mode_number dw 0
 vesa_last_24_mode_x dw 0
 vesa_last_32_mode_x dw 0

load_bleskos:
 ;get memory map
 mov ax, 0x8000
 mov es, ax
 mov di, 0
 
 mov ebx, 0
 mov cx, 20
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
 
 ;get EDID
 mov ax, 0x7800
 mov es, ax
 mov di, 0
 mov dword [di], 0
 mov ax, 0x4F15
 mov bl, 0x1
 mov cx, 0
 mov dx, 0
 int 10h
 
 ;load bleskos
 cmp byte [boot_drive], 0
 je error_floppy_boot
 cmp byte [boot_drive], 1
 je error_floppy_boot
 
 mov dword [0xF008], 5
 mov edx, 5
 mov cx, 8 ;load 8x64 sectors = 256 KB
 mov ax, 0x10
 .load_bleskos_from_hard_disk:
  mov word [0xF000], 0x0010 ;singature
  mov word [0xF002], 64
  mov bx, ax
  shl bx, 12
  mov word [0xF004], bx ;memory offset
  mov bx, ax
  shl bx, 8
  and bx, 0xF000
  mov word [0xF006], bx ;segment
  mov dword [0xF00C], 0
  pusha ;some hardware change values in registers during int 13h so we have to save everything
  mov ah, 0x42
  mov si, 0xF000
  mov dl, byte [boot_drive]
  int 13h
  popa
  jc error_loading
  
  add edx, 64
  mov dword [0xF008], edx
  add ax, 0x8
 loop .load_bleskos_from_hard_disk
 jmp .select_graphic_mode
 
 ;select vesa graphic mode
 .select_graphic_mode:
 cmp word [selected_graphic_mode], 0
 je error_graphic_mode

 mov ax, 0x7000
 mov es, ax
 mov di, 0
 mov cx, word [selected_graphic_mode]
 mov ax, 0x4F01
 int 10h ;graphic mode info
 
 cmp ax, 0x004F
 jne error_graphic_info
 
 mov ax, 0x4F02
 mov bx, word [selected_graphic_mode]
 or bx, 0x4000
 int 10h ;set graphic mode
 
 cmp ax, 0x004F
 jne error_graphic_mode
 
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

 mov esp, 0x00E00000 ;set stack pointer
 jmp 0x10000 ;execute BleskOS

 gdt:
 dq 0 ;first item is null

 gdt_code:
  dw 0xFFFF, 0
  db 0, 10011010b, 11001111b, 0

 gdt_data:
  dw 0xFFFF, 0
  db 0, 10010010b, 11001111b, 0
 gdt_end:

 gdt_wrap:
  dw gdt_end - gdt - 1
  dd gdt
