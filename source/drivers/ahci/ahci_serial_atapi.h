//BleskOS

/*
* MIT License
* Copyright (c) 2023-2024 Vendelín Slezák
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

byte_t satapi_test_unit_ready(dword_t port_base_address, dword_t command_list_memory);
byte_t satapi_detect_disk(dword_t port_base_address, dword_t command_list_memory);
byte_t satapi_start_stop_command(dword_t port_base_address, dword_t command_list_memory, byte_t command);
byte_t satapi_read_capabilities(dword_t port_base_address, dword_t command_list_memory);
byte_t satapi_read_cd_toc(dword_t port_base_address, dword_t command_list_memory, dword_t memory);
byte_t satapi_read(dword_t port_base_address, dword_t command_list_memory, dword_t sector, byte_t number_of_sectors, dword_t memory);
byte_t satapi_read_audio_cd_sector(dword_t port_base_address, dword_t command_list_memory, dword_t sector, byte_t number_of_sectors, dword_t memory);