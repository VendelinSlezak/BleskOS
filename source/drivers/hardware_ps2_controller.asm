;BleskOS

%macro READ_PS2_DATA 0
 call read_ps2_data
%endmacro

%macro WRITE_PS2_DATA 1
 mov bl, %1
 call write_ps2_data
%endmacro

%macro WRITE_PS2_COMMAND 1
 mov bl, %1
 call write_ps2_command
%endmacro

read_ps2_data:
 mov ecx, 1000
 .wait:
  INB 0x64
  and al, 0x1 ;data are in buffer 
  cmp al, 0x1
  je .read
 loop .wait

 .read:
 INB 0x60
 ret

write_ps2_data:
 mov ecx, 1000
 .wait:
  INB 0x64
  and al, 0x2 
  cmp al, 0x0 ;output buffer is empty
  je .write
 loop .wait

 .write:
 OUTB 0x60, bl
 ret

write_ps2_command:
 mov ecx, 1000
 .wait:
  INB 0x64
  and al, 0x2 
  cmp al, 0x0 ;output buffer is empty
  je .write
 loop .wait

 .write:
 OUTB 0x64, bl
 ret

init_ps2_controller:
 ;disable PS/2 controllers
 WRITE_PS2_COMMAND 0xAD
 WRITE_PS2_COMMAND 0xA7

 ;clear buffer
 READ_PS2_DATA
 READ_PS2_DATA
 READ_PS2_DATA

 ;enable interrupts
 WRITE_PS2_COMMAND 0x20
 READ_PS2_DATA
 or al, 0x3 ;enable interrupts bits
 push eax ;save value

 WRITE_PS2_COMMAND 0x60
 pop eax
 mov bl, al
 call write_ps2_data

 ;enable PS/2 controllers
 WRITE_PS2_COMMAND 0xAE
 WRITE_PS2_COMMAND 0xA8

 ret
