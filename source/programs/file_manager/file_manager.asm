;BleskOS

file_manager_up_str db 'File manager', 0
file_manager_down_str db '[F1] Rename [F2] Delete [F3] Copy [F4] Paste [F5] Create folder [b] Go back', 0
file_manager_usb_stick_string db 'Usb stick', 0

file_manager_delete_message_up db 'Are you sure you want to delete this file?', 0
file_manager_delete_message_down db '[enter] Yes [esc] No', 0
file_manager_not_empty_folder_message_up db 'You can delete only empty folder in this version of File manager', 0
file_manager_impossible_message_up db 'This is not possible in this version of File manager', 0
file_manager_impossible_message_down db '[enter] Ok', 0
file_manager_paste_message_up db 'Do you want to copy file?', 0
file_manager_paste_message_down db '[enter] Yes [esc] No', 0

fm_mouse_line dd 20
fm_mouse_column dd 0
%define FM_HDD 1
%define FM_CDROM 2
;3-10 reserved for usb sticks
%define FM_USB_UNKNOWN_FILESYSTEM 0xFF
fm_selected_device dd 0
fm_device_list times 6 dd 0

fm_path times 50 dd 0
fm_path_pointer dd fm_path
fm_folder_state dd 0
fm_first_show_entry_pointer dd MEMORY_FOLDER
fm_selected_entry dd 0

fm_copied_file_entry times 128 db 0
fm_copied_file_device dd 0

