;BleskOS

%define FM_COLOR_BORDER 0x9F6006
%define FM_COLOR_BACKGROUND 0xE28600

%macro FM_CLEAR_MESSAGE 0
 mov dword [cursor_line], 20
 mov dword [cursor_column], 120
 SCREEN_X_SUB eax, 120
 mov dword [square_length], eax
 SCREEN_Y_SUB eax, 40
 mov dword [square_heigth], eax
 mov dword [color], FM_COLOR_BACKGROUND
 call draw_square
 call redraw_screen
%endmacro

%macro FM_SHOW_MESSAGE 1
 mov dword [cursor_line], 20
 mov dword [cursor_column], 120
 SCREEN_X_SUB eax, 120
 mov dword [square_length], eax
 SCREEN_Y_SUB eax, 40
 mov dword [square_heigth], eax
 mov dword [color], FM_COLOR_BACKGROUND
 call draw_square

 mov dword [cursor_line], 30
 mov dword [cursor_column], 128
 mov dword [color], BLACK
 mov esi, %1
 call print
 call redraw_screen
%endmacro

%define FM_HDD 0
%define FM_CDROM 1
%define FM_USB1 2
%define FM_USB2 3
%define FM_USB3 4
%define FM_USB4 5

file_manager_up_str db 'File manager', 0
file_manager_down_str db '[d] Select device [F12] Remove USB [page up/down] Ten items up/down', 0
file_manager_selecting_str db 'You are selecting drive', 0
file_manager_reading_folder_str db 'Reading folder...', 0
file_manager_folder_error_str db 'Error during reading folder', 0
file_manager_no_usb_str db 'No USB device is connected here', 0
file_manager_uninitalized_device_str db 'Error occured during initalization of this device', 0
file_manager_not_fat32_str db 'This USB is not formatted as FAT32', 0
file_manager_no_disk_str db 'No disk is inserted [a] Eject drive', 0
file_manager_no_driver_str db 'Driver for filesystem ISO9690 is not currently written', 0

fm_free_item_str db 'Free item', 0
fm_folder_str db 'Folder', 0
fm_simple_text_str db 'Simple text', 0
item_date_str db '    /  /', 0
fm_item_str db 'Selected item:', 0

file_manager_device dd 0
file_manager_old_device dd 0
fm_folder_number dd 0
fm_path times 60 dd 0
fm_path_pointer times 6 dd 0

fm_folder_memory_pointer dd 0
fm_selected_item dd 0
fm_highlighted_item dd 0
fm_first_show_item dd 0

