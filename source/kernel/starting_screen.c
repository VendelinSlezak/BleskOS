/* 
* BleskOS
*
* MIT License
* Copyright (c) 2023-2026 BleskOS developers
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

/* includes */
#include <kernel/hardware/groups/graphic_output/graphic_output.h>
#include <kernel/hardware/groups/logging/logging.h>
#include <kernel/memory/memory_allocators.h>
#include <kernel/hardware/main.h>

/* functions */
void show_starting_screen(void) {
    // starting screen will be shown only if there is at least one graphic output device
    if(is_there_graphic_output_device == false) {
        return;
    }

    // wait for at least one device to be initialized
    while(get_number_of_graphic_output_devices() == 0) {
        asm volatile("pause");
    }

    // draw starting screen
    uint32_t *buffer = kalloc(get_size_of_double_buffer());
    for(uint32_t i = 0; i < get_size_of_double_buffer() / 4; i++) {
        buffer[i] = 0x888888;
    }
    // TODO: add logo
    redraw_full_screen(buffer);

    // wait for all devices to be initialized
    while(number_of_unintialized_devices != 0) {
        asm volatile("pause");
    }

    // close starting screen
    kfree(buffer);
}