file_manager:
 mov dword [fm_folder_state], 2 
 call fm_redraw 
 call redraw_screen
 call fm_load_folder 
 cmp dword [fm_folder_state], 1
 jne .if_folder_loaded
  .redraw:
  call fm_redraw
  call redraw_screen
 .if_folder_loaded:
 
 mov dword [mcursor_up_side], 20
 mov dword [mcursor_left_side], 0
 SCREEN_Y_SUB eax, 20
 mov dword [mcursor_down_side], eax
 mov eax, dword [screen_x]
 mov dword [mcursor_right_side], eax

 .halt:
  call wait_for_usb_mouse
  
  cmp byte [key_code], KEY_ESC
  je main_window
  
  cmp dword [usb_mouse_data], 0
  jne .mouse_event
  
 cmp dword [fm_folder_state], 1
 jne .halt
  cmp byte [key_code], KEY_F4
  je .paste
  
  cmp byte [key_code], KEY_F5
  je .create_folder
 cmp dword [MEMORY_FOLDER], 0
 je .halt
  cmp byte [key_code], KEY_UP
  je .key_up
  
  cmp byte [key_code], KEY_DOWN
  je .key_down
  
  cmp byte [key_code], KEY_F1
  je .rename
  
  cmp byte [key_code], KEY_F2
  je .delete
  
  cmp byte [key_code], KEY_F3
  je .copy
  
  cmp byte [key_code], KEY_B
  je .go_back
 jmp .halt
 
 .key_up:
  cmp dword [fm_selected_entry], 0xFFFFFFFF
  je .halt
  cmp dword [fm_selected_entry], 0
  je .halt
  
  dec dword [fm_selected_entry]
  
  mov eax, dword [fm_selected_entry]
  shl eax, 7 ;mul 128
  add eax, MEMORY_FOLDER
  cmp dword [fm_first_show_entry_pointer], eax
  je .redraw
  jb .redraw
  sub dword [fm_first_show_entry_pointer], 128
 jmp .redraw
 
 .key_down:
  cmp dword [fm_selected_entry], 0xFFFFFFFF
  je .halt
  mov eax, dword [fm_selected_entry]
  inc eax
  shl eax, 7 ;mul 128
  add eax, MEMORY_FOLDER
  cmp dword [eax], 0
  je .halt
  
  inc dword [fm_selected_entry]
  
  mov eax, dword [fm_first_show_entry_pointer]
  sub eax, MEMORY_FOLDER
  shr eax, 7 ;div 128
  mov ecx, dword [fm_selected_entry]
  sub ecx, eax
  
  SCREEN_Y_SUB eax, 56
  mov ebx, 9
  mov edx, 0
  div ebx ;number of entries on screen
  cmp ecx, eax
  jb .redraw
  add dword [fm_first_show_entry_pointer], 128
 jmp .redraw
 
 .rename:
  cmp dword [fm_selected_entry], 0xFFFFFFFF
  je .halt
  mov eax, dword [fm_selected_device]
  cmp dword [fm_device_list+eax*4], FM_CDROM
  je .halt
  cmp dword [fm_device_list+eax*4], 0
  je .halt
  cmp dword [fm_device_list+eax*4], FM_USB_UNKNOWN_FILESYSTEM
  je .halt
  cmp dword [fm_device_list+eax*4], 3
  jb .rename_if_usb
   mov esi, file_manager_impossible_message_up
   mov edi, file_manager_impossible_message_down
   call show_message_window
   jmp .redraw
  .rename_if_usb:
 
  mov eax, dword [fm_selected_entry]
  shl eax, 7 ;mul 128
  add eax, MEMORY_FOLDER+16
  mov dword [text_input_pointer], eax
  mov dword [text_input_length], 49
  mov dword [cursor_line], 5
  SCREEN_X_SUB eax, COLUMNSZ*50
  mov dword [cursor_column], eax 
  call text_input
  
  mov dword [fm_folder_state], 3
  call fm_redraw
  call redraw_screen
  call fm_save_folder
  call fm_redraw
  call redraw_screen
 jmp .halt
 
 .delete:
  cmp dword [fm_selected_entry], 0xFFFFFFFF
  je .halt
  mov eax, dword [fm_selected_device]
  cmp dword [fm_device_list+eax*4], FM_CDROM
  je .halt
  cmp dword [fm_device_list+eax*4], 0
  je .halt
  cmp dword [fm_device_list+eax*4], FM_USB_UNKNOWN_FILESYSTEM
  je .halt
  cmp dword [fm_device_list+eax*4], 3
  jb .delete_if_usb
   mov esi, file_manager_impossible_message_up
   mov edi, file_manager_impossible_message_down
   call show_message_window
   jmp .redraw
  .delete_if_usb:
  mov eax, dword [fm_selected_entry]
  shl eax, 7 ;mul 128
  add eax, MEMORY_FOLDER
  
  ;delete only empty folder
  cmp word [eax+14], 1
  jne .delete_if_folder
   mov dword [MEMORY_ONE_SECTOR], 0xFFFFFFFF
   mov ebx, dword [eax]
   mov dword [ata_sector], ebx
   mov dword [ata_memory], MEMORY_ONE_SECTOR
   call read_hdd
   cmp dword [MEMORY_ONE_SECTOR], 0
   je .delete_if_folder ;this folder is empty
  
   mov esi, file_manager_not_empty_folder_message_up
   mov edi, file_manager_impossible_message_down
   call show_message_window
   jmp .redraw
  .delete_if_folder:
  
  mov esi, file_manager_delete_message_up
  mov edi, file_manager_delete_message_down
  call show_message_window
  cmp byte [key_code], KEY_ESC
  je .redraw
  
  mov eax, dword [fm_selected_entry]
  shl eax, 7 ;mul 128
  mov ecx, 0x100000
  sub ecx, eax
  add eax, MEMORY_FOLDER
  mov edi, eax
  mov esi, edi
  add esi, 128
  
  mov eax, dword [edi]
  mov dword [jus_file_sector], eax
  mov eax, dword [edi+4]
  mov dword [jus_file_size], eax
  pusha
  call jus_delete_file
  popa
  
  rep movsb ;delete from folder
  
  mov dword [fm_folder_state], 3
  call fm_redraw
  call redraw_screen
  call fm_save_folder
  call fm_redraw
  call redraw_screen
 jmp .halt
 
 .copy:
  cmp dword [fm_selected_entry], 0xFFFFFFFF
  je .halt
  
  mov eax, dword [fm_selected_entry]
  shl eax, 7 ;mul 128
  add eax, MEMORY_FOLDER
  cmp word [eax+14], 1
  jne .copy_if_folder
   mov esi, file_manager_impossible_message_up
   mov edi, file_manager_impossible_message_down
   call show_message_window
   jmp .redraw
  .copy_if_folder:
  mov esi, eax
  mov edi, fm_copied_file_entry
  mov ecx, 128
  rep movsb
  
  mov eax, dword [fm_selected_device]
  mov ebx, [fm_device_list+eax*4]
  mov dword [fm_copied_file_device], ebx
 jmp .redraw
 
 .paste:
  cmp dword [fm_copied_file_device], 0
  je .halt
  mov eax, dword [fm_selected_device]
  cmp dword [fm_device_list+eax*4], FM_CDROM
  je .halt
  
  mov esi, file_manager_paste_message_up
  mov edi, file_manager_paste_message_down
  call show_message_window
  cmp byte [key_code], KEY_ESC
  je .redraw
  
  ;READ FILE
  mov eax, dword [fm_copied_file_entry+4] ;here is length of file in KB
  mov ebx, 1000
  mov edx, 0
  div ebx
  inc eax
  mov dword [allocated_size], eax
  call allocate_memory ;allocate enough memory
  cmp dword [allocated_memory_pointer], 0
  jne .paste_if_not_enough_memory_for_file
   SCREEN_X_SUB eax, 150
   SCREEN_Y_SUB ebx, 40
   DRAW_SQUARE 20, 150, eax, ebx, 0xFF8E00
   mov dword [color], BLACK
   PRINT 'Not enough free memory for copy this file', paste_err_not_enough_free_mem_str, 28, 158
   call redraw_screen
   call wait_for_keyboard
   jmp .redraw
  .paste_if_not_enough_memory_for_file:
  
  SCREEN_X_SUB eax, 150
  SCREEN_Y_SUB ebx, 40
  DRAW_SQUARE 20, 150, eax, ebx, 0xFF8E00
  mov dword [color], BLACK
  PRINT 'Copying file...', copying_file_str, 28, 158
  call redraw_screen
  
  cmp dword [fm_copied_file_device], FM_HDD
  jne .paste_if_from_hdd
   mov eax, dword [allocated_memory_pointer]
   mov dword [jus_memory], eax
   mov eax, dword [fm_copied_file_entry]
   mov dword [jus_file_sector], eax
   mov eax, dword [fm_copied_file_entry+4]
   mov dword [jus_file_size], eax
   call select_hdd
   call jus_read_file
  
   cmp dword [ata_status], IDE_ERROR
   je .paste_error_during_pasting_file
  .paste_if_from_hdd:
  
  cmp dword [fm_copied_file_device], FM_CDROM
  jne .paste_if_from_cdrom
   mov eax, dword [allocated_memory_pointer]
   mov dword [iso9660_file_memory], eax
   mov eax, dword [fm_copied_file_entry]
   mov dword [iso9660_file_lba], eax
   mov eax, dword [fm_copied_file_entry+4]
   shr eax, 1 ;div 2 - convert from KB to cdrom sectors
   inc eax
   mov dword [iso9660_file_length], eax
   call iso9660_read_file
  
   cmp dword [atapi_status], IDE_ERROR
   je .paste_error_during_pasting_file
  .paste_if_from_cdrom:
  
  cmp dword [fm_copied_file_device], 3
  jb .paste_if_from_usb
   mov ebx, dword [fm_copied_file_device]
   sub ebx, 3
   mov dword [msd_number], ebx
   call select_msd
   call msd_read_mbr
   mov eax, dword [first_partition_lba]
   mov dword [fat_base_sector], eax
   call init_fat
   cmp dword [msd_status], MSD_ERROR
   je .paste_error_during_pasting_file
   
   mov eax, dword [allocated_memory_pointer]
   mov dword [fat_memory], eax
   mov eax, dword [fm_copied_file_entry]
   cmp eax, 0
   je .redraw
   mov dword [fat_entry], eax
   call fat_read_file
  
   cmp dword [msd_status], MSD_ERROR
   je .paste_error_during_pasting_file
  .paste_if_from_usb:
  
  ;WRITE FILE
  SCREEN_X_SUB eax, 150
  SCREEN_Y_SUB ebx, 40
  DRAW_SQUARE 20, 150, eax, ebx, 0xFF8E00
  mov dword [color], BLACK
  PRINT 'Writing file...', writing_file_str, 28, 158
  call redraw_screen
  
  mov eax, dword [fm_selected_device]
  cmp dword [fm_device_list+eax*4], FM_HDD
  jne .paste_if_to_hdd
   .save_file_to_hard_disk:
   mov edi, MEMORY_FOLDER
   mov ecx, 1024
   .hdd_find_free_entry:
    cmp dword [edi], 0
    je .hdd_free_entry
    add edi, 128
   loop .hdd_find_free_entry
   jmp .no_free_entry
   
   .hdd_free_entry:
   push edi
   mov esi, fm_copied_file_entry
   mov ecx, 128
   rep movsb ;copy file entry
   
   ;write file and save first sector
   mov eax, dword [fm_copied_file_entry+4]
   mov dword [jus_file_size], eax
   mov eax, dword [allocated_memory_pointer]
   mov dword [jus_memory], eax
   call select_hdd
   call jus_write_file
   pop edi
   cmp dword [ata_status], IDE_ERROR
   je .paste_error_during_pasting_file

   mov eax, dword [jus_file_sector]
   mov dword [edi], eax
  
   ;save folder
   mov ebx, dword [fm_path_pointer]
   mov eax, dword [ebx]
   call jus_rewrite_folder
   cmp dword [ata_status], IDE_ERROR
   je .paste_error_during_rewriting_folder
   
   jmp .paste_done
  .paste_if_to_hdd:
  
  mov eax, dword [fm_selected_device]
  cmp dword [fm_device_list+eax*4], 3
  jb .paste_if_to_usb
   mov ebx, dword [fm_device_list+eax*4]
   sub ebx, 3
   mov dword [msd_number], ebx
   call select_msd
   call msd_read_mbr
   mov eax, dword [first_partition_lba]
   mov dword [fat_base_sector], eax
   call init_fat
   cmp dword [msd_status], MSD_ERROR
   je .paste_error_during_pasting_file
   
   ;calculate if in folder is enough free space
   mov eax, dword [fat_sectors_per_cluster]
   mov ebx, 16
   mul ebx
   mov ebx, 0
   mov edi, MEMORY_FAT32_FOLDER
   mov ecx, 1024
   .fat32_find_free_entry:
    cmp dword [edi], 0
    je .fat32_free_entry
    inc ebx
    cmp ebx, eax
    je .no_free_entry
    add edi, 32
   loop .fat32_find_free_entry
   jmp .no_free_entry
   
   .fat32_free_entry:
   push edi
   
   ;copy name
   mov esi, fm_copied_file_entry+16
   mov ecx, 8
   .fat32_entry_copy_name:
    mov eax, 0
    mov ax, word [esi]
    mov byte [edi], al
    add esi, 2
    inc edi
   loop .fat32_entry_copy_name
   
   ;copy extenstion
   pop edi
   mov eax, dword [fm_copied_file_entry+116]
   mov dword [edi+8], eax
   
   ;type of item
   mov byte [edi+11], 0x20
   mov word [edi+12], 0
   
   ;time
   mov eax, 0
   mov ax, word [fm_copied_file_entry+8] ;year
   sub eax, 1980
   shl eax, 9
   mov ebx, 0
   mov bl, byte [fm_copied_file_entry+10] ;month
   shl ebx, 5
   or eax, ebx
   mov ebx, 0
   mov bl, byte [fm_copied_file_entry+11] ;day
   or eax, ebx
  
   mov ecx, 0 ;hour/minute/second
  
   mov word [edi+14], cx
   mov word [edi+16], ax
   mov word [edi+18], ax
   mov word [edi+22], cx
   mov word [edi+24], ax
  
   ;size
   mov eax, dword [fm_copied_file_entry+4]
   mov ebx, 1024
   mul ebx ;convert from KB to bytes
   mov dword [edi+28], eax
   
   ;write file and save first cluster
   mov eax, dword [fm_copied_file_entry+4]
   mov dword [fat_file_length], eax
   mov eax, dword [allocated_memory_pointer]
   mov dword [fat_memory], eax
   push edi
   call fat_write_file
   pop edi
   cmp dword [msd_status], MSD_ERROR
   je .paste_error_during_pasting_file
  
   mov eax, dword [fat_first_file_cluster]
   shr eax, 16
   mov word [edi+20], ax
   mov eax, dword [fat_first_file_cluster]
   mov word [edi+26], ax
  
   ;save folder
   mov ebx, dword [fm_path_pointer]
   mov eax, dword [ebx]
   call fat_rewrite_folder
   cmp dword [msd_status], MSD_ERROR
   je .paste_error_during_rewriting_folder
  
   jmp .paste_done
  .paste_if_to_usb:
  
  .paste_done:
  call release_memory
 jmp file_manager
  .paste_error_during_pasting_file:
   SCREEN_X_SUB eax, 150
   SCREEN_Y_SUB ebx, 40
   DRAW_SQUARE 20, 150, eax, ebx, 0xFF8E00
   mov dword [color], BLACK
   PRINT 'Error occured during copying file', paste_err_copying_str, 28, 158
   call redraw_screen
   call wait_for_keyboard
  jmp .paste_done
  
  .paste_error_during_rewriting_folder:
   SCREEN_X_SUB eax, 150
   SCREEN_Y_SUB ebx, 40
   DRAW_SQUARE 20, 150, eax, ebx, 0xFF8E00
   mov dword [color], BLACK
   PRINT 'Error occured during rewriting folder', paste_err_saving_folder_str, 28, 158
   call redraw_screen
   call wait_for_keyboard
  jmp .paste_done
  
  .no_free_entry:
   SCREEN_X_SUB eax, 150
   SCREEN_Y_SUB ebx, 40
   DRAW_SQUARE 20, 150, eax, ebx, 0xFF8E00
   mov dword [color], BLACK
   PRINT 'There is no free entry in this folder', paste_err_no_free_entry_str, 28, 158
   call redraw_screen
   call wait_for_keyboard
  jmp .paste_done
  
 .create_folder:
  mov eax, dword [fm_selected_device]
  mov ebx, dword [fm_device_list+eax*4]
  cmp ebx, FM_HDD
  jne .halt
  
  mov esi, MEMORY_FOLDER
  mov ecx, 1024
  .create_folder_hdd_find_free_entry:
   cmp dword [esi], 0
   je .create_folder_hdd_free_entry
   add esi, 128
  loop .create_folder_hdd_find_free_entry
  jmp .no_free_entry
  
  .create_folder_hdd_free_entry:
  ;clear entry
  mov edi, esi
  mov eax, 0
  mov ecx, 128
  rep stosb ;clear entry
   
  ;name
  mov eax, esi
  add eax, 16
  mov dword [text_input_pointer], eax
  mov dword [text_input_length], 49
  mov dword [cursor_line], 5
  SCREEN_X_SUB eax, COLUMNSZ*50
  mov dword [cursor_column], eax
  push esi
  call text_input
  pop esi
  cmp byte [key_code], KEY_ESC
  jne .create_folder_key_esc
   mov edi, esi
   mov eax, 0
   mov ecx, 128
   rep stosb ;delete entry
   jmp .redraw
  .create_folder_key_esc:
  
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
  mov word [esi+14], 1 ;folder
  
  ;size
  mov dword [esi+4], 128 ;every folder has 128 KB
  
  ;write file and save first sector
  mov dword [allocated_size], 1
  push esi
  call allocate_memory
  pop esi
  cmp dword [allocated_memory_pointer], 0
  je .create_folder_no_memory
  
  mov edi, dword [allocated_memory_pointer]
  mov ecx, 0x100000
  mov eax, 0
  rep stosb
  mov dword [jus_file_size], 128
  mov eax, dword [allocated_memory_pointer]
  mov dword [jus_memory], eax
  push esi
  
  pusha
  SCREEN_X_SUB eax, 150
  SCREEN_Y_SUB ebx, 40
  DRAW_SQUARE 20, 150, eax, ebx, 0xFF8E00
  mov dword [color], BLACK
  PRINT 'Writing folder...', writing_folder_str, 28, 158
  call redraw_screen
  popa
  
  call jus_write_file
  pop esi
  cmp dword [ata_status], IDE_ERROR
  je .create_folder_error
  
  mov eax, dword [jus_file_sector]
  mov dword [esi], eax
  
  ;save folder  
  mov ebx, dword [fm_path_pointer]
  mov eax, dword [ebx]
  call jus_rewrite_folder
  cmp dword [ata_status], IDE_ERROR
  je .create_folder_error
  
  .create_folder_done:
  call release_memory
 jmp file_manager
  .create_folder_no_memory:
   SCREEN_X_SUB eax, 150
   SCREEN_Y_SUB ebx, 40
   DRAW_SQUARE 20, 150, eax, ebx, 0xFF8E00
   mov dword [color], BLACK
   PRINT 'Not enough free memory for creating new folder', create_folder_err_no_free_mem_str, 28, 158
   call redraw_screen
   call wait_for_keyboard
  jmp file_manager
  
  .create_folder_error:
   SCREEN_X_SUB eax, 150
   SCREEN_Y_SUB ebx, 40
   DRAW_SQUARE 20, 150, eax, ebx, 0xFF8E00
   mov dword [color], BLACK
   PRINT 'Error occured during creating folder', create_folder_err_str, 28, 158
   call redraw_screen
   call wait_for_keyboard
  jmp .create_folder_done
 
 .go_back:
  cmp dword [fm_path_pointer], fm_path
  je .halt
  
  mov eax, dword [fm_path_pointer]
  mov dword [eax], 0
  sub dword [fm_path_pointer], 4
 jmp file_manager
 
 .mouse_event:
  mov eax, dword [fm_mouse_line]
  mov dword [cursor_line], eax
  mov eax, dword [fm_mouse_column]
  mov dword [cursor_column], eax
  call move_mouse_cursor
  mov eax, dword [cursor_line]
  mov dword [fm_mouse_line], eax
  mov eax, dword [cursor_column]
  mov dword [fm_mouse_column], eax
  
  cmp dword [usb_mouse_dnd], 0x1
  je .mouse_click
  cmp dword [usb_mouse_dnd], 0x2
  je .mouse_click_select_file
 jmp .halt
 
 .mouse_click:
  TEST_CLICK_ZONE_WITH_JUMP change_device, fm_mouse_line, fm_mouse_column, 43, 43+(23*5), 0, 150, .change_device
  
 .mouse_click_select_file:
  cmp dword [fm_folder_state], 1
  jne .halt
  
  SCREEN_Y_SUB eax, 43
  mov ebx, eax
  add ebx, 23
  TEST_CLICK_ZONE_WITH_JUMP button_go_back, fm_mouse_line, fm_mouse_column, eax, ebx, 0, 150, .go_back
  
  SCREEN_Y_SUB eax, 28
  SCREEN_X_SUB ebx, COLUMNSZ
  TEST_CLICK_ZONE_WITH_JUMP click_select_file, fm_mouse_line, fm_mouse_column, 28, eax, 158, ebx, .click_select_file
  cmp dword [fm_selected_entry], 0xFFFFFFFF
  je .halt
  mov dword [fm_selected_entry], 0xFFFFFFFF
 jmp .redraw
 
 .change_device:
  mov eax, dword [fm_mouse_line]
  sub eax, 43
  mov ebx, 23
  mov edx, 0
  div ebx ;in eax is number of clicked item
  
  cmp eax, dword [fm_selected_device]
  je .halt
  
  mov ebx, dword [fm_device_list+eax*4]
  cmp ebx, 0
  je .halt
  
  push eax
  cmp ebx, FM_HDD
  jne .if_device_hdd
   call select_hdd
   jmp .device_selected
  .if_device_hdd:
  
  cmp ebx, FM_CDROM
  jne .if_device_cdrom
   call select_optical_disk
   jmp .device_selected
  .if_device_cdrom:
  
  cmp ebx, FM_USB_UNKNOWN_FILESYSTEM
  jne .if_unknown_filesystem
   pop eax
   mov dword [fm_selected_device], eax
   jmp .redraw
  .if_unknown_filesystem:
  
  cmp ebx, 3
  jb .device_selected
   sub ebx, 3
   mov dword [msd_number], ebx
   call select_msd
   call msd_read_mbr
   mov eax, dword [first_partition_lba]
   mov dword [fat_base_sector], eax
   call init_fat
   cmp dword [msd_status], MSD_ERROR
   jne .if_selected_device_usb_reading_error
    pop eax
    mov dword [fm_selected_device], eax
    call fm_load_folder.reading_folder_error
    jmp .halt
   .if_selected_device_usb_reading_error:
  
  .device_selected:
  pop eax
  
  mov dword [fm_selected_device], eax
  mov dword [fm_selected_entry], 0
  mov dword [fm_first_show_entry_pointer], MEMORY_FOLDER
  mov dword [fm_path_pointer], fm_path
  mov edi, fm_path
  mov ecx, 20
  mov eax, 0
  rep stosd
 jmp file_manager
 
 .click_select_file:
  mov eax, dword [fm_mouse_line]
  sub eax, 28
  mov ebx, 9
  mov edx, 0
  div ebx
  
  cmp eax, dword [fm_selected_entry]
  jne .if_open_file
   shl eax, 7 ;mul 128
   add eax, MEMORY_FOLDER
   
   cmp word [eax+14], 1
   jne .if_open_folder
    cmp dword [fm_path_pointer], fm_path+49*4
    je .error_too_many_folders
    
    add dword [fm_path_pointer], 4
    mov ebx, dword [fm_path_pointer]
    mov ecx, dword [eax] ;pointer to folder data
    mov dword [ebx], ecx
    
    mov dword [fm_selected_entry], 0
    mov dword [fm_first_show_entry_pointer], MEMORY_FOLDER
    jmp file_manager
   .if_open_folder:
   
   jmp .halt
  .if_open_file:
  
  mov dword [fm_selected_entry], eax
  shl eax, 7 ;mul 128
  add eax, dword [fm_first_show_entry_pointer]
  cmp dword [eax], 0
  je .click_unselect_file
 jmp .redraw
  .click_unselect_file:
  mov dword [fm_selected_entry], 0xFFFFFFFF
 jmp .redraw
  .error_too_many_folders:
   SCREEN_X_SUB eax, 150
   SCREEN_Y_SUB ebx, 40
   DRAW_SQUARE 20, 150, eax, ebx, 0xFF8E00
   PRINT 'You can not open deeper than 50 folders [enter] Go back', error_too_many_folders_str, 28, 158
   call redraw_screen
   .error_too_many_folders_halt:
    call wait_for_usb_keyboard
    
    cmp byte [key_code], KEY_ESC
    je .redraw
    cmp byte [key_code], KEY_ENTER
    je .redraw
   jmp .error_too_many_folders
 
