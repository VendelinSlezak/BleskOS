;BleskOS

%define FM_COLOR_BORDER 0x9F6006
%define FM_COLOR_BACKGROUND 0xE28600

%define FM_HDD 0
%define FM_CDROM 1
%define FM_USB1 2
%define FM_USB2 3
%define FM_USB3 4
%define FM_USB4 5

%define FM_NORMAL 1
%define FM_OPEN_DIALOG 2
%define FM_SAVE_DIALOG 3

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

file_manager_up_str db 'File manager', 0
file_manager_down_str db '[d] Select device [F12] Remove USB [b] Previous folder', 0
file_manager_selecting_str db 'You are selecting drive', 0

file_manager_reading_folder_str db 'Reading folder...', 0
file_manager_writing_folder_str db 'Writing folder...', 0
file_manager_folder_reading_error_str db 'Error during reading folder', 0
file_manager_folder_writing_error_str db 'Error during writing folder', 0
file_manager_reading_file_str db 'Reading file...', 0
file_manager_writing_file_str db 'Writing file...', 0
file_manager_file_reading_error_str db 'Error during reading file', 0
file_manager_file_writing_error_str db 'Error during writing file', 0
file_manager_memory_error_str db 'Not enough free memory for file', 0
file_manager_10_folders_error_str db 'Can not open deeper than 10 folders', 0

file_manager_no_usb_str db 'No USB device is connected here', 0
file_manager_uninitalized_device_str db 'Error occured during initalization of this device', 0
file_manager_not_fat32_str db 'This USB is not formatted as FAT32', 0
file_manager_no_disk_str db 'No disk is inserted [a] Eject drive [b] Detect disk', 0
file_manager_no_disk_driver_str db 'This disk is not formatted as ISO9660', 0
fm_free_item_str db 'Free item', 0
fm_folder_str db 'Folder', 0
item_date_str db '    /  /', 0
fm_item_str db 'Selected item:', 0
file_manager_create_name_str db 'Name:', 0

file_manager_device dd 0
file_manager_old_device dd 0
fm_folder_number dd 0
fm_path times 6*10 dd 0
fm_path_pointer times 6 dd 0

fm_selected_item dd 0
fm_highlighted_item dd 0
fm_first_show_item dd 0
fm_last_item dd 0
fm_number_of_items_on_screen dd 0

fm_file_size dd 0
fm_file_block dd 0
fm_file_memory dd 0

file_manager_interface_type dd 0

