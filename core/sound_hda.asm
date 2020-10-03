;BleskOS

%macro HDA_INB 1
  mov edi, dword [hda_mem]
  add edi, %1
  mov al, byte [edi]
%endmacro

%macro HDA_INW 1
  mov edi, dword [hda_mem]
  add edi, %1
  mov ax, word [edi]
%endmacro

%macro HDA_INL 1
  mov edi, dword [hda_mem]
  add edi, %1
  mov eax, dword [edi]
%endmacro

%macro HDA_OUTB 2
  mov edi, dword [hda_mem]
  add edi, %1
  mov byte [edi], %2
%endmacro

%macro HDA_OUTW 2
  mov edi, dword [hda_mem]
  add edi, %1
  mov word [edi], %2
%endmacro

%macro HDA_OUTL 2
  mov edi, dword [hda_mem]
  add edi, %1
  mov dword [edi], %2
%endmacro

%macro HDA_WAIT 3
  mov ecx, 0xFFFF
  .%3_waiting:
    INB 0x3F6
    HDA_INW %1
    and ax, %2
    cmp ax, %2
    je .%3
  loop .%3_waiting

  ret

  .%3:
%endmacro

%define GLOBAL_CAPABITILIES 0x00
%define GLOBAL_CONTROL 0x08
%define INTERRUPT_CONTROL 0x20
%define CORB_CONTROL 0x4C
%define RIRB_CONTROL 0x5C
%define DMA_LOWER_ADDRESS 0x70
%define DMA_UPPER_ADDRESS 0x74
%define VERB_OUTPUT 0x60
%define VERB_INPUT 0x64
%define VERB_STATUS 0x68
%define STREAM_CONTROL 0x80
%define STREAM_STATUS 0x83
%define STREAM_LOWER_ADDRESS 0x98
%define STREAM_UPPER_ADDRESS 0x9C

hda_send_verb:
  HDA_OUTW VERB_STATUS, 0 ;clear bits
  mov eax, dword [hda_verb]
  HDA_OUTL VERB_OUTPUT, eax ;send verb
  HDA_OUTW VERB_STATUS, 1 ;transfer verb
  HDA_WAIT VERB_STATUS, 0x2, verb_wait
  HDA_INL VERB_INPUT ;read response

  ret

hda_set_buffer:
  mov eax, dword [hda_buffer_address]
  mov dword [0x100000], eax
  mov dword [0x100004], 0
  mov eax, dword [hda_buffer_lenght]
  mov dword [0x100008], eax
  mov dword [0x10000C], 0 ;no interrupt

  ret
  

init_hda:
  HDA_OUTW GLOBAL_CONTROL, 1 ;wake up from reset mode
  HDA_WAIT GLOBAL_CONTROL, 1, reset_wait

  HDA_OUTL INTERRUPT_CONTROL, 0x0 ;disable all interrupts

  HDA_OUTB CORB_CONTROL, 0x0 ;stop CORB
  HDA_OUTB RIRB_CONTROL, 0x0 ;stop RIRB

  HDA_OUTL DMA_LOWER_ADDRESS, 0x110000 ;DMA position buffer
  HDA_OUTL DMA_UPPER_ADDRESS, 0x0

  HDA_OUTW STREAM_CONTROL, 0 ;clear RUN bit
  HDA_OUTL STREAM_LOWER_ADDRESS, 0x100000 ;buffer position
  HDA_OUTL STREAM_UPPER_ADDRESS, 0x0

  ret

hda_verb dd 0
hda_buffer_address dd 0
hda_buffer_lenght dd 0
