;BleskOS bootloader

org 0x7C00

bits 16

start:
  mov ax, 0
  mov ds, ax
  mov ss, ax
  mov sp, 0x7C00

  ;READ CODE OF BLESKOS
  mov ax, 0x1000
  mov es, ax   ;segment for BleskOS code
  mov ah, 0x02 ;read function
  mov al, 64   ;sectors to read
  mov bx, 0    ;offset for BleskOS code
  mov ch, 0    ;cylinder
  mov dh, 0    ;head
  mov cl, 2    ;sector
  ;dl - number of boot device drive is defined by BIOS
  int 13h ;read BleskOS code

  ;SWITCH TO PROTECTED MODE
  ;load gdt
  cli
  lgdt [gdt_wrap]

  ;jump into protected mode
  mov eax, cr0
  or eax, 1
  mov cr0, eax
  jmp 0x0008:clear_pipe

  ;NOW WE ARE IN PROTECTED MODE
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

  gdt:
  dq 0 ;null segment

  ;gdt code
  dw 0xFFFF, 0
  db 0, 0x9A, 0xCF, 0

  ;gdt_data
  dw 0xFFFF, 0
  db 0, 0x92, 0xCF, 0
  gdt_end:

  gdt_wrap:
   dw gdt_end - gdt - 1
   dd gdt

times 510-($-$$) db 0
dw 0xAA55