file_manager:
 ;calculate how many items is visible on actual resoultion of screen
 mov eax, dword [screen_y]
 sub eax, 60
 mov ebx, LINESZ
 mov edx, 0
 div ebx
 mov dword [fm_number_of_items_on_screen], eax

 cmp dword [file_manager_interface_type], FM_NORMAL ;file manager from main board
 je .file_manager_normal_interface
 ret

 .file_manager_normal_interface:
 DRAW_WINDOW file_manager_up_str, file_manager_down_str, FM_COLOR_BORDER, FM_COLOR_BACKGROUND
 call file_manager_redraw_devices
 call file_manager_show_folder
 call fm_show_selected_item_number
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

  cmp byte [key_code], KEY_ENTER
  je .fm_key_enter

  cmp byte [key_code], KEY_B
  je .fm_key_b

  cmp byte [key_code], KEY_C
  je .fm_key_c

  cmp byte [key_code], KEY_D
  je .fm_key_d

  cmp byte [key_code], KEY_F12
  je .remove_usb

  cmp byte [key_code], KEY_ESC
  je main_window
 jmp .file_manager_halt

 ;;;;; move up ;;;;;
 .fm_key_up:
  cmp dword [fm_highlighted_item], 0 ;we are on top of screen
  jne .move_highlighted_up

  cmp dword [fm_selected_item], 0 ;we are on top of folder
  je .file_manager_halt

  ;move whole screen
  dec dword [fm_selected_item]
  dec dword [fm_first_show_item]
  call fm_show_selected_item_number
  call file_manager_redraw_file_zone
  call redraw_screen
  jmp .file_manager_halt

  ;move highlighted line
  .move_highlighted_up:
  dec dword [fm_highlighted_item]
  dec dword [fm_selected_item]

  call fm_show_selected_item_number
  call file_manager_redraw_file_zone
  call redraw_screen
  jmp .file_manager_halt

 ;;;;; move down ;;;;;
 .fm_key_down:
  mov eax, dword [fm_number_of_items_on_screen]
  cmp dword [fm_highlighted_item], eax ;we are on bottom of screen
  jne .move_highlighted_down

  cmp dword [fm_selected_item], 2047 ;we are on bottom of folder
  je .file_manager_halt

  ;move whole screen
  inc dword [fm_selected_item]
  inc dword [fm_first_show_item]
  call fm_show_selected_item_number
  call file_manager_redraw_file_zone
  call redraw_screen
  jmp .file_manager_halt

  ;move highlighted line
  .move_highlighted_down:
  inc dword [fm_highlighted_item]
  inc dword [fm_selected_item]
  call fm_show_selected_item_number
  call file_manager_redraw_file_zone
  call redraw_screen
  jmp .file_manager_halt

 ;;;;; open ;;;;;
 .fm_key_enter:
  mov eax, dword [fm_selected_item]
  mov ebx, 128
  mul ebx
  add eax, MEMORY_FOLDER
  mov esi, eax ;get position of item memory

  cmp word [esi+14], 0 ;free item
  je .file_manager_halt

  cmp word [esi+14], 1
  je .open_folder

  ;TODO open other files
  jmp .open_text_file
 jmp .file_manager_halt

 .open_folder:
  mov ebx, dword [file_manager_device]
  add dword [fm_path_pointer+(ebx*4)], 4 ;set pointer to next item
  mov edx, dword [fm_path_pointer+(ebx*4)]
  cmp edx, 40
  jg .10_folders ;can not open deeper that 10 folders

  mov ecx, dword [fm_path_pointer+(ebx*4)]
  mov eax, dword [file_manager_device]
  mov ebx, 40
  mul ebx
  add eax, fm_path
  add ecx, eax

  mov eax, dword [esi]
  mov dword [ecx], eax ;save folder number
  mov dword [fm_folder_number], eax
  call file_manager_show_folder ;this read folder from device and show it
 jmp .file_manager_halt

 .10_folders:
 FM_SHOW_MESSAGE file_manager_10_folders_error_str
 jmp .file_manager_halt
 
 .open_text_file:
  mov eax, dword [esi]
  mov dword [fm_file_block], eax
  mov eax, dword [esi+4]
  mov dword [fm_file_size], eax
  call file_manager_read_file
  call text_editor_convert_to_unicode
 jmp text_editor

 ;;;;; go to previous folder ;;;;;
 .fm_key_b:
  mov ebx, dword [file_manager_device]
  mov edx, dword [fm_path_pointer+(ebx*4)]
  cmp edx, 0
  je .file_manager_halt ;we are in root folder
  sub dword [fm_path_pointer+(ebx*4)], 4 ;set pointer to previous item

  mov ecx, dword [fm_path_pointer+(ebx*4)]
  mov eax, dword [file_manager_device]
  mov ebx, 40
  mul ebx
  add eax, fm_path
  add ecx, eax

  mov eax, dword [ecx]
  mov dword [fm_folder_number], eax
  call file_manager_show_folder ;this read folder from device and show it
 jmp .file_manager_halt

 ;;;;; TODO create folder ;;;;;
 .fm_key_c:
  mov edi, MEMORY_FOLDER
  mov ecx, 2046
  .find_free_entry_for_folder:
   cmp dword [edi], 0
   je .create_folder_item
   add edi, 128
  loop .find_free_entry_for_folder
  jmp .file_manager_halt

  .create_folder_item:
  ;draw dialog window
  push edi
  mov eax, dword [screen_y_center]
  sub eax, 15
  mov dword [cursor_line], eax
  mov eax, dword [screen_x_center]
  sub eax, COLUMNSZ*10
  mov dword [cursor_column], eax
  mov dword [square_length], COLUMNSZ*20
  mov dword [square_heigth], LINESZ*3
  mov dword [color], 0xCC7000
  call draw_square ;erase

  mov dword [color], BLACK
  call draw_empty_square ;draw border

  mov eax, dword [screen_y_center]
  sub eax, 4
  mov dword [cursor_line], eax
  mov eax, dword [screen_x_center]
  sub eax, (COLUMNSZ*10)-COLUMNSZ
  mov dword [cursor_column], eax
  mov esi, file_manager_create_name_str
  call print ;print "Name:"

  mov eax, dword [screen_y_center]
  sub eax, 6
  mov dword [cursor_line], eax
  mov eax, dword [screen_x_center]
  sub eax, COLUMNSZ*3
  mov dword [cursor_column], eax
  pop edi
  add edi, 16
  push edi
  mov dword [text_input_pointer], edi
  mov dword [text_input_length], 8
  call text_input ;text input for name
  pop edi
  sub edi, 16

  mov dword [edi+4], 127 ;size
  call read_time
  mov ax, word [year]
  mov word [edi+8], ax
  mov al, byte [month]
  mov byte [edi+10], al
  mov al, byte [day]
  mov byte [edi+11], al
  mov al, byte [minute]
  mov byte [edi+12], al
  mov word [edi+14], 1 ;folder

  cmp dword [file_manager_device], FM_HDD
  je .save_folder_hard_disk

  ;save folder to usb
  mov esi, MEMORY_NEW_FOLDER
  mov ecx, 512*256
  .clear_new_folder:
   mov byte [esi], 0
   inc esi
  loop .clear_new_folder
  mov dword [fat_file_length], 256 ;in KB
  mov dword [fat_first_file_cluster], 0
  push edi
  call fat_write_file
  pop edi
  mov eax, dword [fat_first_file_cluster]
  mov dword [edi], eax

  call create_new_fat_entry
  call convert_jus_folder_to_fat_folder

  mov eax, dword [file_manager_device]
  sub eax, 2
  mov ebx, dword [fm_path_pointer+(eax*4)]
  push ebx
  mov ebx, 40
  mul ebx
  pop ebx
  add eax, ebx
  mov ecx, dword [fm_path+eax]
  mov dword [fm_folder_number], ecx
  call file_manager_write_folder
  call file_manager_show_folder
  jmp .file_manager_halt

  .save_folder_hard_disk:  
   push edi
   call jus_create_folder
   pop edi

   mov eax, dword [jus_file_descriptor_block]
   mov dword [edi], eax ;save block number

   mov eax, dword [fm_path_pointer]
   mov ebx, dword [fm_path+eax]
   mov dword [jus_block_number], ebx
   mov dword [ata_memory], MEMORY_FOLDER
   call jus_write_block
   
   cmp dword [ata_status], ATA_ERROR
   je .writing_error
   
   call file_manager_show_folder
  jmp .file_manager_halt

  .writing_error:
   FM_SHOW_MESSAGE file_manager_folder_writing_error_str
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
   call file_manager_redraw_file_zone
   call redraw_screen
  jmp .file_manager_halt

  .drive_is_selected:
  FM_CLEAR_MESSAGE

  cmp dword [file_manager_device], FM_HDD
  jne .if_hdd
   call init_jus
  .if_hdd:

  cmp dword [file_manager_device], FM_CDROM
  jne .if_cdrom
   call detect_optical_disk

   cmp dword [disk_state], NO_DISK
   jne .if_no_disk
    FM_SHOW_MESSAGE file_manager_no_disk_str
    jmp .file_manager_halt
   .if_no_disk:

   cmp dword [disk_state], UNKNOWN_DISK_FORMAT
   jne .if_unknown_disk_format
    FM_SHOW_MESSAGE file_manager_no_disk_driver_str
    jmp .file_manager_halt
   .if_unknown_disk_format:
   
   cmp dword [disk_state], ISO9660_DISK
   jne .if_iso9660
    mov eax, dword [iso9660_root_dir_lba]
    mov dword [fm_path+40], eax ;save root folder number
    mov dword [fm_path_pointer+4], 0 ;reset pointer to root folder
   .if_iso9660:
  .if_cdrom:

  cmp dword [file_manager_device], FM_USB1
  jl .if_usb
   mov eax, dword [file_manager_device]
   mov ecx, dword [fm_path_pointer+(eax*4)]
   mov dword [ecx], 0 ;reset path pointer
   mov ebx, 40
   mul ebx
   add eax, fm_path
   mov dword [eax], 0 ;reset folder number
    
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
    
    cmp dword [msd_status], MSD_ERROR
    je .file_manager_halt
    
    mov eax, dword [file_manager_device]
    mov ecx, dword [fm_path_pointer+(eax*4)]
    mov ebx, 40
    mul ebx
    add eax, fm_path
    add eax, ecx
    mov edx, dword [fat_root_dir_cluster]
    mov dword [eax], edx ;save root folder number
   .skip_fat_init:
  .if_usb:

  mov eax, dword [file_manager_device]
  mov ecx, dword [fm_path_pointer+(eax*4)]
  mov ebx, 40
  mul ebx
  add eax, fm_path
  add eax, ecx
  mov edx, dword [eax]
  mov dword [fm_folder_number], edx
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
  call file_manager_show_folder
  call redraw_screen
 jmp .file_manager_halt

