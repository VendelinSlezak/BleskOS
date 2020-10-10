;BleskOS real mode edition

check_if_exist:
  mov ah, 0x41
  mov bx, 0x55AA
  int 13h

  jc .no_device
  mov ax, 1 ;device exist
  ret

  .no_device:
  mov ax, 0 ;device isnt exist
  ret
