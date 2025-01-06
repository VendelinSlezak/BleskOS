;BleskOS

;;;;;
;; MIT License
;; Copyright (c) 2023-2025 Vendelín Slezák
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
 dd bleskos_scalable_font_char_square ; 0x0080
 dd bleskos_scalable_font_char_square ; 0x0081
 dd bleskos_scalable_font_char_square ; 0x0082
 dd bleskos_scalable_font_char_square ; 0x0083
 dd bleskos_scalable_font_char_square ; 0x0084
 dd bleskos_scalable_font_char_square ; 0x0085
 dd bleskos_scalable_font_char_square ; 0x0086
 dd bleskos_scalable_font_char_square ; 0x0087
 dd bleskos_scalable_font_char_square ; 0x0088
 dd bleskos_scalable_font_char_square ; 0x0089
 dd bleskos_scalable_font_char_square ; 0x008A
 dd bleskos_scalable_font_char_square ; 0x008B
 dd bleskos_scalable_font_char_square ; 0x008C
 dd bleskos_scalable_font_char_square ; 0x008D
 dd bleskos_scalable_font_char_square ; 0x008E
 dd bleskos_scalable_font_char_square ; 0x008F
 dd bleskos_scalable_font_char_square ; 0x0090
 dd bleskos_scalable_font_char_square ; 0x0091
 dd bleskos_scalable_font_char_square ; 0x0092
 dd bleskos_scalable_font_char_square ; 0x0093
 dd bleskos_scalable_font_char_square ; 0x0094
 dd bleskos_scalable_font_char_square ; 0x0095
 dd bleskos_scalable_font_char_square ; 0x0096
 dd bleskos_scalable_font_char_square ; 0x0097
 dd bleskos_scalable_font_char_square ; 0x0098
 dd bleskos_scalable_font_char_square ; 0x0099
 dd bleskos_scalable_font_char_square ; 0x009A
 dd bleskos_scalable_font_char_square ; 0x009B
 dd bleskos_scalable_font_char_square ; 0x009C
 dd bleskos_scalable_font_char_square ; 0x009D
 dd bleskos_scalable_font_char_square ; 0x009E
 dd bleskos_scalable_font_char_square ; 0x009F
 dd bleskos_scalable_font_char_square ; 0x00A0 non-breaking space
 dd bleskos_scalable_font_char_square ; 0x00A1 ¡
 dd bleskos_scalable_font_char_square ; 0x00A2 ¢
 dd bleskos_scalable_font_char_square ; 0x00A3 £
 dd bleskos_scalable_font_char_square ; 0x00A4 ¤
 dd bleskos_scalable_font_char_square ; 0x00A5 ¥
 dd bleskos_scalable_font_char_square ; 0x00A6 ¦
 dd bleskos_scalable_font_char_square ; 0x00A7 §
 dd bleskos_scalable_font_char_square ; 0x00A8 ¨
 dd bleskos_scalable_font_char_square ; 0x00A9 ©
 dd bleskos_scalable_font_char_square ; 0x00AA ª
 dd bleskos_scalable_font_char_square ; 0x00AB «
 dd bleskos_scalable_font_char_square ; 0x00AC ¬
 dd bleskos_scalable_font_char_square ; 0x00AD ­
 dd bleskos_scalable_font_char_square ; 0x00AE ®
 dd bleskos_scalable_font_char_square ; 0x00AF ¯
 dd bleskos_scalable_font_char_square ; 0x00B0 °
 dd bleskos_scalable_font_char_square ; 0x00B1 ±
 dd bleskos_scalable_font_char_square ; 0x00B2 ²
 dd bleskos_scalable_font_char_square ; 0x00B3 ³
 dd bleskos_scalable_font_char_square ; 0x00B4 ´
 dd bleskos_scalable_font_char_square ; 0x00B5 µ
 dd bleskos_scalable_font_char_square ; 0x00B6 ¶
 dd bleskos_scalable_font_char_square ; 0x00B7 ·
 dd bleskos_scalable_font_char_square ; 0x00B8 ¸
 dd bleskos_scalable_font_char_square ; 0x00B9 ¹
 dd bleskos_scalable_font_char_square ; 0x00BA º
 dd bleskos_scalable_font_char_square ; 0x00BB »
 dd bleskos_scalable_font_char_square ; 0x00BC ¼
 dd bleskos_scalable_font_char_square ; 0x00BD ½
 dd bleskos_scalable_font_char_square ; 0x00BE ¾
 dd bleskos_scalable_font_char_square ; 0x00BF ¿
 dd bleskos_scalable_font_char_square ; 0x00C0 À
 dd bleskos_scalable_font_char_0xC1_A_with_acute
 dd bleskos_scalable_font_char_square ; 0x00C2 Â
 dd bleskos_scalable_font_char_square ; 0x00C3 Ã
 dd bleskos_scalable_font_char_square ; 0x00C4 Ä
 dd bleskos_scalable_font_char_square ; 0x00C5 Å
 dd bleskos_scalable_font_char_square ; 0x00C6 Æ
 dd bleskos_scalable_font_char_square ; 0x00C7 Ç
 dd bleskos_scalable_font_char_square ; 0x00C8 È
 dd bleskos_scalable_font_char_0xC9_E_with_acute
 dd bleskos_scalable_font_char_square ; 0x00CA Ê
 dd bleskos_scalable_font_char_square ; 0x00CB Ë
 dd bleskos_scalable_font_char_square ; 0x00CC Ì
 dd bleskos_scalable_font_char_0xCD_I_with_acute
 dd bleskos_scalable_font_char_square ; 0x00CE Î
 dd bleskos_scalable_font_char_square ; 0x00CF Ï
 dd bleskos_scalable_font_char_square ; 0x00D0 Ð
 dd bleskos_scalable_font_char_square ; 0x00D1 Ñ
 dd bleskos_scalable_font_char_square ; 0x00D2 Ò
 dd bleskos_scalable_font_char_0xD3_O_with_acute
 dd bleskos_scalable_font_char_0xD4_O_with_circumflex
 dd bleskos_scalable_font_char_square ; 0x00D5 Õ
 dd bleskos_scalable_font_char_square ; 0x00D6 Ö
 dd bleskos_scalable_font_char_square ; 0x00D7 ×
 dd bleskos_scalable_font_char_square ; 0x00D8 Ø
 dd bleskos_scalable_font_char_square ; 0x00D9 Ù
 dd bleskos_scalable_font_char_0xDA_U_with_acute
 dd bleskos_scalable_font_char_square ; 0x00DB Û
 dd bleskos_scalable_font_char_square ; 0x00DC Ü
 dd bleskos_scalable_font_char_0xDD_Y_with_acute
 dd bleskos_scalable_font_char_square ; 0x00DE Þ
 dd bleskos_scalable_font_char_square ; 0x00DF ß
 dd bleskos_scalable_font_char_square ; 0x00E0 à
 dd bleskos_scalable_font_char_0xE1_a_with_acute
 dd bleskos_scalable_font_char_square ; 0x00E2 â
 dd bleskos_scalable_font_char_square ; 0x00E3 ã
 dd bleskos_scalable_font_char_square ; 0x00E4 ä
 dd bleskos_scalable_font_char_square ; 0x00E5 å
 dd bleskos_scalable_font_char_square ; 0x00E6 æ
 dd bleskos_scalable_font_char_square ; 0x00E7 ç
 dd bleskos_scalable_font_char_square ; 0x00E8 è
 dd bleskos_scalable_font_char_0xE9_e_with_acute
 dd bleskos_scalable_font_char_square ; 0x00EA ê
 dd bleskos_scalable_font_char_square ; 0x00EB ë
 dd bleskos_scalable_font_char_square ; 0x00EC ì
 dd bleskos_scalable_font_char_0xED_i_with_acute
 dd bleskos_scalable_font_char_square ; 0x00EE î
 dd bleskos_scalable_font_char_square ; 0x00EF ï
 dd bleskos_scalable_font_char_square ; 0x00F0 ð
 dd bleskos_scalable_font_char_square ; 0x00F1 ñ
 dd bleskos_scalable_font_char_square ; 0x00F2 ò
 dd bleskos_scalable_font_char_0xF3_o_with_acute
 dd bleskos_scalable_font_char_0xF4_o_with_circumflex
 dd bleskos_scalable_font_char_square ; 0x00F5 õ
 dd bleskos_scalable_font_char_square ; 0x00F6 ö
 dd bleskos_scalable_font_char_square ; 0x00F7 ÷
 dd bleskos_scalable_font_char_square ; 0x00F8 ø
 dd bleskos_scalable_font_char_square ; 0x00F9 ù
 dd bleskos_scalable_font_char_0xFA_u_with_acute
 dd bleskos_scalable_font_char_square ; 0x00FB û
 dd bleskos_scalable_font_char_square ; 0x00FC ü
 dd bleskos_scalable_font_char_0xFD_y_with_acute
 dd bleskos_scalable_font_char_square ; 0x00FE þ
 dd bleskos_scalable_font_char_square ; 0x00FF ÿ
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
 dd bleskos_scalable_font_char_square ; 0x011C Ĝ
 dd bleskos_scalable_font_char_square ; 0x011D ĝ
 dd bleskos_scalable_font_char_square ; 0x011E Ğ
 dd bleskos_scalable_font_char_square ; 0x011F ğ
 dd bleskos_scalable_font_char_square ; 0x0120 Ġ
 dd bleskos_scalable_font_char_square ; 0x0121 ġ
 dd bleskos_scalable_font_char_square ; 0x0122 Ģ
 dd bleskos_scalable_font_char_square ; 0x0123 ģ
 dd bleskos_scalable_font_char_square ; 0x0124 Ĥ
 dd bleskos_scalable_font_char_square ; 0x0125 ĥ
 dd bleskos_scalable_font_char_square ; 0x0126 Ħ
 dd bleskos_scalable_font_char_square ; 0x0127 ħ
 dd bleskos_scalable_font_char_square ; 0x0128 Ĩ
 dd bleskos_scalable_font_char_square ; 0x0129 ĩ
 dd bleskos_scalable_font_char_square ; 0x012A Ī
 dd bleskos_scalable_font_char_square ; 0x012B ī
 dd bleskos_scalable_font_char_square ; 0x012C Ĭ
 dd bleskos_scalable_font_char_square ; 0x012D ĭ
 dd bleskos_scalable_font_char_square ; 0x012E Į
 dd bleskos_scalable_font_char_square ; 0x012F į
 dd bleskos_scalable_font_char_square ; 0x0130 İ
 dd bleskos_scalable_font_char_square ; 0x0131 ı
 dd bleskos_scalable_font_char_square ; 0x0132 Ĳ
 dd bleskos_scalable_font_char_square ; 0x0133 ĳ
 dd bleskos_scalable_font_char_square ; 0x0134 Ĵ
 dd bleskos_scalable_font_char_square ; 0x0135 ĵ
 dd bleskos_scalable_font_char_square ; 0x0136 Ķ
 dd bleskos_scalable_font_char_square ; 0x0137 ķ
 dd bleskos_scalable_font_char_square ; 0x0138 ĸ
 dd bleskos_scalable_font_char_square ; 0x0139 Ĺ
 dd bleskos_scalable_font_char_0x13A_l_with_acute
 dd bleskos_scalable_font_char_square ; 0x013B Ļ
 dd bleskos_scalable_font_char_square ; 0x013C ļ
 dd bleskos_scalable_font_char_0x13D_L_with_caron
 dd bleskos_scalable_font_char_0x13E_l_with_caron
 dd bleskos_scalable_font_char_square ; 0x013F Ŀ
 dd bleskos_scalable_font_char_square ; 0x0140 ŀ
 dd bleskos_scalable_font_char_square ; 0x0141 Ł
 dd bleskos_scalable_font_char_square ; 0x0142 ł
 dd bleskos_scalable_font_char_square ; 0x0143 Ń
 dd bleskos_scalable_font_char_square ; 0x0144 ń
 dd bleskos_scalable_font_char_square ; 0x0145 Ņ
 dd bleskos_scalable_font_char_square ; 0x0146 ņ
 dd bleskos_scalable_font_char_0x147_N_with_caron
 dd bleskos_scalable_font_char_0x148_n_with_caron
 dd bleskos_scalable_font_char_square ; 0x0149 ŉ
 dd bleskos_scalable_font_char_square ; 0x014A Ŋ
 dd bleskos_scalable_font_char_square ; 0x014B ŋ
 dd bleskos_scalable_font_char_square ; 0x014C Ō
 dd bleskos_scalable_font_char_square ; 0x014D ō
 dd bleskos_scalable_font_char_square ; 0x014E Ŏ
 dd bleskos_scalable_font_char_square ; 0x014F ŏ
 dd bleskos_scalable_font_char_square ; 0x0150 Ő
 dd bleskos_scalable_font_char_square ; 0x0151 ő
 dd bleskos_scalable_font_char_square ; 0x0152 Œ
 dd bleskos_scalable_font_char_square ; 0x0153 œ
 dd bleskos_scalable_font_char_square ; 0x0154 Ŕ
 dd bleskos_scalable_font_char_0x155_r_with_acute
 dd bleskos_scalable_font_char_square ; 0x0156 Ŗ
 dd bleskos_scalable_font_char_square ; 0x0157 ŗ
 dd bleskos_scalable_font_char_square ; 0x0158 Ř
 dd bleskos_scalable_font_char_square ; 0x0159 ř
 dd bleskos_scalable_font_char_square ; 0x015A Ś
 dd bleskos_scalable_font_char_square ; 0x015B ś
 dd bleskos_scalable_font_char_square ; 0x015C Ŝ
 dd bleskos_scalable_font_char_square ; 0x015D ŝ
 dd bleskos_scalable_font_char_square ; 0x015E Ş
 dd bleskos_scalable_font_char_square ; 0x015F ş
 dd bleskos_scalable_font_char_0x160_S_with_caron
 dd bleskos_scalable_font_char_0x161_s_with_caron
 dd bleskos_scalable_font_char_square ; 0x0162 Ţ
 dd bleskos_scalable_font_char_square ; 0x0163 ţ
 dd bleskos_scalable_font_char_0x164_T_with_caron
 dd bleskos_scalable_font_char_0x165_t_with_caron
 dd bleskos_scalable_font_char_square ; 0x0166 Ŧ
 dd bleskos_scalable_font_char_square ; 0x0167 ŧ
 dd bleskos_scalable_font_char_square ; 0x0168 Ũ
 dd bleskos_scalable_font_char_square ; 0x0169 ũ
 dd bleskos_scalable_font_char_square ; 0x016A Ū
 dd bleskos_scalable_font_char_square ; 0x016B ū
 dd bleskos_scalable_font_char_square ; 0x016C Ŭ
 dd bleskos_scalable_font_char_square ; 0x016D ŭ
 dd bleskos_scalable_font_char_square ; 0x016E Ů
 dd bleskos_scalable_font_char_square ; 0x016F ů
 dd bleskos_scalable_font_char_square ; 0x0170 Ű
 dd bleskos_scalable_font_char_square ; 0x0171 ű
 dd bleskos_scalable_font_char_square ; 0x0172 Ų
 dd bleskos_scalable_font_char_square ; 0x0173 ų
 dd bleskos_scalable_font_char_square ; 0x0174 Ŵ
 dd bleskos_scalable_font_char_square ; 0x0175 ŵ
 dd bleskos_scalable_font_char_square ; 0x0176 Ŷ
 dd bleskos_scalable_font_char_square ; 0x0177 ŷ
 dd bleskos_scalable_font_char_square ; 0x0178 Ÿ
 dd bleskos_scalable_font_char_square ; 0x0179 Ź
 dd bleskos_scalable_font_char_square ; 0x017A ź
 dd bleskos_scalable_font_char_square ; 0x017B Ż
 dd bleskos_scalable_font_char_square ; 0x017C ż
 dd bleskos_scalable_font_char_0x17D_Z_with_caron
 dd bleskos_scalable_font_char_0x17E_z_with_caron
 dd bleskos_scalable_font_char_square ; 0x017F ſ
 dd bleskos_scalable_font_char_square ; 0x0180 ƀ
 dd bleskos_scalable_font_char_square ; 0x0181 Ɓ
 dd bleskos_scalable_font_char_square ; 0x0182 Ƃ
 dd bleskos_scalable_font_char_square ; 0x0183 ƃ
 dd bleskos_scalable_font_char_square ; 0x0184 Ƅ
 dd bleskos_scalable_font_char_square ; 0x0185 ƅ
 dd bleskos_scalable_font_char_square ; 0x0186 Ɔ
 dd bleskos_scalable_font_char_square ; 0x0187 Ƈ
 dd bleskos_scalable_font_char_square ; 0x0188 ƈ
 dd bleskos_scalable_font_char_square ; 0x0189 Ɖ
 dd bleskos_scalable_font_char_square ; 0x018A Ɗ
 dd bleskos_scalable_font_char_square ; 0x018B Ƌ
 dd bleskos_scalable_font_char_square ; 0x018C ƌ
 dd bleskos_scalable_font_char_square ; 0x018D ƍ
 dd bleskos_scalable_font_char_square ; 0x018E Ǝ
 dd bleskos_scalable_font_char_square ; 0x018F Ə
 dd bleskos_scalable_font_char_square ; 0x0190 Ɛ
 dd bleskos_scalable_font_char_square ; 0x0191 Ƒ
 dd bleskos_scalable_font_char_square ; 0x0192 ƒ
 dd bleskos_scalable_font_char_square ; 0x0193 Ɠ
 dd bleskos_scalable_font_char_square ; 0x0194 Ɣ
 dd bleskos_scalable_font_char_square ; 0x0195 ƕ
 dd bleskos_scalable_font_char_square ; 0x0196 Ɩ
 dd bleskos_scalable_font_char_square ; 0x0197 Ɨ
 dd bleskos_scalable_font_char_square ; 0x0198 Ƙ
 dd bleskos_scalable_font_char_square ; 0x0199 ƙ
 dd bleskos_scalable_font_char_square ; 0x019A ƚ
 dd bleskos_scalable_font_char_square ; 0x019B ƛ
 dd bleskos_scalable_font_char_square ; 0x019C Ɯ
 dd bleskos_scalable_font_char_square ; 0x019D Ɲ
 dd bleskos_scalable_font_char_square ; 0x019E ƞ
 dd bleskos_scalable_font_char_square ; 0x019F Ɵ
 dd bleskos_scalable_font_char_square ; 0x01A0 Ơ
 dd bleskos_scalable_font_char_square ; 0x01A1 ơ
 dd bleskos_scalable_font_char_square ; 0x01A2 Ƣ
 dd bleskos_scalable_font_char_square ; 0x01A3 ƣ
 dd bleskos_scalable_font_char_square ; 0x01A4 Ƥ
 dd bleskos_scalable_font_char_square ; 0x01A5 ƥ
 dd bleskos_scalable_font_char_square ; 0x01A6 Ʀ
 dd bleskos_scalable_font_char_square ; 0x01A7 Ƨ
 dd bleskos_scalable_font_char_square ; 0x01A8 ƨ
 dd bleskos_scalable_font_char_square ; 0x01A9 Ʃ
 dd bleskos_scalable_font_char_square ; 0x01AA ƪ
 dd bleskos_scalable_font_char_square ; 0x01AB ƫ
 dd bleskos_scalable_font_char_square ; 0x01AC Ƭ
 dd bleskos_scalable_font_char_square ; 0x01AD ƭ
 dd bleskos_scalable_font_char_square ; 0x01AE Ʈ
 dd bleskos_scalable_font_char_square ; 0x01AF Ư
 dd bleskos_scalable_font_char_square ; 0x01B0 ư
 dd bleskos_scalable_font_char_square ; 0x01B1 Ʊ
 dd bleskos_scalable_font_char_square ; 0x01B2 Ʋ
 dd bleskos_scalable_font_char_square ; 0x01B3 Ƴ
 dd bleskos_scalable_font_char_square ; 0x01B4 ƴ
 dd bleskos_scalable_font_char_square ; 0x01B5 Ƶ
 dd bleskos_scalable_font_char_square ; 0x01B6 ƶ
 dd bleskos_scalable_font_char_square ; 0x01B7 Ʒ
 dd bleskos_scalable_font_char_square ; 0x01B8 Ƹ
 dd bleskos_scalable_font_char_square ; 0x01B9 ƹ
 dd bleskos_scalable_font_char_square ; 0x01BA ƺ
 dd bleskos_scalable_font_char_square ; 0x01BB ƻ
 dd bleskos_scalable_font_char_square ; 0x01BC Ƽ
 dd bleskos_scalable_font_char_square ; 0x01BD ƽ
 dd bleskos_scalable_font_char_square ; 0x01BE ƾ
 dd bleskos_scalable_font_char_square ; 0x01BF ƿ
 dd bleskos_scalable_font_char_square ; 0x01C0 ǀ
 dd bleskos_scalable_font_char_square ; 0x01C1 ǁ
 dd bleskos_scalable_font_char_square ; 0x01C2 ǂ
 dd bleskos_scalable_font_char_square ; 0x01C3 ǃ
 dd bleskos_scalable_font_char_square ; 0x01C4 Ǆ
 dd bleskos_scalable_font_char_square ; 0x01C5 ǅ
 dd bleskos_scalable_font_char_square ; 0x01C6 ǆ
 dd bleskos_scalable_font_char_square ; 0x01C7 Ǉ
 dd bleskos_scalable_font_char_square ; 0x01C8 ǈ
 dd bleskos_scalable_font_char_square ; 0x01C9 ǉ
 dd bleskos_scalable_font_char_square ; 0x01CA Ǌ
 dd bleskos_scalable_font_char_square ; 0x01CB ǋ
 dd bleskos_scalable_font_char_square ; 0x01CC ǌ
 dd bleskos_scalable_font_char_square ; 0x01CD Ǎ
 dd bleskos_scalable_font_char_square ; 0x01CE ǎ
 dd bleskos_scalable_font_char_square ; 0x01CF Ǐ
 dd bleskos_scalable_font_char_square ; 0x01D0 ǐ
 dd bleskos_scalable_font_char_square ; 0x01D1 Ǒ
 dd bleskos_scalable_font_char_square ; 0x01D2 ǒ
 dd bleskos_scalable_font_char_square ; 0x01D3 Ǔ
 dd bleskos_scalable_font_char_square ; 0x01D4 ǔ
 dd bleskos_scalable_font_char_square ; 0x01D5 Ǖ
 dd bleskos_scalable_font_char_square ; 0x01D6 ǖ
 dd bleskos_scalable_font_char_square ; 0x01D7 Ǘ
 dd bleskos_scalable_font_char_square ; 0x01D8 ǘ
 dd bleskos_scalable_font_char_square ; 0x01D9 Ǚ
 dd bleskos_scalable_font_char_square ; 0x01DA ǚ
 dd bleskos_scalable_font_char_square ; 0x01DB Ǜ
 dd bleskos_scalable_font_char_square ; 0x01DC ǜ
 dd bleskos_scalable_font_char_square ; 0x01DD ǝ
 dd bleskos_scalable_font_char_square ; 0x01DE Ǟ
 dd bleskos_scalable_font_char_square ; 0x01DF ǟ
 dd bleskos_scalable_font_char_square ; 0x01E0 Ǡ
 dd bleskos_scalable_font_char_square ; 0x01E1 ǡ
 dd bleskos_scalable_font_char_square ; 0x01E2 Ǣ
 dd bleskos_scalable_font_char_square ; 0x01E3 ǣ
 dd bleskos_scalable_font_char_square ; 0x01E4 Ǥ
 dd bleskos_scalable_font_char_square ; 0x01E5 ǥ
 dd bleskos_scalable_font_char_square ; 0x01E6 Ǧ
 dd bleskos_scalable_font_char_square ; 0x01E7 ǧ
 dd bleskos_scalable_font_char_square ; 0x01E8 Ǩ
 dd bleskos_scalable_font_char_square ; 0x01E9 ǩ
 dd bleskos_scalable_font_char_square ; 0x01EA Ǫ
 dd bleskos_scalable_font_char_square ; 0x01EB ǫ
 dd bleskos_scalable_font_char_square ; 0x01EC Ǭ
 dd bleskos_scalable_font_char_square ; 0x01ED ǭ
 dd bleskos_scalable_font_char_square ; 0x01EE Ǯ
 dd bleskos_scalable_font_char_square ; 0x01EF ǯ
 dd bleskos_scalable_font_char_square ; 0x01F0 ǰ
 dd bleskos_scalable_font_char_square ; 0x01F1 Ǳ
 dd bleskos_scalable_font_char_square ; 0x01F2 ǲ
 dd bleskos_scalable_font_char_square ; 0x01F3 ǳ
 dd bleskos_scalable_font_char_square ; 0x01F4 Ǵ
 dd bleskos_scalable_font_char_square ; 0x01F5 ǵ
 dd bleskos_scalable_font_char_square ; 0x01F6 Ƕ
 dd bleskos_scalable_font_char_square ; 0x01F7 Ƿ
 dd bleskos_scalable_font_char_square ; 0x01F8 Ǹ
 dd bleskos_scalable_font_char_square ; 0x01F9 ǹ
 dd bleskos_scalable_font_char_square ; 0x01FA Ǻ
 dd bleskos_scalable_font_char_square ; 0x01FB ǻ
 dd bleskos_scalable_font_char_square ; 0x01FC Ǽ
 dd bleskos_scalable_font_char_square ; 0x01FD ǽ
 dd bleskos_scalable_font_char_square ; 0x01FE Ǿ
 dd bleskos_scalable_font_char_square ; 0x01FF ǿ
 dd bleskos_scalable_font_char_square ; 0x0200 Ȁ
 
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
 db SF_CHAR_LINE, 75, 100, 95, 40
 
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