;;;;;          METHODS FOR READ/WRITE          ;;;;;
file_manager_show_folder:
 cmp dword [file_manager_device], FM_HDD
 je .hard_disk
 cmp dword [file_manager_device], FM_CDROM
 je .cdrom
 cmp dword [file_manager_device], FM_USB4+1
 jl .usb
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
 mov edi, MEMORY_ISO9660_FOLDER
 mov eax, 0
 mov ecx, 10000
 rep stosd ;clear memory
 
 mov eax, dword [fm_folder_number]
 mov dword [iso9660_file_lba], eax
 mov dword [iso9660_file_length], 100 ;max 100 entries
 mov dword [iso9660_file_memory], MEMORY_ISO9660_FOLDER
 call iso9660_read_file
 call convert_iso9660_folder_to_jus_folder

 mov dword [fm_highlighted_item], 0
 mov dword [fm_selected_item], 0
 mov dword [fm_first_show_item], 0
 call file_manager_redraw_file_zone
 call redraw_screen
 ret

 .usb:
 mov eax, dword [file_manager_device]
 sub eax, 2
 mov ebx, 16
 mul ebx
 add eax, (mass_storage_devices+7)
 cmp byte [eax], NO_MSD
 je .no_usb
 cmp byte [eax], MSD_UNINITALIZED
 je .usb_uninitalized
 cmp byte [eax], MSD_INITALIZED
 je .usb_not_fat32
 cmp dword [fm_folder_number], 2
 jl .done

 FM_SHOW_MESSAGE file_manager_reading_folder_str
 mov eax, dword [fm_folder_number]
 mov dword [fat_entry], eax
 mov dword [fat_memory], MEMORY_FAT32_FOLDER
 call fat_read_file
 cmp dword [msd_status], MSD_ERROR
 je .reading_error

 call convert_fat_folder_to_jus_folder
 call file_manager_redraw_file_zone
 call redraw_screen
 .done:
 ret

 .reading_error:
 FM_SHOW_MESSAGE file_manager_folder_reading_error_str
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

