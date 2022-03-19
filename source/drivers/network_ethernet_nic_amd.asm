;BleskOS

nic_amd_initalization_block:
dw 0x8000 ;promiscuous mode
db 3 << 4 ;8 receive buffers
db 3 << 4 ;8 transmit buffers
.mac0: db 0
.mac1: db 0
.mac2: db 0
.mac3: db 0
.mac4: db 0
.mac5: db 0
dw 0 ;reserved
dd 0, 0 ;logical address filter
dd MEMORY_NIC ;receive buffers
dd MEMORY_NIC+0x1000 ;transmit buffers

nic_amd_receive_buffer dd 0
nic_amd_transmit_buffer dd 0

init_nic_amd:
 ;reset card
 BASE_IND ethernet_card_io_base, 0x18
 BASE_INW ethernet_card_io_base, 0x14
 
 ;set 32 bit mode
 BASE_OUTD ethernet_card_io_base, 0x10, 0
 
 BASE_OUTD ethernet_card_io_base, 0x14, 58
 BASE_IND ethernet_card_io_base, 0x10
 and eax, 0xFF00
 or eax, 0x2
 BASE_OUTD ethernet_card_io_base, 0x10, eax
 
 BASE_OUTD ethernet_card_io_base, 0x14, 20
 BASE_OUTD ethernet_card_io_base, 0x1C, 0x0102 ;BCR register
 
 ;read MAC address
 BASE_IND ethernet_card_io_base, 0x0
 mov byte [ethernet_card_mac+5], al
 mov byte [nic_amd_initalization_block.mac5], al
 mov byte [ethernet_card_mac+4], ah
 mov byte [nic_amd_initalization_block.mac4], ah
 shr eax, 16
 mov byte [ethernet_card_mac+3], al
 mov byte [nic_amd_initalization_block.mac3], al
 mov byte [ethernet_card_mac+2], ah
 mov byte [nic_amd_initalization_block.mac2], ah
 BASE_IND ethernet_card_io_base, 0x4
 mov byte [ethernet_card_mac+1], al
 mov byte [nic_amd_initalization_block.mac1], al
 mov byte [ethernet_card_mac+0], ah
 mov byte [nic_amd_initalization_block.mac0], ah
 
 ;enable ASEL
 BASE_OUTD ethernet_card_io_base, 0x14, 0x2
 BASE_IND ethernet_card_io_base, 0x1C
 and eax, 0x2
 BASE_OUTD ethernet_card_io_base, 0x1C, eax
 
 ;initalize receive buffer list
 mov esi, MEMORY_NIC
 mov eax, MEMORY_NIC+0x2000
 mov ecx, 8
 .init_receive:
  mov dword [esi], eax
  add eax, 1520
  mov dword [esi+4], 0x80000000 | (0xF << 12) | (1520) ;max length of packet
  mov dword [esi+8], 0
  mov dword [esi+12], 0
  add esi, 0x10
 loop .init_receive
 
 ;initalize transmit buffer list
 mov esi, MEMORY_NIC+0x1000
 mov ecx, 8
 .init_transmit:
  mov dword [esi], 0
  mov dword [esi+4], 0
  mov dword [esi+8], 0
  mov dword [esi+12], 0
  add esi, 0x10
 loop .init_transmit
 
 ;enable interrupts
 BASE_OUTD ethernet_card_io_base, 0x14, 0x3
 BASE_IND ethernet_card_io_base, 0x10
 mov eax, 0x0100
 BASE_OUTD ethernet_card_io_base, 0x10, eax
 
 ;initalize card through initalization block
 BASE_OUTD ethernet_card_io_base, 0x14, 0x1
 BASE_OUTD ethernet_card_io_base, 0x10, (nic_amd_initalization_block - $$ + 0x10000) & 0xFFFF
 BASE_OUTD ethernet_card_io_base, 0x14, 0x2
 BASE_OUTD ethernet_card_io_base, 0x10, (nic_amd_initalization_block - $$ + 0x10000) >> 16
 BASE_OUTD ethernet_card_io_base, 0x14, 0x0
 BASE_OUTD ethernet_card_io_base, 0x10, 0x41
 WAIT 50
 
 ;start card
 BASE_OUTD ethernet_card_io_base, 0x14, 0x0
 BASE_OUTD ethernet_card_io_base, 0x10, 0x142
 
 ret
 
nic_amd_send_packet:
 mov eax, dword [nic_amd_transmit_buffer]
 mov ebx, 0x10
 mul ebx
 add eax, MEMORY_NIC+0x1000
 mov esi, eax
 
 mov dword [esi+8], 0
 mov dword [esi+12], 0
 mov eax, dword [packet_pointer]
 mov dword [esi], eax
 mov eax, 0xFFF
 sub eax, dword [packet_length]
 or eax, 0x8300F000
 mov dword [esi+4], eax
 
 inc dword [nic_amd_transmit_buffer]
 and dword [nic_amd_transmit_buffer], 0x7
 ret

nic_amd_irq:
 mov dword [last_arrived_packet], 0
 
 ;read interrupt reason
 BASE_OUTD ethernet_card_io_base, 0x14, 0x0
 BASE_IND ethernet_card_io_base, 0x10
 test eax, 0x400
 jnz .packet_received
 test eax, 0x200
 jnz .packet_transmitted
 jmp .end
 
 .packet_received:
  mov eax, dword [nic_amd_receive_buffer]
  shl eax, 4 ;mul 16
  add eax, MEMORY_NIC
  mov ebx, dword [eax]
  mov dword [eax+4], 0x80000000 | (0xF << 12) | (1520) ;restore pointer
  mov dword [received_packet_pointer], ebx
  
  inc dword [nic_amd_receive_buffer]
  and dword [nic_amd_receive_buffer], 0x7
  
  mov eax, dword [nic_amd_receive_buffer]
  shl eax, 4 ;mul 16
  add eax, MEMORY_NIC
  test dword [eax+4], 0x80000000
  jz .not_last_packet 
   mov dword [last_arrived_packet], 1
  .not_last_packet:
  call ethernet_card_process_packet
  
  mov eax, dword [nic_amd_receive_buffer]
  shl eax, 4 ;mul 16
  add eax, MEMORY_NIC
  test dword [eax+4], 0x80000000
  jz .packet_received
 jmp .end
 
 .packet_transmitted:
  inc dword [number_of_transmitted_packets]
 jmp .end
 
 ;clear interrupt state
 .end:
 BASE_OUTD ethernet_card_io_base, 0x14, 0x0
 BASE_IND ethernet_card_io_base, 0x10
 or eax, 0x7F00
 BASE_OUTD ethernet_card_io_base, 0x10, eax
 
 BASE_OUTD ethernet_card_io_base, 0x14, 0x4
 BASE_IND ethernet_card_io_base, 0x10
 or eax, 0x022A
 BASE_OUTD ethernet_card_io_base, 0x10, eax
 
 ret
