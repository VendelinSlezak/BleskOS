;BleskOS

%define KEY_ESC 0x01
%define KEY_ENTER 0x1C
%define KEY_TAB 0x0F
%define KEY_BACKSPACE 0x0E
%define KEY_CAPSLOCK 0x3A
%define KEY_ESCAPE 0x01
%define KEY_DELETE 0x53
%define KEY_UP 0x48 
%define KEY_DOWN 0x50
%define KEY_LEFT 0x4B
%define KEY_RIGHT 0x4D
%define KEY_LEFT_SHIFT 0x2A

key_code db 0
keyboard_shift db 0
key_ascii db 0
keyboard_wait db 0

keyboard_irq:
 pusha
 mov eax, 0
 INB 0x60
 mov byte [key_code], al
 
;under development, new update will be 21.1.2021
; mov dword [selected_keyboard_set], english
; add eax, dword [selected_keyboard_set]
; mov bl, byte [eax] ;read ascii value of key
; mov byte [key_ascii], bl

 mov dword [keyboard_wait], 0

 EOI_MASTER_PIC
 popa
 iret

wait_for_keyboard:
 mov dword [keyboard_wait], 1
 .wait:
  cmp dword [keyboard_wait], 0
  je .done
  hlt
 jmp .wait

 .done:
 ret