bleskos_scalable_font_char_I_with_diacritic:
 db SF_CHAR_LINE, 50, 20, 50, 100
 db SF_CHAR_LINE, 30, 20, 70, 20
 db SF_CHAR_LINE, 30, 100, 70, 100

 db SF_CHAR_END

bleskos_scalable_font_char_L_with_diacritic:
 db SF_CHAR_LINE, 20, 20, 20, 100
 db SF_CHAR_LINE, 20, 100, 80, 100
 
 db SF_CHAR_END

bleskos_scalable_font_char_N_with_diacritic:
 db SF_CHAR_LINE, 20, 20, 20, 100
 db SF_CHAR_LINE, 20, 20, 80, 100
 db SF_CHAR_LINE, 80, 100, 80, 20
 
 db SF_CHAR_END

bleskos_scalable_font_char_S_with_diacritic:
 db SF_CHAR_BEZIER, 80, 30, 80, 20, 50, 20
 db SF_CHAR_BEZIER, 50, 20, 20, 20, 20, 35
 db SF_CHAR_BEZIER, 20, 35, 20, 60, 50, 60
 db SF_CHAR_BEZIER, 50, 60, 80, 60, 80, 85
 db SF_CHAR_BEZIER, 80, 85, 80, 100, 50, 100
 db SF_CHAR_BEZIER, 50, 100, 20, 100, 20, 90
 
 db SF_CHAR_END

