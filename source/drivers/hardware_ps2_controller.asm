;BleskOS

ps2_exist db 0
ps2_command db 0

read_ps2_controller:
 mov dword [ticks], 0
 .wait:
  INB 0x64
  and al, 0x1
  cmp al, 0x1
  je .read
  hlt
 cmp dword [ticks], 200
 jl .wait

 .read:
 INB 0x60

 ret

write_ps2_controller:
 mov dword [ticks], 0
 .wait:
  INB 0x64
  and al, 0x2
  cmp al, 0
  je .write
  hlt
 cmp dword [ticks], 200
 jl .wait

 .write:
 mov al, byte [ps2_command]
 out 0x60, al

 ret

write_command_ps2_controller:
 mov dword [ticks], 0
 .wait:
  INB 0x64
  and al, 0x2
  cmp al, 0
  je .write
  hlt
 cmp dword [ticks], 200
 jl .wait

 .write:
 mov al, byte [ps2_command]
 out 0x64, al

 ret

init_ps2_controller:
 cmp byte [ps2_exist], 0
 je .done

 ;disable PS/2 controllers
 mov byte [ps2_command], 0xAD
 call write_command_ps2_controller
 mov byte [ps2_command], 0xA7
 call write_command_ps2_controller

 ;enable interrupts
 mov byte [ps2_command], 0x60
 call write_command_ps2_controller
 mov byte [ps2_command], 0x47
 call write_ps2_controller

 ;enable PS/2 controllers
 mov byte [ps2_command], 0xAE
 call write_command_ps2_controller
 mov byte [ps2_command], 0xA8
 call write_command_ps2_controller

 mov dword [ps2_mouse_present], 1

 .done:
 ret
