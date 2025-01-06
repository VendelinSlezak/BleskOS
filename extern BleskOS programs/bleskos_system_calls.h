//BleskOS

/*
* MIT License
* Copyright (c) 2023-2025 Vendelín Slezák
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#define BLESKOS_SYSTEM_CALLS_VERSION 0x20241102 // version from day 02/11/2024

#define PROGRAM_TEXT_MODE 1

// 1) include this file at top of your program
// 2) define main as: void main(unsigned int (*system_call)(unsigned int call, unsigned int args[]))
// 3) now you can use CALL_* macros as methods in your code
// 4) compile your file as follows:
//      gcc -c -m32 -fno-pie -ffreestanding yourProgram.c -o yourProgram.o
//      ld -m elf_i386 -e main --oformat binary -Ttext 0x10000 yourProgram.o -o yourProgram.bin
// 5) now you can run yourProgram.bin in BleskOS through program "Run binary programs"

volatile const char *bleskos_program_signature = "BleskOS program signature";

typedef unsigned char byte_t;
typedef unsigned short word_t;
typedef unsigned int dword_t;
typedef unsigned long long qword_t;

#define SYSTEM_CALL_SUCCESS 0
#define SYSTEM_CALL_ERROR -1

#define STATUS_TRUE 1
#define STATUS_FALSE 0

#define NUMBER_OF_CALL_SET_PROGRAM_MODE 0
enum {
 NUMBER_OF_CALL_RESET_SCREEN = 1,
 NUMBER_OF_CALL_CLEAR_SCREEN,
 NUMBER_OF_CALL_GET_NUMBER_OF_SCREEN_COLUMNS,
 NUMBER_OF_CALL_GET_NUMBER_OF_SCREEN_LINES,
 NUMBER_OF_CALL_HIDE_CHANGES_ON_SCREEN,
 NUMBER_OF_CALL_REDRAW_SCREEN,
 NUMBER_OF_CALL_REDRAW_PART_OF_SCREEN,
 NUMBER_OF_CALL_SHOW_CHANGES_ON_SCREEN,

 NUMBER_OF_CALL_SHOW_CURSOR,
 NUMBER_OF_CALL_HIDE_CURSOR,
 NUMBER_OF_CALL_MOVE_CURSOR,
 NUMBER_OF_CALL_MOVE_CURSOR_TO_COLUMN,
 NUMBER_OF_CALL_MOVE_CURSOR_TO_LINE,
 NUMBER_OF_CALL_GET_CURSOR_COLUMN,
 NUMBER_OF_CALL_GET_CURSOR_LINE,

 NUMBER_OF_CALL_SET_PIXEL,
 NUMBER_OF_CALL_SET_CURSOR_PIXEL_CHARACTER,
 NUMBER_OF_CALL_SET_CURSOR_PIXEL_CH_COLOR,
 NUMBER_OF_CALL_SET_CURSOR_PIXEL_BG_COLOR,
 NUMBER_OF_CALL_SET_CURSOR_PIXEL,
 NUMBER_OF_CALL_PUT_CHARACTER_ON_CURSOR,
 NUMBER_OF_CALL_PUT_PIXEL_ON_CURSOR,
 NUMBER_OF_CALL_GET_PIXEL_CHARACTER,
 NUMBER_OF_CALL_GET_PIXEL_CH_COLOR,
 NUMBER_OF_CALL_GET_PIXEL_BG_COLOR,
 NUMBER_OF_CALL_GET_CURSOR_PIXEL_CHARACTER,
 NUMBER_OF_CALL_GET_CURSOR_PIXEL_CH_COLOR,
 NUMBER_OF_CALL_GET_CURSOR_PIXEL_BG_COLOR,

 NUMBER_OF_CALL_PRINT,
 NUMBER_OF_CALL_PRINT_WITH_CH_COLOR,
 NUMBER_OF_CALL_PRINT_WITH_BG_COLOR,
 NUMBER_OF_CALL_PRINT_NUMBER,
 NUMBER_OF_CALL_PRINT_NUMBER_WITH_CH_COLOR,
 NUMBER_OF_CALL_PRINT_NUMBER_WITH_BG_COLOR,
 NUMBER_OF_CALL_PRINT_FLOAT_NUMBER,
 NUMBER_OF_CALL_PRINT_FLOAT_NUMBER_WITH_CH_COLOR,
 NUMBER_OF_CALL_PRINT_FLOAT_NUMBER_WITH_BG_COLOR,
 NUMBER_OF_CALL_PRINT_HEX_NUMBER,
 NUMBER_OF_CALL_PRINT_HEX_NUMBER_WITH_CH_COLOR,
 NUMBER_OF_CALL_PRINT_HEX_NUMBER_WITH_BG_COLOR,
 NUMBER_OF_CALL_PRINT_ON_CURSOR,
 NUMBER_OF_CALL_PRINT_NUMBER_ON_CURSOR,
 NUMBER_OF_CALL_PRINT_FLOAT_NUMBER_ON_CURSOR,
 NUMBER_OF_CALL_PRINT_HEX_NUMBER_ON_CURSOR,
 NUMBER_OF_CALL_PRINT_TO_BLOCK,

 NUMBER_OF_CALL_SET_CHARACTER_ON_BLOCK,
 NUMBER_OF_CALL_SET_CH_COLOR_ON_BLOCK,
 NUMBER_OF_CALL_SET_BG_COLOR_ON_BLOCK,
 NUMBER_OF_CALL_SET_COLORS_ON_BLOCK,
 NUMBER_OF_CALL_DRAW_FULL_SQUARE,
 NUMBER_OF_CALL_DRAW_EMPTY_SQUARE,
 
 NUMBER_OF_CALL_WAIT_FOR_KEYBOARD,
 NUMBER_OF_CALL_GET_KEY_VALUE,
 NUMBER_OF_CALL_GET_KEY_UNICODE_VALUE,
 NUMBER_OF_CALL_GET_ACTUAL_KEY_VALUE,
 NUMBER_OF_CALL_GET_ACTUAL_KEY_UNICODE_VALUE,
 NUMBER_OF_CALL_IS_THIS_KEY_PRESSED,

 NUMBER_OF_CALL_CURSOR_LINE_INPUT,
 NUMBER_OF_CALL_GET_INPUT_STRING_POINTER,
 NUMBER_OF_CALL_GET_INPUT_STRING_NUMBER,
 NUMBER_OF_CALL_GET_INPUT_STRING_FLOAT_NUMBER,

 NUMBER_OF_CALL_GET_RANDOM_NUMBER,
 NUMBER_OF_CALL_WAIT,
 NUMBER_OF_CALL_COMPARE_MEMORY,
 NUMBER_OF_CALL_MOVE_EXECUTION_TO_METHOD,
 NUMBER_OF_CALL_EXIT_EXECUTION_OF_METHOD,
};

//key values
#define KEY_ESC 0x01
#define KEY_ENTER 0x1C
#define KEY_TAB 0x0F
#define KEY_BACKSPACE 0x0E
#define KEY_CAPSLOCK 0x3A
#define KEY_ESCAPE 0x01
#define KEY_LEFT_SHIFT 0x2A
#define KEY_RIGHT_SHIFT 0x36
#define KEY_LEFT_ALT 0x38
#define KEY_RIGHT_ALT 0xE038
#define KEY_LEFT_CTRL 0x1D
#define KEY_RIGHT_CTRL 0xE01D
#define KEY_NUMBERLOCK 0x45
#define KEY_SCROLLLOCK 0x46

#define KEY_HOME 0xE047
#define KEY_PAGE_UP 0xE049
#define KEY_PAGE_DOWN 0xE051
#define KEY_END 0xE04F
#define KEY_INSERT 0xE052
#define KEY_DELETE 0xE053
#define KEY_UP 0xE048
#define KEY_DOWN 0xE050
#define KEY_LEFT 0xE04B
#define KEY_RIGHT 0xE04D

#define KEY_F1 0x3B
#define KEY_F2 0x3C
#define KEY_F3 0x3D
#define KEY_F4 0x3E
#define KEY_F5 0x3F
#define KEY_F6 0x40
#define KEY_F7 0x41
#define KEY_F8 0x42
#define KEY_F9 0x43
#define KEY_F10 0x44
#define KEY_F11 0x57
#define KEY_F12 0x58

#define KEY_A 0x1E
#define KEY_B 0x30
#define KEY_C 0x2E
#define KEY_D 0x20
#define KEY_E 0x12
#define KEY_F 0x21
#define KEY_G 0x22
#define KEY_H 0x23
#define KEY_I 0x17
#define KEY_J 0x24
#define KEY_K 0x25
#define KEY_L 0x26
#define KEY_M 0x32
#define KEY_N 0x31
#define KEY_O 0x18
#define KEY_P 0x19
#define KEY_Q 0x10
#define KEY_R 0x13
#define KEY_S 0x1F
#define KEY_T 0x14
#define KEY_U 0x16
#define KEY_V 0x2F
#define KEY_W 0x11
#define KEY_X 0x2D
#define KEY_Y 0x15
#define KEY_Z 0x2C
#define KEY_SPACE 0x39

#define KEY_1 0x02
#define KEY_2 0x03
#define KEY_3 0x04
#define KEY_4 0x05
#define KEY_5 0x06
#define KEY_6 0x07
#define KEY_7 0x08
#define KEY_8 0x09
#define KEY_9 0x0A
#define KEY_0 0x0B

#define KEY_DASH 0x0C
#define KEY_EQUAL 0x0D
#define KEY_LEFT_BRACKET 0x1A
#define KEY_RIGHT_BRACKET 0x1B
#define KEY_BACKSLASH 0x2B
#define KEY_SEMICOLON 0x27
#define KEY_SINGLE_QUOTE 0x28
#define KEY_COMMA 0x33
#define KEY_DOT 0x34
#define KEY_FORWARD_SLASH 0x35
#define KEY_BACK_TICK 0x29

#define KEY_KEYPAD_ASTERISK 0x37
#define KEY_KEYPAD_MINUS 0x4A
#define KEY_KEYPAD_PLUS 0x4E
#define KEY_KEYPAD_DOT 0x53
#define KEY_KEYPAD_ENTER 0xE01C
#define KEY_KEYPAD_FORWARD_SLASH 0xE035
#define KEY_KEYPAD_0 0x52
#define KEY_KEYPAD_1 0x4F
#define KEY_KEYPAD_2 0x50
#define KEY_KEYPAD_3 0x51
#define KEY_KEYPAD_4 0x4B
#define KEY_KEYPAD_5 0x4C
#define KEY_KEYPAD_6 0x4D
#define KEY_KEYPAD_7 0x47
#define KEY_KEYPAD_8 0x48
#define KEY_KEYPAD_9 0x49

#define KEY_POWER_BUTTON 0xE05E
#define KEY_PRINT_SCREEN 0xE037

//methods
#define CALL_SET_PROGRAM_MODE(program_mode) (*system_call)(NUMBER_OF_CALL_SET_PROGRAM_MODE, (unsigned int []){program_mode, BLESKOS_SYSTEM_CALLS_VERSION})

#define CALL_RESET_SCREEN()  (*system_call)(NUMBER_OF_CALL_RESET_SCREEN, 0)
#define CALL_CLEAR_SCREEN(background_color, character_color) (*system_call)(NUMBER_OF_CALL_CLEAR_SCREEN, (unsigned int []){background_color, character_color})
#define CALL_GET_NUMBER_OF_SCREEN_COLUMNS()  (*system_call)(NUMBER_OF_CALL_GET_NUMBER_OF_SCREEN_COLUMNS, 0)
#define CALL_GET_NUMBER_OF_SCREEN_LINES()  (*system_call)(NUMBER_OF_CALL_GET_NUMBER_OF_SCREEN_LINES, 0)
#define CALL_HIDE_CHANGES_ON_SCREEN()  (*system_call)(NUMBER_OF_CALL_HIDE_CHANGES_ON_SCREEN, 0)
#define CALL_REDRAW_SCREEN()  (*system_call)(NUMBER_OF_CALL_REDRAW_SCREEN, 0)
#define CALL_REDRAW_PART_OF_SCREEN(column, line, width, height)  (*system_call)(NUMBER_OF_CALL_REDRAW_PART_OF_SCREEN, (unsigned int []){column, line, width, height})
#define CALL_SHOW_CHANGES_ON_SCREEN()  (*system_call)(NUMBER_OF_CALL_SHOW_CHANGES_ON_SCREEN, 0)

#define CALL_SHOW_CURSOR()  (*system_call)(NUMBER_OF_CALL_SHOW_CURSOR, 0)
#define CALL_HIDE_CURSOR()  (*system_call)(NUMBER_OF_CALL_HIDE_CURSOR, 0)
#define CALL_MOVE_CURSOR(column, line) (*system_call)(NUMBER_OF_CALL_MOVE_CURSOR, (unsigned int []){column, line})
#define CALL_MOVE_CURSOR_TO_COLUMN(column) (*system_call)(NUMBER_OF_CALL_MOVE_CURSOR_TO_COLUMN, (unsigned int []){column})
#define CALL_MOVE_CURSOR_TO_LINE(line) (*system_call)(NUMBER_OF_CALL_MOVE_CURSOR_TO_LINE, (unsigned int []){line})
#define CALL_GET_CURSOR_COLUMN()  (*system_call)(NUMBER_OF_CALL_GET_CURSOR_COLUMN, 0)
#define CALL_GET_CURSOR_LINE()  (*system_call)(NUMBER_OF_CALL_GET_CURSOR_LINE, 0)

#define CALL_SET_PIXEL(column, line, character, ch_color, bg_color) (*system_call)(NUMBER_OF_CALL_SET_PIXEL, (unsigned int []){column, line, character, ch_color, bg_color})
#define CALL_SET_CURSOR_PIXEL_CHARACTER(character) (*system_call)(NUMBER_OF_CALL_SET_CURSOR_PIXEL_CHARACTER, (unsigned int []){character})
#define CALL_SET_CURSOR_PIXEL_CH_COLOR(ch_color) (*system_call)(NUMBER_OF_CALL_SET_CURSOR_PIXEL_CH_COLOR, (unsigned int []){ch_color})
#define CALL_SET_CURSOR_PIXEL_BG_COLOR(bg_color) (*system_call)(NUMBER_OF_CALL_SET_CURSOR_PIXEL_BG_COLOR, (unsigned int []){bg_color})
#define CALL_SET_CURSOR_PIXEL(column, line, character, ch_color, bg_color) (*system_call)(NUMBER_OF_CALL_SET_CURSOR_PIXEL, (unsigned int []){character, ch_color, bg_color})
#define CALL_PUT_CHARACTER_ON_CURSOR(character) (*system_call)(NUMBER_OF_CALL_PUT_CHARACTER_ON_CURSOR, (unsigned int []){character})
#define CALL_PUT_PIXEL_ON_CURSOR(character, ch_color, bg_color) (*system_call)(NUMBER_OF_CALL_PUT_CHARACTER_ON_CURSOR, (unsigned int []){character, ch_color, bg_color})
#define CALL_GET_PIXEL_CHARACTER(column, line) (*system_call)(NUMBER_OF_CALL_GET_PIXEL_CHARACTER, (unsigned int []){column, line})
#define CALL_GET_PIXEL_CH_COLOR(column, line) (*system_call)(NUMBER_OF_CALL_GET_PIXEL_CH_COLOR, (unsigned int []){column, line})
#define CALL_GET_PIXEL_BG_COLOR(column, line) (*system_call)(NUMBER_OF_CALL_GET_PIXEL_BG_COLOR, (unsigned int []){column, line})
#define CALL_GET_CURSOR_PIXEL_CHARACTER() (*system_call)(NUMBER_OF_CALL_GET_CURSOR_PIXEL_CHARACTER, (unsigned int []){})
#define CALL_GET_CURSOR_PIXEL_CH_COLOR() (*system_call)(NUMBER_OF_CALL_GET_CURSOR_PIXEL_CH_COLOR, (unsigned int []){})
#define CALL_GET_CURSOR_PIXEL_BG_COLOR() (*system_call)(NUMBER_OF_CALL_GET_CURSOR_PIXEL_BG_COLOR, (unsigned int []){})

#define CALL_PRINT(column, line, string) (*system_call)(NUMBER_OF_CALL_PRINT, (unsigned int []){column, line, (unsigned int)string})
#define CALL_PRINT_WITH_CH_COLOR(column, line, string, ch_color) (*system_call)(NUMBER_OF_CALL_PRINT_WITH_CH_COLOR, (unsigned int []){column, line, (unsigned int)string, ch_color})
#define CALL_PRINT_WITH_BG_COLOR(column, line, string, bg_color) (*system_call)(NUMBER_OF_CALL_PRINT_WITH_BG_COLOR, (unsigned int []){column, line, (unsigned int)string, bg_color})
#define CALL_PRINT_NUMBER(column, line, number) (*system_call)(NUMBER_OF_CALL_PRINT_NUMBER, (unsigned int []){column, line, number})
#define CALL_PRINT_NUMBER_WITH_CH_COLOR(column, line, number, ch_color) (*system_call)(NUMBER_OF_CALL_PRINT_NUMBER_WITH_CH_COLOR, (unsigned int []){column, line, number, ch_color})
#define CALL_PRINT_NUMBER_WITH_BG_COLOR(column, line, number, bg_color) (*system_call)(NUMBER_OF_CALL_PRINT_NUMBER_WITH_BG_COLOR, (unsigned int []){column, line, number, bg_color})
#define CALL_PRINT_FLOAT_NUMBER(column, line, number) (*system_call)(NUMBER_OF_CALL_PRINT_FLOAT_NUMBER, (unsigned int []){column, line, *(unsigned int*)&(float){number}})
#define CALL_PRINT_FLOAT_NUMBER_WITH_CH_COLOR(column, line, number, ch_color) (*system_call)(NUMBER_OF_CALL_PRINT_FLOAT_NUMBER_WITH_CH_COLOR, (unsigned int []){column, line, *(unsigned int*)&(float){number}, ch_color})
#define CALL_PRINT_FLOAT_NUMBER_WITH_BG_COLOR(column, line, number, bg_color) (*system_call)(NUMBER_OF_CALL_PRINT_FLOAT_NUMBER_WITH_BG_COLOR, (unsigned int []){column, line, *(unsigned int*)&(float){number}, bg_color})
#define CALL_PRINT_HEX_NUMBER(column, line, number, number_of_digits) (*system_call)(NUMBER_OF_CALL_PRINT_HEX_NUMBER, (unsigned int []){column, line, number, number_of_digits})
#define CALL_PRINT_HEX_NUMBER_WITH_CH_COLOR(column, line, number, number_of_digits, ch_color) (*system_call)(NUMBER_OF_CALL_PRINT_HEX_NUMBER_WITH_CH_COLOR, (unsigned int []){column, line, number, number_of_digits, ch_color})
#define CALL_PRINT_HEX_NUMBER_WITH_BG_COLOR(column, line, number, number_of_digits, bg_color) (*system_call)(NUMBER_OF_CALL_PRINT_HEX_NUMBER_WITH_BG_COLOR, (unsigned int []){column, line, number, number_of_digits, bg_color})
#define CALL_PRINT_ON_CURSOR(string) (*system_call)(NUMBER_OF_CALL_PRINT_ON_CURSOR, (unsigned int []){(unsigned int)string})
#define CALL_PRINT_NUMBER_ON_CURSOR(number) (*system_call)(NUMBER_OF_CALL_PRINT_NUMBER_ON_CURSOR, (unsigned int []){number})
#define CALL_PRINT_FLOAT_NUMBER_ON_CURSOR(number) (*system_call)(NUMBER_OF_CALL_PRINT_FLOAT_NUMBER_ON_CURSOR, (unsigned int []){*(unsigned int*)&(float){number}})
#define CALL_PRINT_HEX_NUMBER_ON_CURSOR(number, number_of_digits) (*system_call)(NUMBER_OF_CALL_PRINT_HEX_NUMBER_ON_CURSOR, (unsigned int []){number, number_of_digits})
#define PARAMETER_ALIGN_LEFT 0
#define PARAMETER_ALIGN_CENTER 1
#define PARAMETER_ALIGN_RIGHT 2
#define CALL_PRINT_TO_BLOCK(column, line, width, align, string) (*system_call)(NUMBER_OF_CALL_PRINT_TO_BLOCK, (unsigned int []){column, line, width, align, (unsigned int)string})

#define CALL_SET_CHARACTER_ON_BLOCK(column, line, width, height, character) (*system_call)(NUMBER_OF_CALL_SET_CHARACTER_ON_BLOCK, (unsigned int []){column, line, width, height, character})
#define CALL_SET_CH_COLOR_ON_BLOCK(column, line, width, height, ch_color) (*system_call)(NUMBER_OF_CALL_SET_CH_COLOR_ON_BLOCK, (unsigned int []){column, line, width, height, ch_color})
#define CALL_SET_BG_COLOR_ON_BLOCK(column, line, width, height, bg_color) (*system_call)(NUMBER_OF_CALL_SET_BG_COLOR_ON_BLOCK, (unsigned int []){column, line, width, height, bg_color})
#define CALL_SET_COLORS_ON_BLOCK(column, line, width, height, ch_color, bg_color) (*system_call)(NUMBER_OF_CALL_SET_COLORS_ON_BLOCK, (unsigned int []){column, line, width, height, ch_color, bg_color})
#define CALL_DRAW_FULL_SQUARE(column, line, width, height, color) (*system_call)(NUMBER_OF_CALL_DRAW_FULL_SQUARE, (unsigned int []){column, line, width, height, color})
#define CALL_DRAW_EMPTY_SQUARE(column, line, width, height, color) (*system_call)(NUMBER_OF_CALL_DRAW_EMPTY_SQUARE, (unsigned int []){column, line, width, height, color})

#define CALL_WAIT_FOR_KEYBOARD() (*system_call)(NUMBER_OF_CALL_WAIT_FOR_KEYBOARD, 0)
#define CALL_GET_KEY_VALUE() (*system_call)(NUMBER_OF_CALL_GET_KEY_VALUE, 0)
#define CALL_GET_KEY_UNICODE_VALUE() (*system_call)(NUMBER_OF_CALL_GET_KEY_UNICODE_VALUE, 0)
#define CALL_GET_ACTUAL_KEY_VALUE() (*system_call)(NUMBER_OF_CALL_GET_ACTUAL_KEY_VALUE, 0)
#define CALL_GET_ACTUAL_KEY_UNICODE_VALUE() (*system_call)(NUMBER_OF_CALL_GET_ACTUAL_KEY_UNICODE_VALUE, 0)
#define CALL_IS_THIS_KEY_PRESSED(key_value) (*system_call)(NUMBER_OF_CALL_IS_THIS_KEY_PRESSED, (unsigned int []){key_value})

#define CALL_CURSOR_LINE_INPUT(column, line, number_of_chars) (*system_call)(NUMBER_OF_CALL_CURSOR_LINE_INPUT, (unsigned int []){column, line, number_of_chars})
#define CALL_GET_INPUT_STRING_POINTER() (*system_call)(NUMBER_OF_CALL_GET_INPUT_STRING_POINTER, 0)
#define CALL_GET_INPUT_STRING_NUMBER() (*system_call)(NUMBER_OF_CALL_GET_INPUT_STRING_NUMBER, 0)
#define CALL_GET_INPUT_STRING_FLOAT_NUMBER() (*(float*)&((*system_call)(NUMBER_OF_CALL_GET_INPUT_STRING_FLOAT_NUMBER, 0)))

#define CALL_GET_RANDOM_NUMBER(first_value, last_value, step) (*system_call)(NUMBER_OF_CALL_GET_RANDOM_NUMBER, (unsigned int []){first_value, last_value, step})
#define CALL_WAIT(milliseconds) (*system_call)(NUMBER_OF_CALL_WAIT, (unsigned int []){milliseconds})
#define CALL_COMPARE_MEMORY(memory_1, memory_2, size) (*system_call)(NUMBER_OF_CALL_COMPARE_MEMORY, (unsigned int []){(unsigned int)(memory_1), (unsigned int)(memory_1), size})
#define CALL_MOVE_EXECUTION_TO_METHOD(method, calls_per_second) (*system_call)(NUMBER_OF_CALL_MOVE_EXECUTION_TO_METHOD, (unsigned int []){(unsigned int)(&method), calls_per_second})
#define CALL_EXIT_EXECUTION_OF_METHOD() (*system_call)(NUMBER_OF_CALL_EXIT_EXECUTION_OF_METHOD, 0)