bleskos_scalable_font_char_O_with_diacritic:
 db SF_CHAR_BEZIER, 20, 60, 20, 20, 60, 20
 db SF_CHAR_BEZIER, 50, 20, 80, 20, 80, 60
 db SF_CHAR_BEZIER, 80, 60, 80, 100, 60, 100
 db SF_CHAR_BEZIER, 50, 100, 20, 100, 20, 60
 
 db SF_CHAR_END

bleskos_scalable_font_char_T_with_diacritic:
 db SF_CHAR_LINE, 50, 20, 50, 100
 db SF_CHAR_LINE, 20, 20, 80, 20
 
 db SF_CHAR_END

bleskos_scalable_font_char_U_with_diacritic:
 db SF_CHAR_LINE, 20, 20, 20, 60
 db SF_CHAR_LINE, 80, 20, 80, 60
 db SF_CHAR_BEZIER, 80, 60, 80, 100, 50, 100
 db SF_CHAR_BEZIER, 50, 100, 20, 100, 20, 60
 
 db SF_CHAR_END

bleskos_scalable_font_char_Y_with_diacritic:
 db SF_CHAR_LINE, 20, 20, 50, 60
 db SF_CHAR_LINE, 50, 60, 80, 20
 db SF_CHAR_LINE, 50, 60, 50, 100
 
 db SF_CHAR_END

