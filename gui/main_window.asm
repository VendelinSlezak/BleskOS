;BleskOS real mode

main_window:
  DRAW_BACKGROUND 0x20

  HIDE_CURSOR

  SC 0, 0
  DRAW_LINE 79, 0x40
  DRAW_COLUMN 24, 0x40
  SC 24, 0
  DRAW_LINE 80, 0x40
  SC 0, 79
  DRAW_COLUMN 24, 0x40

  SC 2, 2
  PRINT mw_str1
  SC 4, 2
  PRINT mw_str2
  SC 6, 2
  PRINT mw_str3
  SC 7, 2
  PRINT mw_str4
  SC 8, 2
  PRINT mw_str5
  SC 10, 2
  PRINT mw_str6
  SC 12, 2
  PRINT mw_str7
  SC 14, 2
  PRINT mw_str8
  SC 16, 2
  PRINT mw_str9
  SC 17, 2
  PRINT mw_str10
  SC 19, 2
  PRINT mw_str11
  SC 20, 2
  PRINT mw_str12
  SC 22, 2
  PRINT mw_str13

  .halt:
  call wait_for_key

  cmp ah, KEY_F1
  je dp_window

  jmp .halt

  mw_str1 db 'Welcome in BleskOS!', 0
  mw_str2 db 'You can open programs with keyboard:', 0
  mw_str3 db '[a] Text editor', 0
  mw_str4 db '[b] Presentation editor', 0
  mw_str5 db '[c] Table editor', 0
  mw_str6 db '[d] Graphic editor', 0
  mw_str7 db '[e] Sound editor', 0
  mw_str8 db '[f] File manager', 0
  mw_str9 db '[g] Calculator', 0
  mw_str10 db '[h] Timer', 0
  mw_str11 db '[F1] Developer programs', 0
  mw_str12 db '[F2] Hardware info', 0
  mw_str13 db 'You can shutdown computer by press power button.', 0
