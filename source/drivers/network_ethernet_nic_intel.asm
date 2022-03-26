;BleskOS

%macro NIC_INTEL_READ 1
 mov ebx, %1
 call nic_intel_read
%endmacro

%macro NIC_INTEL_WRITE 2
 mov ebx, %1
 mov ecx, %2
 call nic_intel_write
%endmacro

%macro NIC_INTEL_READ_EEPROM_2 1
 mov eax, %1
 shl eax, 2
 or eax, 0x1
 NIC_INTEL_WRITE  0x14, eax
 WAIT 10
 NIC_INTEL_READ  0x14
 shr eax, 16
%endmacro

%macro NIC_INTEL_READ_EEPROM_8 1
 mov eax, %1
 shl eax, 8
 or eax, 0x1
 NIC_INTEL_WRITE  0x14, eax
 WAIT 10
 NIC_INTEL_READ  0x14
 shr eax, 16
%endmacro

%define nic_intel_receive_d MEMORY_NIC+0xA0000
%define nic_intel_transmit_d MEMORY_NIC+0xB0000
nic_intel_last_r dd 0
nic_intel_last_t dd 0

nic_intel_read:
 cmp dword [ethernet_card_bar_type], 0
 je .mmio
 
 BASE_OUTD ethernet_card_io_base, 0x0, ebx
 BASE_IND ethernet_card_io_base, 0x4
 ret
 
 .mmio:
 MMIO_IND ethernet_card_mmio_base, ebx
 ret
 
nic_intel_write:
 cmp dword [ethernet_card_bar_type], 0
 je .mmio
 
 BASE_OUTD ethernet_card_io_base, 0x0, ebx
 BASE_OUTD ethernet_card_io_base, 0x4, ecx
 ret
 
 .mmio:
 MMIO_OUTD ethernet_card_mmio_base, ebx, ecx
 ret

init_nic_intel:
 ;stop transfers
 NIC_INTEL_WRITE 0x100, 0x0
 NIC_INTEL_WRITE 0x400, 0x0
 
 ;reset card
 NIC_INTEL_WRITE 0x0, 0x04000000
 WAIT 100
 
 ;read card MAC
 NIC_INTEL_WRITE 0x10, 0xE ;unlock EEPROM
 NIC_INTEL_WRITE 0x14, 0x1
 WAIT 10
 NIC_INTEL_READ 0x14
 test eax, 0x12
 jz .no_eeprom
 test eax, 0x2
 jnz .eeprom_shl_2
 
 NIC_INTEL_READ_EEPROM_8 0
 mov byte [ethernet_card_mac+5], al
 mov byte [ethernet_card_mac+4], ah
 NIC_INTEL_READ_EEPROM_8 1
 mov byte [ethernet_card_mac+3], al
 mov byte [ethernet_card_mac+2], ah
 NIC_INTEL_READ_EEPROM_8 2
 mov byte [ethernet_card_mac+1], al
 mov byte [ethernet_card_mac+0], ah
 jmp .mac_readed
 
 .eeprom_shl_2:
 NIC_INTEL_READ_EEPROM_2 0
 mov byte [ethernet_card_mac+5], al
 mov byte [ethernet_card_mac+4], ah
 NIC_INTEL_READ_EEPROM_2 1
 mov byte [ethernet_card_mac+3], al
 mov byte [ethernet_card_mac+2], ah
 NIC_INTEL_READ_EEPROM_2 2
 mov byte [ethernet_card_mac+1], al
 mov byte [ethernet_card_mac+0], ah
 jmp .mac_readed
 
 .no_eeprom:
 NIC_INTEL_WRITE 0x10, 0x40 ;lock EEPROM
 NIC_INTEL_READ 0x5400
 mov byte [ethernet_card_mac+0], al
 NIC_INTEL_READ 0x5401
 mov byte [ethernet_card_mac+1], al
 NIC_INTEL_READ 0x5402
 mov byte [ethernet_card_mac+2], al
 NIC_INTEL_READ 0x5403
 mov byte [ethernet_card_mac+3], al
 NIC_INTEL_READ 0x5404
 mov byte [ethernet_card_mac+4], al
 NIC_INTEL_READ 0x5405
 mov byte [ethernet_card_mac+5], al
 .mac_readed:
 
 ;start card
 NIC_INTEL_READ 0x0
 or eax, 0x40
 NIC_INTEL_WRITE 0x0, eax

 ;fill receive and transmit descriptors
 mov edi, nic_intel_receive_d
 mov eax, 0
 mov ecx, 8*128
 rep stosb
 
 mov esi, nic_intel_receive_d
 mov eax, MEMORY_NIC
 mov ecx, 8
 .fill_receive:
  mov dword [esi], eax
  mov dword [esi+4], 0
  mov dword [esi+8], 0
  mov dword [esi+12], 0
  add esi, 16
  add eax, 8*1024 ;every buffer has size 8 KB
 loop .fill_receive
 
 mov edi, nic_intel_transmit_d
 mov eax, 0
 mov ecx, 8*128
 rep stosb
 
 mov esi, nic_intel_transmit_d
 mov ecx, 8
 .fill_transmit:
  mov byte [esi+12], 0x1 ;status = no packet
  add esi, 16
 loop .fill_transmit
 
 ;set receive mechanism
 NIC_INTEL_WRITE 0x2800, nic_intel_receive_d ;pointer to receive descriptors
 NIC_INTEL_WRITE 0x2804, 0
 NIC_INTEL_WRITE 0x2808, 8*16 ;length
 NIC_INTEL_WRITE 0x2810, 0 ;head
 NIC_INTEL_WRITE 0x2818, 7 ;tail
 
 ;set transmit mechanism
 NIC_INTEL_WRITE 0x3800, nic_intel_transmit_d ;pointer to transmit descriptors
 NIC_INTEL_WRITE 0x3804, 0
 NIC_INTEL_WRITE 0x3808, 8*16 ;length
 NIC_INTEL_WRITE 0x3810, 0 ;head
 NIC_INTEL_WRITE 0x3818, 0 ;tail
 
 ;enable receiving and transmitting packets
 NIC_INTEL_WRITE 0x100, ((1 << 1) | (1 << 3) | (1 << 15)) ;receive
 
 mov eax, ((1 << 1) | (1 << 3) | (4 << 15))
 cmp word [ethernet_card_id+2], 0x10F5
 jne .if_e1000e
  or eax, (0x3F << 12) | (0x3 << 28)
  jmp .enable_transmit
 .if_e1000e:
 or eax, (0x40 << 12) ;other cards
 .enable_transmit:
 NIC_INTEL_WRITE 0x400, eax ;transmit
 
 ;enable interrupts
 NIC_INTEL_WRITE 0xD0, 0x1F6DC
 NIC_INTEL_READ 0xC0

 ret
 