bleskos_scalable_font_char_Z_with_diacritic:
 db SF_CHAR_LINE, 20, 20, 80, 20
 db SF_CHAR_LINE, 80, 20, 20, 100
 db SF_CHAR_LINE, 20, 100, 80, 100
 
 db SF_CHAR_END

bleskos_scalable_font_char_0xC1_A_with_acute:
 ;acute
 db SF_CHAR_LINE, 50, 10, 60, 0

 db SF_CHAR_JUMP
 dd bleskos_scalable_font_char_A_with_diacritic

bleskos_scalable_font_char_0xC9_E_with_acute:
 ;acute
 db SF_CHAR_LINE, 50, 10, 60, 0

 db SF_CHAR_JUMP
 dd bleskos_scalable_font_char_E_with_diacritic

bleskos_scalable_font_char_0xCD_I_with_acute:
 ;acute
 db SF_CHAR_LINE, 50, 10, 60, 0

 db SF_CHAR_JUMP
 dd bleskos_scalable_font_char_I_with_diacritic

bleskos_scalable_font_char_0xD3_O_with_acute:
 ;acute
 db SF_CHAR_LINE, 50, 10, 60, 0

 db SF_CHAR_JUMP
 dd bleskos_scalable_font_char_O_with_diacritic

bleskos_scalable_font_char_0xD4_O_with_circumflex:
 ;circumflex
 db SF_CHAR_LINE, 40, 10, 50, 0
 db SF_CHAR_LINE, 50, 0, 60, 10

 db SF_CHAR_JUMP
 dd bleskos_scalable_font_char_O_with_diacritic

