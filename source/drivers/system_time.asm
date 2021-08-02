;BleskOS

second dd 0
minute dd 0
hour dd 0
day dd 0
month dd 0
year dd 0

read_time:
 call read_time_second

 mov dword [minute], 0
 OUTB 0x70, 0x82 ;minutes
 INB 0x71
 mov bl, al
 and bl, 0xF
 mov byte [minute], bl
 shr al, 4
 mov bl, 10
 mul bl
 add byte [minute], al

 mov dword [hour], 0
 OUTB 0x70, 0x84 ;hours
 INB 0x71
 mov cl, al
 and cl, 0x80
 cmp cl, 0x80
 jne .24_hour_format
 add al, 12 ;PM time
 .24_hour_format:
 mov bl, al
 and bl, 0xF
 mov byte [hour], bl
 shr al, 4
 mov bl, 10
 mul bl
 add byte [hour], al

 mov dword [day], 0
 OUTB 0x70, 0x87 ;day
 INB 0x71
 mov bl, al
 and bl, 0xF
 mov byte [day], bl
 shr al, 4
 mov bl, 10
 mul bl
 add byte [day], al

 mov dword [month], 0
 OUTB 0x70, 0x88 ;months
 INB 0x71
 mov bl, al
 and bl, 0xF
 mov byte [month], bl
 shr al, 4
 mov bl, 10
 mul bl
 add byte [month], al

 mov dword [year], 2000
 OUTB 0x70, 0x89 ;years
 INB 0x71
 mov bl, al
 and bl, 0xF
 mov cl, bl
 shr al, 4
 mov bl, 10
 mul bl
 add cl, al
 add byte [year], cl

 ret

read_time_second:
 mov dword [second], 0
 OUTB 0x70, 0x80 ;seconds
 INB 0x71
 mov bl, al
 and bl, 0xF
 mov byte [second], bl
 shr al, 4
 mov bl, 10
 mul bl
 add byte [second], al

 ret
