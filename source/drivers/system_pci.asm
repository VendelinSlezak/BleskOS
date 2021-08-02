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

 PCI_READ 0x08
 and eax, 0xFFFFFF00 ;class, subclass, progif
 
 IF_E eax, 0x04010000, pci_ac97_if ;AC97
  PCI_IO_ENABLE_BUSMASTERING
  PCI_READ_IO_BAR BAR0
  mov word [ac97_nam_base], ax
  PCI_READ_IO_BAR BAR1
  mov word [ac97_nabm_base], ax
  ret
 ENDIF pci_ac97_if

 IF_E eax, 0x04030000, pci_hda_if ;HD Audio
  PCI_MMIO_ENABLE_BUSMASTERING
  PCI_READ_MMIO_BAR BAR0
  mov dword [hda_base], eax
  PCI_WRITE 0x44, 0x00 ;set TD0
  ret
 ENDIF pci_hda_if

 IF_E eax, 0x0C031000, pci_ohci_if ;OHCI
  inc dword [ohci_num_of_ports]
  ret
 ENDIF pci_ohci_if

 IF_E eax, 0x0C030000, pci_uhci_if ;UHCI
  inc dword [uhci_num_of_ports]

  PCI_WRITE 0xC0, 0x8F00 ;disable legacy support
  PCI_READ_IO_BAR BAR4
  mov esi, dword [uhci_pointer]
  mov word [esi], ax
  add dword [uhci_pointer], 2

  ret
 ENDIF pci_uhci_if

 IF_E eax, 0x0C032000, pci_ehci_if ;EHCI
  inc dword [ehci_num_of_ports]

  PCI_READ_MMIO_BAR BAR0
  mov dword [ehci_base], eax

  ;disable legacy support
  add eax, 0x08
  mov ebx, dword [eax]
  shr ebx, 8
  and ebx, 0xFF ;get PCI extend register position
  PCI_WRITE ebx, (1 << 24)

  ret
 ENDIF pci_ehci_if

 IF_E eax, 0x0C033000, pci_xhci_if ;xHCI
  inc dword [xhci_num_of_ports]
  ret
 ENDIF pci_xhci_if

 IF_E eax, 0x02000000, pci_nic_if ;Network card
  PCI_READ_DEVICE_ID
  mov dword [ethernet_card_id], eax
  ret
 ENDIF pci_nic_if

 mov ebx, eax
 and ebx, 0xFFFF0000 ;remove progif
 IF_E ebx, 0x01010000, pci_ide_if ;IDE controller
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

 PCI_SET_IRQ 3 ;for all other devices

 .done:
 ret

scan_pci:
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
