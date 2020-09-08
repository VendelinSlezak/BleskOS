;BleskOS

%macro OUTB 2
  %if %2!=al
    mov al, %2
  %endif
  mov dx, %1
  out dx, al
%endmacro

%macro OUTW 2
  %if %2!=ax
    mov ax, %2
  %endif
  mov dx, %1
  out dx, ax
%endmacro

%macro OUTL 2
  %if %2!=ax
    mov eax, %2
  %endif
  mov dx, %1
  out dx, eax
%endmacro

%macro INB 1
  mov dx, %1
  in al, dx
%endmacro

%macro INW 1
  mov dx, %1
  in ax, dx
%endmacro

%macro INL 1
  mov dx, %1
  in eax, dx
%endmacro
