;BleskOS

org 0x10000

bits 32

start:
 jmp start_bleskos

 %include "source/macros.asm"

 %include "source/drivers/graphic_vesa.asm"
 %include "source/drivers/graphic_font.asm"

 %include "source/drivers/system_pci.asm"
 %include "source/drivers/system_interrupts.asm"
 %include "source/drivers/system_wait.asm"
 %include "source/drivers/system_memory_managment.asm"
 %include "source/drivers/system_acpi.asm"
 %include "source/drivers/system_time.asm"

 %include "source/drivers/sound_ac97.asm"
 %include "source/drivers/sound_hda.asm"

 %include "source/drivers/hardware_keyboard_layouts.asm"
 %include "source/drivers/hardware_ps2_controller.asm"
 %include "source/drivers/hardware_ps2_keyboard.asm"
 %include "source/drivers/hardware_ps2_mouse.asm"

 %include "source/drivers/hardware_storage_center.asm"
 %include "source/drivers/hardware_ide.asm"
 %include "source/drivers/hardware_ahci.asm"
 %include "source/drivers/hardware_paralel_ata.asm"
 %include "source/drivers/hardware_paralel_atapi.asm"
 %include "source/drivers/hardware_serial_ata.asm"
 ;;;%include "source/drivers/hardware_serial_atapi.asm"

 %include "source/drivers/usb_center.asm"
 %include "source/drivers/usb_ohci.asm"
 %include "source/drivers/usb_uhci.asm"
 %include "source/drivers/usb_ehci.asm"
 ;;;%include "source/drivers/usb_xhci.asm"

 %include "source/drivers/usb_keyboard.asm"
 %include "source/drivers/usb_mouse.asm"
 %include "source/drivers/usb_mass_storage.asm"

 %include "source/drivers/filesystem_jus.asm"
 %include "source/drivers/filesystem_fat32.asm"
 %include "source/drivers/filesystem_iso9660.asm"

 %include "source/drivers/network_center.asm"
 %include "source/drivers/network_protocols.asm"
 %include "source/drivers/network_ethernet_nic_intel.asm"
 %include "source/drivers/network_ethernet_nic_amd.asm"

 %include "source/gui/board_main.asm"
 %include "source/gui/board_developer_zone.asm"
 %include "source/gui/board_hardware_centrum.asm"
 %include "source/gui/library_elements.asm"
 %include "source/gui/library_images.asm"
 %include "source/gui/library_programs.asm"
 %include "source/gui/library_file_dialogue.asm"
 %include "source/gui/library_encrypting.asm"
 %include "source/gui/library_convert_graphic_files.asm"
 %include "source/gui/library_copy.asm"

 %include "source/programs/dp_hardware_info.asm"
 %include "source/programs/dp_intel_hd_audio.asm"
 %include "source/programs/dp_ide.asm"
 %include "source/programs/dp_mouse.asm"
 %include "source/programs/dp_acpi.asm"

 %include "source/programs/text_editor/text_editor.asm"
 %include "source/programs/graphic_editor/graphic_editor.asm"
 %include "source/programs/graphic_editor/drawing_things_on_screen.asm"
 %include "source/programs/media_viewer/media_viewer.asm"
 %include "source/programs/internet_browser/html_drawing.asm"
 %include "source/programs/internet_browser/internet_browser.asm"
 %include "source/programs/document_editor/document_editor.asm"
 %include "source/programs/table_editor/table_editor.asm"

%macro START_SCREEN_DRAW_SQUARE 1
 mov eax, dword [screen_x_center]
 sub eax, 167
 mov ebx, dword [screen_y_center]
 add ebx, 31
 DRAW_SQUARE ebx, eax, %1, 14, 0x0900FF
 mov ebx, dword [screen_y_center]
 add ebx, 31
 REDRAW_LINES_SCREEN ebx, 14
%endmacro

%macro START_SCREEN_PRINT 2
 jmp %2_over
 %2 db %1, 0
 %2_over:
 mov eax, dword [screen_y_center]
 add eax, 65
 mov ebx, dword [screen_x]
 DRAW_SQUARE eax, 0, ebx, 10, 0x00C000
 mov esi, %2
 mov eax, dword [screen_y_center]
 add eax, 65
 call print_to_message_window
 mov eax, dword [screen_y_center]
 add eax, 65
 REDRAW_LINES_SCREEN eax, 10
%endmacro

numberstr times 10 dw 0

start_bleskos:
 call init_graphic
 CLEAR_SCREEN 0x00C000
 mov eax, dword [screen_x_center]
 sub eax, 224
 mov ebx, dword [screen_y_center]
 sub ebx, 76
 mov dword [size_of_text], 8
 mov dword [color], BLACK
 PRINT 'BleskOS', start_screen_bleskos_str, ebx, eax
 mov eax, dword [screen_x_center]
 sub eax, 52
 mov ebx, dword [screen_y_center]
 mov dword [size_of_text], 1
 PRINT 'Alpha version', start_screen_alpha_1_str, ebx, eax
 mov eax, dword [screen_x_center]
 sub eax, 168
 mov ebx, dword [screen_y_center]
 add ebx, 30
 DRAW_EMPTY_SQUARE ebx, eax, 336, 15, BLACK
 call redraw_screen

 START_SCREEN_PRINT 'Reading size of RAM memory...', start_screen_ramsize
 call redraw_screen
 call read_memory_size
 START_SCREEN_DRAW_SQUARE 30
 START_SCREEN_PRINT 'Enabling interrupts...', bootstr_interrupts
 call init_idt
 call set_pit
 START_SCREEN_DRAW_SQUARE 60
 START_SCREEN_PRINT 'Scanning PCI...', bootstr_pci
 call scan_pci
 call init_keyboard
 START_SCREEN_DRAW_SQUARE 90
 START_SCREEN_PRINT 'Reading ACPI table...', bootstr_acpi
 call read_acpi
 START_SCREEN_DRAW_SQUARE 120
 START_SCREEN_PRINT 'Initalizing IDE drives...', bootstr_ide
 call init_ahci
 call init_ide
 START_SCREEN_DRAW_SQUARE 150
 START_SCREEN_PRINT 'Initalizing sound card...', bootstr_sound_card
 call init_ac97
 call init_sound_card
 START_SCREEN_DRAW_SQUARE 180
 START_SCREEN_PRINT 'Initalizing PS/2 devices...', bootstr_ps2
 call init_ps2_controller
 call enable_touchpad
 START_SCREEN_DRAW_SQUARE 210
 START_SCREEN_PRINT 'Read JUS informations...', bootstr_jus
 call init_jus
 START_SCREEN_DRAW_SQUARE 240
 START_SCREEN_PRINT 'Initalizing USB ports...', bootstr_usb
 call init_usb_controllers
 START_SCREEN_DRAW_SQUARE 270
 START_SCREEN_PRINT 'Initalizing network cards...', bootstr_nic
 call init_ethernet_card
 call init_network_stack
 call connect_to_network
 START_SCREEN_DRAW_SQUARE 300
 START_SCREEN_PRINT 'Allocating memory for programs...', bootstr_mem_alloc
 call init_memory_of_programs
 START_SCREEN_DRAW_SQUARE 335
 
 call main_window

 HALT
