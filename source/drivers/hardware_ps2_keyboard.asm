;BleskOS

%define KEY_ESC 0x01
%define KEY_ENTER 0x1C
%define KEY_TAB 0x0F
%define KEY_BACKSPACE 0x0E
%define KEY_CAPSLOCK 0x3A
%define KEY_ESCAPE 0x01
%define KEY_DELETE 0x53
%define KEY_LEFT_SHIFT 0x2A
%define KEY_RIGHT_SHIFT 0x36
%define KEY_UP 0x48 
%define KEY_DOWN 0x50
%define KEY_LEFT 0x4B
%define KEY_RIGHT 0x4D

key_code dd 0
keyboard_shift dd 0
key_ascii db 0
keyboard_wait db 0
selected_keyboard_set dd 0

english_keyboard_layout:
 db ' ', ' ' ;zero, escape
 db '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', ' ', ' ' ;first line, backspace, tab
 db 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', ' ', ' ' ;second line, enter, left ctrl
 db 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`', ' ', '\' ;third line, left shift
 db 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', ' ' ;fourth line, right shift
 times 15 db ' ' ;other control keys
 db '7', '8', '9', '-', '4', '5', '6', '+', '1', '2', '3', '0', '.' ;numpad
english_shift_keyboard_layout:
 db ' ', ' ' ;zero, escape
 db '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+', ' ', ' ' ;first line, backspace, tab
 db 'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', ' ', ' ' ;second line, enter, left ctrl
 db 'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', '¨', '~', ' ', '|' ;third line, left shift
 db 'Z', 'X', 'C', 'V', 'B', 'N', 'M', '<', '>', '?', ' ' ;fourth line, right shift
 times 15 db ' ' ;other control keys
 db '7', '8', '9', '-', '4', '5', '6', '+', '1', '2', '3', '0', '.' ;numpad

init_keyboard:
 mov dword [keyboard_shift], 0
 ret

keyboard_irq:
 pusha
 mov eax, 0
 INB 0x60
 mov dword [key_code], eax

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

wait_for_keyboard:
 mov dword [keyboard_wait], 1
 .wait:
  cmp dword [keyboard_wait], 0
  je .done
  hlt
 jmp .wait

 .done:
 ret
