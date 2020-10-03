;BleskOS

read_time:
  mov al, 00h
  out 70h, al
  in al, 71h
  mov byte [seconds], al

  mov al, 02h
  out 70h, al
  in al, 71h
  mov byte [minutes], al

  mov al, 04h
  out 70h, al
  in al, 71h
  mov byte [hours], al

  mov al, 07h
  out 70h, al
  in al, 71h
  mov byte [day], al

  mov al, 08h
  out 70h, al
  in al, 71h
  mov byte [month], al

  mov al, 09h
  out 70h, al
  in al, 71h
  mov byte [year], al

  ret

  seconds db 0
  minutes db 0
  hours db 0
  day db 0
  month db 0
  year db 0
