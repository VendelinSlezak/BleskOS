/* 
* BleskOS
*
* MIT License
* Copyright (c) 2023-2026 BleskOS developers
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

enum {
    HARDWARE_TYPE_TIMER = 1,
    HARDWARE_TYPE_LOGGER,
    HARDWARE_TYPE_WINDOW,
    HARDWARE_TYPE_HUMAN_INPUT_DEVICE
};

typedef struct {
    uint32_t id;
    uint32_t type;
} hardware_list_entry_t;

#define MAX_NUMBER_OF_ENTRIES_IN_HARDWARE_LIST 256
typedef struct {
    uint32_t number_of_entries;
    hardware_list_entry_t entries[];
} hardware_list_t;

extern hardware_list_t *hardware_list;
extern uint32_t number_of_unintialized_devices;