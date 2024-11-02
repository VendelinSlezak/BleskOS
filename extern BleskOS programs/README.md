# BleskOS external programs

From v2024u37 BleskOS provide interface for running .bin files that were compiled for BleskOS. This is experimental feature and has several weaknesses:

* You can run only single program at once
* Every program is loaded on 0x10000
* Program can not be larger than 256 KB
* There are no safety restrictions, program have full control over your computer

Therefore you should run only programs that you personally compiled, so you can be sure it will not broke anything.

## How to write program

If you want to write program for BleskOS, you need to download bleskos_system_calls.h file from this folder, include it at top of your program and define main() as follows:

```
void main(unsigned int (*system_call)(unsigned int call, unsigned int args[]))
```

Now you can use all CALL_* macros that are defined in bleskos_system_calls.h as normal methods, macros will rewrite them into correct calling of system_call() method. If you want to have more methods in program and you want to use CALL_ in them, you need to pass `unsigned int (*system_call)(unsigned int call, unsigned int args[])` parameter into them, for example:

```
dword_t method(unsigned int (*system_call)(unsigned int call, unsigned int args[]), dword_t parameter1, dword_t parameter2)

...

void main(unsigned int (*system_call)(unsigned int call, unsigned int args[])) {
 method(system_call, 1, 2);
}
```

As first thing in main() you need to call `CALL_SET_PROGRAM_MODE()`. Attempt to call any other call before setting program mode will result into killing program. Now there is only one program mode - PROGRAM_TEXT_MODE - that will run program in emulation of text mode similar to VGA text mode, but with difference that this mode uses pixels with size 8x10 screen pixels and is on full screen.

So "Hello world!" program looks like this:

```
#include "bleskos_system_calls.h"

void main(unsigned int (*system_call)(unsigned int call, unsigned int args[])) {
 CALL_SET_PROGRAM_MODE(PROGRAM_TEXT_MODE);
 CALL_PRINT_ON_CURSOR("Hello world!");
 CALL_WAIT_FOR_KEYBOARD(); //otherwise program would end immediately after printing on screen
}
```

If you have this program, you need to compile it with these instructions:

```
gcc -m32 -c -fno-pic -ffreestanding name_of_file.c -o name_of_file.o
ld -m elf_i386 --oformat binary -Ttext 0x10000 -e main name_of_file.o -o name_of_file.bin
```

And then you can copy name_of_file.bin to some image, connect it to BleskOS and open by "Run binary files" option from Main screen.

In this folder is one example program pong.c that is compiled on disk.img that you can find in testing/disk.img as pong.bin.

## Calls

This section explains every call in BleskOS system calls version 02/11/2024. Every call returns dword_t value. If not specified otherwise, calls returns either SYSTEM_CALL_SUCCESS, what mean that call was successfull or SYSTEM_CALL_ERROR means that call was not successfull. Every color is in 0x0RGB format. For keyboard values you can use KEY_* macros. 

### CALL_SET_PROGRAM_MODE(program_mode)

This call sets program mode that program is working in. Now there is only one parameter PROGRAM_TEXT_MODE. This call also checks version of system calls that were used to compile this program. If this version is incompatibile with version of BleskOS running this program, it will kill program immediately. This call needs to be called before any other call.

### CALL_RESET_SCREEN()

This call clears screen with black background and white text, and move cursor to pixel 0, 0.

### CALL_CLEAR_SCREEN(background_color, character_color)

This call clears screen with colors.

### CALL_GET_NUMBER_OF_SCREEN_COLUMNS() CALL_GET_NUMBER_OF_SCREEN_LINES()

These calls return number of columns/lines on screen.

### CALL_HIDE_CHANGES_ON_SCREEN()

After this call any changes on screen will not be shown on monitor, unless you call CALL_REDRAW_SCREEN() or CALL_REDRAW_PART_OF_SCREEN(). You can use it to redraw screen without user noticing redrawing process.

