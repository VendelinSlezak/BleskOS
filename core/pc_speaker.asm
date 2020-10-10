;BleskOS real mode edition

play_pc_speaker:
  mov al, 0xB6
  out 0x43, al

  ;set frequency
  mov al, bl
  out 0x42, al
  mov al, bh
  out 0x42, al

  ;turn pc speaker on
  in al, 0x61
  or al, 0x3 ;set bit 0 and bit 1
  out 0x61, al

  ret

stop_pc_speaker:
  in al, 0x61
  and al, 0xFC ;clear bit 0 and bit 1
  out 0x61, al

  ret
