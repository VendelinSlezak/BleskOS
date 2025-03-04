//BleskOS

/*
* MIT License
* Copyright (c) 2023-2025 BleskOS developers
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#define PATA_EJECT 0x2
#define PATA_SPIN_DOWN 0x0

byte_t patapi_send_packet_command(word_t base_port, word_t alt_base_port, word_t transfer_length);
byte_t patapi_test_unit_ready(word_t base_port, word_t alt_base_port);
byte_t patapi_detect_disk(word_t base_port, word_t alt_base_port);
byte_t patapi_read_capabilities(word_t base_port, word_t alt_base_port);
byte_t patapi_start_stop_command(word_t base_port, word_t alt_base_port, byte_t command);
byte_t patapi_read_cd_toc(word_t base_port, word_t alt_base_port, dword_t memory);
byte_t patapi_read(word_t base_port, word_t alt_base_port, dword_t sector, byte_t number_of_sectors, byte_t *memory);
byte_t patapi_read_audio_cd(word_t base_port, word_t alt_base_port, dword_t sector, byte_t number_of_sectors, byte_t *memory);