nic_intel_send_packet:
 mov eax, dword [nic_intel_last_t]
 mov ebx, 16
 mul ebx
 add eax, nic_intel_transmit_d
 
 mov ebx, dword [packet_pointer]
 mov dword [eax], ebx
 mov bx, word [packet_length]
 mov word [eax+8], bx 
 mov byte [eax+12], 0 ;status
 mov byte [eax+11], 0x3 ;send packet

 inc dword [nic_intel_last_t]
 and dword [nic_intel_last_t], 0x7

 mov eax, dword [nic_intel_last_t]
 NIC_INTEL_WRITE 0x3818, eax
 
 ret

nic_intel_irq:
 NIC_INTEL_READ 0xC0 ;read status
 mov dword [last_arrived_packet], 0
 test eax, 0x80
 jnz .packet_received
 test eax, 0x4
 jnz .packet_transmitted
 
 ret
 
 .packet_transmitted:
  inc dword [number_of_transmitted_packets]
 ret
 
 .packet_received:
  NIC_INTEL_READ 0x2810
  mov ebx, dword [nic_intel_last_r]
  inc ebx
  and ebx, 0x7
  cmp eax, ebx
  jne .if_last_arrived_packet
   mov dword [last_arrived_packet], 1
  .if_last_arrived_packet:
  
  mov eax, dword [nic_intel_last_r]
  shl eax, 4 ;mul 16
  add eax, nic_intel_receive_d
  mov ebx, dword [eax]
  mov dword [eax+4], 0
  mov dword [eax+8], 0
  mov dword [eax+12], 0
  mov dword [received_packet_pointer], ebx
  call ethernet_card_process_packet
  
  mov eax, dword [nic_intel_last_r]
  NIC_INTEL_WRITE 0x2818, eax ;move tail
  
  inc dword [nic_intel_last_r]
  and dword [nic_intel_last_r], 0x7  
 cmp dword [last_arrived_packet], 1
 jne .packet_received
 mov dword [last_arrived_packet], 0
 ret