fm_redraw:
 DRAW_WINDOW file_manager_up_str, file_manager_down_str, 0x884C00, 0xFF8E00
 
 ;draw side panel
 SCREEN_Y_SUB eax, 40
 DRAW_SQUARE 20, 0, 150, eax, 0xC55E00
 mov dword [color], BLACK
 PRINT 'Devices:', devices_str, 28, 8
 
 mov eax, dword [fm_selected_device]
 mov ebx, 23
 mul ebx
 add eax, 43
 DRAW_SQUARE eax, 0, 150, 23, 0xFF0000 ;selected device have red square
 mov dword [color], BLACK
 
 ;create actual device list
 mov edi, fm_device_list
 mov eax, 0
 mov ecx, 6
 rep stosd ;clear device list
 mov dword [fm_device_list], FM_HDD
 mov dword [fm_device_list+4], FM_CDROM
 mov esi, fm_device_list+8
 mov edi, mass_storage_devices+7
 mov eax, 3
 mov ecx, 4
 .find_connected_usb:
  cmp byte [edi], NO_MSD
  je .next_usb_entry
  cmp byte [edi], MSD_FAT32
  je .usb_fat32
  
  mov dword [esi], FM_USB_UNKNOWN_FILESYSTEM
  add esi, 4
  jmp .next_usb_entry
  
  .usb_fat32:
  mov dword [esi], eax
  add esi, 4
  
 .next_usb_entry: 
 add edi, 16
 inc eax
 loop .find_connected_usb
 
 ;print names of devices
 PRINT 'Hard disk', hard_disk_str, 51+(23*0), 8
 PRINT 'CDROM', cdrom_str, 51+(23*1), 8
 mov dword [cursor_line], 51+(23*2)
 mov dword [cursor_column], 8
 mov edi, fm_device_list+8
 mov ecx, 4
 .print_usb_stick_strings:
  cmp dword [edi], 3
  jb .usb_stick_names_printed
  
  mov eax, dword [edi]
  sub eax, 3
  mov ebx, 12
  mul ebx
  add eax, mass_storage_device_label
  mov esi, eax
  cmp dword [edi], FM_USB_UNKNOWN_FILESYSTEM
  jne .if_usb_unknown_filesystem
   mov esi, file_manager_usb_stick_string
  .if_usb_unknown_filesystem:
  push edi
  push ecx
  call print
  pop ecx
  pop edi
  
  add dword [cursor_line], 23
  mov dword [cursor_column], 8
  add edi, 4
 loop .print_usb_stick_strings
 .usb_stick_names_printed:
 
 ;draw buttons
 cmp dword [fm_path_pointer], fm_path
 je .if_draw_button_go_back
  SCREEN_Y_SUB eax, 43
  DRAW_SQUARE eax, 0, 150, 23, 0x028800
  SCREEN_Y_SUB eax, 35
  mov dword [color], BLACK
  PRINT 'Go back', go_back_button_str, eax, 75-(COLUMNSZ*7/2)
 .if_draw_button_go_back:
 
 ;print up string in files area
 mov eax, dword [fm_selected_device]
 mov ebx, dword [fm_device_list+eax*4]
 cmp ebx, FM_USB_UNKNOWN_FILESYSTEM
 jne .if_print_str_usb_unknown_filesystem
  PRINT 'This USB stick is not formatted as FAT32', usb_unknow_filesystem_str, 28, 158
  jmp .done
 .if_print_str_usb_unknown_filesystem:
 cmp dword [fm_folder_state], 0
 jne .if_folder_state_0
  jmp .done
 .if_folder_state_0:
 cmp dword [fm_folder_state], 2
 jne .if_reading_folder
  PRINT 'Reading folder...', reading_folder_str, 28, 158
  jmp .done
 .if_reading_folder:
 cmp dword [fm_folder_state], 3
 jne .if_writing_folder
  PRINT 'Writing folder...', writing_folder_str, 28, 158
  jmp .done
 .if_writing_folder:
 cmp dword [MEMORY_FOLDER], 0
 jne .if_empty_folder
  PRINT 'This folder is empty', this_folder_is_empty_str, 28, 158
  jmp .done
 .if_empty_folder:
 
 ;highlight selected entry
 cmp dword [fm_selected_entry], 0xFFFFFFFF
 je .if_highlight_entry
  mov eax, dword [fm_first_show_entry_pointer]
  sub eax, MEMORY_FOLDER
  shr eax, 7 ;div 128 - number of first showed entry
  mov ebx, dword [fm_selected_entry]
  sub ebx, eax
  mov eax, ebx
  mov ebx, 9
  mul ebx
  add eax, 28
  mov dword [cursor_line], eax
  mov dword [cursor_column], 157
  SCREEN_X_SUB eax, 158+7
  mov dword [square_length], eax
  mov dword [square_heigth], 10
  mov dword [color], 0xFF0000
  call draw_square
 .if_highlight_entry:
 
 ;print files
 mov dword [color], BLACK
 mov dword [cursor_line], 29
 mov edi, dword [fm_first_show_entry_pointer]
 SCREEN_Y_SUB eax, 56
 mov ebx, 9
 mov edx, 0
 div ebx
 mov ecx, eax ;number of entries on screen
 .print_file_entry:
 cmp dword [edi], 0
 je .done
 push ecx
 
  ;print name
  mov dword [cursor_column], 158
  mov esi, edi
  add esi, 16
  push edi
  call print_unicode
  pop edi
  
  ;print type
  SCREEN_X_SUB eax, COLUMNSZ*4
  mov dword [cursor_column], eax
  mov esi, edi
  add esi, 116
  push edi
  call print_ascii
  pop edi
  
  ;print size
  push edi
  SCREEN_X_SUB eax, COLUMNSZ*14
  mov dword [cursor_column], eax
  mov eax, dword [edi+4]
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
  jmp .print_B
  
  ;size KB
  .size_kb:
  mov dword [var_print_value], eax
  call print_var
  add dword [cursor_column], COLUMNSZ
  mov dword [char_for_print], 'K'
  call print_char
  jmp .print_B
  
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

  .print_B:
  add dword [cursor_column], COLUMNSZ
  mov dword [char_for_print], 'B'
  call print_char
  pop edi
  
  ;print time
  SCREEN_X_SUB eax, COLUMNSZ*25
  mov dword [cursor_column], eax
  mov eax, 0
  mov ax, word [edi+8] ;year
  mov dword [var_print_value], eax
  push edi
  call print_var  
  mov dword [char_for_print], '/'
  call print_char
  add dword [cursor_column], COLUMNSZ
  pop edi
  
  mov eax, 0
  mov al, byte [edi+10] ;month
  and eax, 0x1F
  cmp eax, 9
  ja .print_month
   push edi 
   push eax
   mov dword [char_for_print], '0'
   call print_char
   add dword [cursor_column], COLUMNSZ
   pop eax
   pop edi
  .print_month:
  mov dword [var_print_value], eax
  push edi
  call print_var
  mov dword [char_for_print], '/'
  call print_char
  add dword [cursor_column], COLUMNSZ
  pop edi
  
  mov eax, 0
  mov al, byte [edi+11] ;day
  and eax, 0x1F
  cmp eax, 9
  ja .print_day
   push edi 
   push eax
   mov dword [char_for_print], '0'
   call print_char
   add dword [cursor_column], COLUMNSZ
   pop eax
   pop edi
  .print_day:
  mov dword [var_print_value], eax
  push edi
  call print_var
  pop edi
  
 add edi, 128
 add dword [cursor_line], 9
 pop ecx
 dec ecx
 cmp ecx, 0
 jne .print_file_entry
 
 ;update mouse cursor
 .done:
 mov eax, dword [fm_mouse_line]
 mov dword [cursor_line], eax
 mov eax, dword [fm_mouse_column]
 mov dword [cursor_column], eax
 call read_cursor_bg
 call draw_cursor
 
 ret

