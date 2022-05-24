;BleskOS

%macro HDA_GCTL_WRITE 1
 MMIO_OUTD hda_base, 0x08, %1
%endmacro

%macro HDA_GCTL_READ 0
 MMIO_IND hda_base, 0x08
%endmacro

%macro HDA_CAPABILITES_READ 0
 MMIO_INW hda_base, 0x00
%endmacro

%macro HDA_DISABLE_INTERRUPTS 0
 MMIO_OUTD hda_base, 0x20, 0x0
%endmacro

%macro HDA_SET_SSYNC 0
 MMIO_OUTD hda_base, 0x34, 0
 MMIO_OUTD hda_base, 0x38, 0
%endmacro

%macro HDA_TURN_OFF_DMAPOS 0
 MMIO_OUTD hda_base, 0x70, 0
%endmacro

%macro HDA_INPUT_STREAM_TURN_OFF 0
 MMIO_OUTD hda_input_stream_port, 0, 0x00240000
%endmacro

%macro HDA_OUTPUT_STREAM_TURN_OFF 0
 MMIO_OUTD hda_output_stream_port, 0, 0x00140000
%endmacro

%macro HDA_INPUT_STREAM_TURN_ON 0
 MMIO_OUTD hda_input_stream_port, 0, 0x00240002
%endmacro

%macro HDA_OUTPUT_STREAM_TURN_ON 0
 MMIO_OUTD hda_output_stream_port, 0, 0x00140002
%endmacro

%macro HDA_INPUT_STREAM_FORMAT 1
 MMIO_OUTW hda_input_stream_port, 0x12, %1
%endmacro

%macro HDA_OUTPUT_STREAM_FORMAT 1
 MMIO_OUTW hda_output_stream_port, 0x12, %1
%endmacro

%macro HDA_INPUT_STREAM_LENGHT 1
 MMIO_OUTD hda_input_stream_port, 0x08, %1
%endmacro

%macro HDA_OUTPUT_STREAM_LENGHT 1
 MMIO_OUTD hda_output_stream_port, 0x08, %1
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
hda_headphone_node dd 0
hda_mic_node dd 0

hda_data_pointer dd 0
hda_data_length dd 0
hda_data_format dd 0

