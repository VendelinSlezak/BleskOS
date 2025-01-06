//BleskOS

/*
* MIT License
* Copyright (c) 2023-2025 Vendelín Slezák
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#define SYSTEM_BOARD_MEMORY 1
#define SYSTEM_BOARD_PCI 2
#define SYSTEM_BOARD_PC_SPEAKER 3
#define SYSTEM_BOARD_GRAPHIC 4
#define SYSTEM_BOARD_ACPI 5
#define SYSTEM_BOARD_HPET_TIMER 6
#define SYSTEM_BOARD_PS2_CONTROLLER 7
#define SYSTEM_BOARD_IDE_HDD 10
#define SYSTEM_BOARD_IDE_CDROM 11
#define SYSTEM_BOARD_AHCI_HDD 12
#define SYSTEM_BOARD_AHCI_CDROM 13

#define SYSTEM_BOARD_PERFORMANCE_RATING 100

dword_t system_board_selected_item = 0, system_board_num_of_items = 0, system_board_draw_line = 0, system_item_variable = 0;
byte_t system_board_items_list[100];

void system_board_print_hardware_item(byte_t *string);
void system_board_redraw(void);
void system_board(void);