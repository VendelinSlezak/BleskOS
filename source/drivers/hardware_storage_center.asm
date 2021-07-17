;BleskOS

ide_controllers:
dw 0x1F0, 0x3F4
dq 0, 0
dw 0x170, 0x374
dq 0, 0
native_ide_controllers:
times 8 dd 0, 0, 0, 0, 0

ide_pointer dd native_ide_controllers

sata_devices_type:
times 32 dd 0

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
 cmp dword [sata_base], 0
 jne .init_sata

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

 ;set first controller master drive
 mov word [pata_base], 0x1F0
 OUTB 0x1F6, 0x40 ;master drive
 WAIT 2 ;wait for enable drive
 OUTB 0x3F6, 0x2 ;disable interrupt
 call pata_detect_drive

 ret

 .init_sata:
 PSTR 'Serial ATA', serial_ata_str

 ;disable BIOS
 MMIO_OUTD sata_base, 0x28, 0x2
 WAIT 10

 ;enable AHCI
 MMIO_OUTD sata_base, 0x04, 0x80000000

 ;read capabilites
 mov esi, sata_devices_type
 mov eax, 0
 mov ecx, 32
 .read_port:
  call sata_set_port
  MMIO_IND sata_port_base, 0x24
  mov dword [esi], eax
  cmp eax, 0
  je .next_loop

  MMIO_OUTD sata_port_base, 0x18, 0x0
  MMIO_OUTD sata_port_base, 0x40, 0x0
  MMIO_OUTD sata_port_base, 0x00, MEMORY_SATA
  MMIO_OUTD sata_port_base, 0x04, 0x0
  MMIO_OUTD sata_port_base, 0x08, 0x0
  MMIO_OUTD sata_port_base, 0x0C, 0x0
 .next_loop:
 loop .read_port

 mov eax, 0
 call sata_set_port

 ret

read_hdd:
 call pata_read
 ret

write_hdd:
 call pata_write
 ret
