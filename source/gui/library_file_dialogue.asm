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

fd_loaded_folder dd 0
fd_folder_state dd 0
fd_path times 50 dd 0
fd_path_pointer dd fd_path
fd_path_entry dd 0

;STRINGS
fd_no_usb db 'No USB device', 0
fd_usb_not_fat32 db 'USB stick', 0
fd_fat_name times 20 db 0

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

 mov dword [color], BLACK
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
  push esi
  push esi
  
  ;print name
  add esi, 16 ;pointer to name
  mov dword [cursor_column], COLUMNSZ*22
  call print_unicode
  
  ;print type
  pop esi
  cmp word [esi+14], 1
  je .skip_printing_type ;this entry is folder
  add esi, 116 ;pointer to type
  SCREEN_X_SUB eax, COLUMNSZ*4
  mov dword [cursor_column], eax
  call print_ascii
  .skip_printing_type:
  
  ;print size
  SCREEN_X_SUB ecx, COLUMNSZ*14
  mov dword [cursor_column], ecx
  pop esi
  mov eax, dword [esi+4]
  cmp eax, 1024
  jb .size_kb
  cmp eax, 1024*1024
  jb .size_mb
  
  ;size GB
  mov ebx, 1024*1024
  mov edx, 0
  div ebx
  mov dword [var_print_value], eax
  call print_var
  add dword [cursor_column], COLUMNSZ
  mov dword [char_for_print], 'G'
  call print_char
  add dword [cursor_column], COLUMNSZ
  mov dword [char_for_print], 'B'
  call print_char
  jmp .draw_time
  
  ;size KB
  .size_kb:
  mov dword [var_print_value], eax
  call print_var
  add dword [cursor_column], COLUMNSZ
  mov dword [char_for_print], 'K'
  call print_char
  add dword [cursor_column], COLUMNSZ
  mov dword [char_for_print], 'B'
  call print_char
  jmp .draw_time
  
  ;size MB
  .size_mb:
  mov ebx, 1024
  mov edx, 0
  div ebx
  mov dword [var_print_value], eax
  call print_var
  add dword [cursor_column], COLUMNSZ
  mov dword [char_for_print], 'M'
  call print_char
  add dword [cursor_column], COLUMNSZ
  mov dword [char_for_print], 'B'
  call print_char
  jmp .draw_time
  
  ;print time
  .draw_time:
  SCREEN_X_SUB ecx, COLUMNSZ*25
  mov dword [cursor_column], ecx
  pop esi
  ;year
  mov eax, 0
  mov ax, word [esi+8]
  mov dword [var_print_value], eax
  push esi
  call print_var
  pop esi
  ;/
  mov dword [char_for_print], '/'
  call print_char
  add dword [cursor_column], COLUMNSZ
  ;month
  mov eax, 0
  mov al, byte [esi+10]
  mov dword [var_print_value], eax
  push esi
  call print_var
  pop esi
  ;/
  mov dword [char_for_print], '/'
  call print_char
  add dword [cursor_column], COLUMNSZ
  ;day
  mov eax, 0
  mov al, byte [esi+11]
  mov dword [var_print_value], eax
  call print_var
  
  pop esi
  add esi, 128
  pop dword [cursor_line]
  add dword [cursor_line], LINESZ
  pop ecx
 dec ecx
 cmp ecx, 0
 jne .draw_item
 
 .done:
 ret

