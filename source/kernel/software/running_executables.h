/* 
* BleskOS
*
* MIT License
* Copyright (c) 2023-2026 BleskOS developers
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include <kernel/hardware/devices/cpu/scheduler.h>
#include <kernel/hardware/subsystems/screen/screen.h>
#include <kernel/software/spawning_template.h>

#define MAX_NUMBER_OF_SESSIONS 20
typedef struct {
    uint8_t showed_name[16];
    uint32_t does_have_focus;
} session_t;

typedef struct {
    uint8_t *name;
    process_t *process;
    screen_part_t *part;
    spawning_template_t *template;
    uint32_t page_directory_of_virtual_hardware;
    uint32_t is_human_input_streaming_enabled;
    uint32_t are_sessions_enabled;
    uint32_t number_of_sessions;
    session_t *sessions;
} running_executable_t;

typedef struct {
    uint32_t number_of_running_executables;
    running_executable_t *running_executables[];
} running_executable_list_t;

extern running_executable_list_t *running_executable_list;