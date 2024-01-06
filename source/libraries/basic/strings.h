//BleskOS

/*
* MIT License
* Copyright (c) 2023-2024 Vendelín Slezák
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#define UNICODE_COMBINING_ACUTE 0x0301
#define UNICODE_COMBINIG_CARON 0x030C

byte_t are_equal_b_string_b_string(byte_t *string1, byte_t *string2);
byte_t are_equal_w_string_b_string(word_t *string1, byte_t *string2);
byte_t are_equal_b_string_w_string(byte_t *string1, word_t *string2);
byte_t are_equal_w_string_w_string(word_t *string1, word_t *string2);
byte_t is_char(word_t value);
byte_t is_number(dword_t value);
byte_t is_hex_number(dword_t value);
word_t get_unicode_char_with_diacritic(word_t value, word_t diacritic);
dword_t get_number_of_chars_in_ascii_string(byte_t *string);
dword_t get_number_of_chars_in_unicode_string(word_t *string);