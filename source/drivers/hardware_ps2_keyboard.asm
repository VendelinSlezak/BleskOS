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

%define KEY_F1 0x3B
%define KEY_F2 0x3C
%define KEY_F3 0x3D
%define KEY_F4 0x3E
%define KEY_F5 0x3F
%define KEY_F6 0x40
%define KEY_F7 0x41
%define KEY_F8 0x42
%define KEY_F9 0x43
%define KEY_F10 0x44
%define KEY_F11 0x57
%define KEY_F12 0x58

%define KEY_A 0x1E
%define KEY_B 0x30
%define KEY_C 0x2E
%define KEY_D 0x20
%define KEY_E 0x12
%define KEY_F 0x21
%define KEY_G 0x22
%define KEY_H 0x23
%define KEY_I 0x17
%define KEY_J 0x24
%define KEY_K 0x25
%define KEY_L 0x26
%define KEY_M 0x32
%define KEY_N 0x31
%define KEY_O 0x18
%define KEY_P 0x19
%define KEY_Q 0x10
%define KEY_R 0x13
%define KEY_S 0x1F
%define KEY_T 0x14
%define KEY_U 0x16
%define KEY_V 0x2F
%define KEY_W 0x11
%define KEY_X 0x2D
%define KEY_Y 0x15
%define KEY_Z 0x2C

%define KEY_1 0x02
%define KEY_2 0x03
%define KEY_3 0x04
%define KEY_4 0x05
%define KEY_5 0x06
%define KEY_6 0x07
%define KEY_7 0x08
%define KEY_8 0x09
%define KEY_9 0x0A
%define KEY_0 0x0B

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
