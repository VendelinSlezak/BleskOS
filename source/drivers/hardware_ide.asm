;BleskOS

%define IDE_OK 1
%define IDE_ERROR 0

%define IDE_MASTER 1
%define IDE_SLAVE 2

ide_pointer dd native_ide_controllers
ide_controllers:
dw 0x1F0, 0x3F4
dq 0, 0
dw 0x170, 0x374
dq 0, 0
native_ide_controllers:
times 8 dd 0, 0, 0, 0, 0

hard_disk_base dw 0
hard_disk_drive dd 0
hard_disk_size dd 0
cdrom_base dw 0
cdrom_drive dd 0

ata_sector dq 0
ata_number_of_sectors dw 0
ata_memory dd 0
ata_status dd 0

init_ide:
 mov edi, ide_controllers
 mov ecx, 10
 .scan_controller:
 push ecx
  cmp dword [edi], 0
  je .next_cycle ;no controller

  mov dx, word [edi+2]
  add dx, 2 ;command port
  OUTB dx, 0x2 ;disable interrupts

  mov ax, word [edi]
  mov word [pata_base], ax ;read base

  ;scan master drive
  call pata_select_master
  call pata_detect_drive
  mov eax, dword [pata_size]
  mov dword [edi+8], eax

  mov ebx, 0
  BASE_INB pata_base, 4
  mov bl, al
  BASE_INB pata_base, 5
  mov bh, al
  mov dword [edi+4], ebx

  ;scan slave drive
  call pata_select_slave
  call pata_detect_drive
  mov eax, dword [pata_size]
  mov dword [edi+16], eax

  mov ebx, 0
  BASE_INB pata_base, 4
  mov bl, al
  BASE_INB pata_base, 5
  mov bh, al
  mov dword [edi+12], ebx

  ;next item
  add edi, 20
 .next_cycle:
 pop ecx
 dec ecx
 cmp ecx, 0
 jne .scan_controller

 ;FIND HARD DISK AND CDROM BASE AND DRIVE
 mov esi, ide_controllers
 mov ecx, 10
 .find_cdrom:
  mov ax, word [esi]
  mov word [cdrom_base], ax
  mov dword [cdrom_drive], IDE_MASTER
  cmp dword [esi+4], 0x0000EB14
  je .cdrom_founded

  mov dword [cdrom_drive], IDE_SLAVE
  cmp dword [esi+12], 0x0000EB14
  je .cdrom_founded

  add esi, 20
 loop .find_cdrom
 .cdrom_founded:

 mov esi, ide_controllers
 mov ecx, 10
 .find_hard_disk:
  mov ax, word [esi]
  mov word [hard_disk_base], ax
  mov dword [hard_disk_drive], IDE_MASTER
  mov eax, dword [esi+8]
  mov dword [hard_disk_size], eax
  cmp dword [esi+4], 0x00000000
  jne .hard_disk_next_loop
  cmp dword [esi+8], 0
  jne .hard_disk_founded

  mov dword [hard_disk_drive], IDE_SLAVE
  mov eax, dword [esi+16]
  mov dword [hard_disk_size], eax
  cmp dword [esi+12], 0x00000000
  jne .hard_disk_next_loop
  cmp dword [esi+16], 0
  jne .hard_disk_founded

  .hard_disk_next_loop:
  add esi, 20
 loop .find_hard_disk
 .hard_disk_founded:

 ret
