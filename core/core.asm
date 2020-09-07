;BleskOS

%include "core/system_ports.asm"
%include "core/system_idt.asm"
%include "core/system_time.asm"
%include "core/system_pci.asm"
%include "core/system_shutdown.asm"

%include "core/graphic_font.asm"
%include "core/graphic_vesa.asm"
%include "core/graphic_vga.asm"

%include "core/sound_pc_speaker.asm"
%include "core/sound_hda.asm"

%include "core/hardware_keyboard.asm"
%include "core/hardware_hard_disc.asm"
%include "core/hardware_cdrom.asm"

init_core:
  call read_vesa_info
  call draw_background

  call init_idt

  call init_keyboard

  ret
