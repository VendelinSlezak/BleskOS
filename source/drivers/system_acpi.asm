;BleskOS

rsdp_base dd 0
rsdt_base dd 0
facp_base dd 0
dsdt_base dd 0

acpi_command dw 0
acpi_turn_on db 0
acpi_pm1_control dw 0
acpi_pm2_control dw 0

shutdown_value dw 0
shutdown_value_2 dw 0

read_acpi:
 mov byte [ps2_exist], 1

 ;search for RSDP
 mov esi, 0xE0000 ;BIOS area
 mov ecx, 0x20000
 .search_rsdp:
  cmp dword [esi], 0x20445352
  je .test_rsdp_checksum
  .next_loop:
  inc esi
 loop .search_rsdp

 ret

 .test_rsdp_checksum:
 mov eax, 0
 mov bl, 0
 .checksum_loop:
  mov dl, byte [esi+eax]
  add bl, dl
 inc eax
 cmp eax, 20
 jne .checksum_loop

 cmp bl, 0
 jne .next_loop

 .rsdp_found:
 mov dword [rsdp_base], esi
 mov eax, dword [esi+16]
 mov dword [rsdt_base], eax

 ;search for FACP
 mov esi, dword [rsdt_base]
 mov eax, dword [esi+4]
 sub eax, 36 ;header lenght
 mov ebx, 4
 mov edx, 0
 div ebx
 mov ecx, eax ;number of entries
 cmp ecx, 0
 je .done
 add esi, 36 ;data pointers
 .search_facp:
  mov edi, dword [esi]
  cmp dword [edi], 0x50434146
  je .facp_found
  add esi, 4
 loop .search_facp

 .done:
 ret

 .facp_found:
 mov dword [facp_base], edi

 mov ax, word [edi+48]
 mov word [acpi_command], ax
 mov al, byte [edi+52]
 mov byte [acpi_turn_on], al

 mov eax, dword [edi+40]
 mov dword [dsdt_base], eax

 mov ax, word [edi+64]
 mov word [acpi_pm1_control], ax
 mov ax, word [edi+68]
 mov word [acpi_pm2_control], ax
 
 mov byte [ps2_exist], 0x2
 cmp byte [edi+8], 3
 jl .ps2_exist
 mov ax, word [edi+109]
 and ax, 0x2
 mov byte [ps2_exist], al
 .ps2_exist:

 ;turn on ACPI
 cmp word [acpi_command], 0
 je .read_shutdown_value
 mov dx, word [acpi_command]
 INW dx
 and ax, 0x1
 cmp ax, 0x1
 je .read_shutdown_value

 mov dx, word [acpi_command]
 mov al, byte [acpi_turn_on]
 OUTB dx, al

 .read_shutdown_value:
 mov word [shutdown_value], 0x2000
 mov word [shutdown_value_2], 0x2000
 mov esi, dword [dsdt_base]
 mov ecx, dword [esi+4]
 .find_sc5:
  cmp dword [esi], 0x5F35535F ;_S5_
  je .parse_sc5
  inc esi
 loop .find_sc5

 ret

 .parse_sc5:
 mov ax, 0
 mov al, byte [esi+8]
 shl ax, 10
 or word [shutdown_value], ax

 mov ax, 0
 mov al, byte [esi+10]
 shl ax, 10
 or word [shutdown_value_2], ax

 ret

shutdown:
 mov dx, word [acpi_pm1_control]
 mov ax, word [shutdown_value]
 OUTW dx, ax

 mov dx, word [acpi_pm2_control]
 mov ax, word [shutdown_value_2]
 OUTW dx, ax

 WAIT 100

 ret
