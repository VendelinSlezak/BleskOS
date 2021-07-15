;BleskOS

%define DP_IDE_COLOR 0x9F60

dp_ide_drive db 0

dp_ide:
 CLEAR_SCREEN DP_IDE_COLOR

 PRINT 'Selected controller:', controller_str, LINE(1), COLUMN(1)
 mov eax, 0
 mov ax, word [pata_base]
 PRINT_HEX eax, LINE(1), COLUMN(22)

 PRINT 'Selected drive: Master', drive_str, LINE(3), COLUMN(1)

 .info:
 PRINT 'Type:       Size in MB:', type_str, LINE(5), COLUMN(1)
 mov ax, word [pata_info]
 and ax, 0x80
 cmp ax, 0
 jne .atapi
 PRINT 'ATA', ata_str, LINE(5), COLUMN(7)
 jmp .size
 .atapi:
 PRINT 'ATAPI', atapi_str, LINE(5), COLUMN(7)
 .size:
 mov eax, dword [pata_size]
 mov ebx, 2000
 mov edx, 0
 div ebx ;calculate MB
 PRINT_VAR eax, LINE(5), COLUMN(25)

 PRINT '[a] 0x1F0 [b] 0x170 [c] native first [d] native second', keys_str, LINE(7), COLUMN(1)

 cmp dword [ata_status], ATA_OK
 je .redraw
 PRINT 'Not responding', not_responding, LINE(9), COLUMN(1)
 .redraw:
 call redraw_screen

 .dp_ide_halt:
  call wait_for_keyboard

  cmp byte [key_code], KEY_ESC
  je developer_zone

  cmp byte [key_code], KEY_A
  je .key_a

  cmp byte [key_code], KEY_B
  je .key_b

  cmp byte [key_code], KEY_C
  je .key_c

  cmp byte [key_code], KEY_D
  je .key_d
 jmp .dp_ide_halt

 .key_a:
  mov word [pata_base], 0x1F0
  call pata_select_master
  call pata_detect_drive
 jmp dp_ide

 .key_b:
  mov word [pata_base], 0x170
  call pata_select_master
  call pata_detect_drive
 jmp dp_ide

 .key_c:
  mov ax, word [native_ide_controllers]
  cmp ax, 0
  je .dp_ide_halt
  mov word [pata_base], ax
  call pata_select_master
  call pata_detect_drive
 jmp dp_ide

 .key_d:
  mov ax, word [native_ide_controllers+20]
  cmp ax, 0
  je .dp_ide_halt
  mov word [pata_base], ax
  call pata_select_master
  call pata_detect_drive
 jmp dp_ide
