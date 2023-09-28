;BleskOS

;;;;;
;; MIT License
;; Copyright (c) 2023 Vendelín Slezák
;; Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
;; The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
;; THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
;;;;;

%define SF_CHAR_END 0
%define SF_CHAR_LINE 1
%define SF_CHAR_BEZIER 2
%define SF_CHAR_FULL_ELLIPSE 3
%define SF_CHAR_EMPTY_ELLIPSE 4
%define SF_CHAR_JUMP 5

global bleskos_scalable_font

bleskos_scalable_font:
 times 0x20 dd bleskos_scalable_font_char_square
 dd 0 ;space
 dd bleskos_scalable_font_char_0x21_exclamation ;!
 dd bleskos_scalable_font_char_0x22_quotation_mark ;"
 dd bleskos_scalable_font_char_0x23_number_sign ;#
 dd bleskos_scalable_font_char_0x24_dollar_sign ;$
 dd bleskos_scalable_font_char_0x25_percent_sign ;%
 dd bleskos_scalable_font_char_0x26_ampersand ;&
 dd bleskos_scalable_font_char_0x27_apostrophe ;'
 dd bleskos_scalable_font_char_0x28_left_parenthesis ;(
 dd bleskos_scalable_font_char_0x29_right_parenthesis ;)
 dd bleskos_scalable_font_char_0x2A_asterisk ;*
 dd bleskos_scalable_font_char_0x2B_plus ;+
 dd bleskos_scalable_font_char_0x2C_comma ;,
 dd bleskos_scalable_font_char_0x2D_hyphen_minus ;-
 dd bleskos_scalable_font_char_0x2E_dot ;.
 dd bleskos_scalable_font_char_0x2F_solidus ;/
 dd bleskos_scalable_font_char_0x30_0
 dd bleskos_scalable_font_char_0x31_1
 dd bleskos_scalable_font_char_0x32_2
 dd bleskos_scalable_font_char_0x33_3
 dd bleskos_scalable_font_char_0x34_4
 dd bleskos_scalable_font_char_0x35_5
 dd bleskos_scalable_font_char_0x36_6
 dd bleskos_scalable_font_char_0x37_7
 dd bleskos_scalable_font_char_0x38_8
 dd bleskos_scalable_font_char_0x39_9
 dd bleskos_scalable_font_char_0x3A_colon ;:
 dd bleskos_scalable_font_char_0x3B_semicolon ;;
 dd bleskos_scalable_font_char_0x3C_less_than_sign ;<
 dd bleskos_scalable_font_char_0x3D_equal_sign ;=
 dd bleskos_scalable_font_char_0x3E_more_than_sign ;>
 dd bleskos_scalable_font_char_0x3F_question_mark ;?
 dd bleskos_scalable_font_char_0x40_commerical_at ;@
 dd bleskos_scalable_font_char_0x41_A
 dd bleskos_scalable_font_char_0x42_B
 dd bleskos_scalable_font_char_0x43_C
 dd bleskos_scalable_font_char_0x44_D
 dd bleskos_scalable_font_char_0x45_E
 dd bleskos_scalable_font_char_0x46_F
 dd bleskos_scalable_font_char_0x47_G
 dd bleskos_scalable_font_char_0x48_H
 dd bleskos_scalable_font_char_0x49_I
 dd bleskos_scalable_font_char_0x4A_J
 dd bleskos_scalable_font_char_0x4B_K
 dd bleskos_scalable_font_char_0x4C_L
 dd bleskos_scalable_font_char_0x4D_M
 dd bleskos_scalable_font_char_0x4E_N
 dd bleskos_scalable_font_char_0x4F_O
 dd bleskos_scalable_font_char_0x50_P
 dd bleskos_scalable_font_char_0x51_Q
 dd bleskos_scalable_font_char_0x52_R
 dd bleskos_scalable_font_char_0x53_S
 dd bleskos_scalable_font_char_0x54_T
 dd bleskos_scalable_font_char_0x55_U
 dd bleskos_scalable_font_char_0x56_V
 dd bleskos_scalable_font_char_0x57_W
 dd bleskos_scalable_font_char_0x58_X
 dd bleskos_scalable_font_char_0x59_Y
 dd bleskos_scalable_font_char_0x5A_Z
 dd bleskos_scalable_font_char_0x5B_left_bracket ;[
 dd bleskos_scalable_font_char_0x5C_backslash ;\ backslash
 dd bleskos_scalable_font_char_0x5D_right_bracket ;]
 dd bleskos_scalable_font_char_0x5E_caret ;^
 dd bleskos_scalable_font_char_0x5F_underscore ;_
 dd bleskos_scalable_font_char_0x60_backtick ;`
 dd bleskos_scalable_font_char_0x61_a
 dd bleskos_scalable_font_char_0x62_b
 dd bleskos_scalable_font_char_0x63_c
 dd bleskos_scalable_font_char_0x64_d
 dd bleskos_scalable_font_char_0x65_e
 dd bleskos_scalable_font_char_0x66_f
 dd bleskos_scalable_font_char_0x67_g
 dd bleskos_scalable_font_char_0x68_h
 dd bleskos_scalable_font_char_0x69_i
 dd bleskos_scalable_font_char_0x6A_j
 dd bleskos_scalable_font_char_0x6B_k
 dd bleskos_scalable_font_char_0x6C_l
 dd bleskos_scalable_font_char_0x6D_m
 dd bleskos_scalable_font_char_0x6E_n
 dd bleskos_scalable_font_char_0x6F_o
 dd bleskos_scalable_font_char_0x70_p
 dd bleskos_scalable_font_char_0x71_q
 dd bleskos_scalable_font_char_0x72_r
 dd bleskos_scalable_font_char_0x73_s
 dd bleskos_scalable_font_char_0x74_t
 dd bleskos_scalable_font_char_0x75_u
 dd bleskos_scalable_font_char_0x76_v
 dd bleskos_scalable_font_char_0x77_w
 dd bleskos_scalable_font_char_0x78_x
 dd bleskos_scalable_font_char_0x79_y
 dd bleskos_scalable_font_char_0x7A_z
 dd bleskos_scalable_font_char_0x7B_left_curly_bracket ;{
 dd bleskos_scalable_font_char_0x7C_vertical_bar ;|
 dd bleskos_scalable_font_char_0x7D_right_curly_bracket ;}
 dd bleskos_scalable_font_char_0x7E_tilde ;~
 dd bleskos_scalable_font_char_circle
 times 128 dd bleskos_scalable_font_char_square
 dd bleskos_scalable_font_char_0x100_A_with_macron
 dd bleskos_scalable_font_char_0x101_a_with_macron
 dd bleskos_scalable_font_char_0x102_A_with_breve
 dd bleskos_scalable_font_char_0x103_a_with_breve
 dd bleskos_scalable_font_char_0x104_A_with_ogonek
 dd bleskos_scalable_font_char_0x105_a_with_ogonek
 dd bleskos_scalable_font_char_0x106_C_with_acute
 dd bleskos_scalable_font_char_0x107_c_with_acute
 dd bleskos_scalable_font_char_0x108_C_with_circumflex
 dd bleskos_scalable_font_char_0x109_c_with_circumflex
 dd bleskos_scalable_font_char_0x10A_C_with_dot_above
 dd bleskos_scalable_font_char_0x10B_c_with_dot_above
 dd bleskos_scalable_font_char_0x10C_C_with_caron
 dd bleskos_scalable_font_char_0x10D_c_with_caron
 dd bleskos_scalable_font_char_0x10E_D_with_caron
 dd bleskos_scalable_font_char_0x10F_d_with_caron
 dd bleskos_scalable_font_char_0x110_D_with_stroke
 dd bleskos_scalable_font_char_0x111_d_with_stroke
 dd bleskos_scalable_font_char_0x112_E_with_macron
 dd bleskos_scalable_font_char_0x113_e_with_macron
 dd bleskos_scalable_font_char_0x114_E_with_breve
 dd bleskos_scalable_font_char_0x115_e_with_breve
 dd bleskos_scalable_font_char_0x116_E_with_dot_above
 dd bleskos_scalable_font_char_0x117_e_with_dot_above
 dd bleskos_scalable_font_char_0x118_E_with_ogonek
 dd bleskos_scalable_font_char_0x119_e_with_ogonek
 dd bleskos_scalable_font_char_0x11A_E_with_caron
 dd bleskos_scalable_font_char_0x11B_e_with_caron
 times 256 dd bleskos_scalable_font_char_square
 
bleskos_scalable_font_char_0x21_exclamation:
 db SF_CHAR_LINE, 50, 0, 50, 70
 db SF_CHAR_LINE, 50, 90, 50, 100

 db SF_CHAR_END
 
bleskos_scalable_font_char_0x22_quotation_mark:
 db SF_CHAR_LINE, 40, 0, 40, 30
 db SF_CHAR_LINE, 60, 0, 60, 30
 
 db SF_CHAR_END
 
bleskos_scalable_font_char_0x23_number_sign:
 db SF_CHAR_LINE, 20, 100, 50, 0
 db SF_CHAR_LINE, 50, 100, 80, 0
 db SF_CHAR_LINE, 20, 35, 90, 35
 db SF_CHAR_LINE, 10, 70, 80, 70
 
 db SF_CHAR_END
 
bleskos_scalable_font_char_0x24_dollar_sign:
 db SF_CHAR_LINE, 50, 0, 50, 100
 
 db SF_CHAR_BEZIER, 80, 20, 80, 10, 50, 10
 db SF_CHAR_BEZIER, 50, 10, 20, 10, 20, 25
 db SF_CHAR_BEZIER, 20, 25, 20, 50, 50, 50
 db SF_CHAR_BEZIER, 50, 50, 80, 50, 80, 75
 db SF_CHAR_BEZIER, 80, 75, 80, 90, 50, 90
 db SF_CHAR_BEZIER, 50, 90, 20, 90, 20, 80
 
 db SF_CHAR_END
 
bleskos_scalable_font_char_0x25_percent_sign:
 db SF_CHAR_LINE, 20, 100, 80, 0
 
 db SF_CHAR_EMPTY_ELLIPSE, 25, 15, 20, 20
 db SF_CHAR_EMPTY_ELLIPSE, 65, 65, 20, 20
 
 db SF_CHAR_END
 
bleskos_scalable_font_char_0x26_ampersand:
 db SF_CHAR_BEZIER, 30, 25, 30, 0, 50, 0
 db SF_CHAR_BEZIER, 50, 0, 70, 0, 70, 25
 db SF_CHAR_BEZIER, 70, 25, 70, 50, 50, 50
 db SF_CHAR_BEZIER, 50, 50, 30, 50, 30, 25
 
 db SF_CHAR_BEZIER, 20, 75, 20, 50, 50, 50
 db SF_CHAR_LINE, 50, 50, 80, 100
 db SF_CHAR_BEZIER, 50, 100, 20, 100, 20, 75
 db SF_CHAR_BEZIER, 50, 100, 60, 100, 80, 70
 
 db SF_CHAR_END
 
bleskos_scalable_font_char_0x27_apostrophe:
 db SF_CHAR_LINE, 50, 0, 50, 30
 
 db SF_CHAR_END
 
bleskos_scalable_font_char_0x28_left_parenthesis:
 db SF_CHAR_BEZIER, 65, 0, 30, 0, 30, 50
 db SF_CHAR_BEZIER, 30, 50, 30, 100, 65, 100
 
 db SF_CHAR_END
 
bleskos_scalable_font_char_0x29_right_parenthesis:
 db SF_CHAR_BEZIER, 35, 0, 70, 0, 70, 50
 db SF_CHAR_BEZIER, 70, 50, 70, 100, 35, 100
 
 db SF_CHAR_END
 
bleskos_scalable_font_char_0x2A_asterisk:
 db SF_CHAR_LINE, 50, 0, 50, 40
 db SF_CHAR_LINE, 30, 0, 70, 40
 db SF_CHAR_LINE, 30, 40, 70, 0
 
 db SF_CHAR_END
 
bleskos_scalable_font_char_0x2B_plus:
 db SF_CHAR_LINE, 50, 20, 50, 80
 db SF_CHAR_LINE, 20, 50, 80, 50
 
 db SF_CHAR_END
 
bleskos_scalable_font_char_0x2C_comma:
 db SF_CHAR_LINE, 60, 70, 40, 100
 
 db SF_CHAR_END
 
bleskos_scalable_font_char_0x2D_hyphen_minus:
 db SF_CHAR_LINE, 25, 50, 75, 50

 db SF_CHAR_END
 
bleskos_scalable_font_char_0x2E_dot:
 db SF_CHAR_FULL_ELLIPSE, 45, 90, 10, 10
 
 db SF_CHAR_END 
 
bleskos_scalable_font_char_0x2F_solidus:
 db SF_CHAR_LINE, 20, 100, 80, 0
 
 db SF_CHAR_END
 
bleskos_scalable_font_char_0x30_0:
 db SF_CHAR_BEZIER, 25, 50, 25, 0, 50, 0
 db SF_CHAR_BEZIER, 50, 0, 75, 0, 75, 50
 db SF_CHAR_BEZIER, 75, 50, 75, 100, 50, 100
 db SF_CHAR_BEZIER, 50, 100, 25, 100, 25, 50
 
 db SF_CHAR_END
 
bleskos_scalable_font_char_0x31_1:
 db SF_CHAR_LINE, 60, 0, 60, 100
 db SF_CHAR_LINE, 20, 40, 60, 0
 
 db SF_CHAR_END
 
bleskos_scalable_font_char_0x32_2:
 db SF_CHAR_BEZIER, 20, 40, 20, 0, 50, 0
 db SF_CHAR_BEZIER, 50, 0, 80, 0, 80, 40
 db SF_CHAR_LINE, 80, 40, 20, 100
 db SF_CHAR_LINE, 20, 100, 80, 100
 
 db SF_CHAR_END
 
bleskos_scalable_font_char_0x33_3:
 db SF_CHAR_BEZIER, 20, 20, 20, 0, 50, 0
 db SF_CHAR_BEZIER, 50, 0, 80, 0, 80, 30
 db SF_CHAR_BEZIER, 80, 30, 80, 50, 40, 50
 db SF_CHAR_BEZIER, 40, 50, 80, 50, 80, 70
 db SF_CHAR_BEZIER, 80, 70, 80, 100, 50, 100
 db SF_CHAR_BEZIER, 50, 100, 20, 100, 20, 80
 
 db SF_CHAR_END
 
bleskos_scalable_font_char_0x34_4:
 db SF_CHAR_LINE, 60, 0, 60, 100
 db SF_CHAR_LINE, 20, 70, 80, 70
 db SF_CHAR_LINE, 20, 70, 60, 0
 
 db SF_CHAR_END
 
bleskos_scalable_font_char_0x35_5:
 db SF_CHAR_LINE, 80, 0, 20, 0
 db SF_CHAR_LINE, 20, 0, 20, 50
 db SF_CHAR_BEZIER, 20, 50, 20, 40, 50, 40
 db SF_CHAR_BEZIER, 50, 40, 80, 40, 80, 70
 db SF_CHAR_BEZIER, 80, 70, 80, 100, 50, 100
 db SF_CHAR_BEZIER, 50, 100, 20, 100, 20, 80
 
 db SF_CHAR_END
 
bleskos_scalable_font_char_0x36_6:
 db SF_CHAR_BEZIER, 20, 50, 20, 0, 50, 0
 db SF_CHAR_BEZIER, 50, 0, 80, 0, 80, 20
 db SF_CHAR_BEZIER, 80, 70, 80, 100, 50, 100
 db SF_CHAR_BEZIER, 50, 100, 20, 100, 20, 50
 db SF_CHAR_BEZIER, 20, 60, 20, 50, 50, 50
 db SF_CHAR_BEZIER, 50, 50, 80, 50, 80, 70
 
 db SF_CHAR_END
 
bleskos_scalable_font_char_0x37_7:
 db SF_CHAR_LINE, 20, 0, 80, 0
 db SF_CHAR_LINE, 80, 0, 40, 100
  
 db SF_CHAR_END
 
bleskos_scalable_font_char_0x38_8:
 db SF_CHAR_BEZIER, 20, 25, 20, 0, 50, 0
 db SF_CHAR_BEZIER, 50, 0, 80, 0, 80, 25
 db SF_CHAR_BEZIER, 80, 25, 80, 50, 50, 50
 db SF_CHAR_BEZIER, 50, 50, 20, 50, 20, 25
 
 db SF_CHAR_BEZIER, 20, 75, 20, 50, 50, 50
 db SF_CHAR_BEZIER, 50, 50, 80, 50, 80, 75
 db SF_CHAR_BEZIER, 80, 75, 80, 100, 50, 100
 db SF_CHAR_BEZIER, 50, 100, 20, 100, 20, 75
 
 db SF_CHAR_END
 
bleskos_scalable_font_char_0x39_9:
 db SF_CHAR_BEZIER, 80, 50, 80, 100, 50, 100
 db SF_CHAR_BEZIER, 50, 100, 20, 100, 20, 80
 db SF_CHAR_BEZIER, 20, 30, 20, 0, 50, 0
 db SF_CHAR_BEZIER, 50, 0, 80, 0, 80, 50
 db SF_CHAR_BEZIER, 80, 40, 80, 60, 50, 60
 db SF_CHAR_BEZIER, 50, 60, 20, 60, 20, 30
 
 db SF_CHAR_END
 
bleskos_scalable_font_char_0x3A_colon:
 db SF_CHAR_FULL_ELLIPSE, 45, 30, 10, 10
 db SF_CHAR_FULL_ELLIPSE, 45, 70, 10, 10
 
 db SF_CHAR_END 
 
bleskos_scalable_font_char_0x3B_semicolon:
 db SF_CHAR_FULL_ELLIPSE, 45, 30, 10, 10
 db SF_CHAR_LINE, 50, 70, 40, 90
 
 db SF_CHAR_END
 
bleskos_scalable_font_char_0x3C_less_than_sign:
 db SF_CHAR_LINE, 75, 25, 25, 50
 db SF_CHAR_LINE, 25, 50, 75, 75
 
 db SF_CHAR_END
 
bleskos_scalable_font_char_0x3D_equal_sign:
 db SF_CHAR_LINE, 25, 25, 75, 25
 db SF_CHAR_LINE, 25, 75, 75, 75
 
 db SF_CHAR_END
 
bleskos_scalable_font_char_0x3E_more_than_sign:
 db SF_CHAR_LINE, 25, 25, 75, 50
 db SF_CHAR_LINE, 75, 50, 25, 75
 
 db SF_CHAR_END
 
bleskos_scalable_font_char_0x3F_question_mark:
 db SF_CHAR_FULL_ELLIPSE, 50, 95, 5, 5
 db SF_CHAR_LINE, 50, 70, 50, 60
 db SF_CHAR_BEZIER, 50, 60, 50, 40, 65, 40
 db SF_CHAR_BEZIER, 65, 40, 80, 40, 80, 20
 db SF_CHAR_BEZIER, 80, 20, 80, 0, 50, 0
 db SF_CHAR_BEZIER, 50, 0, 20, 0, 20, 30
 
 db SF_CHAR_END
 
bleskos_scalable_font_char_0x40_commerical_at:
 db SF_CHAR_LINE, 80, 70, 80, 20
 db SF_CHAR_BEZIER, 80, 20, 80, 0, 50, 0
 db SF_CHAR_BEZIER, 50, 0, 20, 0, 20, 50
 db SF_CHAR_BEZIER, 20, 50, 20, 100, 50, 100
 db SF_CHAR_BEZIER, 50, 100, 80, 100, 80, 95 
 
 db SF_CHAR_BEZIER, 80, 40, 80, 30, 50, 30
 db SF_CHAR_BEZIER, 50, 30, 40, 30, 40, 50
 db SF_CHAR_BEZIER, 40, 50, 40, 80, 50, 80
 db SF_CHAR_BEZIER, 50, 80, 80, 80, 80, 70
 
 db SF_CHAR_END
 
bleskos_scalable_font_char_0x41_A:
 db SF_CHAR_LINE, 10, 100, 50, 0
 db SF_CHAR_LINE, 50, 0, 90, 100
 db SF_CHAR_LINE, 30, 50, 70, 50
 
 db SF_CHAR_END
 
bleskos_scalable_font_char_0x42_B:
 db SF_CHAR_LINE, 20, 0, 20, 100
 db SF_CHAR_LINE, 20, 0, 50, 0
 db SF_CHAR_BEZIER, 50, 0, 80, 0, 80, 25
 db SF_CHAR_BEZIER, 80, 25, 80, 50, 50, 50
 db SF_CHAR_LINE, 20, 50, 50, 50
 db SF_CHAR_BEZIER, 50, 50, 80, 50, 80, 75
 db SF_CHAR_BEZIER, 80, 75, 80, 100, 50, 100
 db SF_CHAR_LINE, 20, 100, 50, 100
 
 db SF_CHAR_END
 
bleskos_scalable_font_char_0x43_C:
 db SF_CHAR_BEZIER, 80, 20, 80, 0, 50, 0
 db SF_CHAR_BEZIER, 50, 0, 20, 0, 20, 50
 db SF_CHAR_BEZIER, 20, 50, 20, 100, 50, 100
 db SF_CHAR_BEZIER, 50, 100, 80, 100, 80, 80
  
 db SF_CHAR_END
 
bleskos_scalable_font_char_0x44_D:
 db SF_CHAR_LINE, 20, 0, 20, 100
 db SF_CHAR_LINE, 20, 0, 50, 0
 db SF_CHAR_BEZIER, 50, 0, 80, 0, 80, 50
 db SF_CHAR_BEZIER, 80, 50, 80, 100, 50, 100
 db SF_CHAR_LINE, 20, 100, 50, 100
 
 db SF_CHAR_END
 
bleskos_scalable_font_char_0x45_E:
 db SF_CHAR_LINE, 20, 0, 20, 100
 db SF_CHAR_LINE, 20, 0, 80, 0
 db SF_CHAR_LINE, 20, 50, 80, 50
 db SF_CHAR_LINE, 20, 100, 80, 100
 
 db SF_CHAR_END
 
bleskos_scalable_font_char_0x46_F:
 db SF_CHAR_LINE, 20, 0, 20, 100
 db SF_CHAR_LINE, 20, 0, 80, 0
 db SF_CHAR_LINE, 20, 50, 80, 50
 
 db SF_CHAR_END
 
bleskos_scalable_font_char_0x47_G:
 db SF_CHAR_BEZIER, 20, 50, 20, 0, 50, 0
 db SF_CHAR_BEZIER, 50, 0, 80, 0, 80, 20
 db SF_CHAR_BEZIER, 80, 50, 80, 100, 50, 100
 db SF_CHAR_BEZIER, 50, 100, 20, 100, 20, 50
 db SF_CHAR_LINE, 80, 50, 50, 50
 
 db SF_CHAR_END
 
bleskos_scalable_font_char_0x48_H:
 db SF_CHAR_LINE, 20, 0, 20, 100
 db SF_CHAR_LINE, 20, 50, 80, 50
 db SF_CHAR_LINE, 80, 0, 80, 100
 
 db SF_CHAR_END
 
bleskos_scalable_font_char_0x49_I:
 db SF_CHAR_LINE, 50, 0, 50, 100
 db SF_CHAR_LINE, 30, 0, 70, 0
 db SF_CHAR_LINE, 30, 100, 70, 100

 db SF_CHAR_END
 
bleskos_scalable_font_char_0x4A_J:
 db SF_CHAR_LINE, 80, 0, 80, 60
 db SF_CHAR_BEZIER, 80, 60, 80, 100, 50, 100
 db SF_CHAR_BEZIER, 50, 100, 20, 100, 20, 60
 
 db SF_CHAR_END
 
bleskos_scalable_font_char_0x4B_K:
 db SF_CHAR_LINE, 20, 0, 20, 100
 db SF_CHAR_LINE, 20, 50, 80, 0
 db SF_CHAR_LINE, 20, 50, 80, 100
 
 db SF_CHAR_END
 
bleskos_scalable_font_char_0x4C_L:
 db SF_CHAR_LINE, 20, 0, 20, 100
 db SF_CHAR_LINE, 20, 100, 80, 100
 
 db SF_CHAR_END
 
bleskos_scalable_font_char_0x4D_M:
 db SF_CHAR_LINE, 20, 0, 20, 100
 db SF_CHAR_LINE, 20, 0, 50, 50
 db SF_CHAR_LINE, 50, 50, 80, 0
 db SF_CHAR_LINE, 80, 0, 80, 100
 
 db SF_CHAR_END
 
bleskos_scalable_font_char_0x4E_N:
 db SF_CHAR_LINE, 20, 0, 20, 100
 db SF_CHAR_LINE, 20, 0, 80, 100
 db SF_CHAR_LINE, 80, 100, 80, 0
 
 db SF_CHAR_END
 
bleskos_scalable_font_char_0x4F_O:
 db SF_CHAR_BEZIER, 20, 50, 20, 0, 50, 0
 db SF_CHAR_BEZIER, 50, 0, 80, 0, 80, 50
 db SF_CHAR_BEZIER, 80, 50, 80, 100, 50, 100
 db SF_CHAR_BEZIER, 50, 100, 20, 100, 20, 50
 
 db SF_CHAR_END
 
bleskos_scalable_font_char_0x50_P:
 db SF_CHAR_LINE, 20, 0, 20, 100
 db SF_CHAR_LINE, 20, 0, 50, 0
 db SF_CHAR_BEZIER, 50, 0, 80, 0, 80, 25
 db SF_CHAR_BEZIER, 80, 25, 80, 50, 50, 50
 db SF_CHAR_LINE, 20, 50, 50, 50
 
 db SF_CHAR_END
 
bleskos_scalable_font_char_0x51_Q:
 db SF_CHAR_BEZIER, 20, 50, 20, 0, 50, 0
 db SF_CHAR_BEZIER, 50, 0, 80, 0, 80, 50
 db SF_CHAR_BEZIER, 80, 50, 80, 100, 50, 100
 db SF_CHAR_BEZIER, 50, 100, 20, 100, 20, 50
 db SF_CHAR_LINE, 60, 70, 80, 100
 
 db SF_CHAR_END
 
bleskos_scalable_font_char_0x52_R:
 db SF_CHAR_LINE, 20, 0, 20, 100
 db SF_CHAR_LINE, 20, 0, 50, 0
 db SF_CHAR_BEZIER, 50, 0, 80, 0, 80, 25
 db SF_CHAR_BEZIER, 80, 25, 80, 50, 50, 50
 db SF_CHAR_LINE, 20, 50, 50, 50
 db SF_CHAR_LINE, 50, 50, 80, 100
 
 db SF_CHAR_END
 
bleskos_scalable_font_char_0x53_S:
 db SF_CHAR_BEZIER, 80, 15, 80, 0, 50, 0
 db SF_CHAR_BEZIER, 50, 0, 20, 0, 20, 25
 db SF_CHAR_BEZIER, 20, 25, 20, 50, 50, 50
 db SF_CHAR_BEZIER, 50, 50, 80, 50, 80, 75
 db SF_CHAR_BEZIER, 80, 75, 80, 100, 50, 100
 db SF_CHAR_BEZIER, 50, 100, 20, 100, 20, 90
 
 db SF_CHAR_END
 
bleskos_scalable_font_char_0x54_T:
 db SF_CHAR_LINE, 50, 0, 50, 100
 db SF_CHAR_LINE, 20, 0, 80, 0
 
 db SF_CHAR_END
 
bleskos_scalable_font_char_0x55_U:
 db SF_CHAR_LINE, 20, 0, 20, 60
 db SF_CHAR_LINE, 80, 0, 80, 60
 db SF_CHAR_BEZIER, 80, 60, 80, 100, 50, 100
 db SF_CHAR_BEZIER, 50, 100, 20, 100, 20, 60
 
 db SF_CHAR_END
 
bleskos_scalable_font_char_0x56_V:
 db SF_CHAR_LINE, 20, 0, 50, 100
 db SF_CHAR_LINE, 50, 100, 80, 0
 
 db SF_CHAR_END
 
bleskos_scalable_font_char_0x57_W:
 db SF_CHAR_LINE, 0, 0, 25, 100
 db SF_CHAR_LINE, 25, 100, 50, 0
 db SF_CHAR_LINE, 50, 0, 75, 100
 db SF_CHAR_LINE, 75, 100, 100, 0
 
 db SF_CHAR_END
 
bleskos_scalable_font_char_0x58_X:
 db SF_CHAR_LINE, 20, 0, 80, 100
 db SF_CHAR_LINE, 20, 100, 80, 0
 
 db SF_CHAR_END
 
bleskos_scalable_font_char_0x59_Y:
 db SF_CHAR_LINE, 20, 0, 50, 50
 db SF_CHAR_LINE, 50, 50, 80, 0
 db SF_CHAR_LINE, 50, 50, 50, 100
 
 db SF_CHAR_END
 
bleskos_scalable_font_char_0x5A_Z:
 db SF_CHAR_LINE, 20, 0, 80, 0
 db SF_CHAR_LINE, 80, 0, 20, 100
 db SF_CHAR_LINE, 20, 100, 80, 100
 
 db SF_CHAR_END
 
bleskos_scalable_font_char_0x5B_left_bracket:
 db SF_CHAR_LINE, 60, 0, 40, 0
 db SF_CHAR_LINE, 40, 0, 40, 100
 db SF_CHAR_LINE, 40, 100, 60, 100
 
 db SF_CHAR_END
 
bleskos_scalable_font_char_0x5C_backslash:
 db SF_CHAR_LINE, 20, 0, 80, 100
 
 db SF_CHAR_END
 
bleskos_scalable_font_char_0x5D_right_bracket:
 db SF_CHAR_LINE, 40, 0, 60, 0
 db SF_CHAR_LINE, 60, 0, 60, 100
 db SF_CHAR_LINE, 60, 100, 40, 100
 
 db SF_CHAR_END
 
bleskos_scalable_font_char_0x5E_caret:
 db SF_CHAR_LINE, 30, 20, 50, 0
 db SF_CHAR_LINE, 50, 0, 70, 20

 db SF_CHAR_END
 
bleskos_scalable_font_char_0x5F_underscore:
 db SF_CHAR_LINE, 20, 100, 80, 100
 
 db SF_CHAR_END
 
bleskos_scalable_font_char_0x60_backtick:
 db SF_CHAR_LINE, 40, 0, 60, 20
 
 db SF_CHAR_END
 
bleskos_scalable_font_char_0x61_a:
 db SF_CHAR_LINE, 80, 60, 80, 100
 db SF_CHAR_BEZIER, 80, 60, 80, 40, 50, 40
 db SF_CHAR_BEZIER, 50, 40, 20, 40, 20, 50
 db SF_CHAR_LINE, 80, 70, 40, 70
 db SF_CHAR_BEZIER, 40, 70, 20, 70, 20, 85
 db SF_CHAR_BEZIER, 20, 85, 20, 100, 40, 100
 db SF_CHAR_LINE, 40, 100, 60, 100
 db SF_CHAR_BEZIER, 60, 100, 80, 100, 80, 90
 
 db SF_CHAR_END

bleskos_scalable_font_char_0x62_b:
 db SF_CHAR_LINE, 20, 0, 20, 100
 
 db SF_CHAR_BEZIER, 20, 60, 20, 50, 50, 50
 db SF_CHAR_BEZIER, 50, 50, 80, 50, 80, 75
 db SF_CHAR_BEZIER, 80, 75, 80, 100, 50, 100
 db SF_CHAR_BEZIER, 50, 100, 20, 100, 20, 80
 
 db SF_CHAR_END

bleskos_scalable_font_char_0x63_c:
 db SF_CHAR_BEZIER, 80, 50, 80, 40, 50, 40
 db SF_CHAR_BEZIER, 50, 40, 20, 40, 20, 70
 db SF_CHAR_BEZIER, 20, 70, 20, 100, 50, 100
 db SF_CHAR_BEZIER, 50, 100, 80, 100, 80, 90
 
 db SF_CHAR_END

bleskos_scalable_font_char_0x64_d:
 db SF_CHAR_LINE, 80, 0, 80, 100
 
 db SF_CHAR_BEZIER, 80, 60, 80, 50, 50, 50
 db SF_CHAR_BEZIER, 50, 50, 20, 50, 20, 75
 db SF_CHAR_BEZIER, 20, 75, 20, 100, 50, 100
 db SF_CHAR_BEZIER, 50, 100, 80, 100, 80, 80
 
 db SF_CHAR_END
 
bleskos_scalable_font_char_0x65_e:
 db SF_CHAR_LINE, 20, 70, 80, 70
 
 db SF_CHAR_BEZIER, 80, 70, 80, 40, 50, 40
 db SF_CHAR_BEZIER, 50, 40, 20, 40, 20, 70
 db SF_CHAR_BEZIER, 20, 70, 20, 100, 50, 100
 db SF_CHAR_BEZIER, 50, 100, 80, 100, 80, 90
 
 db SF_CHAR_END

bleskos_scalable_font_char_0x66_f:
 db SF_CHAR_LINE, 50, 100, 50, 50
 db SF_CHAR_LINE, 20, 60, 80, 60
 db SF_CHAR_BEZIER, 50, 50, 50, 20, 80, 20
 
 db SF_CHAR_END

bleskos_scalable_font_char_0x67_g:
 db SF_CHAR_LINE, 80, 50, 80, 120
 
 db SF_CHAR_BEZIER, 80, 50, 80, 40, 50, 40
 db SF_CHAR_BEZIER, 50, 40, 20, 40, 20, 65
 db SF_CHAR_BEZIER, 20, 65, 20, 90, 50, 90
 db SF_CHAR_BEZIER, 50, 90, 80, 90, 80, 70
 
 db SF_CHAR_BEZIER, 80, 120, 80, 130, 50, 130
 db SF_CHAR_BEZIER, 50, 130, 20, 130, 20, 120

 db SF_CHAR_END

bleskos_scalable_font_char_0x68_h:
 db SF_CHAR_LINE, 20, 0, 20, 100
 db SF_CHAR_BEZIER, 20, 60, 20, 50, 50, 50
 db SF_CHAR_BEZIER, 50, 50, 80, 50, 80, 60
 db SF_CHAR_LINE, 80, 60, 80, 100
 
 db SF_CHAR_END

bleskos_scalable_font_char_0x69_i:
 db SF_CHAR_LINE, 50, 50, 50, 100
 db SF_CHAR_LINE, 30, 50, 50, 50
 db SF_CHAR_LINE, 30, 100, 70, 100
 db SF_CHAR_LINE, 50, 30, 50, 35
 
 db SF_CHAR_END

bleskos_scalable_font_char_0x6A_j:
 db SF_CHAR_LINE, 50, 50, 50, 100
 db SF_CHAR_LINE, 30, 50, 50, 50
 db SF_CHAR_LINE, 50, 30, 50, 35
 db SF_CHAR_BEZIER, 50, 100, 50, 120, 30, 120
 db SF_CHAR_BEZIER, 30, 120, 0, 120, 0, 100
 
 db SF_CHAR_END

bleskos_scalable_font_char_0x6B_k:
 db SF_CHAR_LINE, 20, 0, 20, 100
 db SF_CHAR_LINE, 20, 70, 70, 30
 db SF_CHAR_LINE, 20, 70, 70, 100
 
 db SF_CHAR_END

bleskos_scalable_font_char_0x6C_l:
 db SF_CHAR_LINE, 50, 0, 50, 100
 db SF_CHAR_LINE, 30, 0, 50, 0
 db SF_CHAR_LINE, 30, 100, 70, 100
 
 db SF_CHAR_END

bleskos_scalable_font_char_0x6D_m:
 db SF_CHAR_LINE, 10, 40, 10, 100
 db SF_CHAR_BEZIER, 10, 60, 10, 40, 25, 40
 db SF_CHAR_BEZIER, 25, 40, 50, 40, 50, 60
 db SF_CHAR_LINE, 50, 60, 50, 100
 db SF_CHAR_BEZIER, 50, 60, 50, 40, 75, 40
 db SF_CHAR_BEZIER, 75, 40, 90, 40, 90, 60
 db SF_CHAR_LINE, 90, 60, 90, 100
 
 db SF_CHAR_END

bleskos_scalable_font_char_0x6E_n:
 db SF_CHAR_LINE, 20, 40, 20, 100
 db SF_CHAR_BEZIER, 20, 60, 20, 40, 50, 40
 db SF_CHAR_BEZIER, 50, 40, 80, 40, 80, 60
 db SF_CHAR_LINE, 80, 60, 80, 100
 
 db SF_CHAR_END

bleskos_scalable_font_char_0x6F_o:
 db SF_CHAR_BEZIER, 20, 70, 20, 40, 50, 40
 db SF_CHAR_BEZIER, 50, 40, 80, 40, 80, 70
 db SF_CHAR_BEZIER, 80, 70, 80, 100, 50, 100
 db SF_CHAR_BEZIER, 50, 100, 20, 100, 20, 70
 
 db SF_CHAR_END

bleskos_scalable_font_char_0x70_p:
 db SF_CHAR_LINE, 20, 40, 20, 130
 
 db SF_CHAR_BEZIER, 20, 50, 20, 40, 50, 40
 db SF_CHAR_BEZIER, 50, 40, 80, 40, 80, 70
 db SF_CHAR_BEZIER, 80, 70, 80, 100, 50, 100
 db SF_CHAR_BEZIER, 50, 100, 20, 100, 20, 70
 
 db SF_CHAR_END

bleskos_scalable_font_char_0x71_q:
 db SF_CHAR_LINE, 80, 40, 80, 130
 
 db SF_CHAR_BEZIER, 80, 50, 80, 40, 50, 40
 db SF_CHAR_BEZIER, 50, 40, 20, 40, 20, 70
 db SF_CHAR_BEZIER, 20, 70, 20, 100, 50, 100
 db SF_CHAR_BEZIER, 50, 100, 80, 100, 80, 70
 
 db SF_CHAR_END

bleskos_scalable_font_char_0x72_r:
 db SF_CHAR_LINE, 20, 40, 20, 100
 db SF_CHAR_BEZIER, 20, 70, 20, 40, 50, 40
 db SF_CHAR_BEZIER, 50, 40, 80, 40, 80, 50
 
 db SF_CHAR_END

bleskos_scalable_font_char_0x73_s:
 db SF_CHAR_BEZIER, 80, 50, 80, 40, 50, 40
 db SF_CHAR_BEZIER, 50, 40, 20, 40, 20, 55
 db SF_CHAR_BEZIER, 20, 55, 20, 70, 50, 70
 db SF_CHAR_BEZIER, 50, 70, 80, 70, 80, 85
 db SF_CHAR_BEZIER, 80, 85, 80, 100, 50, 100
 db SF_CHAR_BEZIER, 50, 100, 20, 100, 20, 90
 
 db SF_CHAR_END

bleskos_scalable_font_char_0x74_t:
 db SF_CHAR_LINE, 50, 25, 50, 100
 db SF_CHAR_LINE, 20, 50, 80, 50
 
 db SF_CHAR_END

bleskos_scalable_font_char_0x75_u:
 db SF_CHAR_LINE, 20, 40, 20, 80
 db SF_CHAR_BEZIER, 20, 80, 20, 100, 50, 100
 db SF_CHAR_BEZIER, 50, 100, 80, 100, 80, 80
 db SF_CHAR_LINE, 80, 40, 80, 100

 db SF_CHAR_END

bleskos_scalable_font_char_0x76_v:
 db SF_CHAR_LINE, 20, 40, 50, 100
 db SF_CHAR_LINE, 50, 100, 80, 40
 
 db SF_CHAR_END

bleskos_scalable_font_char_0x77_w:
 db SF_CHAR_LINE, 0, 40, 25, 100
 db SF_CHAR_LINE, 25, 100, 50, 40
 db SF_CHAR_LINE, 50, 40, 75, 100
 db SF_CHAR_LINE, 75, 100, 100, 40
 
 db SF_CHAR_END

bleskos_scalable_font_char_0x78_x:
 db SF_CHAR_LINE, 20, 40, 80, 100
 db SF_CHAR_LINE, 20, 100, 80, 40
 
 db SF_CHAR_END

bleskos_scalable_font_char_0x79_y:
 db SF_CHAR_LINE, 20, 40, 50, 100
 db SF_CHAR_LINE, 40, 140, 80, 40
 
 db SF_CHAR_END

bleskos_scalable_font_char_0x7A_z:
 db SF_CHAR_LINE, 20, 40, 80, 40
 db SF_CHAR_LINE, 80, 40, 20, 100
 db SF_CHAR_LINE, 20, 100, 80, 100
 
 db SF_CHAR_END

bleskos_scalable_font_char_square:
 db SF_CHAR_LINE, 20, 20, 80, 20
 db SF_CHAR_LINE, 80, 20, 80, 80
 db SF_CHAR_LINE, 80, 80, 20, 80
 db SF_CHAR_LINE, 20, 20, 20, 80
 
 db SF_CHAR_END

bleskos_scalable_font_char_0x7B_left_curly_bracket:
 db SF_CHAR_BEZIER, 70, 0, 50, 0, 50, 25
 db SF_CHAR_BEZIER, 50, 25, 50, 50, 40, 50
 db SF_CHAR_BEZIER, 40, 50, 50, 50, 50, 75
 db SF_CHAR_BEZIER, 50, 75, 50, 100, 70, 100
 
 db SF_CHAR_END

bleskos_scalable_font_char_0x7C_vertical_bar:
 db SF_CHAR_LINE, 50, 0, 50, 100
 
 db SF_CHAR_END
 
bleskos_scalable_font_char_0x7D_right_curly_bracket:
 db SF_CHAR_BEZIER, 30, 0, 50, 0, 50, 25
 db SF_CHAR_BEZIER, 50, 25, 50, 50, 60, 50
 db SF_CHAR_BEZIER, 60, 50, 50, 50, 50, 75
 db SF_CHAR_BEZIER, 50, 75, 50, 100, 30, 100
 
 db SF_CHAR_END

bleskos_scalable_font_char_0x7E_tilde:
 db SF_CHAR_BEZIER, 20, 60, 20, 40, 35, 40
 db SF_CHAR_BEZIER, 35, 40, 50, 40, 50, 50
 db SF_CHAR_BEZIER, 50, 50, 50, 60, 65, 60
 db SF_CHAR_BEZIER, 65, 60, 80, 60, 80, 40
 
 db SF_CHAR_END

bleskos_scalable_font_char_circle:
 db SF_CHAR_FULL_ELLIPSE, 25, 25, 50, 50
 
 db SF_CHAR_END

bleskos_scalable_font_char_A_with_diacritic:
 db SF_CHAR_LINE, 10, 100, 50, 20
 db SF_CHAR_LINE, 50, 20, 90, 100
 db SF_CHAR_LINE, 30, 60, 70, 60
 
 db SF_CHAR_END
 
bleskos_scalable_font_char_C_with_diacritic:
 db SF_CHAR_BEZIER, 80, 40, 80, 20, 50, 20
 db SF_CHAR_BEZIER, 50, 20, 20, 20, 20, 50
 db SF_CHAR_BEZIER, 20, 50, 20, 100, 50, 100
 db SF_CHAR_BEZIER, 50, 100, 80, 100, 80, 80
  
 db SF_CHAR_END

bleskos_scalable_font_char_D_with_diacritic:
 db SF_CHAR_LINE, 20, 20, 20, 100
 db SF_CHAR_LINE, 20, 20, 50, 20
 db SF_CHAR_BEZIER, 50, 20, 80, 20, 80, 50
 db SF_CHAR_BEZIER, 80, 50, 80, 100, 50, 100
 db SF_CHAR_LINE, 20, 100, 50, 100
 
 db SF_CHAR_END

bleskos_scalable_font_char_E_with_diacritic:
 db SF_CHAR_LINE, 20, 20, 20, 100
 db SF_CHAR_LINE, 20, 20, 80, 20
 db SF_CHAR_LINE, 20, 60, 80, 60
 db SF_CHAR_LINE, 20, 100, 80, 100
 
 db SF_CHAR_END 

bleskos_scalable_font_char_0x100_A_with_macron: 
 ;macron
 db SF_CHAR_LINE, 30, 0, 70, 0
 
 db SF_CHAR_JUMP
 dd bleskos_scalable_font_char_A_with_diacritic
 
bleskos_scalable_font_char_0x101_a_with_macron:
 ;macron
 db SF_CHAR_LINE, 30, 20, 70, 20
 
 db SF_CHAR_JUMP
 dd bleskos_scalable_font_char_0x61_a

bleskos_scalable_font_char_0x102_A_with_breve:
 ;breve
 db SF_CHAR_BEZIER, 30, 0, 50, 15, 70, 0
 
 db SF_CHAR_JUMP
 dd bleskos_scalable_font_char_A_with_diacritic

bleskos_scalable_font_char_0x103_a_with_breve:
 ;breve
 db SF_CHAR_BEZIER, 30, 20, 50, 35, 70, 20
 
 db SF_CHAR_JUMP
 dd bleskos_scalable_font_char_0x61_a

bleskos_scalable_font_char_0x104_A_with_ogonek:
 ;ogonek
 db SF_CHAR_BEZIER, 90, 100, 70, 120, 100, 120

 db SF_CHAR_JUMP
 dd bleskos_scalable_font_char_A_with_diacritic

bleskos_scalable_font_char_0x105_a_with_ogonek:
 ;ogonek
 db SF_CHAR_BEZIER, 80, 100, 60, 120, 90, 120
 
 db SF_CHAR_JUMP
 dd bleskos_scalable_font_char_0x61_a

bleskos_scalable_font_char_0x106_C_with_acute:
 ;acute
 db SF_CHAR_LINE, 50, 10, 60, 0

 db SF_CHAR_JUMP
 dd bleskos_scalable_font_char_C_with_diacritic

bleskos_scalable_font_char_0x107_c_with_acute:
 ;acute
 db SF_CHAR_LINE, 50, 25, 65, 15

 db SF_CHAR_JUMP
 dd bleskos_scalable_font_char_0x63_c

bleskos_scalable_font_char_0x108_C_with_circumflex:
 ;circumflex
 db SF_CHAR_LINE, 40, 10, 50, 0
 db SF_CHAR_LINE, 50, 0, 60, 10

 db SF_CHAR_JUMP
 dd bleskos_scalable_font_char_C_with_diacritic

bleskos_scalable_font_char_0x109_c_with_circumflex:
 ;circumflex
 db SF_CHAR_LINE, 40, 25, 50, 15
 db SF_CHAR_LINE, 50, 15, 60, 25

 db SF_CHAR_JUMP
 dd bleskos_scalable_font_char_0x63_c

bleskos_scalable_font_char_0x10A_C_with_dot_above:
 ;dot above
 db SF_CHAR_FULL_ELLIPSE, 45, 0, 10, 10

 db SF_CHAR_JUMP
 dd bleskos_scalable_font_char_C_with_diacritic

bleskos_scalable_font_char_0x10B_c_with_dot_above:
 ;dot above
 db SF_CHAR_FULL_ELLIPSE, 45, 20, 10, 10

 db SF_CHAR_JUMP
 dd bleskos_scalable_font_char_0x63_c

bleskos_scalable_font_char_0x10C_C_with_caron:
 ;caron
 db SF_CHAR_LINE, 40, 0, 50, 15
 db SF_CHAR_LINE, 50, 15, 60, 0

 db SF_CHAR_JUMP
 dd bleskos_scalable_font_char_C_with_diacritic

bleskos_scalable_font_char_0x10D_c_with_caron:
 ;caron
 db SF_CHAR_LINE, 40, 15, 50, 30
 db SF_CHAR_LINE, 50, 30, 60, 15

 db SF_CHAR_JUMP
 dd bleskos_scalable_font_char_0x63_c

bleskos_scalable_font_char_0x10E_D_with_caron:
 ;caron
 db SF_CHAR_LINE, 40, 0, 50, 15
 db SF_CHAR_LINE, 50, 15, 60, 0

 db SF_CHAR_JUMP
 dd bleskos_scalable_font_char_D_with_diacritic

bleskos_scalable_font_char_0x10F_d_with_caron:
 ;caron as line
 db SF_CHAR_BEZIER, 95, 3, 100, 10, 90, 20

 db SF_CHAR_JUMP
 dd bleskos_scalable_font_char_0x64_d

bleskos_scalable_font_char_0x110_D_with_stroke:
 ;stroke
 db SF_CHAR_LINE, 0, 50, 40, 50

 db SF_CHAR_JUMP
 dd bleskos_scalable_font_char_0x44_D

bleskos_scalable_font_char_0x111_d_with_stroke:
 ;stroke
 db SF_CHAR_LINE, 50, 30, 90, 30

 db SF_CHAR_JUMP
 dd bleskos_scalable_font_char_0x64_d

bleskos_scalable_font_char_0x112_E_with_macron:
 ;macron
 db SF_CHAR_LINE, 30, 0, 70, 0
 
 db SF_CHAR_JUMP
 dd bleskos_scalable_font_char_E_with_diacritic

bleskos_scalable_font_char_0x113_e_with_macron:
 ;macron
 db SF_CHAR_LINE, 30, 20, 70, 20
 
 db SF_CHAR_JUMP
 dd bleskos_scalable_font_char_0x65_e

bleskos_scalable_font_char_0x114_E_with_breve:
 ;breve
 db SF_CHAR_BEZIER, 30, 0, 50, 15, 70, 0
 
 db SF_CHAR_JUMP
 dd bleskos_scalable_font_char_E_with_diacritic

bleskos_scalable_font_char_0x115_e_with_breve:
 ;breve
 db SF_CHAR_BEZIER, 30, 20, 50, 35, 70, 20
 
 db SF_CHAR_JUMP
 dd bleskos_scalable_font_char_0x65_e

bleskos_scalable_font_char_0x116_E_with_dot_above:
 ;dot above
 db SF_CHAR_FULL_ELLIPSE, 45, 0, 10, 10

 db SF_CHAR_JUMP
 dd bleskos_scalable_font_char_E_with_diacritic

bleskos_scalable_font_char_0x117_e_with_dot_above:
 ;dot above
 db SF_CHAR_FULL_ELLIPSE, 45, 20, 10, 10

 db SF_CHAR_JUMP
 dd bleskos_scalable_font_char_0x65_e

bleskos_scalable_font_char_0x118_E_with_ogonek:
 ;ogonek
 db SF_CHAR_BEZIER, 80, 100, 60, 120, 90, 120

 db SF_CHAR_JUMP
 dd bleskos_scalable_font_char_E_with_diacritic

bleskos_scalable_font_char_0x119_e_with_ogonek:
 ;ogonek
 db SF_CHAR_BEZIER, 70, 100, 50, 120, 80, 120
 
 db SF_CHAR_JUMP
 dd bleskos_scalable_font_char_0x65_e

bleskos_scalable_font_char_0x11A_E_with_caron:
 ;caron
 db SF_CHAR_LINE, 40, 0, 50, 15
 db SF_CHAR_LINE, 50, 15, 60, 0

 db SF_CHAR_JUMP
 dd bleskos_scalable_font_char_E_with_diacritic

bleskos_scalable_font_char_0x11B_e_with_caron:
 ;caron
 db SF_CHAR_LINE, 40, 15, 50, 30
 db SF_CHAR_LINE, 50, 30, 60, 15

 db SF_CHAR_JUMP
 dd bleskos_scalable_font_char_0x65_e