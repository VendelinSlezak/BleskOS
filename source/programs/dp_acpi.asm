;BleskOS

%define DP_ACPI_COLOR 0xF924

dp_acpi:
 ;orange background
 CLEAR_SCREEN DP_ACPI_COLOR

 ;text
 PRINT 'RDSP base:', rsdp_str, LINE(1), COLUMN(1)
 mov eax, dword [rsdp_base]
 PRINT_HEX eax, LINE(1), COLUMN(12)

 PRINT 'RDST base:', rsdt_str, LINE(3), COLUMN(1)
 mov eax, dword [rsdt_base]
 PRINT_HEX eax, LINE(3), COLUMN(12)

 PRINT 'FACP base:', facp_str, LINE(5), COLUMN(1)
 mov eax, dword [facp_base]
 PRINT_HEX eax, LINE(5), COLUMN(12)

 PRINT 'Shutdown port:', shutdown_port_str, LINE(7), COLUMN(1)
 mov eax, 0
 mov ax, word [acpi_pm1_control]
 PRINT_HEX eax, LINE(7), COLUMN(16)

 PRINT 'Shutdown value:', shutdown_value_str, LINE(9), COLUMN(1)
 mov eax, 0
 mov ax, word [shutdown_value]
 PRINT_HEX eax, LINE(9), COLUMN(17)

 call redraw_screen

 .acpi_halt:
  call wait_for_keyboard

  cmp byte [key_code], KEY_ESC
  je developer_zone
 jmp .acpi_halt


