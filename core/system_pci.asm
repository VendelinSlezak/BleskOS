;BleskOS

%define RTL8139 1
%define RTL8169 2
%define INTEL_E1000 3
%define AMD_PC_NET_II 4
%define AMD_PC_NET_III 5

%define OHCI 1
%define UHCI 2
%define EHCI 3
%define XHCI 4

read_pci:
  mov eax, 0x80000000

  mov ecx, dword [pci_bus]
  shl ecx, 16
  or eax, ecx
  mov ecx, dword [pci_dev]
  shl ecx, 11
  or eax, ecx
  mov ecx, dword [pci_func]
  shl ecx, 8
  or eax, ecx
  mov ecx, dword [pci_offset]
  or eax, ecx

  OUTL 0x0CF8, eax

  INL 0x0CFC
  mov dword [pci_return], eax

  ret

write_pci:
  mov eax, 0x80000000

  mov ecx, dword [pci_bus]
  shl ecx, 16
  or eax, ecx
  mov ecx, dword [pci_dev]
  shl ecx, 11
  or eax, ecx
  mov ecx, dword [pci_func]
  shl ecx, 8
  or eax, ecx
  mov ecx, dword [pci_offset]
  or eax, ecx

  OUTL 0x0CF8, eax

  mov eax, dword [pci_write]
  OUTL 0x0CFC, eax

  ret

read_device:
  mov dword [pci_offset], 8
  call read_pci
  and dword [pci_return], 0xFFFFFF00 ;get class, subclass, progif

  cmp dword [pci_return], 0x04030000 ;Sound card - probably Intel HD Audio
  jne .endif_hda
    mov dword [pci_offset], 0x10
    call read_pci ;bar 0
    mov eax, dword [pci_return]
    and eax, 0xFFFFFFF0 ;get memory
    mov dword [hda_mem], eax

    mov dword [pci_offset], 0x3C
    call read_pci ;irq
    and dword [pci_return], 0x000000FF ;get irq
    mov al, byte [pci_return]
    mov byte [sound_card_irq], al

    mov dword [pci_offset], 0x00
    call read_pci ;vendor and device id
    mov eax, dword [pci_return]
    mov dword [sound_card_info], eax

    ret
  .endif_hda:

  cmp dword [pci_return], 0x02000000 ;Network card
  jne .endif_network_card
    mov dword [pci_offset], 0x3C ;get irq
    call read_pci
    and dword [pci_return], 0x000000FF
    mov al, byte [pci_return]
    mov byte [internet_card_irq], al

    mov dword [pci_offset], 0
    call read_pci ;get vendorid and deviceid
    mov eax, dword [pci_return]
    mov dword [internet_card_id], eax

    cmp dword [pci_return], 0x813910EC ;RTL8139
    jne .endif_rtl8139
      mov dword [internet_card_info], RTL8139

      mov dword [pci_offset], 0x10
      call read_pci ;bar 0
      and dword [pci_return], 0x0000FFFC ;get base port
      mov ax, word [pci_return]
      mov word [rtl8139_base], ax

      ret
    .endif_rtl8139:

    cmp dword [pci_return], 0x816910EC ;RTL8169
    jne .endif_rtl8169
      mov dword [internet_card_info], RTL8169

      ret
    .endif_rtl8169:

    cmp dword [pci_return], 0x100E8086 ;intel e1000
    jne .endif_e1000
      mov dword [internet_card_info], INTEL_E1000

      ret
    .endif_e1000:

    cmp dword [pci_return], 0x20001022 ;AMD PC NET II
    jne .endif_amdpcnetii
      mov dword [internet_card_info], AMD_PC_NET_II

      ret
    .endif_amdpcnetii:

    cmp dword [pci_return], 0x20011022 ;AMD PC NET III
    jne .endif_amdpcnetiii
      mov dword [internet_card_info], AMD_PC_NET_III

      ret
    .endif_amdpcnetiii:
  .endif_network_card:

  cmp dword [pci_return], 0x0C031000 ;OHCI
  jne .endif_ohci
    mov dword [pci_offset], 0x10
    call read_pci ;bar 0
    mov eax, dword [pci_return]
    and eax, 0xFFFFFFF0 ;get memory base

    inc dword [ohci_number_of_ports]
    mov ebx, dword [usb_port_number]
    add ebx, usb_port_type
    mov dword [ebx], OHCI
    add ebx, 40 ;usb_port_base
    mov dword [ebx], eax

    add dword [usb_port_number], 4

    ret
  .endif_ohci:

  cmp dword [pci_return], 0x0C030000 ;UHCI
  jne .endif_uhci
    mov dword [pci_offset], 0xC0
    mov dword [pci_write], 0x8F00
    call write_pci ;disable legacy support

    mov dword [pci_offset], 0x04
    call read_pci
    or dword [pci_return], 0x5
    mov eax, dword [pci_return]
    mov dword [pci_write], eax
    call write_pci

    mov dword [pci_offset], 0x20
    call read_pci ;bar 4
    mov eax, dword [pci_return]
    and eax, 0x0000FFFC ;get port base

    inc dword [uhci_number_of_ports]
    mov ebx, dword [usb_port_number]
    add ebx, usb_port_type
    mov dword [ebx], UHCI
    add ebx, 40 ;usb_port_base
    mov dword [ebx], eax

    add dword [usb_port_number], 4

    ret
  .endif_uhci:

  cmp dword [pci_return], 0x0C032000 ;EHCI
  jne .endif_ehci
    inc dword [ehci_number_of_ports]

    ret
  .endif_ehci:

  cmp dword [pci_return], 0x0C033000 ;xHCI
  jne .endif_xhci
    inc dword [xhci_number_of_ports]

    ret
  .endif_xhci:

  ret

scan_pci:
  mov dword [usb_port_number], 0
  mov dword [ohci_number_of_ports], 0
  mov dword [uhci_number_of_ports], 0
  mov dword [ehci_number_of_ports], 0
  mov dword [xhci_number_of_ports], 0

  mov dword [pci_bus], 0
  mov dword [pci_dev], 0
  mov dword [pci_func], 0

  mov ecx, 256 ;cycle for incrementing bus
  .cycle_bus:
  push ecx
    mov dword [pci_dev], 0

    mov ecx, 32 ;cycle for incrementing device
    .cycle_dev:
    push ecx
      mov dword [pci_offset], 12
      call read_pci
      and dword [pci_return], 0x00800000
      cmp dword [pci_return], 0x00800000 ;is device multifunction?
      jne .singlefunction_device
        mov ecx, 8
        .cycle_func:
        push ecx
          call read_device
          inc dword [pci_func]
        pop ecx
        loop .cycle_func
        mov dword [pci_func], 0
        jmp .next_cycle

      .singlefunction_device:
      call read_device

      .next_cycle:
      inc dword [pci_dev]
    pop ecx
    loop .cycle_dev

    inc dword [pci_bus]
  pop ecx
  loop .cycle_bus

  ret

pci_bus dd 0
pci_dev dd 0
pci_func dd 0
pci_offset dd 0
pci_return dd 0
pci_write dd 0

sound_card_info dd 0
sound_card_irq db 0
hda_mem dd 0
internet_card_id dd 0
internet_card_info dd 0
internet_card_irq db 0
rtl8139_base dw 0
ohci_number_of_ports db 0
uhci_number_of_ports db 0
ehci_number_of_ports db 0
xhci_number_of_ports db 0

usb_port_type times 10 dd 0
usb_port_base times 10 dd 0
usb_port_number dd 0