file_manager_write_folder:
 cmp dword [file_manager_device], FM_HDD
 je .hard_disk
 cmp dword [file_manager_device], FM_CDROM
 je .cdrom
 cmp dword [file_manager_device], 6
 jl .usb
 ret

 .hard_disk:
 FM_SHOW_MESSAGE file_manager_writing_folder_str
 mov eax, dword [fm_folder_number]
 mov dword [jus_block_number], eax
 mov dword [ata_memory], MEMORY_FOLDER
 call jus_write_block ;write file to hard disk
 cmp dword [ata_status], ATA_ERROR
 je .writing_error

 call file_manager_redraw_file_zone
 call redraw_screen
 ret

 .cdrom:
 ret

 .usb:
 FM_SHOW_MESSAGE file_manager_writing_folder_str
 mov eax, dword [fm_folder_number]
 mov dword [fat_entry], eax
 call fat_delete_file
 cmp dword [msd_status], MSD_ERROR
 je .writing_error

 mov eax, dword [fm_folder_number]
 mov dword [fat_first_file_cluster], eax
 mov dword [fat_memory], MEMORY_FAT32_FOLDER
 call fat_write_file
 cmp dword [msd_status], MSD_ERROR
 je .writing_error

 call file_manager_redraw_file_zone
 call redraw_screen
 ret

 .writing_error:
 FM_SHOW_MESSAGE file_manager_folder_writing_error_str
 ret

file_manager_read_file:
 cmp dword [file_manager_device], FM_HDD
 je .hard_disk
 cmp dword [file_manager_device], FM_CDROM
 je .cdrom
 cmp dword [file_manager_device], 6
 jl .usb
 ret

 .hard_disk:
 FM_SHOW_MESSAGE file_manager_reading_file_str
 mov eax, dword [fm_file_block]
 mov dword [jus_block_number], eax
 mov dword [ata_memory], MEMORY_FILE_DESCRIPTOR
 call jus_read_block ;read descriptor block from hard disk
 cmp dword [ata_status], ATA_ERROR
 je .reading_error

 mov eax, dword [fm_file_size]
 mov ebx, 1024
 mov edx, 0
 div ebx ;convert from KB to MB
 inc eax
 mov dword [allocated_size], eax
 call allocate_memory
 cmp dword [allocated_memory_pointer], 0
 je .not_enough_memory
 
 mov eax, dword [allocated_memory_pointer]
 mov dword [ata_memory], eax
 call jus_read_file
 cmp dword [ata_status], ATA_ERROR
 je .reading_error
 ret

 .cdrom:
 mov eax, dword [fm_file_block]
 mov dword [iso9660_file_lba], eax
 mov eax, dword [fm_file_size]
 mov ebx, 2
 mov edx, 0
 div ebx ;optical disk sector is 2 KB
 inc eax
 mov dword [iso9660_file_length], eax
 
 mov eax, dword [fm_file_size]
 mov ebx, 1024
 mov edx, 0
 div ebx ;convert from KB to MB
 inc eax
 mov dword [allocated_size], eax
 call allocate_memory
 cmp dword [allocated_memory_pointer], 0
 je .not_enough_memory
 
 mov eax, dword [allocated_memory_pointer]
 mov dword [iso9660_file_memory], eax
 call iso9660_read_file
 
 ret

 .usb:
 FM_SHOW_MESSAGE file_manager_reading_file_str
 
 mov eax, dword [fm_file_size]
 mov ebx, 1024
 mov edx, 0
 div ebx ;convert from KB to MB
 inc eax
 mov dword [allocated_size], eax
 call allocate_memory
 cmp dword [allocated_memory_pointer], 0
 je .not_enough_memory
 
 mov eax, dword [allocated_memory_pointer]
 mov dword [fat_memory], eax
 mov eax, dword [fm_file_block]
 mov dword [fat_entry], eax
 call fat_read_file
 cmp dword [msd_status], MSD_ERROR
 je .reading_error
 ret

 .reading_error:
 FM_SHOW_MESSAGE file_manager_file_reading_error_str
 ret

 .not_enough_memory:
 FM_SHOW_MESSAGE file_manager_memory_error_str
 ret

file_manager_write_file:
 cmp dword [file_manager_device], FM_HDD
 je .hard_disk
 cmp dword [file_manager_device], FM_CDROM
 je .cdrom
 cmp dword [file_manager_device], 6
 jl .usb
 ret

 .hard_disk:
 FM_SHOW_MESSAGE file_manager_writing_file_str
 mov eax, dword [fm_file_memory]
 mov dword [ata_memory], eax
 mov eax, dword [fm_file_size]
 mov dword [jus_file_length], eax
 call jus_write_file
 cmp dword [ata_status], ATA_ERROR
 je .writing_error
 ret

 .cdrom:
 ret

 .usb:
 FM_SHOW_MESSAGE file_manager_writing_file_str
 mov eax, dword [fm_file_size]
 mov dword [fat_file_length], eax
 mov eax, dword [fm_file_memory]
 mov dword [fat_memory], eax
 call fat_write_file
 cmp dword [msd_status], MSD_ERROR
 je .writing_error
 ret

 .writing_error:
 FM_SHOW_MESSAGE file_manager_file_reading_error_str
 ret

;;;;;           REDRAW METHODS           ;;;;;
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
 mov dword [color], FM_COLOR_BACKGROUND
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
 mov ebx, 128
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
  add eax, 120
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
  mov word [esi+114], 0
  push esi
  mov eax, esi
  add eax, 16
  mov esi, eax
  call print_unicode
  pop esi

 .next_item:
 add esi, 128
 add dword [cursor_line], LINESZ
 pop ecx
 dec ecx
 cmp ecx, 0
 jne .print_item

 ret

fm_show_selected_item_number:
 ;erase
 mov dword [cursor_line], 5
 SCREEN_X_SUB eax, COLUMNSZ*20
 mov dword [cursor_column], eax
 mov dword [square_length], COLUMNSZ*20
 mov dword [square_heigth], LINESZ
 mov dword [color], FM_COLOR_BORDER
 call draw_square

 SCREEN_X_SUB eax, COLUMNSZ*17
 SCREEN_X_SUB ebx, COLUMNSZ*2
 cmp dword [fm_selected_item], 10
 jl .print
 SCREEN_X_SUB eax, COLUMNSZ*18
 SCREEN_X_SUB ebx, COLUMNSZ*3
 cmp dword [fm_selected_item], 100
 jl .print
 SCREEN_X_SUB eax, COLUMNSZ*19
 SCREEN_X_SUB ebx, COLUMNSZ*4
 cmp dword [fm_selected_item], 1000
 jl .print
 SCREEN_X_SUB eax, COLUMNSZ*20
 SCREEN_X_SUB ebx, COLUMNSZ*5

 .print:
 mov dword [cursor_line], 5
 mov dword [cursor_column], eax
 mov dword [color], BLACK
 push esi
 push ebx
 mov esi, fm_item_str
 call print
 pop ebx
 pop esi

 mov dword [cursor_column], ebx
 mov eax, dword [fm_selected_item]
 mov dword [var_print_value], eax
 push esi
 call print_var
 pop esi

 ret
