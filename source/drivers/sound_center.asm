;BleskOS

%define SOUND_CARD_AC97 1
%define SOUND_CARD_HDA 2

sound_memory dd 0
sound_volume dw 0
sound_sample_rate dw 0

sound_card_type dd 0

%macro SET_SOUND_MEMORY 1
 mov dword [sound_memory], %1
%endmacro

%macro SET_VOLUME 1
 mov dword [sound_volume], %1
 call sound_set_volume
%endmacro

%macro SET_SAMPLE_RATE 1
 mov dword [sound_sample_rate], %1
 call sound_set_volume
%endmacro

init_sound_card:
 mov dword [sound_card_type], 0

 IF_NE word [ac97_nam_base], 0, if_ac97
  call init_ac97
  mov dword [sound_card_type], SOUND_CARD_AC97
 ENDIF if_ac97

 ret

sound_set_volume:
 IF_NE word [ac97_nam_base], 0, if_ac97
  call ac97_set_volume
 ENDIF if_ac97

 ret

sound_set_sample_rate:
 IF_NE word [ac97_nam_base], 0, if_ac97
  call ac97_set_sample_rate
 ENDIF if_ac97

 ret

play_sound:
 IF_NE word [ac97_nam_base], 0, if_ac97
  call ac97_fill_buffer
  call ac97_play_sound
 ENDIF if_ac97

 ret

stop_sound:
 IF_NE word [ac97_nam_base], 0, if_ac97
  call ac97_stop_sound
 ENDIF if_ac97

 ret
