;BleskOS

%define PS2_DATA_PORT 0x60
%define PS2_COMMAND_PORT 0x64

ps2_command db 0

init_ps2_controller:
 ;read configuration byte
 OUTB PS2_COMMAND_PORT, 0x20
 INB PS2_DATA_PORT
 mov byte [ps2_command], al

 ;enable interrupts
 OUTB PS2_COMMAND_PORT, 0x60
 or byte [ps2_command], 0x3 ;enable interrupts bits
 mov al, byte [ps2_command]
 OUTB PS2_DATA_PORT, al

 ;enable PS/2 controllers
 OUTB PS2_COMMAND_PORT, 0xAE
 OUTB PS2_COMMAND_PORT, 0xA8

 ret