bleskos_scalable_font_char_0xDA_U_with_acute:
 ;acute
 db SF_CHAR_LINE, 50, 10, 60, 0

 db SF_CHAR_JUMP
 dd bleskos_scalable_font_char_U_with_diacritic

bleskos_scalable_font_char_0xDD_Y_with_acute:
 ;acute
 db SF_CHAR_LINE, 50, 10, 60, 0

 db SF_CHAR_JUMP
 dd bleskos_scalable_font_char_Y_with_diacritic

bleskos_scalable_font_char_0xE1_a_with_acute:
 ;acute
 db SF_CHAR_LINE, 50, 25, 65, 15

 db SF_CHAR_JUMP
 dd bleskos_scalable_font_char_0x61_a

bleskos_scalable_font_char_0xE9_e_with_acute:
 ;acute
 db SF_CHAR_LINE, 50, 25, 65, 15

 db SF_CHAR_JUMP
 dd bleskos_scalable_font_char_0x65_e

bleskos_scalable_font_char_0xED_i_with_acute:
 ;acute
 db SF_CHAR_LINE, 50, 25, 65, 15

 db SF_CHAR_JUMP
 dd bleskos_scalable_font_char_0x69_i

bleskos_scalable_font_char_0xF3_o_with_acute:
 ;acute
 db SF_CHAR_LINE, 50, 25, 65, 15

 db SF_CHAR_JUMP
 dd bleskos_scalable_font_char_0x6F_o

