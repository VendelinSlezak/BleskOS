;BleskOS

ide_pointer dd native_ide_controllers
ide_controllers:
dw 0x1F0, 0x3F4
dq 0, 0
dw 0x170, 0x374
dq 0, 0
native_ide_controllers:
times 8 dd 0, 0, 0, 0, 0

init_ide:
 LOG ' ', 0xA

 mov edi, ide_controllers
 mov ecx, 10
 .scan_controller:
 push ecx
  cmp dword [edi], 0
  je .next_cycle ;no controller
  
  mov dx, word [edi]
  add dx, 7
  in al, dx
  cmp al, 0xFF
  je .next_cycle ;no controller
  
  LOG 'IDE controller on port '

  mov dx, word [edi+2]
  add dx, 2 ;command port
  in al, dx
  or al, 0x2
  out dx, al ;disable interrupts

  mov eax, 0
  mov ax, word [edi]
  mov word [pata_base], ax ;read base
  LOG_HEX eax

  ;scan master drive
  LOG 'MASTER: '
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
  
  LOG 'type '
  LOG_HEX ebx
  LOG 'size in sectors '
  mov eax, dword [pata_size]
  LOG_VAR eax

  ;scan slave drive
  LOG 'SLAVE: '
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
  
  LOG 'type '
  LOG_HEX ebx
  LOG 'size in sectors '
  mov eax, dword [pata_size]
  LOG_VAR eax

  ;next item
  LOG 0xA
  add edi, 20
 .next_cycle:
 pop ecx
 dec ecx
 cmp ecx, 0
 jne .scan_controller

 ;FIND HARD DISK AND CDROM BASE AND DRIVE
 cmp dword [cdrom_base], 0 ;CDROM is connected to AHCI
 jne .cdrom_founded
 mov dword [cdrom_mode], IDE_MODE
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

 cmp dword [hard_disk_base], 0 ;hard disk is connected to AHCI
 jne .hard_disk_founded
 mov dword [hard_disk_mode], IDE_MODE
 mov esi, ide_controllers
 mov ecx, 10
 .find_hard_disk:
  mov ax, word [esi]
  mov word [hard_disk_base], ax
  
  mov dword [hard_disk_drive], IDE_MASTER
  mov eax, dword [esi+8]
  mov dword [hard_disk_size], eax
  cmp dword [esi+4], 0x00000000
  jne .test_slave
  cmp dword [esi+8], 0
  jne .hard_disk_founded

  .test_slave:
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
