;BleskOS

pci_bus dd 0
pci_dev dd 0
pci_func dd 0
pci_offset dd 0
pci_return dd 0
pci_write_value dd 0

%define BAR0 0x10
%define BAR1 0x14
%define BAR2 0x18
%define BAR3 0x1C
%define BAR4 0x20
%define BAR5 0x24

%macro PCI_READ 1
 mov dword [pci_offset], %1
 call pci_read
%endmacro

%macro PCI_WRITE 2
 mov dword [pci_offset], %1
 mov dword [pci_write_value], %2
 call pci_write
%endmacro

%macro PCI_READ_DEVICE_ID 0
 mov dword [pci_offset], 0x00
 call pci_read
%endmacro

%macro PCI_IO_ENABLE_BUSMASTERING 0
 mov dword [pci_offset], 0x04
 call pci_read
 or eax, 0x5
 mov dword [pci_write_value], eax
 call pci_write
%endmacro

%macro PCI_MMIO_ENABLE_BUSMASTERING 0
 mov dword [pci_offset], 0x04
 call pci_read
 or eax, 0x6
 mov dword [pci_write_value], eax
 call pci_write
%endmacro

%macro PCI_READ_BAR_TYPE 1
 mov dword [pci_offset], %1
 call pci_read
 and eax, 0x1
%endmacro

%macro PCI_READ_MMIO_BAR 1
 mov dword [pci_offset], %1
 call pci_read
 and eax, 0xFFFFFFF0
%endmacro

%macro PCI_READ_IO_BAR 1
 mov dword [pci_offset], %1
 call pci_read
 and eax, 0x0000FFFC
%endmacro

%macro PCI_SET_IRQ 1
 mov dword [pci_offset], 0x3C
 mov dword [pci_write_value], %1
 call pci_write
%endmacro

pci_read:
 mov eax, 0x80000000

 mov ebx, dword [pci_bus]
 shl ebx, 16
 or eax, ebx
 mov ebx, dword [pci_dev]
 shl ebx, 11
 or eax, ebx
 mov ebx, dword [pci_func]
 shl ebx, 8
 or eax, ebx
 mov ebx, dword [pci_offset]
 or eax, ebx

 OUTD 0xCF8, eax
 IND 0xCFC
 mov dword [pci_return], eax

 ret

pci_write:
 mov eax, 0x80000000

 mov ebx, dword [pci_bus]
 shl ebx, 16
 or eax, ebx
 mov ebx, dword [pci_dev]
 shl ebx, 11
 or eax, ebx
 mov ebx, dword [pci_func]
 shl ebx, 8
 or eax, ebx
 mov ebx, dword [pci_offset]
 or eax, ebx

 OUTD 0xCF8, eax
 mov eax, dword [pci_write_value]
 OUTD 0xCFC, eax
 mov dword [pci_return], eax

 ret

