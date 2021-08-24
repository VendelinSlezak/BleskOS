;BleskOS

%macro HDA_GCTL_WRITE 1
 mov ebp, dword [hda_base]
 add ebp, 0x08
 mov dword [ebp], %1
%endmacro

%macro HDA_GCTL_READ 0
 mov ebp, dword [hda_base]
 add ebp, 0x08
 mov eax, dword [ebp]
%endmacro

%macro HDA_CAPABILITES_READ 0
 mov eax, 0
 mov ebp, dword [hda_base]
 mov ax, word [ebp]
%endmacro

%macro HDA_DISABLE_INTERRUPTS 0
 mov ebp, dword [hda_base]
 add ebp, 0x20
 mov dword [ebp], 0x0
%endmacro

%macro HDA_SET_SSYNC 0
 mov ebp, dword [hda_base]
 add ebp, 0x34
 mov dword [ebp], 0
 add ebp, 0x4
 mov dword [ebp], 0
%endmacro

%macro HDA_TURN_OFF_CORB_RIRB_DMAPOS 0
 mov ebp, dword [hda_base]
 add ebp, 0x4C ;CORB control
 mov byte [ebp], 0x0
 add ebp, 0x10 ;0x5C RIRB control
 mov byte [ebp], 0x0
 add ebp, 0x14 ;0x70 DMAPOS control
 mov dword [ebp], 0x0
%endmacro

%macro HDA_INPUT_STREAM_TURN_OFF 0
 mov ebp, dword [hda_input_stream_port]
 mov dword [ebp], 0x00200000
%endmacro

%macro HDA_OUTPUT_STREAM_TURN_OFF 0
 mov ebp, dword [hda_output_stream_port]
 mov dword [ebp], 0x00140000
%endmacro

%macro HDA_INPUT_STREAM_TURN_ON 0
 mov ebp, dword [hda_input_stream_port]
 mov dword [ebp], 0x00240002
%endmacro

%macro HDA_OUTPUT_STREAM_TURN_ON 0
 mov ebp, dword [hda_output_stream_port]
 mov dword [ebp], 0x00140002
%endmacro

%macro HDA_INPUT_STREAM_FORMAT 1
 mov ebp, dword [hda_input_stream_port]
 add ebp, 0x12
 mov dword [ebp], %1
%endmacro

%macro HDA_OUTPUT_STREAM_FORMAT 1
 mov ebp, dword [hda_output_stream_port]
 add ebp, 0x12
 mov dword [ebp], %1
%endmacro

%macro HDA_INPUT_STREAM_LENGHT 1
 mov ebp, dword [hda_input_stream_port]
 add ebp, 0x08
 mov dword [ebp], %1
%endmacro

%macro HDA_OUTPUT_STREAM_LENGHT 1
 mov ebp, dword [hda_output_stream_port]
 add ebp, 0x08
 mov dword [ebp], %1
%endmacro

%macro HDA_INPUT_STREAM_SET_BUFFER 0
 mov ebp, dword [hda_input_stream_port]
 add ebp, 0x18
 mov dword [ebp], MEMORY_HDA_BUFFER+0x100
 add ebp, 0x4
 mov dword [ebp], 0

 mov ebp, dword [hda_input_stream_port]
 add ebp, 0x0C
 mov word [ebp], 1 ;two entries in buffer
%endmacro

%macro HDA_OUTPUT_STREAM_SET_BUFFER 0
 mov ebp, dword [hda_output_stream_port]
 add ebp, 0x18
 mov dword [ebp], MEMORY_HDA_BUFFER
 add ebp, 0x4
 mov dword [ebp], 0

 mov ebp, dword [hda_output_stream_port]
 add ebp, 0x0C
 mov word [ebp], 1 ;two entries in buffer
%endmacro

%macro HDA_INPUT_STREAM_POS 0
 mov ebp, dword [hda_output_stream_port]
 add ebp, 0x04
 mov eax, dword [ebp]
%endmacro

%macro HDA_OUTPUT_STREAM_POS 0
 mov ebp, dword [hda_output_stream_port]
 add ebp, 0x04
 mov eax, dword [ebp]
%endmacro

%macro HDA_SET_INPUT_BUFFER 2
 mov dword [MEMORY_HDA_BUFFER+0x100], %1
 mov dword [MEMORY_HDA_BUFFER+0x104], 0
 mov dword [MEMORY_HDA_BUFFER+0x108], %2
 mov dword [MEMORY_HDA_BUFFER+0x10C], 0

 mov dword [MEMORY_HDA_BUFFER+0x110], %1
 mov dword [MEMORY_HDA_BUFFER+0x114], 0
 mov dword [MEMORY_HDA_BUFFER+0x118], %2
 mov dword [MEMORY_HDA_BUFFER+0x11C], 0
