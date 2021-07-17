;BleskOS

%define PS2_DATA_PORT 0x60
%define PS2_COMMAND_PORT 0x64

ps2_command db 0

read_ps2_controller:
 mov ecx, 100
 .wait:
  INB 0x64
  and al, 0x1
  cmp al, 0x1
  je .read
 loop .wait

 .read:
 INB 0x60

 ret

write_ps2_controller:
 mov ecx, 100
 .wait:
  INB 0x64
  and al, 0x2
  cmp al, 0
  je .write
 loop .wait

 .write:
 mov al, byte [ps2_command]
 out 0x60, al

 ret

write_command_ps2_controller:
 mov ecx, 100
 .wait:
  INB 0x64
  and al, 0x2
  cmp al, 0
  je .write
 loop .wait

 .write:
 mov al, byte [ps2_command]
 out 0x64, al

 ret

init_ps2_controller:
 ;read configuration byte
 mov byte [ps2_command], 0x20
 call write_command_ps2_controller
 call read_ps2_controller

 mov bl, al
 or bl, 0x3 ;enable interrupts
 and bl, 0xCF ;enable keyboard and mouse

 ;enable interrupts
 mov byte [ps2_command], 0x60
 call write_command_ps2_controller
 mov byte [ps2_command], bl
 call write_ps2_controller

 ;enable PS/2 controllers
 mov byte [ps2_command], 0xAE
 call write_command_ps2_controller
 mov byte [ps2_command], 0xA8
 call write_command_ps2_controller

 ret