fm_load_folder:
 mov edi, MEMORY_FOLDER
 mov ecx, 0x100000
 mov eax, 0
 rep stosb

 mov eax, dword [fm_selected_device]
 mov ebx, dword [fm_device_list+eax*4]
 
 cmp ebx, FM_HDD
 je .hard_disk
 cmp ebx, FM_CDROM
 je .cdrom
 cmp ebx, FM_USB_UNKNOWN_FILESYSTEM
 je .done
 jmp .usb_stick
 .done:
 ret
 
 .hard_disk:
  mov ebx, dword [fm_path_pointer]
  mov eax, dword [ebx]
  call jus_read_folder
  
  cmp dword [ata_status], IDE_ERROR
  je .reading_folder_error
  
  mov dword [fm_folder_state], 1
  ret
  
 .cdrom:
  call detect_optical_disk
  cmp dword [disk_state], NO_DISK
  je .no_disk
  cmp dword [disk_state], UNKNOWN_DISK_FORMAT
  je .unknown_disk_format

  mov ebx, dword [fm_path_pointer]
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

  mov dword [fm_folder_state], 1
  call convert_iso9660_folder_to_jus_folder
  ret
  
  .no_disk:
   SCREEN_X_SUB eax, 150
   SCREEN_Y_SUB ebx, 40
   DRAW_SQUARE 20, 150, eax, ebx, 0xFF8E00
   mov dword [color], BLACK
   PRINT 'No disk is inserted', no_disk_str, 28, 158
   call redraw_screen
   mov dword [fm_folder_state], 0
   ret
   
  .unknown_disk_format:
   SCREEN_X_SUB eax, 150
   SCREEN_Y_SUB ebx, 40
   DRAW_SQUARE 20, 150, eax, ebx, 0xFF8E00
   mov dword [color], BLACK
   PRINT 'This disk is not formatted as ISO9660', not_iso9660_format_str, 28, 158
   call redraw_screen
   mov dword [fm_folder_state], 0
   ret
 
 .usb_stick:   
  mov eax, ebx
  sub eax, 3
  mov ebx, 16
  mul ebx
  add eax, mass_storage_devices+7
  cmp byte [eax], NO_MSD
  je .no_usb_stick
  cmp byte [eax], MSD_FAT32
  jne .usb_stick_not_fat32
  
  mov eax, dword [fm_selected_device]
  mov ebx, dword [fm_device_list+eax*4]
  sub ebx, 3
  mov dword [msd_number], ebx
  call select_msd
  call msd_read_mbr
  mov eax, dword [first_partition_lba]
  mov dword [fat_base_sector], eax
  call init_fat
  cmp dword [msd_status], MSD_ERROR
  je .reading_folder_error
  
  ;this is usb stick formatted as FAT32
  mov ebx, dword [fm_path_pointer]
  mov eax, dword [ebx]
  
  call fat_read_folder
  cmp dword [msd_status], MSD_ERROR
  je .reading_folder_error

  mov dword [fm_folder_state], 1
  ret
  
  .no_usb_stick:
   SCREEN_X_SUB eax, 150
   SCREEN_Y_SUB ebx, 40
   DRAW_SQUARE 20, 150, eax, ebx, 0xFF8E00
   mov dword [color], BLACK
   PRINT 'No USB stick is connected here', no_usb_stick_str, 28, 158
   call redraw_screen
   mov dword [fm_folder_state], 0
   ret
   
  .usb_stick_not_fat32:
   SCREEN_X_SUB eax, 150
   SCREEN_Y_SUB ebx, 40
   DRAW_SQUARE 20, 150, eax, ebx, 0xFF8E00
   mov dword [color], BLACK
   PRINT 'This USB stick is not formatted as FAT32', usb_stick_not_formatted_as_FAT32_str, 28, 158
   call redraw_screen
   mov dword [fm_folder_state], 0
   ret
   
  .reading_folder_error:
   SCREEN_X_SUB eax, 150
   SCREEN_Y_SUB ebx, 40
   DRAW_SQUARE 20, 150, eax, ebx, 0xFF8E00
   mov dword [color], BLACK
   PRINT 'Error during reading folder', reading_folder_error_str, 28, 158
   call redraw_screen
   mov dword [fm_folder_state], 0
   ret

