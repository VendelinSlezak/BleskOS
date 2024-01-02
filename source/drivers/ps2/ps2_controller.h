//BleskOS

/*
* MIT License
* Copyright (c) 2023-2024 Vendelín Slezák
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#define PS2_CHANNEL_NO_DEVICE_CONNECTED 0
#define PS2_CHANNEL_KEYBOARD_CONNECTED 0xAB
#define PS2_CHANNEL_KEYBOARD_INITALIZED 0xAC
#define PS2_CHANNEL_MOUSE_CONNECTED 0x03
#define PS2_CHANNEL_MOUSE_INITALIZED 0x04

byte_t ps2_first_channel_buffer[10];
byte_t ps2_second_channel_buffer[10];

byte_t ps2_controller_present = DEVICE_PRESENT, ps2_first_channel_present = DEVICE_NOT_PRESENT, ps2_second_channel_present = DEVICE_NOT_PRESENT;
byte_t ps2_first_channel_device = PS2_CHANNEL_NO_DEVICE_CONNECTED, ps2_second_channel_device = PS2_CHANNEL_NO_DEVICE_CONNECTED;
byte_t ps2_first_channel_buffer_pointer = 0, ps2_second_channel_buffer_pointer = 0;

byte_t read_ps2_data(void);
void write_ps2(byte_t value);
void write_command_ps2(byte_t value);
void write_to_first_ps2_channel(byte_t value);
byte_t ps2_first_channel_wait_for_ack(void);
byte_t ps2_first_channel_wait_for_response(void);
void write_to_second_ps2_channel(byte_t value);
byte_t ps2_second_channel_wait_for_ack(void);
byte_t ps2_second_channel_wait_for_response(void);
void initalize_ps2_controller(void);
void ps2_first_channel_irq_handler(void);
void ps2_second_channel_irq_handler(void);