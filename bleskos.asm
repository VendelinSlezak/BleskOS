;BleskOS

org 0x10000

bits 32

start:
  call init_core

  halt:
    hlt
  jmp halt

%include "core/core.asm"
