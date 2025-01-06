//BleskOS

/*
* MIT License
* Copyright (c) 2023-2025 Vendelín Slezák
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

struct layout_of_network_packet_t {
 word_t type_of_layer;
 word_t size_of_layer;
 void (*finalize_layer)(dword_t type_of_next_layer, dword_t size_of_layer);
}__attribute__((packed));
dword_t number_of_layers_in_network_packet;
struct layout_of_network_packet_t layout_of_network_packet[10];

#define MAX_NUMBER_OF_PACKETS_WITH_RESPONSE 10
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
 byte_t (*process_error)(byte_t error_type);
}__attribute__((packed));
struct packet_with_response_interface_t packets_with_response_interface[MAX_NUMBER_OF_PACKETS_WITH_RESPONSE];

//PWRM = PACKET WITH RESPONSE MECHANISM
#define PWRM_NO_FREE_ENTRY MAX_NUMBER_OF_PACKETS_WITH_RESPONSE

#define PWRM_PACKET_TYPE_ARP 1
#define PWRM_PACKET_TYPE_IPV4_TCP 2
#define PWRM_PACKET_TYPE_IPV4_UDP 3

#define PWRM_ERROR_TIMEOUT 1

byte_t *memory_for_building_network_packet;
dword_t size_of_network_packet;

void initalize_network_stack_new(void);
void start_building_network_packet(void);
void network_packet_save_layer(word_t type_of_layer, dword_t size_of_layer, void (*finalize_layer)(dword_t type_of_next_layer, dword_t size_of_layer));
void finalize_builded_network_packet(void);
void send_network_packet(void);

byte_t send_packet_with_response(word_t milliseconds_before_resending, byte_t max_attempts, byte_t response_packet_type, word_t response_packet_sender_port, word_t response_packet_reciever_port, byte_t (*process_response)(byte_t number_of_packet_entry, byte_t *packet_memory, dword_t packet_length), byte_t (*process_error)(byte_t error_type));
void process_packet(byte_t *packet_memory, dword_t packet_size);
void remove_packet_entry(byte_t number_of_packet_entry);
void update_packet_entry(byte_t number_of_packet_entry, word_t milliseconds_before_resending, byte_t max_attempts, byte_t response_packet_type, word_t response_packet_sender_port, word_t response_packet_reciever_port, byte_t process_response(byte_t number_of_packet_entry, byte_t *packet_memory, dword_t packet_length), byte_t process_error(byte_t error_type));
void send_packets_in_queue(void);