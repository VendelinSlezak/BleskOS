;BleskOS bootloader

;What this code do:
; - enable A20 for acess to all RAM memory
; - set highest VESA mode
; - read 256 KB from boot device after bootloader sector - this contains BleskOS code
; - load Global Descriptor Table for acces max 4 GB of RAM memory
; - enter to protected mode(32 bit processor mode)
; - start executing of BleskOS code, whose is loaded in memory 0x10000

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

 ;READ BLESKOS 256 KB FROM HARD DISK
 mov dword [es:0xF008], 1
 mov dword [es:0xF00C], 0
 mov bx, 0x0100
 mov cx, 4
 .read_from_disk:
  mov word [es:0xF000], 0x0010 ;singature
  mov word [es:0xF002], 64 ;number of sectors
  mov ax, 0
  mov al, bl
  shl ax, 8
  mov word [es:0xF004], ax ;offset
  mov ax, 0
  mov al, bh
  shl ax, 12
  mov word [es:0xF006], ax ;segment
  
  mov ah, 0x42
  mov si, 0xF000
  int 13h
  jc .error_1
  
  add bx, 0x80
  add word [es:0xF008], 64 ;start sector
 loop .read_from_disk

 ;FIND VESA MODE
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
 mov word [vesa_mode_number], ax
 cmp word [vesa_32_mode_number], 0
 jne .set_vesa_mode

 mov ax, word [vesa_24_mode_number]
 mov word [vesa_mode_number], ax
 cmp word [vesa_24_mode_number], 0
 jne .set_vesa_mode

 jmp .error_2

 ;SET VESA MODE
 .set_vesa_mode:
 mov ax, 0x4F01
 mov cx, word [vesa_mode_number]
 mov di, 0
 int 10h

 mov ax, 0x4F02
 mov bx, word [vesa_mode_number]
 or bx, 0x4000
 int 0x10
 jmp .enter_protected_mode

 ;ERRORS
 .error_1:
  mov al, '1'
  int 10h
  jmp .halt

 .error_2:
  mov al, '2'
  int 10h
  jmp .halt

 ;HALT
 .halt:
  hlt
 jmp .halt

 ;ENTER PROTECTED MODE
 .enter_protected_mode:
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

 mov esp, 0x00E00000  ;set stack pointer

 ;JUMP TO EXECUTE BLESKOS
 jmp 0x10000

;;; variabiles ;;;
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

 vesa_mode_number dw 0
 vesa_24_mode_number dw 0
 vesa_32_mode_number dw 0
 vesa_last_24_mode_x dw 0
 vesa_last_32_mode_x dw 0

times 510-($-$$) db 0
dw 0xAA55
