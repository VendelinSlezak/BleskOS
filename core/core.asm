;BleskOS real mode edition

init_core:
  mov bl, 0x20
  call draw_background

  mov ah, 0x02
  mov bh, 0
  mov dx, 0x0101
  int 10h
  mov si, str1
  call print

  mov ah, 0x02
  mov bh, 0
  mov dx, 0x0201
  int 10h
  mov si, str2
  call print
  call stop_pc_speaker

  mov ah, 0x02
  mov bh, 0
  mov dx, 0x0301
  int 10h
  mov si, str3
  call print
  mov byte [keyboard_langunge], 2

  ret

  str1 db '> initalizing core', 0
  str2 db '> stopping pc speaker', 0
  str3 db '> setting keyboard', 0

%include "core/vga.asm"
%include "core/pc_speaker.asm"
%include "core/keyboard.asm"
%include "core/memory_devices.asm"
%include "core/jus.asm"