fm_save_folder:
 mov eax, dword [fm_selected_device]
 mov ebx, dword [fm_device_list+eax*4]
 
 cmp ebx, FM_HDD
 je .hard_disk
 cmp ebx, FM_CDROM
 je .done
 cmp ebx, FM_USB_UNKNOWN_FILESYSTEM
 je .done
 jmp .usb_stick
 .done:
 ret
 
 .hard_disk:
  mov ebx, dword [fm_path_pointer]
  mov eax, dword [ebx]
  call jus_rewrite_folder
  
  cmp dword [ata_status], IDE_ERROR
  je .writing_folder_error
  
  mov dword [fm_folder_state], 1
  ret
 
 .usb_stick:
  mov eax, ebx
  sub eax, 3
  mov ebx, 16
  mul ebx
  add eax, mass_storage_devices+7
  cmp byte [eax], NO_MSD
  je .no_usb_stick
  cmp byte [eax], MSD_FAT32
  jne .usb_stick_not_fat32
  
  mov eax, dword [fm_selected_device]
  mov ebx, dword [fm_device_list+eax*4]
  sub ebx, 3
  mov dword [msd_number], ebx
  call select_msd
  call msd_read_mbr
  mov eax, dword [first_partition_lba]
  mov dword [fat_base_sector], eax
  call init_fat
  cmp dword [msd_status], MSD_ERROR
  je .writing_folder_error
  
  ;this is usb stick formatted as FAT32
  mov ebx, dword [fm_path_pointer]
  mov eax, dword [ebx]
  call fat_rewrite_folder
  cmp dword [msd_status], MSD_ERROR
  je .writing_folder_error
  
  mov dword [fm_folder_state], 1
  ret
  
  .no_usb_stick:
   SCREEN_X_SUB eax, 150
   SCREEN_Y_SUB ebx, 40
   DRAW_SQUARE 20, 150, eax, ebx, 0xFF8E00
   mov dword [color], BLACK
   PRINT 'No USB stick is connected here', no_usb_stick_str, 28, 158
   call redraw_screen
   mov dword [fm_folder_state], 0
   ret
   
  .usb_stick_not_fat32:
   SCREEN_X_SUB eax, 150
   SCREEN_Y_SUB ebx, 40
   DRAW_SQUARE 20, 150, eax, ebx, 0xFF8E00
   mov dword [color], BLACK
   PRINT 'This USB stick is not formatted as FAT32', usb_stick_not_formatted_as_FAT32_str, 28, 158
   call redraw_screen
   mov dword [fm_folder_state], 0
   ret
   
  .writing_folder_error:
   SCREEN_X_SUB eax, 150
   SCREEN_Y_SUB ebx, 40
   DRAW_SQUARE 20, 150, eax, ebx, 0xFF8E00
   mov dword [color], BLACK
   PRINT 'Error during writing folder', reading_folder_error_str, 28, 158
   call redraw_screen
   mov dword [fm_folder_state], 0
   ret

