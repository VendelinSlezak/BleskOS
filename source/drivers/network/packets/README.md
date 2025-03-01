# Building packets

Code in this folder is written to make building of network packets easier. When you want to send new packet to network, you need to call method `start_building_network_packet()`, then call methods for specific layers of packet, and then call method `finalize_builded_network_packet()`. After this, you have in memory complete packet ready to be sent on network. You can send it to internet by calling methods from source/drivers/network/send_packets.c `send_builded_packet_to_internet()`, if you only want to send this packet and you do not expect response, or `send_builded_packet_with_response_to_internet()` if you expect response from internet.

Here is example of creating TCP handshake:

```
start_building_network_packet();
network_packet_add_ethernet_layer(internet.router_mac);
network_packet_add_ipv4_layer(internet.our_ip, domain_ip);
network_packet_add_tcp_layer(40000, 80, 0, 0, TCP_LAYER_SYN_FLAG);
finalize_builded_network_packet();
```

Here is brief summary of all `network_packet_add_*_layer()` methods:

### void network_packet_add_ethernet_layer(byte_t receiver_mac_address[6])

If you are connected to internet, you can use `internet.router_mac` from source/drivers/network/main.h.

### void network_packet_add_arp_layer(byte_t type, byte_t sender_mac[6], byte_t sender_ip[4], byte_t target_mac[6], byte_t target_ip[4])

Type can be NETWORK_PACKET_ARP_TYPE_REQUEST or NETWORK_PACKET_ARP_TYPE_RESPONSE.

### void network_packet_add_ipv4_layer(byte_t source_ip_address[4], byte_t destination_ip_address[4])

If you are connected to internet, you can use `internet.our_ip` from source/drivers/network/main.h for source_ip_address[4] parameter.

### void network_packet_add_udp_layer(word_t source_port, word_t destination_port)

### void network_packet_add_tcp_layer(word_t source_port, word_t destination_port, dword_t sequence_number, dword_t acknowledgment_number, byte_t control)

To make control byte, you can use following macros: TCP_LAYER_FIN_FLAG, TCP_LAYER_SYN_FLAG, TCP_LAYER_RST_FLAG, TCP_LAYER_PUSH_FLAG, TCP_LAYER_ACK_FLAG.

### void network_packet_add_dhcp_layer(byte_t *options)

Options add values after magic cookie.

### void network_packet_add_dns_layer(word_t id, word_t qtype, byte_t *domain)

Qtype can be NETWORK_PACKET_DNS_QUESTION_TYPE_IPV4.

### void network_packet_add_http_layer(byte_t *url)

This method generates GET request through HTTP 1.1