//BleskOS

/*
* MIT License
* Copyright (c) 2023-2024 Vendelín Slezák
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#define FILE_TRANSFER_NO_ERROR 0
#define FILE_TRANSFER_ERROR_NO_NETWORK_CONNECTION 1
#define FILE_TRANSFER_ERROR_NETWORK_NOT_REACHABLE 2
#define FILE_TRANSFER_ERROR_DNS_DO_NOT_EXIST 3
#define FILE_TRANSFER_ERROR_CONNECTION_NOT_ESTABILISHED 4
#define FILE_TRANSFER_ERROR_SERVER_NOT_RESPONDING 5
#define FILE_TRANSFER_ERROR_TOO_MANY_REDIRECTIONS 6

dword_t file_tcp_port = 40000;
dword_t file_memory = 0, file_memory_pointer = 0, file_length = 0, file_transferred_length = 0, file_transfer_num_of_redirect = 0;
word_t file_full_url[2048];
byte_t file_transfer_error_type = 0;

dword_t http_transfer_file(byte_t *url);
dword_t https_transfer_file(byte_t *url);
dword_t network_transfer_file_redirect(byte_t *url);
dword_t network_transfer_file(byte_t *url);