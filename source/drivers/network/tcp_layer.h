//BleskOS

/*
* MIT License
* Copyright (c) 2023-2024 Vendelín Slezák
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#define TCP_PROTOCOL_TYPE 6
#define TCP_LAYER_LENGTH 24

#define TCP_NO_CONNECTION 0
#define TCP_HANDSHAKE_SENDED 1
#define TCP_HANDSHAKE_COMPLETE 2
#define TCP_HTTP_REQUEST 3
#define TCP_HTTP_TRANSFERRING_FILE 4
#define TCP_CLOSING_CONNECTION 5
#define TCP_CONNECTION_CLOSED 6

byte_t tcp_connection_status, tcp_new_packet;
word_t tcp_packet_port;
dword_t seq_number, next_seq_number, already_sended_seq, ack_number;

void create_tcp_layer(dword_t memory, word_t source_port, dword_t sequence_number, dword_t acknowledge_number, byte_t control);
void calculate_tcp_checksum(dword_t memory, word_t length_of_data);
void read_tcp_seq_and_ack_numbers(dword_t memory);
void send_tcp_handshake(dword_t ip, word_t source_port);
void send_tcp_acknowledge(dword_t ip, word_t source_port, dword_t sequence_number, dword_t acknowledge_number);
void send_tcp_finalize(dword_t ip, word_t source_port, dword_t sequence_number, dword_t acknowledge_number);
void send_tcp_reset(dword_t ip, word_t source_port, dword_t sequence_number, dword_t acknowledge_number);
