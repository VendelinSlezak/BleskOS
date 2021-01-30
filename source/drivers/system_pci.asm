;BleskOS

pci_bus dd 0
pci_dev dd 0
pci_func dd 0
pci_offset dd 0
pci_return dd 0
pci_write_value dd 0

%define BAR0 0x10
%define BAR1 0x14
%define BAR4 0x20

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

 IF_E eax, 0x04030000, pci_hda_if ;Intel HD Audio
  PCI_MMIO_ENABLE_BUSMASTERING
  ret
 ENDIF pci_hda_if

 IF_E eax, 0x0C031000, pci_ohci_if ;OHCI
  inc dword [ohci_num_of_ports]
  PCI_READ_MMIO_BAR BAR0
  add eax, 0x100 ;legacy control register
  mov dword [eax], 0x9 ;enable legacy support
  ret
 ENDIF pci_ohci_if

 IF_E eax, 0x0C030000, pci_uhci_if ;UHCI
  inc dword [uhci_num_of_ports]
  ret
 ENDIF pci_uhci_if

 IF_E eax, 0x0C032000, pci_ehci_if ;EHCI
  inc dword [ehci_num_of_ports]
  ret
 ENDIF pci_ehci_if

 IF_E eax, 0x0C033000, pci_xhci_if ;xHCI
  inc dword [xhci_num_of_ports]
  ret
 ENDIF pci_xhci_if

 IF_E eax, 0x02000000, pci_nic_if ;Network card
  ret
 ENDIF pci_nic_if

 PCI_SET_IRQ 3 ;for all other devices

 .done:
 ret

scan_pci:
 mov word [ac97_nam_base], 0
 mov word [ac97_nabm_base], 0

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
