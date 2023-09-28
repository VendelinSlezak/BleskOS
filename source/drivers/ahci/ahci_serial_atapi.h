//BleskOS

/*
* MIT License
* Copyright (c) 2023 Vendelín Slezák
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

byte_t satapi_send_command(dword_t base_port, dword_t commands_memory, dword_t fis_memory, dword_t transfer_length, dword_t cmd1, dword_t cmd2, dword_t cmd3, dword_t memory);
byte_t satapi_eject_drive(dword_t base_port, dword_t commands_memory, dword_t fis_memory);
byte_t satapi_detect_disk(dword_t base_port, dword_t commands_memory, dword_t fis_memory);
byte_t satapi_read_drive_capabilites(dword_t base_port, dword_t commands_memory, dword_t fis_memory);
byte_t satapi_read_sector(dword_t base_port, dword_t commands_memory, dword_t fis_memory, dword_t sector, dword_t memory);