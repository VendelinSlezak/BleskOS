//BleskOS

/*
* MIT License
* Copyright (c) 2023-2025 BleskOS developers
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#define NETWORK_TRANSFER_FREE_ENTRY 0
#define NETWORK_TRANSFER_USED_ENTRY 1

#define NETWORK_TRANSFER_ERROR_INVALID_URL 0xFF
#define NETWORK_TRANSFER_INVALID_TRANSFER_NUMBER 0
#define NETWORK_TRANSFER_TRANSFERRING_DATA 1
#define NETWORK_TRANSFER_ERROR 2
#define NETWORK_TRANSFER_DONE 3

#define NETWORK_TRANSFER_TYPE_HTTP 1

#define MAX_NUMBER_OF_NETWORK_TRANSFERS 10
struct network_transfer_info_t {
 byte_t status;
 byte_t type_of_transfer;
 byte_t number_of_transfer_entry;
 byte_t max_number_of_redirections;
 byte_t url[MAX_LENGTH_OF_URL];
}__attribute__((packed));
struct network_transfer_info_t network_transfers[MAX_NUMBER_OF_NETWORK_TRANSFERS];

#define NETWORK_TRANSFER_ERROR_NO_FREE_ENTRY MAX_NUMBER_OF_NETWORK_TRANSFERS
#define NETWORK_TRANSFER_NO_ENTRY_FOUNDED MAX_NUMBER_OF_NETWORK_TRANSFERS

void initalize_network_transfers(void);
byte_t download_file_from_url(byte_t *url);
byte_t download_file_from_url_by_http_s_protocols(byte_t *url);
byte_t get_number_of_network_transfer_from_type_of_transfer(byte_t type_of_transfer, byte_t number_of_transfer_entry);
byte_t get_status_of_network_transfer(byte_t transfer_number);
byte_t *get_file_memory_of_network_transfer(byte_t transfer_number);
dword_t get_file_size_of_network_transfer(byte_t transfer_number);
dword_t get_transferred_file_size_of_network_transfer(byte_t transfer_number);
void close_network_transfer(byte_t transfer_number);
void kill_network_transfer(byte_t transfer_number);