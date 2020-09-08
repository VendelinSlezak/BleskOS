;BleskOS

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

read_device:
  mov dword [pci_offset], 8
  call read_pci
  and dword [pci_return], 0xFFFFFF00 ;get class, subclass, progif

  cmp dword [pci_return], 0x04030000 ;Intel HD Audio
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
    mov byte [hda_irq], al

    ret
  .endif_hda:

  cmp dword [pci_return], 0x02000000 ;Network card
  jne .endif_network_card
    mov dword [pci_offset], 0
    call read_pci ;get vendorid and deviceid

    cmp dword [pci_return], 0x813910EC ;RTL8139
    jne .endif_rtl8139
      mov dword [pci_offset], 0x10
      call read_pci ;bar 0
      and dword [pci_return], 0x0000FFFC ;get base port
      mov ax, word [pci_return]
      mov word [rtl8139_base], ax

      mov dword [pci_offset], 0x3C
      call read_pci ;irq
      and dword [pci_return], 0x000000FF ;get irq
      mov al, byte [pci_return]
      mov byte [rtl8139_irq], al

      ret
    .endif_rtl8139:
  .endif_network_card:

  ret

scan_pci:
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

hda_mem dd 0
hda_irq db 0
rtl8139_base dw 0
rtl8139_irq db 0
