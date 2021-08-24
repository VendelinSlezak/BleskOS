;BleskOS

;This macros make other code more readable

%define MEMORY_RAM_SCREEN 0x01000000
%define MEMORY_JUS_BN 0x00100000
%define MEMORY_FILE_DESCRIPTOR 0x00200000
%define MEMORY_FOLDER 0x00300000
%define MEMORY_NEW_FOLDER 0x00400000
%define MEMORY_HDA_BUFFER 0x00500000
%define MEMORY_CORB 0x00510000
%define MEMORY_RIRB 0x00520000
%define MEMORY_HDA_DMAPOS 0x00530000
%define MEMORY_AC97_BUFFER 0x00540000
%define MEMORY_OHCI 0x00600000
%define MEMORY_UHCI 0x00700000
%define MEMORY_EHCI 0x00800000
%define MEMORY_xHCI 0x00900000
%define MEMORY_FAT32_FOLDER 0x00A00000
%define MEMORY_ISO9660_FOLDER 0x00B00000
%define FREE_MEMORY 0x02000000

%macro INB 1
 %if %1!=dx
 mov dx, %1
 %endif
 in al, dx
%endmacro

%macro BASE_INB 2
 mov dx, word [%1]
 add dx, %2
 in al, dx
%endmacro

%macro MMIO_INB 2
 mov ebp, dword [%1]
 add ebp, %2
 mov al, byte [ebp]
%endmacro

%macro INW 1
 %if %1!=dx
 mov dx, %1
 %endif
 in ax, dx
%endmacro

%macro BASE_INW 2
 mov dx, word [%1]
 add dx, %2
 in ax, dx
%endmacro

%macro MMIO_INW 2
 mov ebp, dword [%1]
 add ebp, %2
 mov ax, word [ebp]
%endmacro

%macro IND 1
 %if %1!=dx
 mov dx, %1
 %endif
 in eax, dx
%endmacro

%macro BASE_IND 2
 mov dx, word [%1]
 add dx, %2
 in eax, dx
%endmacro

%macro MMIO_IND 2
 mov ebp, dword [%1]
 add ebp, %2
 mov eax, dword [ebp]
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

%macro BASE_OUTB 3
 mov dx, word [%1]
 add dx, %2
 %if %3!=al
 mov al, %3
 %endif
 out dx, al
%endmacro

%macro MMIO_OUTB 3
 mov ebp, dword [%1]
 add ebp, %2
 mov byte [ebp], %3
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

%macro BASE_OUTW 3
 mov dx, word [%1]
 add dx, %2
 %if %3!=ax
 mov ax, %3
 %endif
 out dx, ax
%endmacro

%macro MMIO_OUTW 3
 mov ebp, dword [%1]
 add ebp, %2
 mov word [ebp], %3
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

%macro BASE_OUTD 3
 mov dx, word [%1]
 add dx, %2
 %if %3!=eax
 mov eax, %3
 %endif
 out dx, eax
%endmacro

%macro MMIO_OUTD 3
 mov ebp, dword [%1]
 add ebp, %2
 mov dword [ebp], %3
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
 jmp .%1 ;long jump for any lenght of code
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

%macro ENDIF 1
 .%1:
%endmacro

%macro MOV_0xF_SHIFT 3
 mov %1, %2
 shr %1, %3
 and %1, 0xF
%endmacro
