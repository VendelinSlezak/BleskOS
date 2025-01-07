//BleskOS

/*
* MIT License
* Copyright (c) 2023-2025 Vendelín Slezák
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#define MAX_NUMBER_OF_PACKETS_WITH_RESPONSE 20
struct packet_with_response_interface_t {
 byte_t packet[2048];
 word_t packet_length;
 word_t milliseconds_before_resending;
 dword_t milliseconds_count;
 byte_t max_attempts;
 byte_t response_packet_type;
 word_t response_packet_sender_port;
 word_t response_packet_reciever_port;
 byte_t (*process_response)(byte_t number_of_packet_entry, byte_t *packet_memory, dword_t packet_length);
 void (*process_error)(byte_t number_of_packet_entry, byte_t error_type);
}__attribute__((packed));
struct packet_with_response_interface_t packets_with_response_interface[MAX_NUMBER_OF_PACKETS_WITH_RESPONSE];

//PWRM = PACKET WITH RESPONSE MECHANISM
#define PWRM_NO_FREE_ENTRY MAX_NUMBER_OF_PACKETS_WITH_RESPONSE

#define PWRM_PACKET_TYPE_ARP 1
#define PWRM_PACKET_TYPE_IPV4_TCP 2
#define PWRM_PACKET_TYPE_IPV4_UDP 3

#define PWRM_ERROR_TIMEOUT 1

#define PWRM_CONTINUE_TRANSFER 0
#define PWRM_END_TRANSFER 1

void initalize_send_packet_with_response_interface(void);
void send_builded_packet_to_internet(void);
byte_t send_builded_packet_with_response_to_internet(word_t milliseconds_before_resending, byte_t max_attempts, byte_t response_packet_type, word_t response_packet_sender_port, word_t response_packet_reciever_port, byte_t (*process_response)(byte_t number_of_packet_entry, byte_t *packet_memory, dword_t packet_length), void (*process_error)(byte_t number_of_packet_entry, byte_t error_type));
void remove_packet_entry(byte_t number_of_packet_entry);
void update_packet_entry(byte_t number_of_packet_entry, word_t milliseconds_before_resending, byte_t max_attempts, byte_t response_packet_type, word_t response_packet_sender_port, word_t response_packet_reciever_port, byte_t (*process_response)(byte_t number_of_packet_entry, byte_t *packet_memory, dword_t packet_length), void (*process_error)(byte_t number_of_packet_entry, byte_t error_type));
void send_packets_in_queue(void);