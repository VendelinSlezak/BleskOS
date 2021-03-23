;BleskOS

;This macros make other code more readable

%define MEMORY_RAM_SCREEN 0x00100000
%define MEMORY_JUS_BNF 0x00200000
%define MEMORY_FILE_BLOCK_NUMBERS 0x00300000
%define MEMORY_AC97_BDL 0x00400000
%define MEMORY_HDA_BDL 0x00410000

%macro INB 1
 %if %1!=dx
 mov dx, %1
 %endif
 in al, dx
%endmacro

%macro INW 1
 %if %1!=dx
 mov dx, %1
 %endif
 in ax, dx
%endmacro

%macro IND 1
 %if %1!=dx
 mov dx, %1
 %endif
 in eax, dx
%endmacro

%macro OUTB 2
 %if %1!=dx
 mov dx, %1
 %endif
 %if %2!=al
 mov al, %2
 %endif
 out dx, al
%endmacro

%macro OUTW 2
 %if %1!=dx
 mov dx, %1
 %endif
 %if %2!=ax
 mov ax, %2
 %endif
 out dx, ax
%endmacro

%macro OUTD 2
 %if %1!=dx
 mov dx, %1
 %endif
 %if %2!=eax
 mov eax, %2
 %endif
 out dx, eax
%endmacro

%macro HALT 0
 .halt:
  hlt
 jmp .halt
%endmacro

%macro FOR 2
 mov ecx, %1
 .%2:
 push ecx
%endmacro

%macro ENDFOR 1
 pop ecx
 dec ecx
 cmp ecx, 0
 je .for_end_%1
 jmp .%1 ;long jump for any lenght of for code
 .for_end_%1:
%endmacro

%macro FOR_VAR 3
 mov ecx, %1
 mov %2, 0
 .%3:
 push ecx
%endmacro

%macro ENDFOR_VAR 2
 pop ecx
 dec ecx
 inc %2
 cmp ecx, 0
 jne .%1
%endmacro

%macro IF_E 3
 cmp %1, %2
 jne .%3
%endmacro

%macro IF_NE 3
 cmp %1, %2
 je .%3
%endmacro

%macro IF_H 3
 cmp %1, %2
 jl .%3
 je .%3
%endmacro

%macro IF_HE 3
 cmp %1, %2
 jl .%3
%endmacro

%macro IF_L 3
 cmp %1, %2
 jg .%3
 je .%3
%endmacro

%macro IF_LE 3
 cmp %1, %2
 jg .%3
%endmacro

%macro JMP_ENDIF 1
 jmp .%1
%endmacro

%define ELSE

%macro ENDIF 1
 .%1:
%endmacro

%macro MOV_0xF_SHIFT 3
 mov %1, %2
 shr %1, %3
 and %1, 0xF
%endmacro

%macro MOV_BYTE_SHIFT 3
 mov %1, %2
 shr %1, %3
 and %1, 0xFF
%endmacro

%macro MOV_WORD_SHIFT 3
 mov %1, %2
 shr %1, %3
 and %1, 0xFFFF
%endmacro

%macro MOV_DWORD_SHIFT 3
 mov %1, %2
 shr %1, %3
%endmacro