bleskos_scalable_font_char_0xF4_o_with_circumflex:
 ;circumflex
 db SF_CHAR_LINE, 40, 25, 50, 15
 db SF_CHAR_LINE, 50, 15, 60, 25
 
 db SF_CHAR_JUMP
 dd bleskos_scalable_font_char_0x6F_o

bleskos_scalable_font_char_0xFA_u_with_acute:
 ;acute
 db SF_CHAR_LINE, 50, 25, 65, 15

 db SF_CHAR_JUMP
 dd bleskos_scalable_font_char_0x75_u

bleskos_scalable_font_char_0xFD_y_with_acute:
 ;acute
 db SF_CHAR_LINE, 50, 25, 65, 15

 db SF_CHAR_JUMP
 dd bleskos_scalable_font_char_0x79_y

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

bleskos_scalable_font_char_0x13A_l_with_acute:
 ;acute
 db SF_CHAR_LINE, 70, 25, 90, 0

 db SF_CHAR_JUMP
 dd bleskos_scalable_font_char_0x6C_l

bleskos_scalable_font_char_0x13D_L_with_caron:
 ;caron
 db SF_CHAR_LINE, 40, 0, 50, 15
 db SF_CHAR_LINE, 50, 15, 60, 0

 db SF_CHAR_JUMP
 dd bleskos_scalable_font_char_L_with_diacritic

