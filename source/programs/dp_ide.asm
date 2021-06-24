;BleskOS

%define DP_IDE_COLOR 0x35A1
%define MASTER 1
%define SLAVE 2

dp_ide_drive db 0

dp_ide:
 CLEAR_SCREEN DP_IDE_COLOR

 PRINT 'Selected controller:', controller_str, LINE(1), COLUMN(1)
 mov eax, 0
 mov ax, word [pata_base]
 PRINT_HEX eax, LINE(1), COLUMN(22)

 PRINT 'Selected drive:', drive_str, LINE(3), COLUMN(1)
 cmp byte [dp_ide_drive], SLAVE
 je .slave
 PRINT 'Master', master_str, LINE(3), COLUMN(17)
 jmp .info
 .slave:
 PRINT 'Slave', slave_str, LINE(3), COLUMN(17)

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
 mov eax, dword [pata_info+200]
 mov ebx, 2000
 mov edx, 0
 div ebx ;calculate MB
 PRINT_VAR eax, LINE(5), COLUMN(20)
 PRINT '[a] 0x1F0 [b] 0x170 [c] master [d] slave [F1] read info', keys_str, LINE(7), COLUMN(1)

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

  cmp byte [key_code], KEY_F1
  je .key_f1
 jmp .dp_ide_halt

 .key_a:
  mov word [pata_base], 0x1F0
 jmp dp_ide

 .key_b:
  mov word [pata_base], 0x170
 jmp dp_ide

 .key_c:
  call pata_select_master
  mov byte [dp_ide_drive], MASTER
 jmp dp_ide

 .key_d:
  call pata_select_slave
  mov byte [dp_ide_drive], SLAVE
 jmp dp_ide

 .key_f1:
  call pata_detect_drive
 jmp dp_ide
