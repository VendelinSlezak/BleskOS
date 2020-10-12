;BleskOS real mode

read_time:
  ;seconds
  mov al, 0x00
  out 0x70, al
  in al, 0x71
  mov bl, al ;copy value
  and bl, 0x0F
  mov byte [time_s], bl
  shr al, 4
  mov bl, 10
  mul bl ;from hexadecimal to decadic
  add byte [time_s], al

  ;minutes
  mov al, 0x02
  out 0x70, al
  in al, 0x71
  mov bl, al ;copy value
  and bl, 0x0F
  mov byte [time_m], bl
  shr al, 4
  mov bl, 10
  mul bl ;from hexadecimal to decadic
  add byte [time_m], al

  ;hours
  mov al, 0x04
  out 0x70, al
  in al, 0x71
  mov bl, al ;copy value
  and bl, 0x0F
  mov byte [time_h], bl
  shr al, 4
  mov bl, 10
  mul bl ;from hexadecimal to decadic
  add byte [time_h], al

  ;day
  mov al, 0x07
  out 0x70, al
  in al, 0x71
  mov bl, al ;copy value
  and bl, 0x0F
  mov byte [time_d], bl
  shr al, 4
  mov bl, 10
  mul bl ;from hexadecimal to decadic
  add byte [time_d], al

  ;month
  mov al, 0x08
  out 0x70, al
  in al, 0x71
  mov bl, al ;copy value
  and bl, 0x0F
  mov byte [time_n], bl
  shr al, 4
  mov bl, 10
  mul bl ;from hexadecimal to decadic
  add byte [time_n], al

  ;years
  mov al, 0x09
  out 0x70, al
  in al, 0x71
  mov bl, al ;copy value
  and bl, 0x0F
  mov byte [time_y], bl
  shr al, 4
  mov bl, 10
  mul bl ;from hexadecimal to decadic
  add byte [time_y], al
  add word [time_y], 2000

  ret

  time_s db 0 ;seconds
  time_m db 0 ;minutes
  time_h db 0 ;hours
  time_d db 0 ;day
  time_n db 0 ;month
  time_y dw 0 ;year
