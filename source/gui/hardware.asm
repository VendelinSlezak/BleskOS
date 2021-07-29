;BleskOS

hardware_zone:
 CLEAR_SCREEN 0x0600

 PRINT 'BleskOS version 1', bleskos_str, LINE(1), COLUMN(1)
 PRINT '[F1] Shutdown', shutdown_str, LINE(3), COLUMN(1)

 call redraw_screen

 .hardware_halt:
  call wait_for_keyboard

  cmp byte [key_code], KEY_ESC
  je main_window

  cmp byte [key_code], KEY_F1
  je .shutdown
 jmp .hardware_halt

 .shutdown:
 call shutdown
 jmp main_window
