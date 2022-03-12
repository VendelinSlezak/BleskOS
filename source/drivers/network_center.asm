;BleskOS

ethernet_card_id dd 0
ethernet_card_irq_num db 0
ethernet_card_io_base dw 0
ethernet_card_mmio_base dd 0
ethernet_card_mac times 6 db 0

packet_pointer dd 0
packet_length dd 0

init_ethernet_card:
 cmp dword [ethernet_card_id], 0
 je .done
 
 ;;cmp word [ethernet_card_id], 0x1022
 ;;je init_nic_amd
 
 .done:
 ret
 
nic_send_packet:
 ;;cmp word [ethernet_card_id], 0x1022
 ;;je nic_amd_send_packet
 
 ret

ethernet_card_irq:
 cmp word [ethernet_card_id], 0x1022
 jne .if_amd_nic
  ;;call nic_amd_irq
  jmp .done
 .if_amd_nic:
 
 .done:
 cmp byte [ethernet_card_irq_num], 7
 jg .slave
 EOI_MASTER_PIC
 iret
 
 .slave:
 EOI_SLAVE_PIC
 iret
