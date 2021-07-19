;BleskOS

ac97_nam_base dw 0
ac97_nabm_base dw 0
ac97_sound_data dd 0
ac97_volume dd 0

init_ac97:
 cmp word [ac97_nam_base], 0
 je .end

 PSTR 'Sound card AC97', ac97_string

 ;resume from cold reset
 BASE_OUTD ac97_nabm_base, 0x2C, 0x2
 WAIT 2
 
 ;reset all streams
 BASE_OUTB ac97_nabm_base, 0x0B, 0x2
 BASE_OUTB ac97_nabm_base, 0x1B, 0x2
 BASE_OUTB ac97_nabm_base, 0x2B, 0x2
 WAIT 2

 ;register reset
 BASE_OUTW ac97_nam_base, 0x0, 0xFF

 ;set volume
 BASE_OUTW ac97_nam_base, 0x2, 0x0
 BASE_OUTW ac97_nam_base, 0x18, 0x0808

 ;set output stream
 BASE_OUTD ac97_nabm_base, 0x10, MEMORY_AC97_BUFFER
 BASE_OUTB ac97_nabm_base, 0x15, 1

 .end:
 ret

ac97_set_volume:
 ;calculate from 100 to AC97 volume range
 mov eax, dword [ac97_volume]
 mov ebx, 32
 mul ebx
 mov ebx, 100
 mov edx, 0
 div ebx

 mov ah, al ;same volume left and right
 BASE_OUTW ac97_nam_base, 0x18, ax

 ret

ac97_play_sound:
 mov eax, dword [ac97_sound_data]
 mov dword [MEMORY_AC97_BUFFER+0], eax
 mov dword [MEMORY_AC97_BUFFER+4], 0x4000FFFE ;lenght 64 KB

 ;start transfer
 BASE_OUTB ac97_nabm_base, 0x1B, 0x2

 ret

ac97_stop_sound:
 BASE_OUTB ac97_nabm_base, 0x1B, 0x0

 ret
