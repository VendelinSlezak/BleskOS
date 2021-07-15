;BleskOS

ide_controllers:
dw 0x1F0, 0x3F4
dq 0, 0
dw 0x170, 0x374
dq 0, 0
native_ide_controllers:
times 8 dd 0, 0, 0, 0, 0

ide_pointer dd native_ide_controllers

ata_sector dq 0
ata_number_of_sectors dw 0
ata_memory dd 0
ata_status dd 0

%macro READ_HARD_DISK 3
 mov dword [ata_sector], %1
 mov dword [ata_number_of_sectors], %2
 mov dword [ata_memory], %3
 call read_hdd
%endmacro

%macro WRITE_HARD_DISK 3
 mov dword [ata_sector], %1
 mov dword [ata_number_of_sectors], %2
 mov dword [ata_memory], %3
 call write_hdd
%endmacro

init_ide_devices:
 mov edi, ide_controllers
 mov ecx, 10
 .scan_controller:
 push ecx
  cmp dword [edi], 0
  je .next_cycle ;no controller

  mov dx, word [edi+2]
  add dx, 2 ;right port
  OUTB dx, 0x2 ;disable interrupts

  mov ax, word [edi]
  mov word [pata_base], ax ;read base

  ;scan master drive
  call pata_select_master
  call pata_detect_drive
  mov eax, dword [pata_size]
  mov dword [edi+4], eax
  mov eax, dword [pata_size+4]
  mov dword [edi+8], eax

  ;scan slave drive
  call pata_select_slave
  call pata_detect_drive
  mov eax, dword [pata_size]
  mov dword [edi+12], eax
  mov eax, dword [pata_size+4]
  mov dword [edi+16], eax

  ;next item
  add edi, 20
 .next_cycle:
 pop ecx
 loop .scan_controller

 ;set first controller master drive
 mov word [pata_base], 0x1F0
 OUTB 0x1F6, 0x40 ;master drive
 WAIT 2 ;wait for enable drive
 OUTB 0x3F6, 0x2 ;disable interrupt
 call pata_detect_drive

 mov eax, dword [ide_controllers+4]
 PHEX eax
 mov eax, dword [ide_controllers+24]
 PHEX eax
 mov eax, dword [ide_controllers+44]
 PHEX eax
 mov eax, dword [ide_controllers+64]
 PHEX eax

 ret

read_hdd:
 call pata_read
 ret

write_hdd:
 call pata_write
 ret