%endmacro

%macro HDA_SET_OUTPUT_BUFFER 2
 mov dword [MEMORY_HDA_BUFFER+0x00], %1
 mov dword [MEMORY_HDA_BUFFER+0x04], 0
 mov dword [MEMORY_HDA_BUFFER+0x08], %2
 mov dword [MEMORY_HDA_BUFFER+0x0C], 0

 mov dword [MEMORY_HDA_BUFFER+0x10], %1
 mov dword [MEMORY_HDA_BUFFER+0x14], 0
 mov dword [MEMORY_HDA_BUFFER+0x18], %2
 mov dword [MEMORY_HDA_BUFFER+0x1C], 0
%endmacro

%define HDA_PIO_INTERFACE 1
%define HDA_CORB_RIRB_INTERFACE 2

hda_base dd 0
hda_input_stream_port dd 0
hda_output_stream_port dd 0

verb_interface dd 0
verb_codec dd 0
verb_node dd 0
verb_verb dd 0
verb_command dd 0
hda_response dd 0
corb_pointer db 0
rirb_pointer db 0

hda_codec_id dd 0
hda_max_volume dd 0
hda_volume dd 0 ;range from 0(no volume) to 100(max volume)
hda_nodes_list times 128 dd 0

hda_line_out_node dd 0
hda_line_in_node dd 0
hda_speaker_node dd 0
hda_mic_node dd 0

hda_data_pointer dd 0
hda_data_length dd 0
hda_data_format dd 0

init_sound_card:
 cmp dword [hda_base], 0
 je .done

 PSTR 'Sound card: HD Audio', exist_str
 
 ;SET OPERATIONAL STATE
 HDA_GCTL_WRITE 0x1
 WAIT 20
 HDA_GCTL_READ
 and eax, 0x1
 cmp eax, 0x0 ;still in reset, something is wrong
 je .done

 PSTR 'Card is in operational state', oper_state_str

 ;get nubmer of output and input ports
 mov eax, dword [hda_base]
 add eax, 0x80
 mov dword [hda_input_stream_port], eax ;port of first input stream
 HDA_CAPABILITES_READ
 and ax, 0x0F00
 shr ax, 8 ;number of input streams is in ax
 mov bx, 0x20
 mul bx
 add ax, 0x80
 add eax, dword [hda_base]
 mov dword [hda_output_stream_port], eax ;port of first output stream

 ;OTHER SETTINGS
 HDA_DISABLE_INTERRUPTS
 HDA_TURN_OFF_CORB_RIRB_DMAPOS
 HDA_INPUT_STREAM_TURN_OFF
 HDA_OUTPUT_STREAM_TURN_OFF
 HDA_INPUT_STREAM_SET_BUFFER
 HDA_OUTPUT_STREAM_SET_BUFFER
 HDA_SET_SSYNC

 ;set CORB and RIRB
 MMIO_OUTB hda_base, 0x4C, 0x0 ;stop CORB
 MMIO_OUTB hda_base, 0x5C, 0x0 ;stop RIRB
 WAIT 10
 MMIO_OUTD hda_base, 0x40, MEMORY_CORB
 MMIO_OUTD hda_base, 0x44, 0x00000000
 MMIO_OUTB hda_base, 0x4E, 0x2 ;size 256 entries
 MMIO_OUTW hda_base, 0x4A, 0x8000 ;reset read pointer
 WAIT 10
 MMIO_OUTW hda_base, 0x4A, 0x0000
 WAIT 10
 MMIO_OUTW hda_base, 0x48, 0 ;write pointer
 mov byte [corb_pointer], 1

 MMIO_OUTD hda_base, 0x50, MEMORY_RIRB
 MMIO_OUTD hda_base, 0x54, 0x00000000
 MMIO_OUTB hda_base, 0x5E, 0x2 ;size 256 entries
 MMIO_OUTW hda_base, 0x58, 0x8000 ;reset write pointer
 WAIT 10
 MMIO_OUTW hda_base, 0x58, 0x0000
 WAIT 10
 MMIO_OUTW hda_base, 0x5A, 0x0 ;interrupt control
 mov byte [rirb_pointer], 1

 MMIO_OUTB hda_base, 0x5C, 0x2 ;start RIRB
 MMIO_OUTB hda_base, 0x4C, 0x2 ;start CORB

 ;FIND CODEC
 mov dword [verb_interface], HDA_PIO_INTERFACE
 mov dword [verb_codec], 0
 mov dword [verb_node], 0
 mov dword [verb_verb], 0xF00
 mov dword [verb_command], 0x00
 mov ecx, 16
 .find_codec:
 push ecx
  call hda_send_verb
  cmp dword [hda_response], 0x00000000
  je .next_codec
  cmp dword [hda_response], 0xFFFFFFFF
  je .next_codec

  mov eax, dword [hda_response]
  mov dword [hda_codec_id], eax
  jmp .codec
 .next_codec:
 pop ecx
 inc dword [verb_codec]
 loop .find_codec

 mov dword [verb_interface], HDA_CORB_RIRB_INTERFACE
 mov dword [verb_codec], 0
 mov dword [verb_node], 0
 mov dword [verb_verb], 0xF00
 mov dword [verb_command], 0x00
 mov ecx, 16
 .find_codec_2:
 push ecx
  call hda_send_verb
  cmp dword [hda_response], 0x00000000
  je .next_codec_2
  cmp dword [hda_response], 0xFFFFFFFF
  je .next_codec_2

  mov eax, dword [hda_response]
  mov dword [hda_codec_id], eax
  jmp .codec
 .next_codec_2:
 pop ecx
 inc dword [verb_codec]
 loop .find_codec_2

 .done:
 ret ;no codec founded

 .codec:
 pop ecx
