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

sata_devices_type:
times 32 dd 0

hard_disk_base dw 0
hard_disk_drive dd 0
hard_disk_size dd 0
cdrom_base dw 0
cdrom_drive dd 0

serial_hard_disk_port dd 0
serial_cdrom_port dd 0

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

 .init_sata:
 PSTR 'Serial ATA', serial_ata_str

 ;disable BIOS
 MMIO_OUTD sata_base, 0x28, 0x2
 WAIT 100

 ;enable AHCI
 MMIO_OUTD sata_base, 0x04, 0x80000000
 WAIT 10

 ;read capabilites
 mov esi, sata_devices_type
 mov eax, 0
 mov ecx, 32
 .read_port:
 push ecx
  call sata_set_port
  MMIO_IND sata_port_base, 0x24
  mov dword [esi], eax ;save value
  cmp eax, 0xFFFFFFFF ;port is not present
  je .next_loop
  cmp eax, 0 ;no drive attached
  je .next_loop

  MMIO_OUTD sata_port_base, 0x18, 0x0 ;stop transfers
  WAIT 10
  MMIO_OUTD sata_port_base, 0x40, 0x0
  MMIO_OUTD sata_port_base, 0x00, MEMORY_SATA ;pointer to commands
  MMIO_OUTD sata_port_base, 0x04, 0x0
  MMIO_OUTD sata_port_base, 0x08, 0x0
  MMIO_OUTD sata_port_base, 0x0C, 0x0
  MMIO_OUTD sata_port_base, 0x38, 0x1 ;we will use first command
 .next_loop:
 pop ecx
 dec ecx
 cmp ecx, 0
 jne .read_port

 ;find serial hard disk
 mov esi, sata_devices_type
 mov eax, 0
 mov ecx, 32
 .find_serial_hard_disk:
  mov dword [serial_hard_disk_port], eax
  cmp dword [esi], 0x00000101
  je .serial_hard_disk_found
 loop .find_serial_hard_disk
 .serial_hard_disk_found:

 ret

read_hdd:
 call pata_read
 ret

write_hdd:
 call pata_write
 ret
