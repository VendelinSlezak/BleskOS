;BleskOS real mode

dp_window:
  DRAW_BACKGROUND 0x60

  SC 0, 0
  DRAW_LINE 79, 0x40
  DRAW_COLUMN 24, 0x40
  SC 24, 0
  DRAW_LINE 80, 0x40
  SC 0, 79
  DRAW_COLUMN 24, 0x40

  SC 2, 2
  PRINT dp_str1
  SC 4, 2
  PRINT dp_str2
  SC 6, 2
  PRINT dp_str3

  .halt:
  call wait_for_key

  cmp ah, KEY_ESC
  je main_window

  jmp .halt

  dp_str1 db 'Developer zone', 0
  dp_str2 db '[a] VGA', 0
  dp_str3 db '[b] VESA', 0
