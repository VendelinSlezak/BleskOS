;BleskOS

sata_controller_base times 5 dd 0
sata_base_pointer dd sata_controller_base
sata_devices times 10 dd 0, 0

sata_base dd 0
sata_port_base dd 0

sata_sector dd 0
sata_number_of_sectors dd 0
sata_memory dd 0

ahci_set_port:
 mov ebx, 0x80
 mul ebx

 add eax, 0x100 ;offset to first port

 mov dword [sata_port_base], eax
 mov eax, dword [sata_base]
 add dword [sata_port_base], eax

 ret

init_ahci:
 cmp dword [sata_controller_base], 0
 je .done

 PSTR 'Serial ATA', serial_ata_str

 mov edi, sata_controller_base
 mov ecx, 5
 .detect_ahci:
 push ecx
  mov eax, dword [edi]
  mov dword [sata_base], eax
  push edi
  call ahci_detect_drive
  pop edi

 pop ecx
 add edi, 4
 cmp dword [edi], 0
 je .done
 loop .detect_ahci

 .done:
 mov eax, dword [sata_devices]
 PHEX eax
 mov eax, dword [sata_devices+4]
 PHEX eax
 mov eax, dword [sata_devices+8]
 PHEX eax
 mov eax, dword [sata_devices+12]
 PHEX eax
 ret

ahci_detect_drive:
 ;disable BIOS
 MMIO_OUTD sata_base, 0x28, 0x2
 WAIT 100

 ;enable AHCI
 MMIO_OUTD sata_base, 0x04, 0x80000000
 WAIT 10

 ;read capabilites
 mov esi, sata_devices
 mov eax, 0
 mov ecx, 32
 .read_port:
 push ecx
 push eax
  call ahci_set_port
  MMIO_IND sata_port_base, 0x24 ;signature
  cmp eax, 0xFFFFFFFF ;port is not present
  je .next_loop
  cmp eax, 0 ;no drive attached
  je .next_loop

  ;save drive
  mov ebx, dword [sata_port_base]
  mov dword [esi], ebx
  mov dword [esi+4], eax

  ;init port
  MMIO_OUTD sata_port_base, 0x18, 0x0 ;stop transfers
  WAIT 10
  MMIO_OUTD sata_port_base, 0x40, 0x0
  MMIO_OUTD sata_port_base, 0x00, MEMORY_SATA ;pointer to commands
  MMIO_OUTD sata_port_base, 0x04, 0x0
  MMIO_OUTD sata_port_base, 0x08, 0x0
  MMIO_OUTD sata_port_base, 0x0C, 0x0
  MMIO_OUTD sata_port_base, 0x38, 0x1 ;we will use first command
 .next_loop:
 pop eax
 inc eax
 pop ecx
 dec ecx
 cmp ecx, 0
 jne .read_port

 .done:
 ret