pci_read_device:
 PCI_READ_DEVICE_ID
 cmp dword [pci_return], 0xFFFFFFFF ;no device
 je .done
 
 LOG 'bus '
 mov eax, dword [pci_bus]
 LOG_VAR eax
 LOG 'device '
 mov eax, dword [pci_dev]
 LOG_VAR eax
 LOG 'func '
 mov eax, dword [pci_func]
 LOG_VAR eax
 LOG 'PCI device: '

 PCI_READ 0x08
 and eax, 0xFFFFFF00 ;class, subclass, progif
 
 IF_E eax, 0x03000000, pci_graphic_card_if
  LOG 'Graphic card '
 
  PCI_READ_DEVICE_ID
  mov dword [graphic_card_id], eax
  LOG_HEX eax
  LOG 0xA
  
  PCI_READ_MMIO_BAR BAR0
  mov dword [graphic_card_base], eax
  
  ret
 ENDIF pci_graphic_card_if

 IF_E eax, 0x04010000, pci_ac97_if ;AC97
  LOG 'Sound card AC97', 0xA
  
  PCI_IO_ENABLE_BUSMASTERING
  PCI_READ_IO_BAR BAR0
  mov word [ac97_nam_base], ax
  PCI_READ_IO_BAR BAR1
  mov word [ac97_nabm_base], ax
  ret
 ENDIF pci_ac97_if

 IF_E eax, 0x04030000, pci_hda_if ;HD Audio
  LOG 'Sound card Intel HD Audio', 0xA
 
  PCI_MMIO_ENABLE_BUSMASTERING
  PCI_READ_MMIO_BAR BAR0
  mov dword [hda_base], eax
  PCI_WRITE 0x44, 0x00 ;set TD0
  ret
 ENDIF pci_hda_if

 IF_E eax, 0x0C031000, pci_ohci_if ;OHCI
  LOG 'OHCI controller', 0xA
  inc dword [ohci_num_of_controllers]

  PCI_READ_MMIO_BAR BAR0
  mov esi, dword [ohci_pointer]
  mov dword [esi], eax ;save base
  add dword [ohci_pointer], 4

  ret
 ENDIF pci_ohci_if

 IF_E eax, 0x0C030000, pci_uhci_if ;UHCI
  LOG 'UHCI controller', 0xA
  inc dword [uhci_num_of_controllers]

  PCI_WRITE 0xC0, 0x8F00 ;disable legacy support
  PCI_READ_IO_BAR BAR4
  mov esi, dword [uhci_pointer]
  mov word [esi], ax
  add dword [uhci_pointer], 2

  ret
 ENDIF pci_uhci_if

 IF_E eax, 0x0C032000, pci_ehci_if ;EHCI
  LOG 'EHCI controller', 0xA
  inc dword [ehci_num_of_controllers]

  PCI_READ_MMIO_BAR BAR0
  mov dword [ehci_base], eax

  mov esi, dword [ehci_pointer]
  mov dword [esi], eax ;save base
  add dword [ehci_pointer], 4

  ;disable legacy support
  add eax, 0x08
  mov ebx, dword [eax]
  shr ebx, 8
  and ebx, 0xFF ;get PCI extend register position
  PCI_WRITE ebx, (1 << 24)
  
  PCI_SET_IRQ 3

  ret
 ENDIF pci_ehci_if

 IF_E eax, 0x0C033000, pci_xhci_if ;xHCI
  LOG 'xHCI controller', 0xA
  inc dword [xhci_num_of_controllers]
  
  PCI_READ_MMIO_BAR BAR0
  mov esi, dword [xhci_pointer]
  mov dword [esi], eax ;save base
  add dword [xhci_pointer], 4
  
  ret
 ENDIF pci_xhci_if

 IF_E eax, 0x02000000, pci_nic_if ;Network card
  LOG 'Network card '
  
  PCI_READ_DEVICE_ID
  mov dword [ethernet_card_id], eax
  
  cmp eax, 0x813910EC ;realtek 8139
  jne .if_realtek_nic_8139
   LOG 'Realtek 8139', 0xA
   
   PCI_IO_ENABLE_BUSMASTERING
   PCI_READ_IO_BAR BAR0
   mov word [ethernet_card_io_base], ax
   mov dword [nic_realtek_type], 0
   jmp .nic_founded
  .if_realtek_nic_8139:
  
  ;realtek 8169
  cmp eax, 0x816110EC
  je .if_realtek_nic_8169
  cmp eax, 0x816810EC
  je .if_realtek_nic_8169
  cmp eax, 0x816910EC
  je .if_realtek_nic_8169
  cmp eax, 0xC1071259
  je .if_realtek_nic_8169
  cmp eax, 0x10321737
  je .if_realtek_nic_8169
  cmp eax, 0x011616EC
  je .if_realtek_nic_8169
  
  cmp ax, 0x8086 ;intel
  jne .if_intel_nic
   LOG 'Intel '
   mov eax, dword [ethernet_card_id]
   LOG_HEX eax
   LOG 0xA
   
   PCI_MMIO_ENABLE_BUSMASTERING
   PCI_READ_IO_BAR BAR0
   mov word [ethernet_card_io_base], ax
   PCI_READ_MMIO_BAR BAR0
   mov dword [ethernet_card_mmio_base], eax
   PCI_READ_BAR_TYPE BAR0
   mov dword [ethernet_card_bar_type], eax
   jmp .nic_founded
  .if_intel_nic:
  
  cmp ax, 0x1022 ;AMD
  jne .if_amd_nic
   LOG 'AMD PC-net III', 0xA
   
   PCI_IO_ENABLE_BUSMASTERING
   PCI_READ_IO_BAR BAR0
   mov word [ethernet_card_io_base], ax
   jmp .nic_founded
  .if_amd_nic:
  
  cmp ax, 0x14E4 ;broadcom
  jne .if_broadcom_nic
   LOG 'Broadcom ', 0xA
   mov eax, dword [ethernet_card_id]
   LOG_HEX eax
   LOG 0xA
   
   PCI_MMIO_ENABLE_BUSMASTERING
   PCI_READ_MMIO_BAR BAR0
   mov dword [ethernet_card_mmio_base], eax
   jmp .nic_founded
  .if_broadcom_nic:
  
  mov eax, dword [ethernet_card_id]
  LOG_HEX eax
  LOG 0xA
  
  ret
  
  .if_realtek_nic_8169:
  LOG 'Realtek 8169 compatibile '
  mov eax, dword [ethernet_card_id]
  LOG_HEX eax
  LOG 0xA
  
  PCI_IO_ENABLE_BUSMASTERING
  PCI_READ_IO_BAR BAR0
  mov dword [ethernet_card_io_base], eax
  mov dword [nic_realtek_type], 1
   
  .nic_founded:
  PCI_READ 0x3C
  mov byte [ethernet_card_irq_num], al
  and eax, 0xFF
  add eax, 32 ;skip ISR
  mov ebx, 8
  mul ebx
  add eax, idt
  
  ;create IDT entry
  mov word [eax], ethernet_card_irq
  mov word [eax+2], 0x0008
  mov word [eax+4], 0x8E00
  mov word [eax+6], (ethernet_card_irq - $$ + 0x10000) >> 16
  
  mov eax, 0
  mov al, byte [ethernet_card_irq_num]
  LOG 'Network card IRQ: '
  LOG_VAR eax
  LOG 0xA
  ret
 ENDIF pci_nic_if
 
 IF_E eax, 0x01060100, pci_serial_ata_if ;Serial ATA
  LOG 'Serial ATA controller', 0xA
 
  PCI_READ_MMIO_BAR BAR5
  cmp dword [eax+0x10], 0x00010200
  jl .detect_sata_type
  test dword [eax+0x24], 0x1
  jz .detect_sata_type ;BIOS ownership is not supported
  
  mov dword [eax+0x28], 0x2 ;take ownership
  mov dword [ticks], 0
  .wait_for_handoff:
   mov ebx, dword [eax+0x28]
   and ebx, 0x3
   cmp ebx, 0x2
   je .detect_sata_type
  cmp dword [ticks], 500
  jl .wait_for_handoff
  ret ;broken controller
  
  .detect_sata_type:
  test dword [eax+4], 0x80000000
  jz .ide_interface

  add eax, 0x100 ;first SATA port
  mov esi, dword [ahci_devices_pointer]
  mov ecx, 32
  .ahci_detect_devices:
   cmp dword [eax+0x24], 0xFFFFFFFF
   je .next_item
   cmp dword [eax+0x24], 0x00000000
   je .next_item
   
   mov dword [esi], eax ;save port number
   mov ebx, dword [eax+0x24]
   mov dword [esi+4], ebx ;save signature
   add esi, 8
   add dword [ahci_devices_pointer], 8
  .next_item:
  add eax, 0x80
  loop .ahci_detect_devices
  
  ret
 ENDIF pci_serial_ata_if

 mov ebx, eax
 and ebx, 0xFFFF0000 ;remove progif
 IF_E ebx, 0x01010000, pci_ide_if ;IDE controller
  LOG 'IDE controller', 0xA
  .ide_interface:
  mov edi, dword [ide_pointer]

  PCI_READ_IO_BAR BAR0
  cmp ax, 0
  je .if_first_controller_native
  cmp ax, 0x1F0
  je .if_first_controller_native
   mov word [edi], ax
   PCI_READ_IO_BAR BAR1
   mov word [edi+2], ax

   add edi, 20 ;next item
   add dword [ide_pointer], 20
  .if_first_controller_native:

  PCI_READ_IO_BAR BAR2
  cmp ax, 0
  je .if_second_controller_native
  cmp ax, 0x170
  je .if_second_controller_native
   mov word [edi], ax
   PCI_READ_IO_BAR BAR3
   mov word [edi+2], ax

   add dword [ide_pointer], 20 ;next item
  .if_second_controller_native:

  ret
 ENDIF pci_ide_if

 LOG_HEX eax ;class/subclass/progif
 LOG 0xA

 PCI_SET_IRQ 5 ;for all other devices

 .done:
 ret

scan_pci:
 LOG ' ', 0xA, 'PCI devices:', 0xA

 ;clear
 mov esi, native_ide_controllers
 mov ecx, 160
 .clear_ide:
  mov byte [esi], 0
  inc esi
 loop .clear_ide

 FOR_VAR 256, dword [pci_bus], pci_for1
  FOR_VAR 32, dword [pci_dev], pci_for2

   mov dword [pci_func], 0
   PCI_READ 0x0C
   and eax, 0x00800000
   IF_E eax, 0x00800000, pci_if1 ;is device multifunction?
    FOR_VAR 8, dword [pci_func], pci_for3
     call pci_read_device
    ENDFOR_VAR pci_for3, dword [pci_func]
    jmp .next_cycle ;skip code for singlefunction device
   ENDIF pci_if1

   ;for singlefunction devices
   call pci_read_device

   .next_cycle:
  ENDFOR_VAR pci_for2, dword [pci_dev]
 ENDFOR_VAR pci_for1, dword [pci_bus]

 ret