init_sound_card:
 cmp dword [hda_base], 0
 je .done

 LOG ' ', 0xA, 'Sound card Intel HD Audio'
 
 ;SET OPERATIONAL STATE
 HDA_GCTL_WRITE 0x1
 WAIT 20
 HDA_GCTL_READ
 and eax, 0x1
 cmp eax, 0x0 ;still in reset, something is wrong
 je .done

 LOG ' is in operational state', 0xA

 ;get number of output and input ports
 mov eax, dword [hda_base]
 add eax, 0x80
 mov dword [hda_input_stream_port], eax ;port of first input stream
 HDA_CAPABILITES_READ
 LOG 'Stream capability: '
 LOG_HEX eax
 and ax, 0x0F00
 shr ax, 8 ;number of input streams is in ax
 mov bx, 0x20
 mul bx
 add ax, 0x80
 add eax, dword [hda_base]
 mov dword [hda_output_stream_port], eax ;port of first output stream

 ;OTHER SETTINGS
 HDA_DISABLE_INTERRUPTS
 HDA_TURN_OFF_DMAPOS
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
  LOG 0xA, 'PIO interface', 0xA
  jmp .codec
 .next_codec:
 pop ecx
 inc dword [verb_codec]
 loop .find_codec

 MMIO_OUTB hda_base, 0x5C, 0x2 ;start RIRB
 MMIO_OUTB hda_base, 0x4C, 0x2 ;start CORB

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
  LOG 0xA, 'CORB/RIRB interface', 0xA
  jmp .codec
 .next_codec_2:
 pop ecx
 inc dword [verb_codec]
 loop .find_codec_2

 .done:
 LOG 'Error: no codec was founded', 0xA
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
  LOG 'Node '
  mov eax, 0
  mov al, byte [verb_node]
  LOG_HEX eax
  
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
  
  LOG 'type '
  mov eax, 0
  mov al, byte [esi+1]
  LOG_HEX eax
  ;get connection info
  mov dword [verb_verb], 0xF02
  mov dword [verb_command], 0x00
  call hda_send_verb
  mov eax, dword [hda_response]
  LOG 'connected to '
  LOG_HEX eax
  LOG 0xA

  ;turn widget on
  mov dword [verb_verb], 0x705
  mov dword [verb_command], 0x00
  call hda_send_verb

 .next_cycle:
 inc dword [verb_node]
 add esi, 2
 pop ecx
 dec ecx
 cmp ecx, 0
 jne .scan_node
 
 ;FIND SPEAKER NODE
 mov esi, hda_nodes_list
 mov ecx, 128
 .find_speaker_node:
  cmp byte [esi+1], 0x11 ;speaker
  je .speaker
  cmp byte [esi+1], 0x10 ;line out
  je .line_out
  jmp .next_loop
  
  .speaker:
  mov eax, 0
  mov al, byte [esi]
  mov dword [verb_node], eax
  
  ;get capabilites of pin
  mov dword [verb_verb], 0xF00
  mov dword [verb_command], 0x9
  call hda_send_verb
  mov eax, dword [hda_response]
  
  ;now we can find if is amp present
  mov eax, dword [hda_response]
  test eax, 0x4
  jz .speaker_not_present
  
  mov eax, dword [verb_node]
  mov dword [hda_speaker_node], eax ;save speaker node
  jmp .speaker_founded 
  
  .speaker_not_present:
  cmp dword [hda_speaker_node], 0
  jne .next_loop
  mov eax, dword [verb_node]
  mov dword [hda_speaker_node], eax 
  jmp .next_loop
  
  .line_out:
  cmp dword [hda_line_out_node], 0
  jne .next_loop
  mov eax, 0
  mov al, byte [esi]
  mov dword [verb_node], eax
  mov dword [hda_line_out_node], eax 
  LOG 'Line Out node '
  LOG_HEX eax
  LOG 0xA
  jmp .next_loop
 .next_loop:
 add esi, 2
 dec ecx
 cmp ecx, 0
 jne .find_speaker_node
 
 .done:
 ret

 .speaker_founded:
 mov eax, dword [hda_speaker_node]
 mov dword [verb_node], eax
 LOG 'speaker node '
 LOG_HEX eax
 call hda_enable_pin_output
 
 ;found path - NOT COMPLETE!
 LOG 'PATH: '
 mov dword [verb_verb], 0xF02
 mov dword [verb_command], 0x00
 call hda_send_verb
 mov eax, dword [hda_response]
 and eax, 0xFF ;get first entry
 
 mov dword [verb_node], eax
 mov dword [verb_verb], 0xF00
 mov dword [verb_command], 0x9
 call hda_send_verb
 mov eax, dword [hda_response]
 shr eax, 20
 and eax, 0xF ;parse type of node
 
 cmp eax, 0x0
 je .enable_output_converter ;speaker is directly connected to output converter
 cmp eax, 0x2
 je .path_node_mixer
 
 jmp .done ;TODO another types of nodes
 
 .path_node_mixer:
  mov eax, dword [verb_node]
  LOG 'Mixer '
  LOG_HEX eax
 
  mov dword [verb_command], 0
  mov ecx, 10 ;scan 10 nodes connected to mixer
  .scan_mixer_node:
   mov dword [verb_verb], 0xF02
   call hda_send_verb
   mov eax, dword [hda_response]
   and eax, 0xFF
   cmp eax, 0
   je .done ;this mixer do not have connection to output converter
   
   ;read type of node
   mov ebx, dword [verb_node]
   mov edx, dword [verb_command]
   mov dword [verb_node], eax
   mov dword [verb_verb], 0xF00
   mov dword [verb_command], 0x9
   push ebx
   push edx
   call hda_send_verb
   pop edx
   pop ebx
   
   mov eax, dword [hda_response]
   shr eax, 20
   and eax, 0xF
   cmp eax, 0x0
   je .enable_output_converter ;we found output converter
   
   mov dword [verb_node], ebx
   inc edx
   mov dword [verb_command], edx
  loop .scan_mixer_node
 
 .enable_output_converter:
 LOG 'Audio output '
 mov eax, dword [verb_node]
 LOG_HEX eax
 call hda_enable_audio_output
 LOG 0xA
 jmp .done

hda_send_verb:
 push ecx
 cmp dword [verb_interface], HDA_PIO_INTERFACE
 je .pio

 call hda_send_verb_corb
 pop ecx
 ret

 .pio:
 call hda_send_verb_pio
 pop ecx
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
 
hda_enable_pin_output:
 ;enable pin
 mov dword [verb_verb], 0xF07
 mov dword [verb_command], 0x00
 call hda_send_verb
 mov eax, dword [hda_response]
 and eax, 0x20 ;set enable bit
 mov dword [verb_verb], 0x707
 mov dword [verb_command], eax
 call hda_send_verb
 
 ;enable external speaker
 mov dword [verb_verb], 0xF0C
 mov dword [verb_command], 0x00
 call hda_send_verb
 mov eax, dword [hda_response]
 or eax, 0x2
 mov dword [verb_verb], 0x70C
 mov dword [verb_command], eax
 call hda_send_verb
 
 ;set volume
 mov dword [hda_volume], 100
 call hda_set_volume
 
 ret
 
hda_enable_mixer:
 mov dword [hda_volume], 100
 call hda_set_volume
 
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
 mov dword [verb_command], 0x0011 ;16 bit and 2 channels
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
 mov cx, word [hda_data_format]
 HDA_SET_OUTPUT_BUFFER eax, ebx
 HDA_OUTPUT_STREAM_LENGHT ebx
 HDA_OUTPUT_STREAM_FORMAT cx
 HDA_OUTPUT_STREAM_TURN_ON

 ret

hda_stop_sound:
 HDA_OUTPUT_STREAM_TURN_OFF

 ret

hda_record_sound:
 mov eax, dword [hda_data_pointer]
 mov ebx, dword [hda_data_length]
 mov cx, word [hda_data_format]
 HDA_SET_INPUT_BUFFER eax, ebx
 HDA_INPUT_STREAM_LENGHT ebx
 HDA_INPUT_STREAM_FORMAT cx
 HDA_INPUT_STREAM_TURN_ON

 ret

hda_stop_recording:
 HDA_INPUT_STREAM_TURN_OFF

 ret
