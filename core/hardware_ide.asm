;BleskOS

ide_set_drive:
  OUTB 0x1F6, 0x40 ;hard disc master
  OUTB 0x176, 0xE0 ;cdrom master

  ret

ata_irq:
  pusha
  INB 0x1F7
  OUTB 0xA0, 0x20
  OUTB 0x20, 0x20
  popa

  iret

atapi_irq:
  pusha
  INB 0x177
  OUTB 0xA0, 0x20
  OUTB 0x20, 0x20
  popa

  iret

ata_read:
  mov ax, word [ata_nos]
  mov al, ah
  OUTB 0x1F2, al

  mov eax, dword [ata_sector]
  shr eax, 24
  OUTB 0x1F3, al

  mov eax, dword [ata_sector+4]
  OUTB 0x1F4, al

  mov eax, dword [ata_sector+4]
  mov al, ah
  OUTB 0x1F5, al

  mov ax, word [ata_nos]
  OUTB 0x1F2, al

  mov eax, dword [ata_sector]
  OUTB 0x1F3, al

  mov eax, dword [ata_sector]
  shr eax, 8
  OUTB 0x1F4, al

  mov eax, dword [ata_sector]
  shr eax, 16
  OUTB 0x1F5, al

  ;read command
  OUTB 0x1F7, 0x24

  mov ecx, 0xFFFF
  .wait:
    INB 0x3F6 ;alternative status port
    and al, 0x88
    cmp al, 0x08 ;is device ready to send sector?
    je .read
  loop .wait

  mov byte [ata_return], 0 ;something is wrong
  ret

  .read:
  mov ecx, 0
  mov cx, word [ata_nos]
  .number_of_sectors:
  push ecx
    mov ecx, 256
    .read_sector:
      INW 0x1F0
      mov edx, dword [ata_memory_pointer]
      mov word [edx], ax
      add dword [ata_memory_pointer], 2
    loop .read_sector
  pop ecx
  loop .number_of_sectors

  mov byte [ata_return], 1 ;transfer is done right
  ret

ata_write:
  mov ax, word [ata_nos]
  mov al, ah
  OUTB 0x1F2, al

  mov eax, dword [ata_sector]
  shr eax, 24
  OUTB 0x1F3, al

  mov eax, dword [ata_sector+4]
  OUTB 0x1F4, al

  mov eax, dword [ata_sector+4]
  mov al, ah
  OUTB 0x1F5, al

  mov ax, word [ata_nos]
  OUTB 0x1F2, al

  mov eax, dword [ata_sector]
  OUTB 0x1F3, al

  mov eax, dword [ata_sector]
  shr eax, 8
  OUTB 0x1F4, al

  mov eax, dword [ata_sector]
  shr eax, 16
  OUTB 0x1F5, al

  ;write command
  OUTB 0x1F7, 0x34

  mov ecx, 0xFFFF
  .wait:
    INB 0x3F6 ;alternative status port
    and al, 0x88
    cmp al, 0x08 ;is device ready to send sector?
    je .read
  loop .wait

  mov byte [ata_return], 0 ;something is wrong
  ret

  .read:
  mov ecx, 0
  mov cx, word [ata_nos]
  .number_of_sectors:
  push ecx
    mov ecx, 256
    .read_sector:
      mov edx, dword [ata_memory_pointer]
      mov ax, word [edx]
      add dword [ata_memory_pointer], 2
      OUTW 0x1F0, ax
    loop .read_sector
  pop ecx
  loop .number_of_sectors

  mov byte [ata_return], 1 ;transfer is done right
  ret


ata_return db 0
ata_sector dq 0
ata_nos dw 0
ata_memory_pointer dd 0
