;BleskOS real mode edition

org 0x10000

bits 16

start:
  mov ax, 0x1000
  mov ds, ax ;set data segment

  call init_core

  halt:
    hlt
  jmp halt

%include "core/core.asm"
%include "programs/dp_devices.asm"
