;BleskOS

ahci_devices:
times 10 dd 0, 0
ahci_devices_pointer dd ahci_devices

ahci_port_base dd 0

init_ahci:
 mov eax, ahci_devices
 mov ecx, 10
 .search_for_hdd:
  cmp dword [eax+4], 0x00000101
  je .hdd_founded
  add eax, 8
 loop .search_for_hdd
 jmp .cdrom
 
 .hdd_founded:
 mov ebx, dword [eax]
 mov dword [hard_disk_base], ebx
 
 .cdrom:
 mov eax, ahci_devices
 mov ecx, 10
 .search_for_cdrom:
  cmp dword [eax+4], 0xEB140101
  je .cdrom_founded
  add eax, 8
 loop .search_for_cdrom
 jmp .enable_drives

 .cdrom_founded:
 mov ebx, dword [eax]
 mov dword [cdrom_base], ebx
 
 .enable_drives:
 mov eax, dword [hard_disk_base]
 mov dword [ahci_port_base], eax
 cmp eax, 0
 je .if_ahci_hdd
  mov dword [hard_disk_mode], AHCI_MODE
  call enable_ahci_port
  call sata_read_drive_info
  mov eax, dword [sata_drive_size]
  mov dword [hard_disk_size], eax
 .if_ahci_hdd
 
 mov eax, dword [cdrom_base]
 mov dword [ahci_port_base], eax
 cmp eax, 0
 je .if_ahci_cdrom
  mov dword [cdrom_mode], AHCI_MODE
  call enable_ahci_port
 .if_ahci_cdrom
 
 ret

enable_ahci_port:
 mov eax, dword [ahci_port_base]
 PHEX eax
 
 MMIO_OUTD ahci_port_base, 0x18, 0x0 ;stop port
 WAIT 100

 MMIO_OUTD ahci_port_base, 0x10, 0xFFFFFFFF ;clear interrupts state
 MMIO_OUTD ahci_port_base, 0x14, 0x0 ;disable interrupts
 
 MMIO_OUTD ahci_port_base, 0x00, MEMORY_AHCI ;commands
 MMIO_OUTD ahci_port_base, 0x04, 0
 MMIO_OUTD ahci_port_base, 0x08, MEMORY_AHCI+0x1000 ;receive FIS
 MMIO_OUTD ahci_port_base, 0x0C, 0
 
 MMIO_OUTD ahci_port_base, 0x38, 0x0 ;clear commands
 MMIO_OUTD ahci_port_base, 0x18, 0x11 ;start port
 WAIT 100

 ret
