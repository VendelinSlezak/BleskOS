//BleskOS

/*
* MIT License
* Copyright (c) 2023-2025 Vendelín Slezák
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#define HFT_STATUS_FREE_ENTRY 0
#define HFT_STATUS_DNS_REQUEST 1
#define HFT_STATUS_TCP_TRANSFER 2
#define HFT_STATUS_ERROR 3
#define HFT_STATUS_DONE 4

#define MAX_NUMBER_OF_SIMULTANOUS_NETWORK_FILE_TRANSFERS 5
struct http_file_transfer_t {
 byte_t status;
 dword_t starting_time;
 byte_t url[MAX_LENGTH_OF_URL];
 byte_t pwrm_entry;

 byte_t domain_ip[4];

 word_t ouc_tcp_port;
 dword_t sequence_number;
 dword_t acknowledgment_number;

 struct byte_stream_descriptor_t *data;
 byte_t *file_memory;
 dword_t file_size;
}__attribute__((packed));
struct http_file_transfer_t http_file_transfers[MAX_NUMBER_OF_SIMULTANOUS_NETWORK_FILE_TRANSFERS];

#define HTTP_ERROR_NO_FREE_ENTRIES MAX_NUMBER_OF_SIMULTANOUS_NETWORK_FILE_TRANSFERS
#define HFT_ENTRY_NOT_FOUND MAX_NUMBER_OF_SIMULTANOUS_NETWORK_FILE_TRANSFERS

dword_t free_tcp_port_for_http_transfer = 40000;

void initalize_http_protocol(void);
byte_t get_number_of_hft_entry_from_pwrm_entry(byte_t pwrm_entry);

byte_t http_download_file_from_url(byte_t *url);
byte_t dns_reply_received(byte_t number_of_packet_entry, byte_t *packet_memory, dword_t packet_length);
byte_t tcp_server_handshake_received(byte_t number_of_packet_entry, byte_t *packet_memory, dword_t packet_length);
byte_t tcp_http_data_received(byte_t number_of_packet_entry, byte_t *packet_memory, dword_t packet_length);
byte_t tcp_fin_received(byte_t number_of_packet_entry, byte_t *packet_memory, dword_t packet_length);

byte_t dns_request_error(byte_t number_of_packet_entry, byte_t error);
byte_t tcp_handshake_error(byte_t number_of_packet_entry, byte_t error);
byte_t tcp_http_error(byte_t number_of_packet_entry, byte_t error);
byte_t tcp_fin_error(byte_t number_of_packet_entry, byte_t error);

byte_t get_hft_status(dword_t hft_entry);
byte_t *get_hft_file_memory(dword_t hft_entry);
dword_t get_hft_file_size(dword_t hft_entry);
void hft_close_entry(dword_t hft_entry);
void hft_kill_entry_transfer(dword_t hft_entry);