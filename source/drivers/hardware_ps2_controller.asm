;BleskOS

%macro READ_PS2_DATA 0
 WAIT 5
 INB 0x60
%endmacro

%macro WRITE_PS2_DATA 1
 WAIT 5
 OUTB 0x60, %1
%endmacro

%macro WRITE_PS2_COMMAND 1
 WAIT 5
 OUTB 0x64, %1
%endmacro

init_ps2_controller:
 ;disable PS/2 controllers
 WRITE_PS2_COMMAND 0xAD
 WRITE_PS2_COMMAND 0xA7

 ;enable interrupts
 WRITE_PS2_COMMAND 0x20
 READ_PS2_DATA
 or al, 0x3 ;enable interrupts bits
 push eax ;save value

 WRITE_PS2_COMMAND 0x60
 pop eax
 mov bl, al
 WRITE_PS2_DATA al

 ;enable PS/2 controllers
 WRITE_PS2_COMMAND 0xAE
 WRITE_PS2_COMMAND 0xA8

 ret
