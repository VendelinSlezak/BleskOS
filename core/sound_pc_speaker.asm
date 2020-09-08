;BleskOS

play_pc_speaker:
  mov al, 0xB6
  out 0x43, al

  ;set frequency
  mov ax, cx
  out 0x42, al
  mov al, ah
  out 0x42, al

  ;turn pc speaker
  in al, 0x61
  or al, 0x3
  out 0x61, al

  ret
%macro PLAY_PC_SPEAKER 1
  %if %1!=cx
  mov cx, %1
  %endif
  call play_pc_speaker
%endmacro

stop_pc_speaker:
  in al, 0x61
  and al, 0xFC
  out 61h, al

  ret
