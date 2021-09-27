;BleskOS

sata_command_table:
db 0x27 ;H2D FIS type
db 0x80 ;flags
.cmd: db 0
db 0
.lba0: db 0 ;LBA 0
.lba1: db 0 ;LBA 1
.lba2: db 0 ;LBA 2
db 0xE0 ;device head
.lba3: db 0 ;LBA 3
.lba4: db 0 ;LBA 4
.lba5: db 0 ;LBA 5
db 0
.sector_count: db 0
db 0
db 0
db 0x8
times 0x70 db 0
.memory: dd 0
dd 0
dd 0
dd 0x1FF ;byte count
times 0x100 db 0

sata_drive_info times 512 db 0
sata_drive_size dd 0

sata_read_drive_info:
 ;clear
 mov edi, MEMORY_AHCI
 mov eax, 0
 mov ecx, 0x200
 rep stosb
 
 ;command header
 mov dword [MEMORY_AHCI+0x00], 0x00010005
 mov dword [MEMORY_AHCI+0x04], 0
 mov dword [MEMORY_AHCI+0x08], MEMORY_AHCI+0x100
 mov dword [MEMORY_AHCI+0x0C], 0
 
 ;command table
 mov byte [sata_command_table.cmd], 0xEC ;drive block
 
 mov byte [sata_command_table.lba0], 0
 mov byte [sata_command_table.lba1], 0
 mov byte [sata_command_table.lba2], 0
 mov byte [sata_command_table.lba3], 0
 
 mov dword [sata_command_table.sector_count], 0
 
 mov dword [sata_command_table.memory], sata_drive_info
 
 mov esi, sata_command_table
 mov edi, MEMORY_AHCI+0x100
 mov ecx, 0x100
 rep movsb
 
 ;send command
 MMIO_OUTD ahci_port_base, 0x38, 0x1 ;send first command
 mov dword [ticks], 0
 .wait:
  MMIO_IND ahci_port_base, 0x38
  test eax, 0x1
  jz .transfer_is_done
 cmp dword [ticks], 100
 jl .wait
 
 MMIO_OUTD ahci_port_base, 0x18, 0x0 ;stop port
 WAIT 10
 MMIO_OUTD ahci_port_base, 0x38, 0x0 ;erase command
 WAIT 10
 MMIO_OUTD ahci_port_base, 0x18, 0x11 ;start port
 
 ret
 
 .transfer_is_done:
 mov eax, dword [sata_drive_info+200]
 mov ebx, dword [sata_drive_info+120]
 mov dword [sata_drive_size], ebx
 cmp ebx, 0
 jne .done
 mov dword [sata_drive_size], eax
 .done:
 ret
 
sata_read:
 mov dword [ata_status], ATA_ERROR
 
 ;clear
 mov edi, MEMORY_AHCI
 mov eax, 0
 mov ecx, 0x200
 rep stosb
 
 ;command header
 mov dword [MEMORY_AHCI+0x00], 0x00010005
 mov dword [MEMORY_AHCI+0x04], 0
 mov dword [MEMORY_AHCI+0x08], MEMORY_AHCI+0x100
 mov dword [MEMORY_AHCI+0x0C], 0
 
 ;command table
 mov byte [sata_command_table.cmd], 0x25 ;read command
 
 mov al, byte [ata_sector+0]
 mov byte [sata_command_table.lba0], al
 mov al, byte [ata_sector+1]
 mov byte [sata_command_table.lba1], al
 mov al, byte [ata_sector+2]
 mov byte [sata_command_table.lba2], al
 mov al, byte [ata_sector+3]
 mov byte [sata_command_table.lba3], al
 
 mov dword [sata_command_table.sector_count], 0x1
 
 mov eax, dword [ata_memory]
 mov dword [sata_command_table.memory], eax
 
 mov esi, sata_command_table
 mov edi, MEMORY_AHCI+0x100
 mov ecx, 0x100
 rep movsb
 
 ;send command
 MMIO_OUTD ahci_port_base, 0x38, 0x1 ;send first command
 mov dword [ticks], 0
 .wait:
  MMIO_IND ahci_port_base, 0x38
  test eax, 0x1
  jz .transfer_is_done
 cmp dword [ticks], 100
 jl .wait
 
 MMIO_OUTD ahci_port_base, 0x18, 0x0 ;stop port
 WAIT 10
 MMIO_OUTD ahci_port_base, 0x38, 0x0 ;erase command
 WAIT 10
 MMIO_OUTD ahci_port_base, 0x18, 0x11 ;start port
 
 ret
 
 .transfer_is_done:
 mov dword [ata_status], ATA_OK
 ret