bleskos_scalable_font_char_0x13E_l_with_caron:
 ;caron as line
 db SF_CHAR_BEZIER, 70, 3, 75, 10, 65, 20

 db SF_CHAR_JUMP
 dd bleskos_scalable_font_char_0x6C_l

bleskos_scalable_font_char_0x147_N_with_caron:
 ;caron
 db SF_CHAR_LINE, 40, 0, 50, 15
 db SF_CHAR_LINE, 50, 15, 60, 0

 db SF_CHAR_JUMP
 dd bleskos_scalable_font_char_N_with_diacritic

bleskos_scalable_font_char_0x148_n_with_caron:
 ;caron
 db SF_CHAR_LINE, 40, 15, 50, 30
 db SF_CHAR_LINE, 50, 30, 60, 15

 db SF_CHAR_JUMP
 dd bleskos_scalable_font_char_0x6E_n

bleskos_scalable_font_char_0x155_r_with_acute:
 ;acute
 db SF_CHAR_LINE, 50, 25, 65, 15

 db SF_CHAR_JUMP
 dd bleskos_scalable_font_char_0x72_r

bleskos_scalable_font_char_0x160_S_with_caron:
 ;caron
 db SF_CHAR_LINE, 40, 0, 50, 15
 db SF_CHAR_LINE, 50, 15, 60, 0

 db SF_CHAR_JUMP
 dd bleskos_scalable_font_char_S_with_diacritic