codec_find_widgets:
 mov dword [hda_line_out_node], 0
 mov dword [hda_line_in_node], 0
 mov dword [hda_speaker_node], 0
 mov dword [hda_mic_node], 0
 mov esi, hda_nodes_list
 mov ecx, 128
 .clear:
  mov word [esi], 0xFF
  add esi, 2
 loop .clear

 mov dword [verb_node], 0
 mov esi, hda_nodes_list
 mov ecx, 128 ;scan 128 nodes
 .scan_node:
 push ecx
  mov dword [verb_verb], 0xF00
  mov dword [verb_command], 0x9 ;get widget type
  call hda_send_verb

  ;no widget
  cmp dword [hda_response], 0xFFFFFFFF
  je .next_cycle
  cmp dword [hda_response], 0x00000000
  je .next_cycle

  ;get type of node
  shr dword [hda_response], 20

  ;write it to list
  mov al, byte [verb_node]
  mov byte [esi], al
  mov al, byte [hda_response]
  mov byte [esi+1], al
  IF_E byte [hda_response], 0x4, if_pin
   ;read pin type
   mov dword [verb_verb], 0xF1C
   mov dword [verb_command], 0x00
   call hda_send_verb

   mov eax, dword [hda_response]
   shr eax, 20
   and eax, 0xF
   or eax, 0x10

   mov byte [esi+1], al
  ENDIF if_pin

  ;turn widget on
  mov dword [verb_verb], 0x705
  mov dword [verb_command], 0x00
  call hda_send_verb

  ;turn on every audio output
  IF_E byte [esi+1], 0x0, if_audio_output
   mov dword [hda_data_format], 0x0010 ;16 bit, 48000 sample rate
   call hda_enable_audio_output
  ENDIF if_audio_output

 .next_cycle:
 inc dword [verb_node]
 add esi, 2
 pop ecx
 dec ecx
 cmp ecx, 0
 jne .scan_node

 .done:
 ret

hda_send_verb:
 cmp dword [verb_interface], HDA_PIO_INTERFACE
 je .pio

 call hda_send_verb_corb
 ret

 .pio:
 call hda_send_verb_pio
 ret

hda_send_verb_corb:
 mov eax, 0
 mov ebx, dword [verb_codec]
 shl ebx, 28
 or eax, ebx
 mov ebx, dword [verb_node]
 shl ebx, 20
 or eax, ebx
 mov ebx, dword [verb_verb]
 shl ebx, 8
 or eax, ebx
 or eax, dword [verb_command]

 mov ebx, 0
 mov bl, byte [corb_pointer]
 mov dword [MEMORY_CORB+(ebx*4)], eax ;write verb to CORB

 mov ebx, 0
 mov bl, byte [corb_pointer]
 MMIO_OUTW hda_base, 0x48, bx ;update pointer

 mov dword [ticks], 0
 .wait:
  MMIO_INW hda_base, 0x58
  cmp al, bl
  je .done
 cmp dword [ticks], 5
 jl .wait

 .done:
 mov ebx, 0
 mov bl, byte [rirb_pointer]
 mov eax, dword [MEMORY_RIRB+(ebx*8)]
 mov dword [hda_response], eax
 inc byte [corb_pointer]
 inc byte [rirb_pointer]

 ret

