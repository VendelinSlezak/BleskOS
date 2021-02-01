;BleskOS bootloader

;What this code do:
; - enable A20 for acess to all RAM memory
; - find highest VESA video mode with 16 bpp
; - load VESA info mode block to memory 0x70000 (it includes mode x, y, bpp, linear frame buffer pointer)
; - set VESA graphic mode 0x114 what means mode 800x600x16
; - read 128 KB from boot device(must be emulated as hard disk) after bootloader sector - this contains BleskOS code
; - load Global Descriptor Table for acces max 4 GB of RAM memory
; - enter to protected mode(32 bit processor mode)
; - start executing of BleskOS code, whose is loaded in memory 0x10000

org 0x7C00

bits 16

start:
 xor ax, ax
 mov ds, ax
 mov ss, ax
 mov sp, 0x7C00

 ;ENABLE A20
 in al, 0x92
 or al, 0x2
 out 0x92, al

 ;FIND HIGHEST 16 BPP VESA MODE
 mov word [vesa_actual_mode], 0x4100
 .scan_vesa_mode:
  mov ax, 0x7000
  mov es, ax
  mov word [es:0x00], 0 ;clear
  mov byte [es:0x19], 0 ;clear
  mov di, 0x00
  mov ax, 0x4F01
  mov cx, word [vesa_actual_mode]
  int 0x10 ;load info

  mov bx, word [es:0x00]
  and bx, 0x90
  cmp bx, 0x90 ;is mode graphic with linear frame buffer?
  jne .next_scan_cycle

  cmp byte [es:0x19], 16 ;16 bpp mode?
  jne .if_16_bpp
   mov word [vesa_mode], cx ;if yes, save it
  .if_16_bpp:

 .next_scan_cycle:
 inc word [vesa_actual_mode]
 cmp word [vesa_actual_mode], 0x4120
 jl .scan_vesa_mode
 
 ;LOAD VESA INFO
 mov ax, 0x7000
 mov es, ax
 mov di, 0
 mov ax, 0x4F01
 mov cx, word [vesa_mode]
 int 0x10
 ;SET VESA MODE
 mov ax, 0x4F02
 mov bx, word [vesa_mode]
 int 0x10

 ;READ BLESKOS 128 KB, drive in dl is defined by BIOS
 mov ah, 0x42
 mov si, disk_packet1
 int 13h
 mov ah, 0x42
 mov si, disk_packet2
 int 13h
 mov ah, 0x42
 mov si, disk_packet3
 int 13h
 mov ah, 0x42
 mov si, disk_packet4
 int 13h

 ;ENTER PROTECTED MODE
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
 vesa_actual_mode dd 0
 vesa_mode dd 0

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

times 510-($-$$) db 0
dw 0xAA55
