;BleskOS

%define AC97_RESET 0x00
%define AC97_MASTER_VOL 0x02
%define AC97_MIC_VOL 0x0E
%define AC97_PCM_VOL 0x18
%define AC97_EXT_CONTROL 0x2A
%define AC97_EXT_FRONT_RATE 0x2C
%define AC97_EXT_LR_RATE 0x32
%define AC97_OUTPUT_BUFFER 0x10
%define AC97_OUTPUT_LVI 0x10
%define AC97_OUTPUT_CONTROL 0x10
%define AC97_GLOBAL_CONTROL 0x2C

%define AC97_NORMAL_ENTRY 0x0000
%define AC97_END_ENTRY 0x4000

ac97_nam_base dw 0
ac97_nabm_base dw 0

%macro AC97_N_OUT 2
 mov dx, word [ac97_nam_base]
 add dx, %1
 mov ax, %2
 out dx, ax
%endmacro

%macro AC97_GC_OUT 1
 mov dx, word [ac97_nabm_base]
 add dx, AC97_GLOBAL_CONTROL
 mov eax, %1
 out dx, eax
%endmacro

%macro AC97_SET_BUFFER 1
 mov dx, word [ac97_nabm_base]
 add dx, AC97_OUTPUT_BUFFER
 mov eax, %1
 out dx, eax
%endmacro

%macro AC97_OUT_CONTROL 1
 mov dx, word [ac97_nabm_base]
 add dx, AC97_OUTPUT_CONTROL
 mov eax, %1
 out dx, eax
%endmacro

%macro AC97_OUT_LVI 1
 mov dx, word [ac97_nabm_base]
 add dx, AC97_OUTPUT_LVI
 mov al, %1
 out dx, al
%endmacro

%macro AC97_FILL_ENTRY 1
 mov ebx, dword [sound_memory]

 mov dword [eax], ebx
 add eax, 4
 mov word [eax], 0xFFFE ;max lenght of buffer
 add eax, 2
 mov word [eax], %1
 add eax, 2

 add dword [sound_memory], 0x20000 ;16 bit 2 channel sound
%endmacro

init_ac97:
 AC97_GC_OUT 0x2 ;resume from cold reset
 WAIT 2
 AC97_N_OUT AC97_RESET, 1 ;any value cause register reset
 WAIT 2

 AC97_N_OUT AC97_MASTER_VOL, 0x0000 ;max volume

 AC97_SET_BUFFER MEMORY_AC97_BDL

 ret

ac97_set_volume:
 AC97_N_OUT AC97_PCM_VOL, word [sound_volume]

 ret

ac97_set_sample_rate:
 AC97_N_OUT AC97_EXT_CONTROL, 0x1 ;set sample rate
 AC97_N_OUT AC97_EXT_FRONT_RATE, word [sound_sample_rate]
 AC97_N_OUT AC97_EXT_LR_RATE, word [sound_sample_rate]

 ret

ac97_fill_buffer:
 mov eax, MEMORY_AC97_BDL
 AC97_FILL_ENTRY AC97_NORMAL_ENTRY
 AC97_FILL_ENTRY AC97_NORMAL_ENTRY
 AC97_FILL_ENTRY AC97_NORMAL_ENTRY
 AC97_FILL_ENTRY AC97_NORMAL_ENTRY
 AC97_FILL_ENTRY AC97_NORMAL_ENTRY
 AC97_FILL_ENTRY AC97_NORMAL_ENTRY
 AC97_FILL_ENTRY AC97_NORMAL_ENTRY
 AC97_FILL_ENTRY AC97_END_ENTRY

 ret

ac97_play_sound:
 AC97_OUT_LVI 8
 AC97_OUT_CONTROL 0x2 ;start playing

 ret

ac97_stop_sound:
 AC97_OUT_CONTROL 0x0
 WAIT 2
 AC97_OUT_CONTROL 0x1 ;reset stream
 WAIT 2
 AC97_OUT_CONTROL 0x0

 ret
