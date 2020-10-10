;BleskOS real mode edition bootloader

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

  jc .error ;if error by read

  ;JUMP TO EXECUTE BLESKOS
  jmp 0x1000:0x0

  .error:
    mov ah, 0x0E
    mov al, 'E'
    int 10h
  halt:
    hlt
  jmp halt

times 0x1B4-($-$$) db 0
times 10 db 0
;partition 1
db 0x80
;first sector
db 0 ;head
db 2 ;sector
db 0 ;cylinder
db 0 ;free partition
;last sector
db 0 ;head
db 18 ;sector
db 0
dd 1
dd 10000
times 16 db 0
times 16 db 0
times 16 db 0
dw 0xAA55
