;BleskOS

keyboard_wait dd 0
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
 mov word [key_unicode], 0

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
 mov word [key_unicode], 0
 cmp dword [key_code], 0x79
 jg .clear_wait ;released key

 mov eax, english_keyboard_layout
 IF_E dword [keyboard_shift], 1, if_shift_yes
  mov eax, english_shift_keyboard_layout
 ENDIF if_shift_yes
 mov ecx, dword [key_code]

 mov bx, word [eax+(ecx*2)] ;read unicode value of key
 mov word [key_unicode], bx

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
  mov byte [key_code], al
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
