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

 %include "source/drivers/sound_ac97.asm"
 %include "source/drivers/sound_hda.asm"

 %include "source/drivers/hardware_ps2_controller.asm"
 %include "source/drivers/hardware_ps2_keyboard.asm"
 %include "source/drivers/hardware_ps2_mouse.asm"

 %include "source/drivers/hardware_storage_center.asm"
 %include "source/drivers/hardware_ide.asm"
 %include "source/drivers/hardware_paralel_ata.asm"
 %include "source/drivers/hardware_paralel_atapi.asm"

 %include "source/drivers/filesystem_jus.asm"
 ;;%include "source/drivers/filesystem_fat32.asm"

 %include "source/drivers/usb_center.asm"
 %include "source/drivers/usb_uhci.asm"
 %include "source/drivers/usb_ehci.asm"
 
 %include "source/drivers/usb_mouse.asm"
 %include "source/drivers/usb_mass_storage.asm"

 %include "source/drivers/network_center.asm"

 %include "source/gui/program_library.asm"
 %include "source/gui/main.asm"
 %include "source/gui/developer_zone.asm"
 %include "source/gui/hardware.asm"
 %include "source/gui/elements.asm"
 %include "source/gui/text_mode.asm"

 %include "source/programs/dp_hardware_info.asm"
 %include "source/programs/dp_vesa.asm"
 %include "source/programs/dp_intel_hd_audio.asm"
 %include "source/programs/dp_ide.asm"

start_bleskos:
 call test_graphic_mode
 call init_graphic
 CLEAR_SCREEN 0x0600
 call redraw_screen

 PSTR 'Enabling interrupts...', bootstr_interrupts
 call init_idt
 call set_pit
 PSTR 'Scanning PCI...', bootstr_pci
 call scan_pci
 call init_keyboard
 PSTR 'Reading ACPI table...', bootstr_acpi
 call read_acpi
 PSTR 'Initalizing IDE drives...', bootstr_ide
 call init_ide_devices
 PSTR 'Initalizing sound card...', bootstr_sound_card
 call init_ac97
 call init_sound_card
 PSTR 'Initalizing PS/2 devices...', bootstr_ps2
 call init_ps2_controller
 call enable_touchpad
 PSTR 'Initalizing USB ports...', bootstr_usb
 call init_usb_ports

 call main_window

 HALT
