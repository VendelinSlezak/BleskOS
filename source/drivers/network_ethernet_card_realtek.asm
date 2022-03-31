;BleskOS

; not complete, especially RTL8169!

%define RTL_8139 0
%define RTL_8169 1

nic_realtek_type dd 0
nic_realtek_send_port dw 0x20

realtek_8139_receive_pointer dd MEMORY_NIC+0x1000

init_nic_realtek:
 cmp dword [nic_realtek_type], RTL_8169
 je .realtek_8169
 
 ;turn card on
 BASE_OUTB ethernet_card_io_base, 0x52, 0x00
 
 ;reset card
 BASE_OUTB ethernet_card_io_base, 0x37, 0x10
 WAIT 50
 BASE_OUTB ethernet_card_io_base, 0x37, 0x0
 
 ;read MAC
 BASE_INB ethernet_card_io_base, 0x0
 mov byte [ethernet_card_mac+5], al
 BASE_INB ethernet_card_io_base, 0x1
 mov byte [ethernet_card_mac+4], al
 BASE_INB ethernet_card_io_base, 0x2
 mov byte [ethernet_card_mac+3], al
 BASE_INB ethernet_card_io_base, 0x3
 mov byte [ethernet_card_mac+2], al
 BASE_INB ethernet_card_io_base, 0x4
 mov byte [ethernet_card_mac+1], al
 BASE_INB ethernet_card_io_base, 0x5
 mov byte [ethernet_card_mac+0], al
 
 ;enable interrupts
 BASE_OUTW ethernet_card_io_base, 0x3C, 0xFFFF
 
 ;init receiving packets
 BASE_OUTD ethernet_card_io_base, 0x44, 0x8F | (0x3 << 11) | (0x7 << 13) ;all packets to 64 KB buffer
 BASE_OUTD ethernet_card_io_base, 0x30, MEMORY_NIC+0x1000 ;receiving buffer
 
 ;init transmitting packets
 BASE_OUTD ethernet_card_io_base, 0x40, (3 << 24) | (7 << 8) ;max 2048 length
 
 ;start card
 BASE_OUTB ethernet_card_io_base, 0x37, 0xC
 
 mov edi, MEMORY_NIC+0x1000
 mov eax, 0
 mov ecx, 1024*64
 rep stosb
 
 ret
 
 .realtek_8169:
 ;reset card
 BASE_OUTB ethernet_card_io_base, 0x37, 0x10
 WAIT 50
 
 ;read MAC
 BASE_INB ethernet_card_io_base, 0x0
 mov byte [ethernet_card_mac+5], al
 BASE_INB ethernet_card_io_base, 0x1
 mov byte [ethernet_card_mac+4], al
 BASE_INB ethernet_card_io_base, 0x2
 mov byte [ethernet_card_mac+3], al
 BASE_INB ethernet_card_io_base, 0x3
 mov byte [ethernet_card_mac+2], al
 BASE_INB ethernet_card_io_base, 0x4
 mov byte [ethernet_card_mac+1], al
 BASE_INB ethernet_card_io_base, 0x5
 mov byte [ethernet_card_mac+0], al
 
 ;unlock registers
 BASE_OUTB ethernet_card_io_base, 0x50, 0xC0
 
 ;enable interrupts
 BASE_OUTW ethernet_card_io_base, 0x3C, 0xFFFF
 
 ;create rx descriptor
 mov dword [MEMORY_NIC+0x0], 0xC0003FFF
 mov dword [MEMORY_NIC+0x4], 0
 mov dword [MEMORY_NIC+0x8], MEMORY_NIC+0x1000
 mov dword [MEMORY_NIC+0xC], 0
 
 ;init receiving packets
 BASE_OUTD ethernet_card_io_base, 0x44, 0x8F | (7 << 8) | (0xE << 12)
 BASE_OUTB ethernet_card_io_base, 0x37, 0x4 ;enable receiving packets, must be set before configuring transmit
 BASE_OUTW ethernet_card_io_base, 0xDA, 0x1FFF ;max packet size
 BASE_OUTD ethernet_card_io_base, 0xE4, MEMORY_NIC+0 ;first rx descriptor
 
 ;create tx descriptor
 mov dword [MEMORY_NIC+0x100], 0x40000000
 mov dword [MEMORY_NIC+0x104], 0
 mov dword [MEMORY_NIC+0x108], 0
 mov dword [MEMORY_NIC+0x10C], 0
 
 ;init transmitting packets
 BASE_OUTB ethernet_card_io_base, 0xEC, 0x3B ;max packet size
 BASE_OUTD ethernet_card_io_base, 0x40, (3 << 24) | (7 << 8) ;max 2048 length
 BASE_OUTD ethernet_card_io_base, 0x20, MEMORY_NIC+0x100 ;first tx descriptor
 
 ;start card
 BASE_OUTB ethernet_card_io_base, 0x37, 0xC
 
 ;lock registers
 BASE_OUTW ethernet_card_io_base, 0x50, 0x0

 ret

