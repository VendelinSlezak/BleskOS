//BleskOS

/*
* MIT License
* Copyright (c) 2023-2025 BleskOS developers
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#define NETWORK_PACKET_HTTP_LAYER 0x0801

#define HTTP_STATUS_TYPE_INFORMATIONAL 1
#define HTTP_STATUS_TYPE_SUCCESS 2
#define HTTP_STATUS_TYPE_REDIRECTION 3
#define HTTP_STATUS_TYPE_CLIENT_ERROR 4
#define HTTP_STATUS_TYPE_SERVER_ERROR 5

#define NO_TRANSFER_ENCODING 0
#define TRANSFER_ENCODING_CHUNKED 1
#define UNSUPPORTED_TRANSFER_ENCODING 2
struct http_info_t {
 byte_t status_type;
 word_t status_code;
 dword_t content_length;
 byte_t transfer_encoding_type;
 byte_t *location_data_pointer;
 dword_t location_data_size;
 byte_t *body_data_pointer;
}__attribute__((packed)) http_info;

byte_t *network_packet_http_layer;

dword_t network_packet_size_of_http_layer;

void network_packet_add_string_to_http_layer(byte_t *string);
void network_packet_add_bytes_to_http_layer(byte_t *mem, dword_t size);
void network_packet_add_http_layer(byte_t *url);
byte_t parse_http_layer(byte_t *http_layer_pointer, dword_t http_layer_size);