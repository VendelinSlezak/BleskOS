//BleskOS

/*
* MIT License
* Copyright (c) 2023-2025 Vendelín Slezák
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#define HTTP_RESPONSE_200_OK 0x30303220
#define HTTP_RESPONSE_301_MOVED_PERMANENTLY 0x31303320
#define HTTP_RESPONSE_302_MOVED_TEMPORARILY 0x32303320

byte_t http_url_new_location[2048];
dword_t http_response_type, http_data_start_mem, http_data_in_packet_length, http_content_length;

void send_http_get(byte_t *url, dword_t ip, word_t source_port);
void process_http_reply(dword_t memory, dword_t end_of_memory);
