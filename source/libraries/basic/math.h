//BleskOS

/*
* MIT License
* Copyright (c) 2023-2024 Vendelín Slezák
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

dword_t math_year = 0, math_month = 0, math_day = 0;

int abs(int i);
float fabs(float i);
int floor(float i);
dword_t power(dword_t value, dword_t exponent);
dword_t get_number_of_digits_in_number(dword_t number);
dword_t convert_byte_string_to_number(dword_t mem);
dword_t convert_word_string_to_number(dword_t mem);
float convert_word_string_to_float_number(dword_t mem);
dword_t convert_hex_word_string_to_number(dword_t mem);
void covert_number_to_byte_string(dword_t number, dword_t mem);
void covert_number_to_word_string(dword_t number, dword_t mem);
void convert_unix_time(dword_t unix_time);