sata_write:
 mov dword [ata_status], ATA_ERROR
 
 ;clear
 mov edi, MEMORY_AHCI
 mov eax, 0
 mov ecx, 0x200
 rep stosb
 
 ;command header
 mov dword [MEMORY_AHCI+0x00], 0x00010045
 mov dword [MEMORY_AHCI+0x04], 0
 mov dword [MEMORY_AHCI+0x08], MEMORY_AHCI+0x100
 mov dword [MEMORY_AHCI+0x0C], 0
 
 ;command table
 mov byte [sata_command_table.cmd], 0x35 ;write command
 
 mov al, byte [ata_sector+0]
 mov byte [sata_command_table.lba0], al
 mov al, byte [ata_sector+1]
 mov byte [sata_command_table.lba1], al
 mov al, byte [ata_sector+2]
 mov byte [sata_command_table.lba2], al
 mov al, byte [ata_sector+3]
 mov byte [sata_command_table.lba3], al
 
 mov dword [sata_command_table.sector_count], 0x1
 
 mov eax, dword [ata_memory]
 mov dword [sata_command_table.memory], eax
 
 mov esi, sata_command_table
 mov edi, MEMORY_AHCI+0x100
 mov ecx, 0x100
 rep movsb
 
 ;send command
 MMIO_OUTD ahci_port_base, 0x38, 0x1 ;send first command
 mov dword [ticks], 0
 .wait:
  MMIO_IND ahci_port_base, 0x38
  test eax, 0x1
  jz .transfer_is_done
 cmp dword [ticks], 100
 jl .wait
 
 MMIO_OUTD ahci_port_base, 0x18, 0x0 ;stop port
 WAIT 10
 MMIO_OUTD ahci_port_base, 0x38, 0x0 ;erase command
 WAIT 10
 MMIO_OUTD ahci_port_base, 0x18, 0x11 ;start port
 
 ret
 
 .transfer_is_done:
 mov dword [ata_status], ATA_OK
 ret

sata_delete:
 mov dword [ata_status], ATA_ERROR
 
 ;clear
 mov edi, MEMORY_AHCI
 mov eax, 0
 mov ecx, 0x200
 rep stosb
 
 ;command header
 mov dword [MEMORY_AHCI+0x00], 0x00010045
 mov dword [MEMORY_AHCI+0x04], 0
 mov dword [MEMORY_AHCI+0x08], MEMORY_AHCI+0x100
 mov dword [MEMORY_AHCI+0x0C], 0
 
 ;command table
 mov byte [sata_command_table.cmd], 0x35 ;write command
 
 mov al, byte [ata_sector+0]
 mov byte [sata_command_table.lba0], al
 mov al, byte [ata_sector+1]
 mov byte [sata_command_table.lba1], al
 mov al, byte [ata_sector+2]
 mov byte [sata_command_table.lba2], al
 mov al, byte [ata_sector+3]
 mov byte [sata_command_table.lba3], al
 
 mov dword [sata_command_table.sector_count], 0x1
 
 mov edi, MEMORY_AHCI+0x400
 mov eax, 0
 mov ecx, 512
 rep stosb ;clear
 mov dword [sata_command_table.memory], MEMORY_AHCI+0x400
 
 mov esi, sata_command_table
 mov edi, MEMORY_AHCI+0x100
 mov ecx, 0x100
 rep movsb
 
 ;send command
 MMIO_OUTD ahci_port_base, 0x38, 0x1 ;send first command
 mov dword [ticks], 0
 .wait:
  MMIO_IND ahci_port_base, 0x38
  test eax, 0x1
  jz .transfer_is_done
 cmp dword [ticks], 100
 jl .wait
 
 MMIO_OUTD ahci_port_base, 0x18, 0x0 ;stop port
 WAIT 10
 MMIO_OUTD ahci_port_base, 0x38, 0x0 ;erase command
 WAIT 10
 MMIO_OUTD ahci_port_base, 0x18, 0x11 ;start port
 
 ret
 
 .transfer_is_done:
 mov dword [ata_status], ATA_OK
 ret
