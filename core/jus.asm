;BleskOS real mode edition

read_file:
  mov bx, 128 ;every file is 64 kb lenght
  mul bx
  add ax, 128 ;first 64 kb is for BleskOS
  mov word [disk_packet_sector], ax

  mov ah, 0x42
  mov si, disk_packet
  mov dl, byte [drive_number]
  int 13h ;read file

  ret

write_file:
  mov bx, 128 ;every file is 64 kb lenght
  mul bx
  add ax, 128 ;first 64 kb is for BleskOS
  mov word [disk_packet_sector], ax

  mov ah, 0x43
  mov si, disk_packet
  mov dl, byte [drive_number]
  int 13h ;write file

  ret

  
disk_packet:
  dw 0x0010 ;singature
disk_packet_nos:
  dw 127 ;number of sectors
disk_packet_offset:
  dw 0x0
disk_packet_segment:
  dw 0x0
disk_packet_sector:
  dq 0

drive_number db 0
