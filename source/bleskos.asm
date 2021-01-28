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
 %include "source/drivers/sound_ac97.asm"
 %include "source/drivers/sound_center.asm"
 %include "source/drivers/hardware_ps2_controller.asm"
 %include "source/drivers/hardware_ps2_keyboard.asm"
 %include "source/drivers/hardware_ps2_mouse.asm"
 %include "source/drivers/hardware_ide.asm"
 %include "source/drivers/hardware_paralel_ata.asm"
 %include "source/drivers/filesystem_jus.asm"
 %include "source/gui/program_library.asm"
 %include "source/gui/main.asm"

start_bleskos:
 call init_graphic
 CLEAR_SCREEN WHITE
 call redraw_screen

 PSTR 'Scanning PCI...', bootstr_pci
 call scan_pci
 call init_keyboard
 PSTR 'Enabling interrupts...', bootstr_interrupts
 call init_idt
 call set_pit
 PSTR 'Initalizing IDE drives...', bootstr_ide
 call init_ide_devices
 PSTR 'Initalizing sound card...', bootstr_sound_card
 call init_sound_card
 PSTR 'Initalizing PS/2 devices...', bootstr_ps2
 call init_ps2_controller
 call init_ps2_mouse

 call main_gui

 HALT
