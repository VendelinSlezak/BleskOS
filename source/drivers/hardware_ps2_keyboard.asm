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

 call keyboard_convert_to_unicode

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
