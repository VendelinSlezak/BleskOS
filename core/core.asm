;BleskOS real mode

init_core:
  mov byte [boot_device_number], dl
  mov bl, 0x20
  call draw_background
  call stop_pc_speaker
  mov byte [keyboard_langunge], 2
  call read_time

  ret

%include "core/vga.asm"
%include "core/pc_speaker.asm"
%include "core/keyboard.asm"
%include "core/memory_devices.asm"
%include "core/jus.asm"
%include "core/time.asm"
%include "core/wait.asm"
