;BleskOS

ac97_nam_base dw 0
ac97_nabm_base dw 0
ac97_sound_data dd 0
ac97_sound_length dw 0
ac97_sample_rate dd 0
ac97_volume dd 0
ac97_last_entry dd 0
ac97_last_entry_length dw 0xFFFF

init_ac97:
 cmp word [ac97_nam_base], 0
 je .end

 PSTR 'Sound card AC97', ac97_string

 ;resume from cold reset
 BASE_OUTD ac97_nabm_base, 0x2C, 0x2
 WAIT 20
 
 ;reset all streams
 BASE_OUTB ac97_nabm_base, 0x0B, 0x2
 BASE_OUTB ac97_nabm_base, 0x1B, 0x2
 BASE_OUTB ac97_nabm_base, 0x2B, 0x2
 WAIT 20
 BASE_OUTB ac97_nabm_base, 0x15, 0

 ;register reset
 BASE_OUTW ac97_nam_base, 0x0, 0xFF

 ;set volume
 BASE_OUTW ac97_nam_base, 0x2, 0x0
 BASE_OUTW ac97_nam_base, 0x18, 0x0
 mov dword [ac97_volume], 100

 ;set output stream
 BASE_OUTD ac97_nabm_base, 0x10, MEMORY_AC97_BUFFER
 
 ;sample rate 48000
 BASE_OUTW ac97_nam_base, 0x2A, 0x1
 mov dword [ac97_sample_rate], 48000
 call ac97_set_sample_rate

 .end:
 ret
 
ac97_set_sample_rate:
 mov eax, dword [ac97_sample_rate]

 BASE_OUTW ac97_nam_base, 0x2C, ax
 BASE_OUTW ac97_nam_base, 0x2E, ax
 BASE_OUTW ac97_nam_base, 0x30, ax
 BASE_OUTW ac97_nam_base, 0x32, ax
 
 ret

ac97_set_volume:
 cmp dword [ac97_volume], 100
 je .max_volume
 cmp dword [ac97_volume], 0
 je .no_volume

 ;calculate to AC97 volume range
 mov eax, 100
 sub eax, dword [ac97_volume]
 mov ebx, 32
 mul ebx
 mov ebx, 100
 mov edx, 0
 div ebx

 mov ah, al ;same volume left and right
 BASE_OUTW ac97_nam_base, 0x18, ax

 ret
 
 .max_volume:
 BASE_OUTW ac97_nam_base, 0x18, 0x0
 ret
 
 .no_volume:
 BASE_OUTW ac97_nam_base, 0x18, 0x8000
 ret
 
ac97_sound_position:
 BASE_INB ac97_nabm_base, 0x14
 
 ret

ac97_fill_buffer:
 mov eax, dword [ac97_last_entry]
 shl eax, 3 ;mul 8
 add eax, MEMORY_AC97_BUFFER
 mov ebx, dword [ac97_sound_data]
 mov dword [eax+0], ebx
 mov ebx, 0
 mov bx, word [ac97_sound_length]
 dec bx
 mov dword [eax+4], ebx
 
 ;update pointer in LVI register
 mov eax, dword [ac97_last_entry]
 BASE_OUTB ac97_nabm_base, 0x15, al
 inc dword [ac97_last_entry]
 and dword [ac97_last_entry], 0x1F

 ret
 
ac97_play_sound:
 BASE_OUTB ac97_nabm_base, 0x1B, 0x1
 BASE_OUTW ac97_nabm_base, 0x16, 0x1C
 
 ret

ac97_stop_sound:
 BASE_OUTB ac97_nabm_base, 0x1B, 0x0
 BASE_OUTW ac97_nabm_base, 0x16, 0x1C

 ret

ac97_clear_buffer:
 mov edi, MEMORY_AC97_BUFFER
 mov eax, 0
 mov ecx, 32*2
 rep stosd

 BASE_INB ac97_nabm_base, 0x14
 inc al
 and al, 0x1F
 mov byte [ac97_last_entry], al
 
 jmp ac97_stop_sound
