/* 
* BleskOS
*
* MIT License
* Copyright (c) 2023-2026 BleskOS developers
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include <syslib.h>
#include <userspace_library/main.h>
#include <userspace_library/logging.h>
#include <userspace_library/time.h>
#include <userspace_library/bleskalloc.h>
#include <userspace_library/screen.h>
#include <userspace_library/bitmap_font.h>
#include <userspace_library/simple_gui.h>

const syslib_t syslib_got = {
    &log,

    &get_time_in_microseconds,

    &malloc,
    &aligned_malloc,
    &calloc,
    &realloc,
    &free,
    
    &event_loop,
    &update_screen,
    &draw_window_background_color,
    &draw_char_on_screen,
    &add_key_event,
};