hda_send_verb_pio:
 mov eax, 0

 mov ebx, dword [verb_codec]
 shl ebx, 28
 or eax, ebx

 mov ebx, dword [verb_node]
 shl ebx, 20
 or eax, ebx

 mov ebx, dword [verb_verb]
 shl ebx, 8
 or eax, ebx

 or eax, dword [verb_command]

 ;immediate command
 MMIO_OUTW hda_base, 0x68, 0x2

 ;immediate output
 MMIO_OUTD hda_base, 0x60, eax

 ;immediate command
 MMIO_OUTW hda_base, 0x68, 0x1

 ;wait
 mov dword [ticks], 0
 .wait_for_response:
  MMIO_INW hda_base, 0x68
  and ax, 0x3
  cmp ax, 0x2
  je .wait_end

  cmp dword [ticks], 2
  jl .wait_for_response
 .wait_end:

 ;immediate input
 MMIO_IND hda_base, 0x64
 mov dword [hda_response], eax

 ;immediate command
 MMIO_OUTW hda_base, 0x68, 0x2

 ret

hda_set_volume:
 mov dword [verb_verb], 0xF00
 mov dword [verb_command], 0x12 ;get amplifier info
 call hda_send_verb

 mov eax, dword [hda_response]
 and eax, 0xFF00
 cmp eax, 0
 je .done ;volume can not be changed

 ;calculate from range 0 to 100 to node range
 mov eax, dword [hda_response]
 and eax, 0xFF ;max volume
 mov ebx, dword [hda_volume]
 mul ebx
 mov ebx, 100
 mov edx, 0
 div ebx
 or eax, 0xB000 ;output right and left
 mov dword [verb_verb], 0x300
 mov dword [verb_command], eax
 call hda_send_verb

 .done:
 ret

hda_enable_audio_output:
 ;unmute output
 mov dword [hda_volume], 100
 call hda_set_volume

 ;set stream 1
 mov dword [verb_verb], 0x706
 mov dword [verb_command], 0x10
 call hda_send_verb

 ;set data format
 mov dword [verb_verb], 0x200
 mov eax, dword [hda_data_format]
 mov dword [verb_command], eax
 call hda_send_verb

 ;turn EAPD on
 mov dword [verb_verb], 0xF0C
 mov dword [verb_command], 0x00
 call hda_send_verb
 mov eax, dword [hda_response]
 or eax, 0x2
 mov dword [verb_verb], 0x70C
 mov dword [verb_command], eax
 call hda_send_verb

 ret

hda_enable_output_pin:
 cmp dword [verb_node], 0
 je .done

 ;unmute pin
 mov dword [hda_volume], 100
 call hda_set_volume

 ;enable output pin
 mov dword [verb_verb], 0xF07
 mov dword [verb_command], 0x00
 call hda_send_verb
 mov eax, dword [hda_response]
 or eax, 0x40 ;enable output
 mov dword [verb_verb], 0x707
 mov dword [verb_command], eax
 call hda_send_verb

 ;turn EAPD on
 mov dword [verb_verb], 0xF0C
 mov dword [verb_command], 0x00
 call hda_send_verb
 mov eax, dword [hda_response]
 or eax, 0x2
 mov dword [verb_verb], 0x70C
 mov dword [verb_command], eax
 call hda_send_verb

 ;get connection entry
 mov dword [verb_verb], 0xF02
 mov dword [verb_command], 0x0
 call hda_send_verb
 mov eax, dword [hda_response]
 and eax, 0xFF
 mov dword [verb_node], eax
 mov dword [verb_verb], 0xF00
 mov dword [verb_command], 0x9
 call hda_send_verb
 mov eax, dword [hda_response]
 shr eax, 20

 ;if it is mixer, unmute it
 cmp eax, 0x4
 jne .if_mixer
  mov dword [hda_volume], 100
  call hda_set_volume
 .if_mixer:

 .done:
 ret

hda_play_sound:
 mov eax, dword [hda_data_pointer]
 mov ebx, dword [hda_data_length]
 mov ecx, dword [hda_data_format]
 HDA_SET_OUTPUT_BUFFER eax, ebx
 HDA_OUTPUT_STREAM_LENGHT ebx
 HDA_OUTPUT_STREAM_FORMAT ecx
 HDA_OUTPUT_STREAM_TURN_ON

 ret

hda_stop_sound:
 HDA_OUTPUT_STREAM_TURN_OFF

 ret

hda_record_sound:
 mov eax, dword [hda_data_pointer]
 mov ebx, dword [hda_data_length]
 mov ecx, dword [hda_data_format]
 HDA_SET_INPUT_BUFFER eax, ebx
 HDA_INPUT_STREAM_LENGHT ebx
 HDA_INPUT_STREAM_FORMAT ecx
 HDA_INPUT_STREAM_TURN_ON

 ret

hda_stop_recording:
 HDA_INPUT_STREAM_TURN_OFF

 ret
