;BleskOS

sata_base dd 0
sata_port_base dd 0
sata_sector dd 0
sata_number_of_sectors dd 0
sata_memory dd 0

sata_set_port:
 mov ebx, 0x80
 mul ebx

 add eax, 0x100 ;offset to first port

 mov dword [sata_port_base], eax
 mov eax, dword [sata_base]
 add dword [sata_port_base], eax

 ret

sata_read:
 mov dword [MEMORY_SATA+0], 0x00010005
 mov dword [MEMORY_SATA+4], 0x00000000
 mov dword [MEMORY_SATA+8], MEMORY_SATA+0x100
 mov dword [MEMORY_SATA+12], 0x00000000

 jmp .zero

 mov byte [MEMORY_SATA+0x100+0], 0x27 ;H2D FIS command
 mov byte [MEMORY_SATA+0x100+1], 0x80 ;flags
 mov byte [MEMORY_SATA+0x100+2], 0x25 ;read DMA ext
 mov byte [MEMORY_SATA+0x100+3], 0x0 ;features
 mov al, byte [sata_sector]
 mov byte [MEMORY_SATA+0x100+4], al ;LBA & 0xFF
 mov al, byte [sata_sector+1]
 mov byte [MEMORY_SATA+0x100+5], al ;LBA >> 8
 mov al, byte [sata_sector+2]
 mov byte [MEMORY_SATA+0x100+6], al ;LBA >> 16
 mov byte [MEMORY_SATA+0x100+7], 0x40 ;drive
 mov al, byte [sata_sector+3]
 mov byte [MEMORY_SATA+0x100+8], al ;LBA >> 24
 mov byte [MEMORY_SATA+0x100+9], 0x0 ;LBA >> 32
 mov byte [MEMORY_SATA+0x100+10], 0x0 ;LBA >> 40
 mov byte [MEMORY_SATA+0x100+11], 0x0 ;features ext
 mov ax, word [sata_number_of_sectors]
 mov word [MEMORY_SATA+0x100+12], ax ;sector count
 mov byte [MEMORY_SATA+0x100+14], 0x0 ;reserved
 mov byte [MEMORY_SATA+0x100+15], 0x08 ;control
 mov byte [MEMORY_SATA+0x100+16], 0x0 ;reserved

 mov eax, dword [sata_memory]
 mov dword [MEMORY_SATA+0x180+0], eax ;data pointer
 mov dword [MEMORY_SATA+0x180+4], 0x0
 mov dword [MEMORY_SATA+0x180+8], 0x0
 mov eax, dword [sata_number_of_sectors]
 mov ebx, 0x200 ;one sector
 mul ebx
 dec eax
 mov dword [MEMORY_SATA+0x180+12], eax ;data byte count

 .zero:
 mov byte [MEMORY_SATA+0x100+0], 0x27 ;H2D FIS command
 mov byte [MEMORY_SATA+0x100+1], 0x80 ;flags
 mov byte [MEMORY_SATA+0x100+2], 0x25 ;read DMA ext
 mov byte [MEMORY_SATA+0x100+3], 0x0 ;features
 mov byte [MEMORY_SATA+0x100+4], 0x0 ;LBA & 0xFF
 mov byte [MEMORY_SATA+0x100+5], 0x0 ;LBA >> 8
 mov byte [MEMORY_SATA+0x100+6], 0x0 ;LBA >> 16
 mov byte [MEMORY_SATA+0x100+7], 0x40 ;drive
 mov byte [MEMORY_SATA+0x100+8], 0x0 ;LBA >> 24
 mov byte [MEMORY_SATA+0x100+9], 0x0 ;LBA >> 32
 mov byte [MEMORY_SATA+0x100+10], 0x0 ;LBA >> 40
 mov byte [MEMORY_SATA+0x100+11], 0x0 ;features ext
 mov word [MEMORY_SATA+0x100+12], 0x1 ;sector count
 mov byte [MEMORY_SATA+0x100+14], 0x0 ;reserved
 mov byte [MEMORY_SATA+0x100+15], 0x08 ;control
 mov byte [MEMORY_SATA+0x100+16], 0x0 ;reserved

 mov dword [MEMORY_SATA+0x180+0], MEMORY_SATA+0x500 ;data pointer
 mov dword [MEMORY_SATA+0x180+4], 0x0
 mov dword [MEMORY_SATA+0x180+8], 0x0
 mov dword [MEMORY_SATA+0x180+12], 0x1FF ;data byte count

 ;start transfer
 MMIO_OUTD sata_port_base, 0x18, 0x1
 WAIT 200
 MMIO_IND sata_port_base, 0x18
 PHEX eax
 mov eax, dword [MEMORY_SATA+0x180+12]
 PHEX eax

 ret