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
 %include "source/drivers/hardware_ps2_keyboard.asm"

start_bleskos:
 call init_graphic
 CLEAR_SCREEN WHITE
 call redraw_screen

 PSTR 'Scanning PCI...', bootstr_pci
 call scan_pci
 call init_idt
 call set_pit

 HALT
