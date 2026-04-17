/* 
* BleskOS
*
* MIT License
* Copyright (c) 2023-2025 BleskOS developers
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include <kernel/hardware/groups/human_input/human_input.h>

typedef struct {
    uint32_t device_id;

    void (*set_receive_function)(void *, void (*)(void *, uint8_t *, uint32_t));
    uint32_t (*send_command)(uint8_t command);
    uint32_t (*send_command_with_payload)(uint8_t command, uint8_t payload);
    uint32_t (*send_command_with_return)(uint8_t command);
    uint32_t (*send_command_with_payload_and_return)(uint8_t command, uint8_t payload);

    uint32_t scancode_set;
    uint32_t *parse_map;
    uint32_t parsing_state;
    
    uint32_t key_state[INPUT_KEY_COUNT];
} ps2_keyboard_t;