;BleskOS real edition

%define ENGLISH_KEYBOARD 1
%define SLOVAK_KEYBOARD 2

%define KEY_ESC 0x01
%define KEY_ENTER 0x1C
%define KEY_UP 0x48 
%define KEY_DOWN 0x50
%define KEY_LEFT 0x4B
%define KEY_RIGHT 0x4D

wait_for_key:
  mov ah, 0x00
  int 16h

  cmp byte [keyboard_langunge], SLOVAK_KEYBOARD
  jne .endif1
    cmp al, 'z'
    jne .endif_sk_z
      mov al, 'y'
      jmp .done
    .endif_sk_z:

    cmp al, 'Z'
    jne .endif_sk_Z
      mov al, 'Y'
      jmp .done
    .endif_sk_Z:

    cmp al, 'y'
    jne .endif_sk_y
      mov al, 'z'
      jmp .done
    .endif_sk_y:

    cmp al, 'Y'
    jne .endif_sk_Y
      mov al, 'Z'
      jmp .done
    .endif_sk_Y:

    ret
  .endif1:

  .done:
  ret

read_press_key:
  mov ah, 0x01
  int 16h

  jz .done ;if no key

  mov ah, 0x00
  int 16h ;read key

  .done:
  ret

  keyboard_langunge db 0