file_manager:
 DRAW_WINDOW file_manager_up_str, file_manager_down_str, FM_COLOR_BORDER, FM_COLOR_BACKGROUND
 mov dword [file_manager_device], FM_HDD
 call file_manager_redraw_devices
 call file_manager_show_folder
 call redraw_screen

 .file_manager_halt:
  call wait_for_keyboard

  cmp byte [key_code], KEY_UP
  je .fm_key_up

  cmp byte [key_code], KEY_DOWN
  je .fm_key_down

  cmp byte [key_code], KEY_PAGE_UP
  je .fm_key_up ;TODO

  cmp byte [key_code], KEY_PAGE_DOWN
  je .fm_key_down ;TODO

  cmp byte [key_code], KEY_D
  je .fm_key_d

  cmp byte [key_code], KEY_F12
  je .remove_usb

  cmp byte [key_code], KEY_ESC
  je main_window
 jmp .file_manager_halt

 ;;;;; move up ;;;;;
 .fm_key_up:
  cmp dword [fm_highlighted_item], 0
  jne .move_highlighted_up

  cmp dword [fm_selected_item], 0
  je .file_manager_halt

  dec dword [fm_selected_item]
  dec dword [fm_first_show_item]
  call fm_show_item_message
  call file_manager_redraw_file_zone
  call redraw_screen
  jmp .file_manager_halt

  .move_highlighted_up:
  dec dword [fm_highlighted_item]
  call file_manager_redraw_file_zone
  call redraw_screen
 jmp .file_manager_halt

 ;;;;; move down ;;;;;
 .fm_key_down:
  mov eax, dword [screen_y]
  sub eax, 60
  mov ebx, LINESZ
  mov edx, 0
  div ebx
  cmp dword [fm_highlighted_item], eax
  jne .move_highlighted_down

  cmp dword [fm_selected_item], 2047
  je .file_manager_halt

  inc dword [fm_selected_item]
  inc dword [fm_first_show_item]
  call fm_show_item_message
  call file_manager_redraw_file_zone
  call redraw_screen
  jmp .file_manager_halt

  .move_highlighted_down:
  inc dword [fm_highlighted_item]
  call file_manager_redraw_file_zone
  call redraw_screen
 jmp .file_manager_halt

 ;;;;; select device ;;;;;
 .fm_key_d:
  mov eax, dword [file_manager_device]
  mov dword [file_manager_old_device], eax

  FM_SHOW_MESSAGE file_manager_selecting_str

  .selecting_drive_halt:
   call wait_for_keyboard

   cmp byte [key_code], KEY_UP
   je .selecting_key_up

   cmp byte [key_code], KEY_DOWN
   je .selecting_key_down

   cmp byte [key_code], KEY_ESC
   je .selecting_esc

   cmp byte [key_code], KEY_ENTER
   je .drive_is_selected
  jmp .selecting_drive_halt

  .selecting_key_up:
   cmp dword [file_manager_device], 0
   je .selecting_drive_halt
   dec dword [file_manager_device]
   call file_manager_redraw_devices
   call redraw_screen
  jmp .selecting_drive_halt

  .selecting_key_down:
   cmp dword [file_manager_device], 5
   je .selecting_drive_halt
   inc dword [file_manager_device]
   call file_manager_redraw_devices
   call redraw_screen
  jmp .selecting_drive_halt

  .selecting_esc:
  mov eax, dword [file_manager_old_device]
  mov dword [file_manager_device], eax
  call file_manager_redraw_devices
  FM_CLEAR_MESSAGE
  ret

  .drive_is_selected:
  FM_CLEAR_MESSAGE
  cmp dword [file_manager_device], 2
  jl .if_usb
   mov eax, dword [file_manager_device]
   sub eax, 2
   mov dword [msd_number], eax
   call select_msd
   cmp byte [esi+7], MSD_FAT32
   jne .skip_fat_init
    call msd_read_mbr
    mov eax, dword [first_partition_lba]
    mov dword [fat_base_sector], eax
    call init_fat
   .skip_fat_init:
  .if_usb:

  mov eax, dword [file_manager_device]
  mov ebx, dword [fm_path_pointer+(eax*4)]
  mov dword [fm_folder_number], ebx
  call file_manager_show_folder
 jmp .file_manager_halt

 ;;;;; remove USB drive ;;;;;
 .remove_usb:
  cmp dword [file_manager_device], 2
  jl .file_manager_halt

  mov eax, dword [file_manager_device]
  sub eax, 2
  mov ebx, 16
  mul ebx
  add eax, mass_storage_devices
  mov dword [eax], 0
  mov dword [eax+4], 0
  mov dword [eax+8], 0
  mov dword [eax+12], 0

  mov eax, dword [file_manager_device]
  sub eax, 2
  mov ebx, 12
  mul ebx
  add eax, mass_storage_device_label
  mov dword [eax], 0
  mov dword [eax+4], 0
  mov dword [eax+8], 0

  call file_manager_redraw_devices
  call redraw_screen
 jmp .file_manager_halt