### CALL_REDRAW_SCREEN()

This call redraws whole screen on monitor.

### CALL_REDRAW_PART_OF_SCREEN(column, line, width, height)

This call redraw specified part of screen on monitor.

### CALL_SHOW_CHANGES_ON_SCREEN()

After this call any changes on screen will be immediately shown on monitor. This is default mode after starting program.

### CALL_SHOW_CURSOR()

This call shows text mode cursor.

### CALL_HIDE_CURSOR()

This call hides text mode cursor.

### CALL_MOVE_CURSOR(column, line) CALL_MOVE_CURSOR_TO_COLUMN(column) CALL_MOVE_CURSOR_TO_LINE(line)

These calls moves text mode cursor to specified position.

### CALL_GET_CURSOR_COLUMN() CALL_GET_CURSOR_LINE()

These calls return position of cursor.

### CALL_SET_PIXEL(column, line, character, ch_color, bg_color)

This call sets specified pixel on screen.

### CALL_SET_CURSOR_PIXEL_CHARACTER(character) CALL_SET_CURSOR_PIXEL_CH_COLOR(ch_color) CALL_SET_CURSOR_PIXEL_BG_COLOR(bg_color) CALL_SET_CURSOR_PIXEL(column, line, character, ch_color, bg_color)

These calls sets specified property of pixel on which is text mode cursor. If you want to set specified property of any pixel on screen, you can use calls CALL_SET_*_ON_BLOCK with block size 1x1.

### CALL_PUT_CHARACTER_ON_CURSOR(character)

This call prints character on cursor and moves cursor.

### CALL_PUT_PIXEL_ON_CURSOR(character, ch_color, bg_color)

This call prints pixel on cursor and moves cursor.

### CALL_GET_PIXEL_CHARACTER(column, line) CALL_GET_PIXEL_CH_COLOR(column, line) CALL_GET_PIXEL_BG_COLOR(column, line)

These calls returns properties of specified pixel.

### CALL_GET_CURSOR_PIXEL_CHARACTER() CALL_GET_CURSOR_PIXEL_CH_COLOR() CALL_GET_CURSOR_PIXEL_BG_COLOR()

These calls returns properties of text mode cursor pixel.

### CALL_PRINT(column, line, string) CALL_PRINT_WITH_CH_COLOR(column, line, string, ch_color) CALL_PRINT_WITH_BG_COLOR(column, line, string, bg_color)

These calls prints string from specified pixel and you can also specify character or background color of string.

### CALL_PRINT_NUMBER(column, line, number) CALL_PRINT_NUMBER_WITH_CH_COLOR(column, line, number, ch_color) CALL_PRINT_NUMBER_WITH_BG_COLOR(column, line, number, bg_color)

These calls prints integer number from specified pixel and you can also specify character or background color of string.

### CALL_PRINT_FLOAT_NUMBER(column, line, number) CALL_PRINT_FLOAT_NUMBER_WITH_CH_COLOR(column, line, number, ch_color) CALL_PRINT_FLOAT_NUMBER_WITH_BG_COLOR(column, line, number, bg_color)

These calls prints float number from specified pixel and you can also specify character or background color of string.

### CALL_PRINT_HEX_NUMBER(column, line, number, number_of_digits) CALL_PRINT_HEX_NUMBER_WITH_CH_COLOR(column, line, number, number_of_digits, ch_color) CALL_PRINT_HEX_NUMBER_WITH_BG_COLOR(column, line, number, number_of_digits, bg_color)

These calls prints hex number from specified pixel and you can also specify how many characters from number you want to print and also character or background color of string.

### CALL_PRINT_ON_CURSOR(string) CALL_PRINT_NUMBER_ON_CURSOR(number) CALL_PRINT_FLOAT_NUMBER_ON_CURSOR(number) CALL_PRINT_HEX_NUMBER_ON_CURSOR(number, number_of_digits)

