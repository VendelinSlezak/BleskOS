;BleskOS

sata_read:
 mov dword [MEMORY_SATA+0x500], 0xFFEEDDCC

 mov dword [MEMORY_SATA+0], 0x00010005
 mov dword [MEMORY_SATA+4], 0x00000000
 mov dword [MEMORY_SATA+8], MEMORY_SATA+0x1000
 mov dword [MEMORY_SATA+12], 0x00000000

 mov byte [MEMORY_SATA+0x1000+0], 0x27 ;H2D FIS command
 mov byte [MEMORY_SATA+0x1000+1], 0x80 ;flags
 mov byte [MEMORY_SATA+0x1000+2], 0x25 ;read DMA ext
 mov byte [MEMORY_SATA+0x1000+3], 0x0 ;features
 mov al, byte [sata_sector]
 mov byte [MEMORY_SATA+0x1000+4], al ;LBA & 0xFF
 mov al, byte [sata_sector+1]
 mov byte [MEMORY_SATA+0x1000+5], al ;LBA >> 8
 mov al, byte [sata_sector+2]
 mov byte [MEMORY_SATA+0x1000+6], al ;LBA >> 16
 mov byte [MEMORY_SATA+0x1000+7], 0xE0 ;drive
 mov al, byte [sata_sector+3]
 mov byte [MEMORY_SATA+0x1000+8], al ;LBA >> 24
 mov byte [MEMORY_SATA+0x1000+9], 0x0 ;LBA >> 32
 mov byte [MEMORY_SATA+0x1000+10], 0x0 ;LBA >> 40
 mov byte [MEMORY_SATA+0x1000+11], 0x0 ;features ext
 mov ax, word [sata_number_of_sectors]
 mov word [MEMORY_SATA+0x1000+12], ax ;sector count
 mov byte [MEMORY_SATA+0x1000+14], 0x0 ;reserved
 mov byte [MEMORY_SATA+0x1000+15], 0x08 ;control
 mov byte [MEMORY_SATA+0x1000+16], 0x0 ;reserved

 mov eax, dword [sata_memory]
 mov dword [MEMORY_SATA+0x1080+0], eax ;data pointer
 mov dword [MEMORY_SATA+0x1080+4], 0x0
 mov dword [MEMORY_SATA+0x1080+8], 0x0
 mov eax, dword [sata_number_of_sectors]
 mov ebx, 0x200 ;one sector
 mul ebx
 dec eax
 mov dword [MEMORY_SATA+0x180+12], eax ;data byte count

 ;start transfer
 MMIO_OUTD sata_port_base, 0x18, 0x10000001
 WAIT 200
 MMIO_IND sata_port_base, 0x18
 PHEX eax
 mov eax, dword [MEMORY_SATA+4]
 PHEX eax
 mov eax, dword [MEMORY_SATA+0x180+12]
 PHEX eax
 mov eax, dword [MEMORY_SATA+0x500]
 PHEX eax

 ret