file_manager_show_folder:
 cmp dword [file_manager_device], 0
 je .hard_disk
 cmp dword [file_manager_device], 1
 je .cdrom
 cmp dword [file_manager_device], 2
 je .usb_1
 cmp dword [file_manager_device], 3
 je .usb_2
 cmp dword [file_manager_device], 4
 je .usb_3
 cmp dword [file_manager_device], 5
 je .usb_4
 ret

 .hard_disk:
 FM_SHOW_MESSAGE file_manager_reading_folder_str
 mov eax, dword [fm_folder_number]
 mov dword [jus_block_number], eax
 mov dword [ata_memory], MEMORY_FOLDER
 call jus_read_block ;load file from hard disk
 cmp dword [ata_status], ATA_ERROR
 je .reading_error

 mov dword [fm_highlighted_item], 0
 mov dword [fm_selected_item], 0
 mov dword [fm_first_show_item], 0
 call file_manager_redraw_file_zone
 call redraw_screen
 ret

 .cdrom:
 FM_SHOW_MESSAGE file_manager_no_driver_str
 ret

 .usb_1:
 cmp byte [mass_storage_devices+7], NO_MSD
 je .no_usb
 cmp byte [mass_storage_devices+7], MSD_UNINITALIZED
 je .usb_uninitalized
 cmp byte [mass_storage_devices+7], MSD_INITALIZED
 je .usb_not_fat32
 cmp dword [fm_folder_number], 0
 je .usb_root_folder
 jmp .usb

 .usb_2:
 cmp byte [mass_storage_devices+16+7], NO_MSD
 je .no_usb
 cmp byte [mass_storage_devices+16+7], MSD_UNINITALIZED
 je .usb_uninitalized
 cmp byte [mass_storage_devices+16+7], MSD_INITALIZED
 je .usb_not_fat32
 cmp dword [fm_folder_number], 0
 je .usb_root_folder
 jmp .usb

 .usb_3:
 cmp byte [mass_storage_devices+32+7], NO_MSD
 je .no_usb
 cmp byte [mass_storage_devices+32+7], MSD_UNINITALIZED
 je .usb_uninitalized
 cmp byte [mass_storage_devices+32+7], MSD_INITALIZED
 je .usb_not_fat32
 cmp dword [fm_folder_number], 0
 je .usb_root_folder
 jmp .usb

 .usb_4:
 cmp byte [mass_storage_devices+48+7], NO_MSD
 je .no_usb
 cmp byte [mass_storage_devices+48+7], MSD_UNINITALIZED
 je .usb_uninitalized
 cmp byte [mass_storage_devices+48+7], MSD_INITALIZED
 je .usb_not_fat32
 cmp dword [fm_folder_number], 0
 je .usb_root_folder
 jmp .usb

 .usb_root_folder:
 FM_SHOW_MESSAGE file_manager_reading_folder_str
 mov eax, dword [fat_root_dir_cluster]
 mov dword [fat_entry], eax
 mov dword [fat_memory], MEMORY_FAT32_FOLDER
 call fat_read_file
 cmp dword [msd_status], MSD_ERROR
 je .reading_error

 call convert_fat_folder_to_jus_folder
 call file_manager_redraw_file_zone
 call redraw_screen
 ret

 .usb:
 FM_SHOW_MESSAGE file_manager_reading_folder_str
 mov eax, dword [fm_folder_number]
 mov dword [fat_entry], eax
 mov dword [fat_memory], MEMORY_FAT32_FOLDER
 call fat_read_file
 cmp dword [msd_status], MSD_ERROR
 je .reading_error
 ret

 .reading_error:
 FM_SHOW_MESSAGE file_manager_folder_error_str
 ret

 .no_usb:
 FM_SHOW_MESSAGE file_manager_no_usb_str
 ret

 .usb_uninitalized:
 FM_SHOW_MESSAGE file_manager_uninitalized_device_str
 ret

 .usb_not_fat32:
 FM_SHOW_MESSAGE file_manager_not_fat32_str
 ret

