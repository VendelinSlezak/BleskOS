;BleskOS

%define DP_IDE_COLOR 0x9F60

dp_ide_drive db 0

dp_ide:
 CLEAR_SCREEN DP_IDE_COLOR

 mov dword [color], BLACK
 mov dword [cursor_line], LINE(1)

 mov esi, ide_controllers
 mov ecx, 10
 .print_table:
 push ecx
  mov dword [cursor_column], COLUMN(1)
  mov eax, dword [esi]
  mov dword [hex_print_value], eax
  push esi
  call print_hex
  pop esi

  mov dword [cursor_column], COLUMN(12)
  mov eax, dword [esi+4]
  mov dword [hex_print_value], eax
  push esi
  call print_hex
  pop esi

  mov dword [cursor_column], COLUMN(23)
  mov eax, dword [esi+8]
  mov dword [hex_print_value], eax
  push esi
  call print_hex
  pop esi

  mov dword [cursor_column], COLUMN(34)
  mov eax, dword [esi+12]
  mov dword [hex_print_value], eax
  push esi
  call print_hex
  pop esi

  mov dword [cursor_column], COLUMN(45)
  mov eax, dword [esi+16]
  mov dword [hex_print_value], eax
  push esi
  call print_hex
  pop esi

  add esi, 20
  add dword [cursor_line], LINESZ
 pop ecx
 dec ecx
 cmp ecx, 0
 jne .print_table

 PRINT '[a] Eject cdrom', commands_str, LINE(12), COLUMN(1)

 call redraw_screen

 .dp_ide_halt:
  call wait_for_usb_keyboard

  cmp byte [key_code], KEY_ESC
  je developer_zone

  cmp byte [key_code], KEY_A
  je .key_a
 jmp .dp_ide_halt

 .key_a:
  mov ax, word [cdrom_base]
  mov word [patapi_base], ax

  cmp dword [cdrom_drive], IDE_MASTER
  je .master

  cmp dword [cdrom_drive], IDE_SLAVE
  je .slave

  jmp .dp_ide_halt

  .master:
  call patapi_select_master
  call patapi_eject_drive
  jmp .dp_ide_halt

  .slave:
  call patapi_select_slave
  call patapi_eject_drive
 jmp .dp_ide_halt