file_dialog_draw_devices:
 ;clear background
 DRAW_SQUARE LINESZ*5, COLUMNSZ, COLUMNSZ*20, LINESZ*2*10, 0x884E10 ;brown
 
 ;draw title
 SCREEN_X_SUB eax, COLUMNSZ*2
 DRAW_SQUARE LINE(1), COLUMN(1), eax, LINESZ*3, 0x60BD00 ;green
 mov dword [color], BLACK
 cmp dword [fd_type_of_dialog], FD_OPEN
 jne .if_open
  PRINT 'Open file           [d] Select medium [b] Previous folder [enter] Open file', open_dialog_title_str, LINE(2)+1, COLUMN(2)
 .if_open:
 cmp dword [fd_type_of_dialog], FD_SAVE
 jne .if_save
  PRINT 'Save file           [d] Select medium [b] Previous folder [s] Save file [enter] Rewrite file', save_dialog_title_str, LINE(2)+1, COLUMN(2)
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
 mov dword [color], BLACK
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
  
  mov dword [fd_path_entry], 0
  mov dword [fd_path_pointer], fd_path
  mov edi, fd_path
  mov eax, 0
  mov ecx, 50
  rep stosd
  
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
  mov ebx, dword [fd_path_pointer]
  mov eax, dword [ebx]
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
  
  mov ebx, dword [fd_path_pointer]
  mov eax, dword [ebx]
  cmp eax, 0
  jne .if_cdrom_root_folder
   mov eax, dword [iso9660_root_dir_lba]
  .if_cdrom_root_folder:
  mov dword [iso9660_file_lba], eax
  mov dword [iso9660_file_length], 1 ;only one sector
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
  mov ebx, dword [fd_path_pointer]
  mov eax, dword [ebx]
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
  
  cmp byte [key_code], KEY_B
  je .key_b
  
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
 
 .key_b:
  cmp dword [fd_path_entry], 0
  je .halt
  dec dword [fd_path_entry]
  sub dword [fd_path_pointer], 4
  mov dword [fd_first_file], 0
  mov dword [fd_highlighted_file], 0
  mov dword [fd_selected_file], 0
  call file_dialog_load_folder
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
  
  ;folder
  cmp word [esi+14], 1
  je .open_folder
  
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
  
  ;folder
  cmp word [esi+14], 1
  je .open_folder
  
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
  
  ;folder
  cmp word [esi+14], 1
  je .open_folder
  
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
  
  ;OPEN FOLDER
  .open_folder:
   cmp dword [fd_path_entry], 50
   je .error_too_many_folders
   inc dword [fd_path_entry]
   add dword [fd_path_pointer], 4
   mov ebx, dword [fd_path_pointer]
   mov eax, dword [esi]
   mov dword [ebx], eax
   mov dword [fd_first_file], 0
   mov dword [fd_highlighted_file], 0
   mov dword [fd_selected_file], 0
   call file_dialog_load_folder
  jmp .halt
  
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
  
  .error_too_many_folders:
   CLEAR_SCREEN 0x884E10 ;brown
   call file_dialog_draw_devices
   PRINT 'You can not open deeper that 50 folders', too_many_folders_str, LINESZ*5, COLUMNSZ*22
   call redraw_screen
  jmp .halt
 