;;;;;      REDRAW METHODS      ;;;;;
file_manager_redraw_devices:
 ;clear space
 mov dword [cursor_line], 20
 mov dword [cursor_column], 0
 mov dword [square_length], COLUMNSZ*15
 SCREEN_Y_SUB eax, 40
 mov dword [square_heigth], eax
 mov dword [color], 0xCC7000
 call draw_square

 ;draw background of selected device
 mov dword [cursor_column], 0
 mov eax, dword [file_manager_device]
 mov ebx, 20
 mul ebx
 add eax, 25
 mov dword [cursor_line], eax
 mov dword [square_length], COLUMNSZ*15
 mov dword [square_heigth], 20
 mov dword [color], 0xFFCF00
 call draw_square

 ;print strings of devices
 PRINT 'Hard disk', fm_hdd_str, 30+LINE(0), COLUMN(2)
 PRINT 'CDROM', fm_cdrom_str, 30+LINE(2), COLUMN(2)
 IF_NE byte [mass_storage_devices+(0*16)+7], MSD_FAT32, if_usb_1_not_fat
  PRINT 'USB 1', fm_usb1_str, 30+LINE(4), COLUMN(2)
 ENDIF if_usb_1_not_fat
 IF_NE byte [mass_storage_devices+(1*16)+7], MSD_FAT32, if_usb_2_not_fat
  PRINT 'USB 2', fm_usb2_str, 30+LINE(6), COLUMN(2)
 ENDIF if_usb_2_not_fat
 IF_NE byte [mass_storage_devices+(2*16)+7], MSD_FAT32, if_usb_3_not_fat
  PRINT 'USB 3', fm_usb3_str, 30+LINE(8), COLUMN(2)
 ENDIF if_usb_3_not_fat
 IF_NE byte [mass_storage_devices+(3*16)+7], MSD_FAT32, if_usb_4_not_fat
  PRINT 'USB 4', fm_usb4_str, 30+LINE(10), COLUMN(2)
 ENDIF if_usb_4_not_fat
 IF_E byte [mass_storage_devices+(0*16)+7], MSD_FAT32, if_usb_1_string
  mov dword [cursor_line], 30+LINE(4)
  mov dword [cursor_column], COLUMN(2)
  mov dword [color], BLACK
  mov esi, mass_storage_device_label
  call print
 ENDIF if_usb_1_string
 IF_E byte [mass_storage_devices+(1*16)+7], MSD_FAT32, if_usb_2_string
  mov dword [cursor_line], 30+LINE(6)
  mov dword [cursor_column], COLUMN(2)
  mov dword [color], BLACK
  mov esi, mass_storage_device_label+12
  call print
 ENDIF if_usb_2_string
 IF_E byte [mass_storage_devices+(2*16)+7], MSD_FAT32, if_usb_3_string
  mov dword [cursor_line], 30+LINE(8)
  mov dword [cursor_column], COLUMN(2)
  mov dword [color], BLACK
  mov esi, mass_storage_device_label+24
  call print
 ENDIF if_usb_3_string
 IF_E byte [mass_storage_devices+(3*16)+7], MSD_FAT32, if_usb_4_string
  mov dword [cursor_line], 30+LINE(10)
  mov dword [cursor_column], COLUMN(2)
  mov dword [color], BLACK
  mov esi, mass_storage_device_label+36
  call print
 ENDIF if_usb_4_string

 ret