These calls prints on cursor.

### CALL_PRINT_TO_BLOCK(column, line, width, align, string)

This call will print and wrap string to specified block on screen and align it (for align parameter use PARAMETER_ALIGN_LEFT/PARAMETER_ALIGN_CENTER/PARAMETER_ALIGN_RIGHT).

### CALL_SET_CHARACTER_ON_BLOCK(column, line, width, height, character) CALL_SET_CH_COLOR_ON_BLOCK(column, line, width, height, ch_color) CALL_SET_BG_COLOR_ON_BLOCK(column, line, width, height, bg_color) CALL_SET_COLORS_ON_BLOCK(column, line, width, height, ch_color, bg_color)

These calls set property on specified square of pixels.

### CALL_DRAW_FULL_SQUARE(column, line, width, height, color) CALL_DRAW_EMPTY_SQUARE(column, line, width, height, color)

These calls draw square, what mean that pixels will have same character and backgroud color and character will be set to space.

### CALL_WAIT_FOR_KEYBOARD()

This call waits until user presses some key.

### CALL_GET_KEY_VALUE() CALL_GET_KEY_UNICODE_VALUE()

These calls returns value/unicode of key that was pressed in last CALL_WAIT_FOR_KEYBOARD().

### CALL_GET_ACTUAL_KEY_VALUE() CALL_GET_ACTUAL_KEY_UNICODE_VALUE()

These calls returns value/unicode of key that is actually pressed.

### CALL_IS_THIS_KEY_PRESSED(key_value)

This call checks if is this key now pressed and returns STATUS_TRUE or STATUS_FALSE.

### CALL_CURSOR_LINE_INPUT(column, line, number_of_chars)

This call provides input on one line from specified position on screen. User will be able to write by text mode cursor into specified space until ESC (SYSTEM_CALL_ERROR will be returned) or ENTER (SYSTEM_CALL_SUCCESS will be returned) is pressed.

### CALL_GET_INPUT_STRING_POINTER()

This call returns start of memory where is saved input from CALL_CURSOR_LINE_INPUT(). Every character is saved on 2 bytes in unicode. String is terminated by 0.

### CALL_GET_INPUT_STRING_NUMBER() CALL_GET_INPUT_STRING_FLOAT_NUMBER()

These calls returns integer/float number that is written at start of string from CALL_CURSOR_LINE_INPUT().

### CALL_GET_RANDOM_NUMBER(first_value, last_value, step)

This call returns random integer number between first_value and last_value, from numbers that are specified by step. For example, if first_value is 1 and last_value is 5 and step is 1, call will choose from 1, 2, 3, 4, 5. If first_value is 10 and last_value is 25 and step is 5, call will choose from 10, 15, 20, 25.

### CALL_WAIT(milliseconds)

This call will wait for specified number of milliseconds. Attempt to wait longer than 5000 milliseconds (5 seconds) will be refused.

### CALL_COMPARE_MEMORY(memory_1, memory_2, size)

This call will compare memory on two pointers, and if it is equal, returns STATUS_TRUE otherwise STATUS_FALSE.

### CALL_MOVE_EXECUTION_TO_METHOD(method, calls_per_second)

This call starts infinite while that will periodically call some method (it must be defined as `void method(unsigned int (*system_call)(unsigned int call, unsigned int args[]))`) specified number of times per second. Maximum number of calls per second is 100. While will continue until called method calls CALL_EXIT_EXECUTION_OF_METHOD(). Only after this will this call return.

This call can be used especially in games, where you can easily just write method that handles redrawing with possible events, and call it periodically by this call to get required FPS.

### CALL_EXIT_EXECUTION_OF_METHOD()

This call can be called in method that is called by CALL_MOVE_EXECUTION_TO_METHOD(). It is signal, that after returning from such method, CALL_MOVE_EXECUTION_TO_METHOD() should be stopped.

You can look at pong.c to see real use of these two calls.