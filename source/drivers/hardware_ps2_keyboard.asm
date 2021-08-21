;BleskOS

key_code dd 0
keyboard_shift dd 0
key_ascii dw 0
keyboard_wait db 0
selected_keyboard_set dd 0
keyboard_special_code dd 0

init_keyboard:
 mov dword [keyboard_shift], 0
 mov dword [keyboard_special_code], 0
 ret

keyboard_irq:
 pusha

 mov eax, 0
 INB 0x60
 mov dword [key_code], eax

 cmp dword [keyboard_special_code], 1
 je .special_code
 cmp al, 0xE0
 je .set_special_code

 ;if some shift key is presses
 cmp al, KEY_CAPSLOCK
 je .reverse_shift_state
 cmp al, KEY_LEFT_SHIFT
 je .reverse_shift_state
 cmp al, KEY_LEFT_SHIFT+0x80
 je .reverse_shift_state
 cmp al, KEY_RIGHT_SHIFT
 je .reverse_shift_state
 cmp al, KEY_RIGHT_SHIFT+0x80
 je .reverse_shift_state
 jmp .select_layout ;no shift key is pressed

 .reverse_shift_state:
 or dword [keyboard_shift], 0xFFFFFFFE ;other bytes will be zero
 not dword [keyboard_shift] ;reverse

 ;select layout
 .select_layout:
 mov byte [key_ascii], 0
 cmp dword [key_code], 0x79
 jg .clear_wait ;released key

 mov eax, english_keyboard_layout
 IF_E dword [keyboard_shift], 1, if_shift_yes
  mov eax, english_shift_keyboard_layout
 ENDIF if_shift_yes
 add eax, dword [key_code]

 mov bl, byte [eax] ;read ascii value of key
 mov byte [key_ascii], bl

 .clear_wait:
 mov dword [keyboard_wait], 0

 EOI_MASTER_PIC
 popa
 iret

 .set_special_code:
 mov dword [keyboard_special_code], 1
 jmp .clear_wait

 .special_code:
  mov dword [keyboard_special_code], 0
  mov byte [key_ascii], 0
  mov byte [key_code], 0

  cmp al, 0x4B
  je .key_left
  cmp al, 0x4D
  je .key_right
  cmp al, 0x48
  je .key_up
  cmp al, 0x49
  je .page_up
  cmp al, 0x50
  je .key_down
  cmp al, 0x51
  je .page_down
  jmp .clear_wait

  .page_up:
  mov byte [key_code], KEY_PAGE_UP
  jmp .clear_wait

  .page_down:
  mov byte [key_code], KEY_PAGE_DOWN
  jmp .clear_wait

  .key_up:
  mov byte [key_code], KEY_UP
  jmp .clear_wait

  .key_down:
  mov byte [key_code], KEY_DOWN
  jmp .clear_wait

  .key_right:
  mov byte [key_code], KEY_RIGHT
  jmp .clear_wait

  .key_left:
  mov byte [key_code], KEY_LEFT
  jmp .clear_wait

wait_for_keyboard:
 mov dword [keyboard_wait], 1

 .wait:
  cmp dword [keyboard_wait], 0
  je .done
  hlt
 jmp .wait

 .done:
 ret
