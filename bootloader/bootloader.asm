;BleskOS bootloader

;What this code do:
; - enable A20 for acess to all RAM memory
; - set VESA graphic mode 800x600x16
; - read 128 KB from boot device(must be emulated as hard disk) after bootloader sector - this contains BleskOS code
; - load Global Descriptor Table for acces max 4 GB of RAM memory
; - enter to protected mode(32 bit processor mode)
; - start executing of BleskOS code, whose is loaded in memory 0x10000

org 0x7C00

bits 16

start:
 jmp .code
 times 0x3E db 0 ;BPB
 .code:
 xor ax, ax
 mov ds, ax
 mov ss, ax
 mov es, ax
 mov sp, 0x7C00

 ;IF NOT EMULATED AS HARD DISK
 cmp dl, 0
 je .error_1
 cmp dl, 1
 je .error_1

 ;ENABLE A20
 in al, 0x92
 or al, 0x2
 out 0x92, al

 ;READ BLESKOS 128 KB, drive in dl is defined by BIOS
 mov ah, 0x42
 mov si, disk_packet1
 int 13h
 jc .error_2
 mov ah, 0x42
 mov si, disk_packet2
 int 13h
 jc .error_2
 mov ah, 0x42
 mov si, disk_packet3
 int 13h
 jc .error_2
 mov ah, 0x42
 mov si, disk_packet4
 int 13h
 jc .error_2
 
 ;FIND VESA MODE
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
  cmp byte [es:0x19], 16
  jne .next_mode
  mov bx, word [vesa_last_mode_x]
  cmp bx, word [es:12]
  jg .next_mode

  mov word [vesa_mode_number], cx
  mov bx, word [es:12]
  mov word [vesa_last_mode_x], bx

 .next_mode:
 inc cx
 cmp cx, 0x150
 jne .find_mode

 cmp word [vesa_mode_number], 0
 jne .set_vesa_mode

 ;mode was not founded
 jmp .error_3

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
  mov ah, 0x0E
  mov al, 'E'
  int 10h
  mov al, '1'
  int 10h
  jmp .halt

 .error_2:
  mov ah, 0x0E
  mov al, 'E'
  int 10h
  mov al, '2'
  int 10h
  jmp .halt

 .error_3:
  mov ah, 0x0E
  mov al, 'E'
  int 10h
  mov al, '3'
  int 10h

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

 mov esp, 0x80000  ;set stack pointer

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

 disk_packet1:
  dw 0x0010, 64, 0x0000, 0x1000
  dq 1
 disk_packet2:
  dw 0x0010, 64, 0x8000, 0x1000
  dq 65
 disk_packet3:
  dw 0x0010, 64, 0x0000, 0x2000
  dq 129
 disk_packet4:
  dw 0x0010, 64, 0x8000, 0x2000
  dq 193

 vesa_mode_number dw 0
 vesa_last_mode_x dw 0

times 510-($-$$) db 0
dw 0xAA55
