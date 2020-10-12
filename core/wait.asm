;BleskOS real mode

wait_ms:
  mov bx, 1000
  mul bx

  mov cx, dx
  mov dx, ax
  mov ah, 0x86
  int 15h ;wait

  ret