bleskos_scalable_font_char_0x161_s_with_caron:
 ;caron
 db SF_CHAR_LINE, 40, 15, 50, 30
 db SF_CHAR_LINE, 50, 30, 60, 15

 db SF_CHAR_JUMP
 dd bleskos_scalable_font_char_0x73_s

bleskos_scalable_font_char_0x164_T_with_caron:
 ;caron
 db SF_CHAR_LINE, 40, 0, 50, 15
 db SF_CHAR_LINE, 50, 15, 60, 0

 db SF_CHAR_JUMP
 dd bleskos_scalable_font_char_T_with_diacritic

bleskos_scalable_font_char_0x165_t_with_caron:
 ;caron
 db SF_CHAR_LINE, 40, 15, 50, 30
 db SF_CHAR_LINE, 50, 30, 60, 15

 db SF_CHAR_JUMP
 dd bleskos_scalable_font_char_0x74_t

bleskos_scalable_font_char_0x17D_Z_with_caron:
 ;caron
 db SF_CHAR_LINE, 40, 0, 50, 15
 db SF_CHAR_LINE, 50, 15, 60, 0

 db SF_CHAR_JUMP
 dd bleskos_scalable_font_char_Z_with_diacritic

bleskos_scalable_font_char_0x17E_z_with_caron:
 ;caron
 db SF_CHAR_LINE, 40, 15, 50, 30
 db SF_CHAR_LINE, 50, 30, 60, 15

 db SF_CHAR_JUMP
 dd bleskos_scalable_font_char_0x7A_z