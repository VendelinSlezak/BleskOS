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

%macro HDA_OUTPUT_STREAM_TURN_ON 0
 mov ebp, dword [hda_output_stream_port]
 mov dword [ebp], 0x00140002
%endmacro

%macro HDA_OUTPUT_STREAM_FORMAT 1
 mov ebp, dword [hda_output_stream_port]
 add ebp, 0x12
 mov dword [ebp], %1
%endmacro

%macro HDA_OUTPUT_STREAM_LENGHT 1
 mov ebp, dword [hda_output_stream_port]
 add ebp, 0x08
 mov dword [ebp], %1
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

%macro HDA_OUTPUT_STREAM_POS 0
 mov ebp, dword [hda_output_stream_port]
 add ebp, 0x04
 mov eax, dword [ebp]
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

hda_base dd 0
hda_input_stream_port dd 0
hda_output_stream_port dd 0

verb_codec dd 0
verb_node dd 0
verb_verb dd 0
verb_command dd 0
hda_response dd 0

hda_codec_id dd 0
hda_max_volume dd 0
hda_volume dd 0 ;range from 0(no volume) to 100(max volume)
hda_audio_output_list_pointer dd hda_audio_output_list
hda_audio_output_numof dd 0
hda_audio_output_list times 32 dd 0
hda_audio_input_list_pointer dd hda_audio_input_list
hda_audio_input_numof dd 0
hda_audio_input_list times 32 dd 0

hda_data_pointer dd 0
hda_data_lenght dd 0
hda_data_format dd 0

init_sound_card:
 cmp dword [hda_base], 0
 je .end

 PSTR 'Sound card: HD Audio', exist_str
 mov eax, dword [hda_base]
 PHEX eax
 HDA_CAPABILITES_READ
 PHEX eax
 
 ;SET OPERATIONAL STATE
 HDA_GCTL_WRITE 0x1
 WAIT 2
 HDA_GCTL_READ
 and eax, 0x1
 cmp eax, 0x0 ;still in reset, something is wrong
 je .end

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
 HDA_OUTPUT_STREAM_SET_BUFFER
 HDA_SET_SSYNC

 ;FIND WIDGETS
 mov dword [verb_codec], 0
 mov dword [verb_node], 0
 mov dword [verb_verb], 0xF00
 mov dword [verb_command], 0x00
 call hda_send_verb
 mov eax, dword [hda_response]
 mov dword [hda_codec_id], eax

 mov ecx, 32 ;scan 32 nodes
 .scan_node:
 push ecx
  inc dword [verb_node]
  mov dword [verb_verb], 0xF00
  mov dword [verb_command], 0x9 ;widget type
  call hda_send_verb

  ;no widget
  cmp dword [hda_response], 0xFFFFFFFF
  je .next_cycle
  cmp dword [hda_response], 0x00000000
  je .next_cycle

  ;get type of node
  shr dword [hda_response], 20

  IF_E dword [hda_response], 0x0, if_audio_output
   PSTR 'Audio Output', audio_output_str
   inc dword [hda_audio_output_numof]
   mov eax, dword [verb_node]
   mov esi, dword [hda_audio_output_list_pointer]
   mov dword [esi], eax
   add dword [hda_audio_output_list_pointer], 4
   PHEX eax
   jmp .next_cycle
  ENDIF if_audio_output

  IF_E dword [hda_response], 0x1, if_audio_input
   PSTR 'Audio Input', audio_input_str
   inc dword [hda_audio_input_numof]
   mov eax, dword [verb_node]
   mov esi, dword [hda_audio_input_list_pointer]
   mov dword [esi], eax
   add dword [hda_audio_input_list_pointer], 4
   PHEX eax
   jmp .next_cycle
  ENDIF if_audio_input

 .next_cycle:
 pop ecx
 dec ecx
 cmp ecx, 0
 jne .scan_node

 ;SET OUTPUT NODE
 mov eax, dword [hda_audio_output_list]
 mov dword [verb_node], eax
 call hda_set_output_node

 .end:
 ret

hda_send_verb:
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
 mov ebp, dword [hda_base]
 add ebp, 0x68
 mov word [ebp], 0x2

 ;immediate output
 mov ebp, dword [hda_base]
 add ebp, 0x60
 mov dword [ebp], eax

 ;immediate command
 mov ebp, dword [hda_base]
 add ebp, 0x68
 mov word [ebp], 0x1

 ;wait
 mov ebp, dword [hda_base]
 add ebp, 0x68
 mov dword [ticks], 0
 .wait_for_response:
  mov ax, word [ebp]
  and ax, 0x3
  cmp ax, 0x2
  je .wait_end

  cmp dword [ticks], 2
  jl .wait_for_response
 .wait_end:

 ;immediate input
 mov ebp, dword [hda_base]
 add ebp, 0x64
 mov eax, dword [ebp]
 mov dword [hda_response], eax

 ;immediate command
 mov ebp, dword [hda_base]
 add ebp, 0x68
 mov word [ebp], 0x2

 ret

hda_set_output_node:
 mov dword [verb_verb], 0xF00
 mov dword [verb_command], 0x12 ;get amplifier info
 call hda_send_verb

 mov eax, dword [hda_response]
 and eax, 0xFF
 mov dword [hda_max_volume], eax

 mov dword [verb_verb], 0x706 ;set stream
 mov dword [verb_command], 0x10 ;stream 1 channel 1
 call hda_send_verb

 mov dword [verb_verb], 0x300 ;set volume
 mov eax, dword [hda_max_volume]
 or eax, 0xB000
 mov dword [verb_command], eax ;output volume left and right
 call hda_send_verb

 ret

hda_set_volume:
 ;calculate from range 0 to 100 to node range
 mov eax, dword [hda_max_volume]
 mov ebx, dword [hda_volume]
 mul ebx
 mov ebx, 100
 div ebx
 or eax, 0xB000
 mov dword [verb_verb], 0x300
 mov dword [verb_command], eax
 call hda_send_verb

 ret

hda_play_sound:
 mov eax, dword [hda_data_pointer]
 mov ebx, dword [hda_data_lenght]
 mov ecx, dword [hda_data_format]
 HDA_SET_OUTPUT_BUFFER eax, ebx
 HDA_OUTPUT_STREAM_LENGHT ebx
 HDA_OUTPUT_STREAM_FORMAT ecx
 HDA_OUTPUT_STREAM_TURN_ON

 ret

hda_stop_sound:
 HDA_OUTPUT_STREAM_TURN_OFF

 ret