nic_realtek_send_packet:
 cmp dword [nic_realtek_type], RTL_8169
 je .realtek_8169
 
 mov eax, dword [packet_pointer]
 mov dx, word [nic_realtek_send_port]
 add dx, word [ethernet_card_io_base]
 out dx, eax
 
 mov dx, word [nic_realtek_send_port]
 add dx, word [ethernet_card_io_base]
 sub dx, 0x10
 mov eax, dword [packet_length]
 out dx, eax
 
 add word [nic_realtek_send_port], 0x4
 cmp word [nic_realtek_send_port], 0x30
 je .base_port
 
 ret
 
 .base_port:
 mov word [nic_realtek_send_port], 0x20
 ret
 
 .realtek_8169:
 mov eax, dword [packet_length]
 and eax, 0x3FFF
 or eax, 0xF0000000
 mov dword [MEMORY_NIC+0x100], eax
 
 mov eax, dword [packet_pointer]
 mov dword [MEMORY_NIC+0x108], eax
 
 BASE_OUTB ethernet_card_io_base, 0x38, 0x40 ;send packet
 
 ret

nic_realtek_irq:
 mov dword [last_arrived_packet], 0
 
 mov eax, 0
 BASE_INW ethernet_card_io_base, 0x3E
 BASE_OUTW ethernet_card_io_base, 0x3E, ax
 
 .irq_type:
 test ax, 0x1
 jnz .receive
 test ax, 0x2
 jnz .restore_receive_descriptor
 test ax, 0x10
 jnz .receive
 
 test ax, 0x4
 jnz .transmit
 
 test ax, 0x20
 jnz .connection_change

 ret

 .receive:
 inc dword [number_of_received_packets]
 
 cmp dword [nic_realtek_type], RTL_8139
 jne .realtek_8169
 
 mov eax, dword [realtek_8139_receive_pointer]
 push eax
 add eax, 4
 mov dword [received_packet_pointer], eax
 
 mov eax, dword [realtek_8139_receive_pointer] ;length of packet
 mov ebx, 0
 mov bx, word [eax+2]
 push ebx
 add ebx, 7
 add dword [realtek_8139_receive_pointer], ebx
 and dword [realtek_8139_receive_pointer], 0xFFFFFFFC
 cmp dword [realtek_8139_receive_pointer], MEMORY_NIC+0x1000+0x10000
 jb .above_realtek_8139_buffer
  push eax
  mov eax, dword [realtek_8139_receive_pointer]
  sub eax, MEMORY_NIC+0x1000+0x10000
  mov ebx, eax
  add ebx, MEMORY_NIC+0x1000
  pop eax
  mov dword [realtek_8139_receive_pointer], ebx
 .above_realtek_8139_buffer:
 mov dword [eax], 0 ;delete previous packet header
 
 mov eax, dword [realtek_8139_receive_pointer]
 cmp dword [eax], 0
 jne .not_last_packet
  mov dword [last_arrived_packet], 1
 .not_last_packet:
 call ethernet_card_process_packet
 
 mov eax, dword [realtek_8139_receive_pointer]
 sub eax, MEMORY_NIC+0x1000
 cmp eax, 0
 je .write_capr
 sub eax, 0x10
 .write_capr:
 BASE_OUTW ethernet_card_io_base, 0x38, ax ;tell card what we already readed
 
 pop ecx
 pop edi
 mov eax, 0
 rep stosb
 
 cmp dword [last_arrived_packet], 1
 jne .receive

 cmp dword [realtek_8139_receive_pointer], MEMORY_NIC+0x1000+0x10000
 jb .done
  push eax
  mov eax, dword [realtek_8139_receive_pointer]
  sub eax, MEMORY_NIC+0x1000+0x10000
  mov ebx, eax
  add ebx, MEMORY_NIC+0x1000
  pop eax
  mov dword [realtek_8139_receive_pointer], ebx
 .done:
 ret
 
 .realtek_8169:
 mov dword [received_packet_pointer], MEMORY_NIC+0x1000
 call ethernet_card_process_packet 
 .restore_receive_descriptor:
 mov dword [MEMORY_NIC+0x0], 0xC0003FFF ;entry is available
 ret
 
 .transmit:
  inc dword [number_of_transmitted_packets]
 ret
 
 .connection_change:
 PSTR 'there was change on cable connection', constr
 ret
