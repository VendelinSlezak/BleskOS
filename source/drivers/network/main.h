//BleskOS

/*
* MIT License
* Copyright (c) 2023-2025 Vendelín Slezák
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#define MAX_SIZE_OF_ONE_PACKET 2048
#define MAX_LENGTH_OF_URL 2048

#define INTERNET_STATUS_DISCONNECTED 0
#define INTERNET_STATUS_CONNECTING 1
#define INTERNET_STATUS_CONNECTED 2
#define INTERNET_CONNECTION_ETHERNET 1
struct internet_status_t {
 byte_t status;

 byte_t connection_type;
 byte_t connection_device_number;
 byte_t (*send_packet)(byte_t *packet_memory, dword_t packet_size);

 byte_t our_mac[6];
 byte_t router_mac[6];

 byte_t our_ip[4];
 byte_t router_ip[4];
 byte_t dns_server_ip[4];
}__attribute__((packed));
struct internet_status_t internet;

void initalize_network_connection(void);
void check_change_in_internet_connection(void);
void network_irq(void);