file_manager_redraw_file_zone:
 mov dword [cursor_line], 20
 mov dword [cursor_column], 120
 SCREEN_X_SUB eax, 120
 mov dword [square_length], eax
 SCREEN_Y_SUB eax, 40
 mov dword [square_heigth], eax
 mov dword [color], 0xE28600
 call draw_square

 PRINT 'Name', name_str, 25, 130
 SCREEN_X_SUB eax, COLUMNSZ*35
 PRINT 'Type', type_str, 25, eax
 SCREEN_X_SUB eax, COLUMNSZ*25
 PRINT 'Date', date_str, 25, eax
 SCREEN_X_SUB eax, COLUMNSZ*14
 PRINT 'Size in KB', size_str, 25, eax

 mov eax, dword [fm_highlighted_item]
 mov ebx, LINESZ
 mul ebx
 add eax, 35
 mov dword [cursor_line], eax
 mov dword [cursor_column], 130
 SCREEN_X_SUB eax, 130+COLUMNSZ
 mov dword [square_length], eax
 mov dword [square_heigth], LINESZ
 mov dword [color], 0xFF0000
 call draw_square ;highlighted file

 mov esi, MEMORY_FOLDER
 mov eax, dword [fm_first_show_item]
 mov ebx, 64
 mul ebx
 add esi, eax ;pointer to first item

 mov eax, dword [screen_y]
 sub eax, 50
 mov ebx, LINESZ
 mov edx, 0
 div ebx
 mov ecx, eax ;number of lines
 mov dword [cursor_line], 35
 .print_item:
 push ecx
  mov ax, word [esi+14]
  cmp ax, 0
  je .free_item
  cmp ax, 1
  je .folder
  cmp ax, 2
  je .type_from_name
  jmp .show_item_info

  .free_item:
  SCREEN_X_SUB eax, COLUMNSZ*35
  mov dword [cursor_column], eax
  mov dword [color], BLACK
  push esi
  mov esi, fm_free_item_str
  call print
  pop esi
  jmp .next_item

  .folder:
  SCREEN_X_SUB eax, COLUMNSZ*35
  mov dword [cursor_column], eax
  mov dword [color], BLACK
  push esi
  mov esi, fm_folder_str
  call print
  pop esi
  jmp .show_item_info

  .type_from_name:
  SCREEN_X_SUB eax, COLUMNSZ*35
  mov dword [cursor_column], eax
  mov dword [color], BLACK
  mov byte [esi+59], 0
  push esi
  mov eax, esi
  add eax, 56
  mov esi, eax
  call print_ascii
  pop esi
  jmp .show_item_info

  .show_item_info:
  ;print size of file
  SCREEN_X_SUB eax, COLUMNSZ*14
  mov dword [cursor_column], eax
  mov ebx, dword [esi+4]
  mov dword [var_print_value], ebx
  mov dword [color], BLACK
  push esi
  call print_var
  pop esi

  ;print year of file
  SCREEN_X_SUB eax, COLUMNSZ*25
  mov dword [cursor_column], eax
  push esi
  mov esi, item_date_str
  call print
  pop esi
  SCREEN_X_SUB eax, COLUMNSZ*25
  mov dword [cursor_column], eax
  mov bx, word [esi+8] ;year
  mov dword [var_print_value], 0
  mov word [var_print_value], bx
  push esi
  call print_var
  pop esi

  ;print month of file
  SCREEN_X_SUB eax, COLUMNSZ*20
  mov dword [cursor_column], eax
  mov dword [var_print_value], 0
  mov al, byte [esi+10]
  and al, 0xF
  mov byte [var_print_value], al
  cmp al, 10
  jl .month_with_zero
  push esi
  call print_var
  pop esi
  jmp .day
  .month_with_zero:
  push esi
  mov dword [char_for_print], '0'
  call print_char
  add dword [cursor_column], COLUMNSZ
  call print_var
  add dword [cursor_column], COLUMNSZ
  pop esi

  ;print day of file
  .day:
  mov dword [var_print_value], 0
  mov al, byte [esi+11]
  and al, 0x1F
  mov byte [var_print_value], al
  cmp al, 10
  jl .day_with_zero
  push esi
  call print_var
  pop esi
  jmp .name
  .day_with_zero:
  push esi
  mov dword [char_for_print], '0'
  call print_char
  add dword [cursor_column], COLUMNSZ
  call print_var
  pop esi

  .name:
  mov dword [cursor_column], 130
  mov dword [color], BLACK
  mov byte [esi+63], 0
  push esi
  mov eax, esi
  add eax, 16
  mov esi, eax
  call print_ascii
  pop esi

 .next_item:
 add esi, 64
 add dword [cursor_line], LINESZ
 pop ecx
 dec ecx
 cmp ecx, 0
 jne .print_item

 ret

fm_show_item_message:
 FM_CLEAR_MESSAGE
 mov dword [cursor_line], 5
 SCREEN_X_SUB eax, COLUMNSZ*18
 mov dword [cursor_column], eax
 mov dword [color], BLACK
 push esi
 mov esi, fm_item_str
 call print
 pop esi
 SCREEN_X_SUB eax, COLUMNSZ*5
 mov dword [cursor_column], eax
 mov eax, dword [fm_selected_item]
 mov dword [var_print_value], eax
 push esi
 call print_var
 pop esi
 ret