file_dialog_save:
 mov dword [fd_type_of_dialog], FD_SAVE
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
  
  cmp byte [key_code], KEY_B
  je .key_b
  
  ;these keys work only if is folder succesfully loaded
  cmp dword [fd_folder_state], 0
  je .halt
  
  cmp byte [key_code], KEY_UP
  je .key_up
  
  cmp byte [key_code], KEY_DOWN
  je .key_down
  
  cmp byte [key_code], KEY_S
  je .key_s
  
  cmp byte [key_code], KEY_ENTER
  je .key_enter
 jmp .halt
 
 .done:
 mov dword [fd_return], FD_NO_FILE
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
 
 .key_b:
  cmp dword [fd_path_entry], 0
  je .halt
  dec dword [fd_path_entry]
  sub dword [fd_path_pointer], 4
  mov dword [fd_first_file], 0
  mov dword [fd_highlighted_file], 0
  mov dword [fd_selected_file], 0
  call file_dialog_load_folder
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
  mov eax, dword [fd_selected_file]
  mov ebx, 128
  mul ebx
  add eax, MEMORY_FOLDER
  mov esi, eax
  cmp dword [esi], 0
  je .halt
  
  ;folder
  cmp word [esi+14], 1
  je .open_folder

  ;there is some file, so we can rewrite it
  cmp dword [fd_selected_medium], 0
  je .rewrite_file_on_hard_disk
  cmp dword [fd_selected_medium], 1
  jne .rewrite_file_on_usb_stick
  jmp .halt
  
  ;REWRITE FILE ON HARD DISK
  .rewrite_file_on_hard_disk:
  mov eax, dword [esi]
  mov dword [jus_file_sector], eax
  mov eax, dword [esi+4]
  mov dword [jus_file_size], eax
  push esi
  call jus_delete_file
  pop esi
  
  ;size
  mov eax, dword [file_size]
  mov dword [esi+4], eax
  
  ;print message
  push esi
  push eax
  CLEAR_SCREEN 0x884E10 ;brown
  call file_dialog_draw_devices
  PRINT 'Rewriting file...', rewriting_file_str1, LINESZ*5, COLUMNSZ*22
  call redraw_screen
  pop eax
  pop esi
  
  ;write file and save first sector
  mov dword [jus_file_size], eax
  mov eax, dword [file_memory]
  mov dword [jus_memory], eax
  push esi
  call jus_write_file
  pop esi
  cmp dword [ata_status], IDE_ERROR
  je .error_during_writing_file
  
  mov eax, dword [jus_file_sector]
  mov dword [esi], eax
  
  ;save folder
  mov ebx, dword [fd_path_pointer]
  mov eax, dword [ebx]
  call jus_rewrite_folder
  cmp dword [ata_status], IDE_ERROR
  je .error_during_rewriting_folder
  
  ret
  
  ;REWRITE FILE ON USB STICK
  .rewrite_file_on_usb_stick:
  ;find where is entry of selected file
  mov esi, MEMORY_FAT32_FOLDER+64
  mov eax, 0
  mov ecx, 1024
  .rewriting_find_selected_fat_entry:
   cmp byte [esi+11], 0xF ;long file name entry
   je .rewriting_next_fat_entry
   mov bl, byte [esi+11]
   test bl, 0xE ;hidden, system, volume id
   jnz .rewriting_next_fat_entry
   cmp byte [esi], 0xE5 ;not used item
   je .rewriting_next_fat_entry
   
   cmp eax, dword [fd_selected_file]
   je .entry_founded
   inc eax
   
   .rewriting_next_fat_entry:
   add esi, 32
  loop .rewriting_find_selected_fat_entry
  ret
  
  .entry_founded:
  push esi
  CLEAR_SCREEN 0x884E10 ;brown
  call file_dialog_draw_devices
  PRINT 'Rewriting file...', rewriting_file_str2, LINESZ*5, COLUMNSZ*22
  call redraw_screen
  pop esi
  mov eax, dword [esi]
  mov dword [fat_entry], eax
  push esi
  call fat_delete_file
  pop esi
  
  ;size
  mov eax, dword [file_size]
  mov ebx, 1024
  mul ebx ;convert from KB to bytes
  mov dword [esi+28], eax
  
  ;write file and save first cluster
  mov eax, dword [file_size]
  mov dword [fat_file_length], eax
  mov eax, dword [file_memory]
  mov dword [fat_memory], eax
  push esi
  call fat_write_file
  pop esi
  cmp dword [msd_status], MSD_ERROR
  je .error_during_writing_file
  
  mov eax, dword [fat_first_file_cluster]
  shr eax, 16
  mov word [esi+20], ax
  mov eax, dword [fat_first_file_cluster]
  mov word [esi+26], ax
  
  ;save folder
  mov ebx, dword [fd_path_pointer]
  mov eax, dword [ebx]
  call fat_rewrite_folder
  cmp dword [msd_status], MSD_ERROR
  je .error_during_rewriting_folder
  
  ret
  
  ;OPEN FOLDER
  .open_folder:
   cmp dword [fd_path_entry], 50
   je .error_too_many_folders
   inc dword [fd_path_entry]
   add dword [fd_path_pointer], 4
   mov ebx, dword [fd_path_pointer]
   mov eax, dword [esi]
   mov dword [ebx], eax
   mov dword [fd_first_file], 0
   mov dword [fd_highlighted_file], 0
   mov dword [fd_selected_file], 0
   call file_dialog_load_folder
  jmp .halt
  
 .key_s:
  cmp dword [fd_selected_medium], 0
  je .save_file_to_hard_disk
  cmp dword [fd_selected_medium], 1
  jne .save_file_to_usb_stick
  jmp .halt
  
  ;SAVE FILE TO HARD DISK
  .save_file_to_hard_disk:
  mov esi, MEMORY_FOLDER
  mov ecx, 1024
  .hdd_find_free_entry:
   cmp dword [esi], 0
   je .hdd_free_entry
   add esi, 128
  loop .hdd_find_free_entry
  jmp .no_free_entry
  
  .hdd_free_entry:
  ;name
  mov eax, esi
  add eax, 16
  mov dword [text_input_pointer], eax
  mov dword [text_input_length], 50
  mov dword [cursor_line], LINESZ*2
  SCREEN_X_SUB eax, COLUMNSZ*52
  mov dword [cursor_column], eax
  push esi
  call text_input
  pop esi
  
  ;extension
  mov eax, dword [file_type]
  mov dword [esi+116], eax
  
  ;time
  call read_time
  mov eax, 0
  mov ax, word [year]
  mov word [esi+8], ax
  mov ax, 0
  mov al, byte [month]
  mov byte [esi+10], al
  mov al, byte [day]
  mov byte [esi+11], al
  mov al, byte [hour]
  mov byte [esi+12], al
  mov al, byte [minute]
  mov byte [esi+13], al
  
  ;type
  mov word [esi+14], 2
  
  ;size
  mov eax, dword [file_size]
  mov dword [esi+4], eax
  
  ;print message
  push esi
  push eax
  CLEAR_SCREEN 0x884E10 ;brown
  call file_dialog_draw_devices
  PRINT 'Saving file...', saving_file_str1, LINESZ*5, COLUMNSZ*22
  call redraw_screen
  pop eax
  pop esi
  
  ;write file and save first sector
  mov dword [jus_file_size], eax
  mov eax, dword [file_memory]
  mov dword [jus_memory], eax
  push esi
  call jus_write_file
  pop esi
  cmp dword [ata_status], IDE_ERROR
  je .error_during_writing_file
  
  mov eax, dword [jus_file_sector]
  mov dword [esi], eax
  
  ;save folder
  mov ebx, dword [fd_path_pointer]
  mov eax, dword [ebx]
  call jus_rewrite_folder
  cmp dword [ata_status], IDE_ERROR
  je .error_during_rewriting_folder
  
  ret
  
  ;SAVE FILE TO USB STICK
  .save_file_to_usb_stick:
  ;calculate if in folder is enough free space
  mov eax, dword [fat_sectors_per_cluster]
  mov ebx, 16
  mul ebx
  mov ebx, 0
  mov esi, MEMORY_FAT32_FOLDER
  mov ecx, 1024
  .fat32_find_free_entry:
   cmp dword [esi], 0
   je .fat32_free_entry
   inc ebx
   cmp ebx, eax
   je .no_free_entry
   add esi, 32
  loop .fat32_find_free_entry
  jmp .no_free_entry
  
  .fat32_free_entry:
  ;name
  mov dword [text_input_pointer], fd_fat_name
  mov dword [text_input_length], 8
  mov dword [cursor_line], LINESZ*2
  SCREEN_X_SUB eax, COLUMNSZ*10
  mov dword [cursor_column], eax
  push esi
  call text_input
  pop esi
  ;convert from unicode to ascii
  mov al, byte [fd_fat_name]
  mov byte [esi], al
  mov al, byte [fd_fat_name+2]
  mov byte [esi+1], al
  mov al, byte [fd_fat_name+4]
  mov byte [esi+2], al
  mov al, byte [fd_fat_name+6]
  mov byte [esi+3], al
  mov al, byte [fd_fat_name+8]
  mov byte [esi+4], al
  mov al, byte [fd_fat_name+10]
  mov byte [esi+5], al
  mov al, byte [fd_fat_name+12]
  mov byte [esi+6], al
  mov al, byte [fd_fat_name+14]
  mov byte [esi+7], al
  
  ;extension
  mov eax, dword [file_type]
  mov dword [esi+8], eax
  
  ;type of item
  mov byte [esi+11], 0x20
  mov word [esi+12], 0
  
  ;time
  call read_time
  mov eax, dword [year]
  sub eax, 1980
  shl eax, 9
  mov ebx, dword [month]
  shl ebx, 5
  or eax, ebx
  or eax, dword [day]
  
  mov ecx, dword [hour]
  shl ecx, 11
  mov ebx, dword [minute]
  shl ebx, 5
  or ecx, ebx
  or ecx, dword [second]
  
  mov word [esi+14], cx
  mov word [esi+16], ax
  mov word [esi+18], ax
  mov word [esi+22], cx
  mov word [esi+24], ax
  
  ;size
  mov eax, dword [file_size]
  mov ebx, 1024
  mul ebx ;convert from KB to bytes
  mov dword [esi+28], eax
  
  ;message
  push esi
  CLEAR_SCREEN 0x884E10 ;brown
  call file_dialog_draw_devices
  PRINT 'Saving file...', saving_file_str2, LINESZ*5, COLUMNSZ*22
  call redraw_screen
  pop esi
  
  ;write file and save first cluster
  mov eax, dword [file_size]
  mov dword [fat_file_length], eax
  mov eax, dword [file_memory]
  mov dword [fat_memory], eax
  push esi
  call fat_write_file
  pop esi
  cmp dword [msd_status], MSD_ERROR
  je .error_during_writing_file
  
  mov eax, dword [fat_first_file_cluster]
  shr eax, 16
  mov word [esi+20], ax
  mov eax, dword [fat_first_file_cluster]
  mov word [esi+26], ax
  
  ;save folder
  mov ebx, dword [fd_path_pointer]
  mov eax, dword [ebx]
  call fat_rewrite_folder
  cmp dword [msd_status], MSD_ERROR
  je .error_during_rewriting_folder
  
  ret
  
  ;ERRORS
  .no_free_entry:
   CLEAR_SCREEN 0x884E10 ;brown
   call file_dialog_draw_devices
   PRINT 'ERROR: not enough free memory for this file in this folder', not_enough_free_memory_in_folder_str, LINESZ*5, COLUMNSZ*22
   call redraw_screen
  jmp .halt
  
  .error_during_writing_file:
   CLEAR_SCREEN 0x884E10 ;brown
   call file_dialog_draw_devices
   PRINT 'error occured during writing file', error_during_writing_file_str, LINESZ*5, COLUMNSZ*22
   call redraw_screen
  jmp .halt
  
  .error_during_rewriting_folder:
   CLEAR_SCREEN 0x884E10 ;brown
   call file_dialog_draw_devices
   PRINT 'error occured during rewriting folder', error_during_rewriting_folder_str, LINESZ*5, COLUMNSZ*22
   call redraw_screen
  jmp .halt
  
  .error_too_many_folders:
   CLEAR_SCREEN 0x884E10 ;brown
   call file_dialog_draw_devices
   PRINT 'You can not open deeper that 50 folders', too_many_folders_str, LINESZ*5, COLUMNSZ*22
   call redraw_screen
  jmp .halt
