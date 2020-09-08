;BleskOS

%define KEYBOARD_SLOVAK 1

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

init_keyboard:
  mov byte [keyboard_shift], 0
  SET_IDT 1, keyboard_irq
  mov byte [keyboard_langunge], KEYBOARD_SLOVAK

  ret

keyboard_irq:
  mov eax, 0
  INB 0x60
  mov byte [keyboard_key], al

  cmp al, KEY_CAPSLOCK ;if shift=0 after this it is 1 and reverse
  jne .endifcapslock
    cmp byte [keyboard_shift], 0
    jne .movshift0
    mov byte [keyboard_shift], 1
    jmp .endifcapslock
    .movshift0:
    mov byte [keyboard_shift], 0
  .endifcapslock:

  cmp al, KEY_LEFT_SHIFT
  jne .endifshift_press
    mov byte [keyboard_shift], 1
  .endifshift_press:

  cmp al, KEY_LEFT_SHIFT+0x80
  jne .endifshift_release
    mov byte [keyboard_shift], 0
  .endifshift_release:

  cmp byte [keyboard_langunge], KEYBOARD_SLOVAK
  jne .endif1
    mov ebx, keyboard_slovak_normal
    cmp byte [keyboard_shift], 1
    jne .endif1shift
      mov ebx, keyboard_slovak_shift
    .endif1shift:

    add ebx, eax ;move to key
    mov ah, byte [ebx] ;get value
    mov byte [keyboard_ascii], ah ;set varibile
  .endif1:

  mov byte [keyboard_wait_for_key], 0

  OUTB 0x20, 0x20

  iret

wait_for_key:
  mov byte [keyboard_wait_for_key], 1

  .halt:
    hlt ;wait for interrupt
    cmp byte [keyboard_wait_for_key], 0
    je .done
  jmp .halt

  .done:
  ret

keyboard_langunge db 0
keyboard_wait_for_key db 0
keyboard_key db 0
keyboard_ascii db 0
keyboard_shift db 0

keyboard_slovak_normal:
db ' ', ' ', '+', 'l', 's', 'c', 't', 'z', 'y', 'a', 'i', 'e', '=', '=', ' '
db ' ', 'q', 'w', 'e', 'r', 't', 'z', 'u', 'i', 'o', 'p', '.', '.', ' ', ' '
db 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', '.', '!', ';', ' '
db '.', 'y', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '-', ' ', '*'
times 15 db ' '
db '7', '8', '9', '-', '4', '5', '6', '+', '1', '2', '3', ' ', ' '
times 256-0x53 db ' '

keyboard_slovak_shift:
db ' ', ' ', '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '%', ' ', ' '
db ' ', 'Q', 'W', 'E', 'R', 'T', 'Z', 'U', 'I', 'O', 'P', '/', '(', ' ', ' '
db 'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', '"', '!', '\', ' '
db ')', 'Y', 'X', 'C', 'V', 'B', 'N', 'M', '?', ':', '_', ' ', '*'
times 15 db ' '
db '7', '8', '9', '-', '4', '5', '6', '+', '1', '2', '3', ' ', ' '
times 256-0x53 db ' '
