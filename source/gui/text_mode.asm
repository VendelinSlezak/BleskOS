;BleskOS

text_mode_string db 'BleskOS can not be started because graphic mode was not found', 0

test_graphic_mode:
 cmp word [0x6000], 0xEE01
 je text_mode

 ret ;we are in graphic mode

text_mode:
 mov esi, 0xB8000
 mov ecx, 2000
 .clear_screen:
  mov byte [esi], ' '
  inc esi
  mov byte [esi], 0x20
  inc esi
 loop .clear_screen

 mov esi, 0xB8000+162
 mov edi, text_mode_string
 .print_string:
  cmp byte [edi], 0
  je text_mode_halt

  mov al, byte [edi]
  mov byte [esi], al

  inc edi
  add esi, 2
 jmp .print_string

 text_mode_halt:
  hlt
 jmp text_mode_halt