fm_load_file:
 mov ebx, dword [fm_selected_entry]
 shl ebx, 7 ;mul 128
 add ebx, MEMORY_FOLDER
 
 mov eax, dword [ebx+4] ;here is length of file in KB
 mov ebx, 1000
 mov edx, 0
 div ebx
 inc eax
 mov dword [allocated_size], eax
 call allocate_memory ;allocate enough memory
 cmp dword [allocated_memory_pointer], 0
 jne .if_not_enough_memory_for_file
  SCREEN_X_SUB eax, 150
  SCREEN_Y_SUB ebx, 40
  DRAW_SQUARE 20, 150, eax, ebx, 0xFF8E00
  mov dword [color], BLACK
  PRINT 'Not enough free memory for this file', err_not_enough_free_mem_str, 28, 158
  call redraw_screen
  call wait_for_keyboard
  mov eax, 0
  ret
 .if_not_enough_memory_for_file:
  
 SCREEN_X_SUB eax, 150
 SCREEN_Y_SUB ebx, 40
 DRAW_SQUARE 20, 150, eax, ebx, 0xFF8E00
 mov dword [color], BLACK
 PRINT 'Reading file...', reading_file_str, 28, 158
 call redraw_screen
  
 mov eax, dword [fm_selected_device]
 mov ebx, dword [fm_device_list+eax*4]
 cmp dword [ebx], FM_HDD
 jne .if_hdd
  mov eax, dword [allocated_memory_pointer]
  mov dword [jus_memory], eax
  mov eax, dword [fm_copied_file_entry]
  mov dword [jus_file_sector], eax
  mov eax, dword [fm_copied_file_entry+4]
  mov dword [jus_file_size], eax
  call select_hdd
  call jus_read_file
 
 ;; cmp dword [ata_status], IDE_ERROR
 ;; je .error_during_reading_file
 .if_hdd:
  
  cmp dword [fm_copied_file_device], FM_CDROM
  jne .paste_if_from_cdrom
   mov eax, dword [allocated_memory_pointer]
   mov dword [iso9660_file_memory], eax
   mov eax, dword [fm_copied_file_entry]
   mov dword [iso9660_file_lba], eax
   mov eax, dword [fm_copied_file_entry+4]
   shr eax, 1 ;div 2 - convert from KB to cdrom sectors
   inc eax
   mov dword [iso9660_file_length], eax
   call iso9660_read_file
  
 ;;  cmp dword [atapi_status], IDE_ERROR
 ;;  je .paste_error_during_pasting_file
  .paste_if_from_cdrom:
  
  cmp dword [fm_copied_file_device], 3
  jb .paste_if_from_usb
   mov ebx, dword [fm_copied_file_device]
   sub ebx, 3
   mov dword [msd_number], ebx
   call select_msd
   call msd_read_mbr
   mov eax, dword [first_partition_lba]
   mov dword [fat_base_sector], eax
   call init_fat
 ;;  cmp dword [msd_status], MSD_ERROR
 ;;  je .paste_error_during_pasting_file
   
   mov eax, dword [allocated_memory_pointer]
   mov dword [fat_memory], eax
   mov eax, dword [fm_copied_file_entry]
   ;cmp eax, 0
   ;je .redraw
   mov dword [fat_entry], eax
   call fat_read_file
  
  ;; cmp dword [msd_status], MSD_ERROR
  ;; je .paste_error_during_pasting_file
  .paste_if_from_usb:
