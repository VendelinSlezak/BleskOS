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
%define KEY_CTRL 0x1D

%define KEY_HOME 0x47
%define KEY_PAGE_UP 0x49
%define KEY_PAGE_DOWN 0x51
%define KEY_END 0x4F
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
%define KEY_SPACE 0x39

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

%define KEY_DASH 0x0C
%define KEY_EQUAL 0x0D
%define KEY_LEFT_BRACKET 0x1A
%define KEY_RIGHT_BRACKET 0x1B
%define KEY_BACKSLASH 0x2B
%define KEY_SEMICOLON 0x27
%define KEY_SINGLE_QUOTE 0x28
%define KEY_COMMA 0x33
%define KEY_DOT 0x34
%define KEY_FORWARD_SLASH 0x35
%define KEY_BACK_TICK 0x29

english_keyboard_layout:
 dw 0, 0 ;zero, escape
 dw '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', 0, 0 ;first line, backspace, tab
 dw 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', 0xA, 0 ;second line, enter, left ctrl
 dw 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '`', '`', 0, '\' ;third line, left shift
 dw 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0, '*', 0, ' ' ;fourth line, right shift
 times 10 dw 0 ;other control keys
 dw '7', '8', '9', '-', '4', '5', '6', '+', '1', '2', '3', '0', '.' ;numpad
 times 230 dw 0
english_shift_keyboard_layout:
 dw 0, 0 ;zero, escape
 dw '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+', 0, 0 ;first line, backspace, tab
 dw 'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', 0xA, 0 ;second line, enter, left ctrl
 dw 'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', '"', '~', 0, '|' ;third line, left shift
 dw 'Z', 'X', 'C', 'V', 'B', 'N', 'M', '<', '>', '?', 0, '*', 0, ' ' ;fourth line, right shift
 times 10 dw 0 ;other control keys
 dw '7', '8', '9', '-', '4', '5', '6', '+', '1', '2', '3', '0', '.' ;numpad
 times 230 dw 0
 
slovak_keyboard_layout:
 dw 0, 0 ;zero, escape
 dw '+', 0x13E, 0x161, 0x10D, 0x165, 0x17E, 0xFD, 0xE1, 0xED, 0xE9, '=', 0, 0, 0 ;first line, backspace, tab
 dw 'q', 'w', 'e', 'r', 't', 'z', 'u', 'i', 'o', 'p', 0xFA, 0xE4, 0xA, 0 ;second line, enter, left ctrl
 dw 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', 0xF4, 0xA7, ';', 0, 0x148 ;third line, left shift
 dw 'y', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '-', 0, '\', 0, ' ' ;fourth line, right shift
 times 10 dw 0 ;other control keys
 dw '7', '8', '9', '-', '4', '5', '6', '+', '1', '2', '3', '0', '.' ;numpad
 times 230 dw 0
slovak_shift_keyboard_layout:
 dw 0, 0 ;zero, escape
 dw '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '%', 0, 0, 0 ;first line, backspace, tab
 dw 'Q', 'W', 'E', 'R', 'T', 'Z', 'U', 'I', 'O', 'P', '/', '(', 0xA, 0 ;second line, enter, left ctrl
 dw 'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', '"', '!', '°', 0, ')' ;third line, left shift
 dw 'Y', 'X', 'C', 'V', 'B', 'N', 'M', '?', ':', '_', 0, '|', 0, ' ' ;fourth line, right shift
 times 10 dw 0 ;other control keys
 dw '7', '8', '9', '-', '4', '5', '6', '+', '1', '2', '3', '0', '.' ;numpad
 times 230 dw 0

usb_keyboard_layout:
 db 0, 0, 0, 0
 db KEY_A,  KEY_B,  KEY_C,  KEY_D,  KEY_E,  KEY_F,  KEY_G,  KEY_H,  KEY_I,  KEY_J,  KEY_K,  KEY_L,  KEY_M,  KEY_N,  KEY_O,  KEY_P,  KEY_Q,  KEY_R,  KEY_S,  KEY_T,  KEY_U,  KEY_V,  KEY_W,  KEY_X,  KEY_Y,  KEY_Z
 db KEY_1, KEY_2, KEY_3, KEY_4, KEY_5, KEY_6, KEY_7, KEY_8, KEY_9, KEY_0
 db KEY_ENTER, KEY_ESC, KEY_BACKSPACE, KEY_TAB, KEY_SPACE
 db KEY_DASH, KEY_EQUAL, KEY_LEFT_BRACKET, KEY_RIGHT_BRACKET, KEY_BACKSLASH, 0, KEY_SEMICOLON, KEY_SINGLE_QUOTE, KEY_BACK_TICK, KEY_COMMA, KEY_DOT, KEY_FORWARD_SLASH
 db KEY_CAPSLOCK, KEY_F1, KEY_F2, KEY_F3, KEY_F4, KEY_F5, KEY_F6, KEY_F7, KEY_F8, KEY_F9, KEY_F10, KEY_F11, KEY_F12
 db 0, 0, 0, 0, 0, KEY_PAGE_UP, KEY_DELETE, 0, KEY_PAGE_DOWN, KEY_RIGHT, KEY_LEFT, KEY_DOWN, KEY_UP
 times 142 db 0
 db KEY_LEFT_SHIFT, 0, 0, 0, KEY_RIGHT_SHIFT, 0, 0

key_code dd 0
keyboard_shift dd 0
keyboard_ctrl dd 0
key_unicode dw 0
selected_keyboard_set dd english_keyboard_layout
selected_keyboard_shift_set dd english_shift_keyboard_layout

keyboard_convert_to_unicode: 
 ;if ctrl key is pressed
 cmp al, KEY_CTRL
 jne .if_ctrl_pressed
  mov dword [keyboard_ctrl], 1
 .if_ctrl_pressed:
 cmp al, KEY_CTRL+0x80
 jne .if_ctrl_released
  mov dword [keyboard_ctrl], 0
 .if_ctrl_released:
 
 ;if some shift key is pressed
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
 jg .done ;released key

 mov eax, dword [selected_keyboard_set]
 IF_E dword [keyboard_shift], 1, if_shift_yes
  mov eax, dword [selected_keyboard_shift_set]
 ENDIF if_shift_yes

 mov ecx, dword [key_code]
 mov bx, word [eax+(ecx*2)] ;read unicode value of key
 mov word [key_unicode], bx

 .done:
 ret
