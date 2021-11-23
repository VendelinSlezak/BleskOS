;BleskOS

; this library is for save and open dialogs

;DEFINITIONS
;type of medium
%define FD_MEDIUM_RAM 1
%define FD_MEDIUM_HDD 2

%define FD_NO_FILE 0
%define FD_FILE_RAM_IMAGE 1
%define FD_FILE 2
%define FD_WRITING_OK 3

;VARIABILES
file_memory dd 0
file_size dd 0
fd_type_of_dialog_str dd 0
fd_selected_medium dd 0
fd_return dd 0

fd_ram_image_1 dd MEMORY_RAM_IMAGE_1
fd_ram_image_2 dd MEMORY_RAM_IMAGE_2
fd_ram_image_3 dd MEMORY_RAM_IMAGE_3
fd_ram_image_4 dd MEMORY_RAM_IMAGE_4

;STRINGS
fd_open_str db 'Open file dialog', 0
fd_save_str db 'Save file dialog', 0

;METHODS
library_fd_draw_bg:
 CLEAR_SCREEN 0x884E10 ;brown
 SCREEN_X_SUB eax, COLUMN(2)
 
 ;draw up square
 DRAW_SQUARE LINE(1), COLUMN(1), eax, LINESZ*3-1, 0x60BD00 ;green
 mov dword [cursor_line], LINE(2)
 mov dword [cursor_column], COLUMN(2)
 mov dword [color], BLACK
 mov esi, dword [fd_type_of_dialog_str]
 mov dword [size_of_text], 1
 mov dword [type_of_text], PLAIN
 call print
 
 ;draw selected medium square
 mov eax, dword [fd_selected_medium]
 mov ebx, 13
 mul ebx
 add eax, LINESZ*4-5
 DRAW_SQUARE eax, COLUMN(1), COLUMNSZ*17, LINESZ*2, 0xFF0000 ;red
 
 ;print strings of mediums
 PRINT '[F3] RAM memory', ram_memory_str, (LINESZ*4-5)+(LINESZ*2)*1, COLUMN(2)
 PRINT '[F4] Hard disk', hard_disk_str, (LINESZ*4-5)+(LINESZ*2)*2, COLUMN(2)
 
 ;draw medium files
 cmp dword [fd_selected_medium], FD_MEDIUM_RAM
 jne .if_ram
  PRINT '[1] First image [2] Second image [3] Third image [4] Fourth image', ram_memory_files_str, LINE(5), COLUMN(20)
  jmp .redraw
 .if_ram:
 
 .redraw:
 call redraw_screen
 
 ret
 
file_dialog_open:
 mov dword [fd_type_of_dialog_str], fd_open_str
 
 ;MEDIUM RAM
 .medium_ram:
  mov dword [fd_selected_medium], FD_MEDIUM_RAM
  call library_fd_draw_bg
 
  .ram_halt:
   call wait_for_keyboard
   
   cmp byte [key_code], KEY_ESC
   je .done
  
   cmp byte [key_code], KEY_F4
   je .medium_hard_disk
   
   cmp byte [key_code], KEY_1
   je .ram_image_1
   
   cmp byte [key_code], KEY_2
   je .ram_image_2
   
   cmp byte [key_code], KEY_3
   je .ram_image_3
   
   cmp byte [key_code], KEY_4
   je .ram_image_4
  jmp .ram_halt
  
  .ram_image_1:
   mov dword [file_memory], MEMORY_RAM_IMAGE_1
   mov dword [file_size], 0x100000
   mov dword [fd_return], FD_FILE_RAM_IMAGE
   ret
   
  .ram_image_2:
   mov dword [file_memory], MEMORY_RAM_IMAGE_2
   mov dword [file_size], 0x100000
   mov dword [fd_return], FD_FILE_RAM_IMAGE
   ret
   
  .ram_image_3:
   mov dword [file_memory], MEMORY_RAM_IMAGE_3
   mov dword [file_size], 0x100000
   mov dword [fd_return], FD_FILE_RAM_IMAGE
   ret
   
  .ram_image_4:
   mov dword [file_memory], MEMORY_RAM_IMAGE_4
   mov dword [file_size], 0x100000
   mov dword [fd_return], FD_FILE_RAM_IMAGE
   ret
 
 ;MEDIUM HARD DISK
 .medium_hard_disk:
  mov dword [fd_selected_medium], FD_MEDIUM_HDD
  call library_fd_draw_bg
  
  .hdd_halt:
   call wait_for_keyboard
   
   cmp byte [key_code], KEY_ESC
   je .done
   
   cmp byte [key_code], KEY_F3
   je .medium_ram
  jmp .hdd_halt
  
 .done:
 mov dword [fd_return], FD_NO_FILE
 ret

file_dialog_save:
 mov dword [fd_type_of_dialog_str], fd_save_str
 
 ;MEDIUM RAM
 .medium_ram:
  mov dword [fd_selected_medium], FD_MEDIUM_RAM
  call library_fd_draw_bg
 
  .ram_halt:
   call wait_for_keyboard
   
   cmp byte [key_code], KEY_ESC
   je .done
  
   cmp byte [key_code], KEY_F4
   je .medium_hard_disk
   
   cmp byte [key_code], KEY_1
   je .ram_image_1
   
   cmp byte [key_code], KEY_2
   je .ram_image_2
   
   cmp byte [key_code], KEY_3
   je .ram_image_3
   
   cmp byte [key_code], KEY_4
   je .ram_image_4
  jmp .ram_halt
  
  .ram_image_1:
   mov esi, dword [file_memory]
   mov edi, MEMORY_RAM_IMAGE_1
   mov ecx, 0x100000 ;one MB
   rep movsb ;save
   mov dword [fd_return], FD_WRITING_OK
   ret
   
  .ram_image_2:
   mov esi, dword [file_memory]
   mov edi, MEMORY_RAM_IMAGE_2
   mov ecx, 0x100000 ;one MB
   rep movsb ;save
   mov dword [fd_return], FD_WRITING_OK
   ret
   
  .ram_image_3:
   mov esi, dword [file_memory]
   mov edi, MEMORY_RAM_IMAGE_3
   mov ecx, 0x100000 ;one MB
   rep movsb ;save
   mov dword [fd_return], FD_WRITING_OK
   ret
   
  .ram_image_4:
   mov esi, dword [file_memory]
   mov edi, MEMORY_RAM_IMAGE_4
   mov ecx, 0x100000 ;one MB
   rep movsb ;save
   mov dword [fd_return], FD_WRITING_OK
   ret
 
 ;MEDIUM HARD DISK
 .medium_hard_disk:
  mov dword [fd_selected_medium], FD_MEDIUM_HDD
  call library_fd_draw_bg
  
  .hdd_halt:
   call wait_for_keyboard
   
   cmp byte [key_code], KEY_ESC
   je .done
   
   cmp byte [key_code], KEY_F3
   je .medium_ram
  jmp .hdd_halt
  
 .done:
 mov dword [fd_return], FD_NO_FILE
 ret
