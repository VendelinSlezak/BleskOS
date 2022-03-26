;BleskOS

ethernet_card_id dd 0
ethernet_card_irq_num db 0
ethernet_card_bar_type dd 0
ethernet_card_io_base dw 0
ethernet_card_mmio_base dd 0
ethernet_card_mac times 6 db 0

packet_pointer dd 0
packet_length dd 0
received_packet_pointer dd 0
received_packet_length dd 0

number_of_received_packets dd 0
number_of_transmitted_packets dd 0

init_ethernet_card:
 cmp dword [ethernet_card_id], 0
 je .done
 
 mov dword [number_of_received_packets], 0

 cmp word [ethernet_card_id], 0x8086
 je init_nic_intel
 
 cmp word [ethernet_card_id], 0x1022
 je init_nic_amd
 
 .done:
 ret
 
nic_send_packet:
 cmp word [ethernet_card_id], 0x8086
 je nic_intel_send_packet
 
 cmp word [ethernet_card_id], 0x1022
 je nic_amd_send_packet
 
 ret

ethernet_card_irq:
 pusha
 
 cmp word [ethernet_card_id], 0x8086
 jne .if_intel_nic
  call nic_intel_irq
  jmp .done
 .if_intel_nic:
 
 cmp word [ethernet_card_id], 0x1022
 jne .if_amd_nic
  call nic_amd_irq
  jmp .done
 .if_amd_nic:
 
 .done:
 popa
 cmp byte [ethernet_card_irq_num], 7
 ja .slave
 EOI_MASTER_PIC
 iret
 
 .slave:
 EOI_SLAVE_PIC
 iret
