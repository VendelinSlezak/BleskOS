;BleskOS

; this library is for save and open dialogs

;DEFINITIONS
%define FD_OPEN 1
%define FD_SAVE 2

%define FD_MEDIUM_RAM 1
%define FD_MEDIUM_HDD 2

%define FD_NO_FILE 0
%define FD_FILE_RAM_IMAGE 1
%define FD_FILE 2
%define FD_WRITING_OK 3

;VARIABILES
file_memory dd 0
file_size dd 0
file_type dd 0
fd_type_of_dialog dd 0
fd_selected_medium dd 0
fd_last_selected_medium dd 0
fd_return dd 0

fd_files_on_screen dd 0
fd_selected_file dd 0
fd_first_file dd 0
fd_highlighted_file dd 0
fd_hdd_path times 10 dd 0

fd_loaded_folder dd 0
fd_folder_state dd 0

;STRINGS
fd_no_usb db 'No USB device', 0
fd_usb_not_fat32 db 'USB stick', 0

file_dialog_draw_items:
 ;clear background
 SCREEN_X_SUB eax, COLUMNSZ*22
 SCREEN_Y_SUB ebx, LINESZ*5
 DRAW_SQUARE LINESZ*5, COLUMNSZ*22, eax, ebx, 0x884E10
 
 ;draw highlighted file
 mov eax, dword [fd_highlighted_file]
 mov ebx, LINESZ
 mul ebx
 add eax, LINESZ*5
 SCREEN_X_SUB ebx, COLUMNSZ*23
 DRAW_SQUARE eax, COLUMNSZ*22, ebx, LINESZ, 0xFF0000 ;red

 mov dword [cursor_line], LINESZ*5+1
 mov eax, dword [fd_first_file]
 mov ebx, 128
 mul ebx
 add eax, MEMORY_FOLDER
 mov esi, eax
 mov ecx, dword [fd_files_on_screen]
 .draw_item:
  cmp dword [esi], 0
  je .done
  
  push ecx
  push dword [cursor_line]
  push esi
  push esi
  
  ;print name
  add esi, 16 ;pointer to name
  mov dword [cursor_column], COLUMNSZ*22
  call print_unicode
  
  ;print type
  pop esi
  add esi, 116 ;pointer to type
  SCREEN_X_SUB eax, COLUMNSZ*4
  mov dword [cursor_column], eax
  call print_ascii
  
  pop esi
  add esi, 128
  pop dword [cursor_line]
  add dword [cursor_line], LINESZ
  pop ecx
 loop .draw_item
 
 .done:
 ret

file_dialog_draw_devices:
 ;clear background
 DRAW_SQUARE LINESZ*5, COLUMNSZ, COLUMNSZ*20, LINESZ*2*10, 0x884E10 ;brown
 
 ;draw title
 SCREEN_X_SUB eax, COLUMNSZ*2
 DRAW_SQUARE LINE(1), COLUMN(1), eax, LINESZ*3, 0x60BD00 ;green
 cmp dword [fd_type_of_dialog], FD_OPEN
 jne .if_open
  PRINT 'Open file           [d] Select medium [enter] Open file', open_dialog_title_str, LINE(2)+1, COLUMN(2)
 .if_open:
 cmp dword [fd_type_of_dialog], FD_SAVE
 jne .if_save
  PRINT 'Save file           [d] Select medium', save_dialog_title_str, LINE(2)+1, COLUMN(2)
 .if_save:

 ;draw square of selected medium
 mov eax, dword [fd_selected_medium]
 mov ebx, LINESZ*2
 mul ebx
 add eax, LINESZ*5 ;skip title
 mov dword [cursor_line], eax
 mov dword [cursor_column], COLUMNSZ
 mov dword [square_heigth], LINESZ*2
 mov dword [square_length], COLUMNSZ*20
 mov dword [color], 0xFF0000 ;red
 call draw_square
 
 ;print labels
 PRINT 'Hard disk', hard_disk_str, LINESZ*5+6, COLUMNSZ*2
 PRINT 'Optical disk', optical_disk_str, LINESZ*5+LINESZ*2+6, COLUMNSZ*2
 mov dword [cursor_line], LINESZ*5+LINESZ*4+6
 mov dword [cursor_column], COLUMNSZ*2
 mov dword [color], BLACK
 mov edi, mass_storage_devices+7
 mov ebp, mass_storage_device_label
 mov ecx, 4
 .print_labels_of_usb:
 push ecx
  cmp byte [edi], NO_MSD
  je .no_usb_stick
  cmp byte [edi], MSD_FAT32
  je .usb_stick_fat32
  
  mov esi, fd_usb_not_fat32
  push ebp
  push edi
  call print_ascii
  pop edi
  pop ebp
  jmp .next_device
  
  .no_usb_stick:
  mov esi, fd_no_usb
  push ebp
  push edi
  call print_ascii
  pop edi
  pop ebp
  jmp .next_device
  
  .usb_stick_fat32:
  mov esi, ebp
  push ebp
  push edi
  call print_ascii
  pop edi
  pop ebp
 .next_device:
 add edi, 16
 add ebp, 12
 add dword [cursor_line], LINESZ*2
 mov dword [cursor_column], COLUMNSZ*2
 pop ecx
 loop .print_labels_of_usb
 PRINT '[F12] RAM memory', ram_memory_str, LINESZ*5+LINESZ*12+6, COLUMNSZ*2 
 
 ret
 
file_dialog_change_device:
 mov eax, dword [fd_selected_medium]
 mov dword [fd_last_selected_medium], eax
 
 SCREEN_Y_SUB eax, LINESZ*2
 PRINT 'You are selecting medium', selecting_medium_str, eax, COLUMNSZ
 call redraw_screen
 
 .halt:
  call wait_for_keyboard
  
  cmp byte [key_code], KEY_ESC
  je .esc
  cmp byte [key_code], KEY_UP
  je .device_up
  cmp byte [key_code], KEY_DOWN
  je .device_down
  cmp byte [key_code], KEY_ENTER
  je .enter
 jmp .halt
 
 .esc:
  mov eax, dword [fd_last_selected_medium]
  mov dword [fd_selected_medium], eax
  CLEAR_SCREEN 0x884E10 ;brown
  call file_dialog_draw_devices
  call redraw_screen
  ret
 
 .device_up:
  cmp dword [fd_selected_medium], 0
  je .halt
  dec dword [fd_selected_medium]
  call file_dialog_draw_devices
  SCREEN_Y_SUB eax, LINESZ*2
  PRINT 'You are selecting medium', selecting_medium_str2, eax, COLUMNSZ
  call redraw_screen
 jmp .halt
 
 .device_down:
  cmp dword [fd_selected_medium], 5
  je .halt
  inc dword [fd_selected_medium]
  call file_dialog_draw_devices
  SCREEN_Y_SUB eax, LINESZ*2
  PRINT 'You are selecting medium', selecting_medium_str3, eax, COLUMNSZ
  call redraw_screen
 jmp .halt
 
 .enter:
  CLEAR_SCREEN 0x884E10 ;brown
  call file_dialog_draw_devices
  PRINT 'Loading folder...', loading_folder_str, LINESZ*5, COLUMNSZ*22
  call redraw_screen
  
  cmp dword [fd_selected_medium], 2
  jb .if_usb
   mov eax, dword [fd_selected_medium]
   sub eax, 2
   mov dword [msd_number], eax
   mov ebx, 16
   mul ebx
   add eax, mass_storage_devices+7
   cmp byte [eax], NO_MSD
   je .if_usb
   cmp byte [eax], MSD_FAT32
   jne .if_usb
  
   ;this is usb stick formatted as FAT32
   call select_msd
   call msd_read_mbr
   mov eax, dword [first_partition_lba]
   mov dword [fat_base_sector], eax
   call init_fat
   cmp dword [msd_status], MSD_ERROR
   je file_dialog_load_folder.reading_folder_error
  .if_usb:
  
  call file_dialog_load_folder
 ret
 
file_dialog_load_folder:
 cmp dword [fd_selected_medium], 0
 je .hard_disk
 cmp dword [fd_selected_medium], 1
 je .cdrom
 jmp .usb_stick
 ret
 
 .hard_disk:
  mov eax, 0 ;root folder
  call jus_read_folder
  
  cmp dword [ata_status], IDE_ERROR
  je .reading_folder_error
  
  mov dword [fd_folder_state], 1
  call file_dialog_draw_items
  call redraw_screen
  ret
  
 .cdrom:
  call detect_optical_disk
  cmp dword [disk_state], NO_DISK
  je .no_disk
  cmp dword [disk_state], UNKNOWN_DISK_FORMAT
  je .unknown_disk_format
  
  mov eax, dword [iso9660_root_dir_lba]
  mov dword [iso9660_file_lba], eax
  mov eax, dword [iso9660_root_dir_length]
  and eax, 0xFFF
  mov dword [iso9660_file_length], eax
  mov dword [iso9660_file_memory], MEMORY_ISO9660_FOLDER
  call iso9660_read_file
  
  cmp dword [atapi_status], IDE_ERROR
  je .reading_folder_error
  
  mov dword [fd_folder_state], 1
  call convert_iso9660_folder_to_jus_folder
  call file_dialog_draw_items
  call redraw_screen
  ret
  
  .no_disk:
   CLEAR_SCREEN 0x884E10 ;brown
   call file_dialog_draw_devices
   PRINT 'No disk is inserted', no_disk_str, LINESZ*5, COLUMNSZ*22
   call redraw_screen
   mov dword [fd_folder_state], 0
   ret
   
  .unknown_disk_format:
   CLEAR_SCREEN 0x884E10 ;brown
   call file_dialog_draw_devices
   PRINT 'This disk is not formatted as ISO9660', not_iso9660_format_str, LINESZ*5, COLUMNSZ*22
   call redraw_screen
   mov dword [fd_folder_state], 0
   ret
 
 .usb_stick:
  mov eax, dword [fd_selected_medium]
  sub eax, 2
  mov ebx, 16
  mul ebx
  add eax, mass_storage_devices+7
  cmp byte [eax], NO_MSD
  je .no_usb_stick
  cmp byte [eax], MSD_FAT32
  jne .usb_stick_not_fat32
  
  ;this is usb stick formatted as FAT32
  mov eax, 0 ;root folder
  call fat_read_folder
  cmp dword [msd_status], MSD_ERROR
  je .reading_folder_error
  
  mov dword [fd_folder_state], 1
  call file_dialog_draw_items
  call redraw_screen
  ret
  
  .no_usb_stick:
   CLEAR_SCREEN 0x884E10 ;brown
   call file_dialog_draw_devices
   PRINT 'No USB stick is connected here', no_usb_stick_str, LINESZ*5, COLUMNSZ*22
   call redraw_screen
   mov dword [fd_folder_state], 0
   ret
   
  .usb_stick_not_fat32:
   CLEAR_SCREEN 0x884E10 ;brown
   call file_dialog_draw_devices
   PRINT 'This USB stick is not formatted as FAT32', usb_stick_not_formatted_as_FAT32_str, LINESZ*5, COLUMNSZ*22
   call redraw_screen
   mov dword [fd_folder_state], 0
   ret
   
  .reading_folder_error:
   CLEAR_SCREEN 0x884E10 ;brown
   call file_dialog_draw_devices
   PRINT 'Error during reading folder', reading_folder_error_str, LINESZ*5, COLUMNSZ*22
   call redraw_screen
   mov dword [fd_folder_state], 0
   ret

file_dialog_open:
 mov dword [fd_type_of_dialog], FD_OPEN
 CLEAR_SCREEN 0x884E10 ;brown
 call file_dialog_draw_devices
 mov dword [fd_selected_medium], 0 ;hard disk
 call file_dialog_change_device.enter ;load folder

 .halt:
  call wait_for_keyboard
  
  cmp byte [key_code], KEY_ESC
  je .done
  
  cmp byte [key_code], KEY_F12
  je .ram_memory
  
  cmp byte [key_code], KEY_D
  je .key_d
  
  ;these keys work only if is folder succesfully loaded
  cmp dword [fd_folder_state], 0
  je .halt
  
  cmp byte [key_code], KEY_UP
  je .key_up
  
  cmp byte [key_code], KEY_DOWN
  je .key_down
  
  cmp byte [key_code], KEY_ENTER
  je .key_enter
 jmp .halt
 
 .done:
 mov dword [fd_return], FD_NO_FILE
 ret
 
 .ram_memory:
 mov esi, MEMORY_RAM_IMAGE
 mov edi, dword [file_memory]
 mov ecx, 0x100000
 rep movsb
 
 mov dword [file_size], 1024 ;one MB
 mov dword [fd_return], FD_FILE_RAM_IMAGE
 ret
 
 .key_d:
  call file_dialog_change_device
 jmp .halt
 
 .key_up:
  cmp dword [fd_highlighted_file], 0
  jne .go_up
  cmp dword [fd_first_file], 0
  je .halt
  
  dec dword [fd_first_file]
  jmp .go_up_redraw
  
  .go_up:
  dec dword [fd_highlighted_file]
  .go_up_redraw:
  dec dword [fd_selected_file]
  call file_dialog_draw_items
  call redraw_screen
 jmp .halt
 
 .key_down:
  mov eax, dword [fd_files_on_screen]
  cmp dword [fd_highlighted_file], eax
  jne .go_down
  mov ebx, 1024
  sub ebx, eax
  cmp dword [fd_first_file], ebx
  je .halt
  
  inc dword [fd_first_file]
  jmp .go_down_redraw
  
  .go_down:
  inc dword [fd_highlighted_file]
  .go_down_redraw:
  inc dword [fd_selected_file]
  call file_dialog_draw_items
  call redraw_screen
 jmp .halt
 
 .key_enter:
  cmp dword [fd_selected_medium], 0
  je .load_file_from_hard_disk
  cmp dword [fd_selected_medium], 1
  je .load_file_from_cdrom
  jmp .load_file_from_usb_stick
  
  ;LOADING FROM HARD DISK
  .load_file_from_hard_disk:
  mov eax, dword [fd_selected_file]
  mov ebx, 128
  mul ebx
  add eax, MEMORY_FOLDER
  mov esi, eax
  cmp dword [esi], 0
  je .halt
  
  mov eax, dword [esi+4] ;here is length of file in KB
  mov ebx, 1000
  mov edx, 0
  div ebx
  inc eax
  mov dword [allocated_size], eax
  push esi
  call allocate_memory ;allocate enough memory
  pop esi
  cmp dword [allocated_memory_pointer], 0
  je .not_enough_memory_for_file
  
  mov eax, dword [allocated_memory_pointer]
  mov dword [file_memory], eax
  
  ;print message
  push esi
  push eax
  CLEAR_SCREEN 0x884E10 ;brown
  call file_dialog_draw_devices
  PRINT 'Loading file...', loading_file_str1, LINESZ*5, COLUMNSZ*22
  call redraw_screen
  pop eax
  pop esi
  
  ;load file
  mov dword [jus_memory], eax
  mov eax, dword [esi]
  mov dword [jus_file_sector], eax
  mov eax, dword [esi+4]
  mov dword [jus_file_size], eax
  push esi
  call jus_read_file
  pop esi
  
  cmp dword [ata_status], IDE_ERROR
  je .error_during_reading_file
  
  ;set variabiles
  mov eax, dword [esi+4]
  mov ebx, 1024
  mul ebx
  mov dword [file_size], eax
  mov eax, dword [esi+116]
  mov dword [file_type], eax
  cmp dword [esi+116], 'TXT'
  jne .if_hdd_txt
   mov dword [file_type], 'BTXT'
  .if_hdd_txt:
  mov dword [fd_return], FD_FILE
  
  ret
  
  ;LOADING FROM CDROM
  .load_file_from_cdrom:
  mov eax, dword [fd_selected_file]
  mov ebx, 128
  mul ebx
  add eax, MEMORY_FOLDER
  mov esi, eax
  cmp dword [esi], 0
  je .halt
  
  mov eax, dword [esi+4] ;here is length of file in KB
  mov ebx, 1000
  mov edx, 0
  div ebx
  inc eax
  mov dword [allocated_size], eax
  push esi
  call allocate_memory ;allocate enough memory
  pop esi
  cmp dword [allocated_memory_pointer], 0
  je .not_enough_memory_for_file
  
  mov eax, dword [allocated_memory_pointer]
  mov dword [file_memory], eax
  
  ;print message
  push esi
  push eax
  CLEAR_SCREEN 0x884E10 ;brown
  call file_dialog_draw_devices
  PRINT 'Loading file...', loading_file_str2, LINESZ*5, COLUMNSZ*22
  call redraw_screen
  pop eax
  pop esi
  
  ;load file
  mov dword [iso9660_file_memory], eax
  mov eax, dword [esi]
  mov dword [iso9660_file_lba], eax
  mov eax, dword [esi+4]
  shr eax, 1 ;div 2 - convert from KB to cdrom sectors
  inc eax
  mov dword [iso9660_file_length], eax
  push esi
  call iso9660_read_file
  pop esi
  
  cmp dword [atapi_status], IDE_ERROR
  je .error_during_reading_file
  
  ;set variabiles
  mov eax, dword [esi+4]
  mov ebx, 1024
  mul ebx
  mov dword [file_size], eax
  mov eax, dword [esi+116]
  mov dword [file_type], eax
  mov dword [fd_return], FD_FILE
  
  ret
  
  ;LOADING FROM USB STICK
  .load_file_from_usb_stick:
  mov eax, dword [fd_selected_file]
  mov ebx, 128
  mul ebx
  add eax, MEMORY_FOLDER
  mov esi, eax
  cmp dword [esi], 0
  je .halt
  
  mov eax, dword [esi+4] ;here is length of file in KB
  mov ebx, 1000
  mov edx, 0
  div ebx
  inc eax
  mov dword [allocated_size], eax
  push esi
  call allocate_memory ;allocate enough memory
  pop esi
  cmp dword [allocated_memory_pointer], 0
  je .not_enough_memory_for_file
  
  mov eax, dword [allocated_memory_pointer]
  mov dword [file_memory], eax
  
  ;print message
  push esi
  push eax
  CLEAR_SCREEN 0x884E10 ;brown
  call file_dialog_draw_devices
  PRINT 'Loading file...', loading_file_str3, LINESZ*5, COLUMNSZ*22
  call redraw_screen
  pop eax
  pop esi
  
  ;load file
  mov dword [fat_memory], eax
  mov eax, dword [esi]
  cmp eax, 0
  je .halt
  mov dword [fat_entry], eax
  push esi
  call fat_read_file
  pop esi
  
  cmp dword [msd_status], MSD_ERROR
  je .error_during_reading_file
  
  ;set variabiles
  mov eax, dword [esi+4]
  mov ebx, 1024
  mul ebx
  mov dword [file_size], eax
  mov eax, dword [esi+116]
  mov dword [file_type], eax
  mov dword [fd_return], FD_FILE
  
  ret
  
  ;ERRORS
  .not_enough_memory_for_file:
   CLEAR_SCREEN 0x884E10 ;brown
   call file_dialog_draw_devices
   PRINT 'ERROR: not enough free memory for this file, you have to close some files', not_enough_free_memory_str, LINESZ*5, COLUMNSZ*22
   call redraw_screen
  jmp .halt
  
  .error_during_reading_file:
   CLEAR_SCREEN 0x884E10 ;brown
   call file_dialog_draw_devices
   PRINT 'Error occured during reading this file', reading_error_str, LINESZ*5, COLUMNSZ*22
   call redraw_screen
  jmp .halt
 
file_dialog_save:
 mov dword [fd_type_of_dialog], FD_SAVE
 CLEAR_SCREEN 0x884E10 ;brown
 call file_dialog_draw_devices
 call redraw_screen
 .halt:
  call wait_for_keyboard
  
  cmp byte [key_code], KEY_ESC
  je .done
  
  cmp byte [key_code], KEY_F12
  je .ram_memory
  
  cmp byte [key_code], KEY_D
  je .key_d
 jmp .halt
 
 .done:
 ret
 
 .ram_memory:
 mov esi, dword [file_memory]
 mov edi, MEMORY_RAM_IMAGE
 mov ecx, 0x100000
 rep movsb
 ret
 
 .key_d:
  call file_dialog_change_device
 jmp .halt
