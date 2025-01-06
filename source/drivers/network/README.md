# Network drivers

This folder contains all code that is connected to network.

During boot process, method scan_pci() from source/drivers/system/pci.c founds all network cards and saves informations about them. Ethernet cards are saved to structure `ethernet_cards[]` from source/drivers/network/ethernet/main.h. Later during boot process is called method `initalize_network_connection()` from main.c. This method initalizes everything that is need to use network. After this method you can start use internet connection.

From point of higher layers of code, you need to know about structure `internet` from main.h. If `internet.status` equals INTERNET_STATUS_DISCONNECTED. it means that BleskOS is not currently connected to internet, if `internet.status` equals INTERNET_STATUS_CONNECTING, it means that BleskOS is in process of connecting and if INTERNET_STATUS_CONNECTED, it means that BleskOS is connected to internet. BleskOS will automatically connect/disconnect to internet. And if you are connected to internet, you can download files from URL by method `download_file_from_url(byte_t *url)`. This method will return number of transfer, or NETWORK_TRANSFER_ERROR_NO_FREE_ENTRY. Transfer will immediately start on background. There can be maximum of 5 transfers at one time. It is on you to find out that transfer is completed and close it.

To this you have following methods:

```
byte_t get_status_of_network_transfer(byte_t transfer_number)
byte_t *get_file_memory_of_network_transfer(byte_t transfer_number)
dword_t get_file_size_of_network_transfer(byte_t transfer_number)
void close_network_transfer(byte_t transfer_number)
void kill_network_transfer(byte_t transfer_number)
```

Method `get_status_of_network_transfer()` returns actual status of transfer. It can be NETWORK_TRANSFER_TRANSFERRING_DATA, NETWORK_TRANSFER_ERROR or NETWORK_TRANSFER_DONE. If you want to stop transfer in status NETWORK_TRANSFER_TRANSFERRING_DATA, you have to use `kill_network_transfer()`. It will immediately close connection and remove entry. If is transfer in state  NETWORK_TRANSFER_DONE, methods `get_file_memory_of_network_transfer()` and `get_file_size_of_network_transfer()` will return informations about downloaded file from url. When you processed file, or transfer ended in NETWORK_TRANSFER_ERROR and you are done with this transfer, you should call method `close_network_transfer()` that unloads everything from memory. It also unloads file that is on `get_file_memory_of_network_transfer()`, so if you want to use it even after closing connection, you have to copy it before closing transfer.

Example:


```
if(internet.status != INTERNET_STATUS_CONNECTED) {
 return;
}

byte_t transfer_number = download_file_from_url("example.com");
if(transfer_number == NETWORK_TRANSFER_ERROR_NO_FREE_ENTRY) {
 log("Too much files are currently being transferred!");
}

while(get_status_of_network_transfer(transfer_number) == NETWORK_TRANSFER_TRANSFERRING_DATA) {
 wait(1);
}

if(get_status_of_network_transfer(transfer_number) == NETWORK_TRANSFER_DONE) {
 log("Transfer of file from example.com was successfull");
 byte_t *file_memory = get_file_memory_of_network_transfer(transfer_number);
 dword_t file_size = get_file_size_of_network_transfer(transfer_number);

 // processing file ...

}
else if(get_status_of_network_transfer(transfer_number) == NETWORK_TRANSFER_ERROR) {
 log("There was error during transfer");
}

close_network_transfer(transfer_number);
```

## main.c

### void initalize_network_connection(void)

This method is called during boot. It initalizes everything about network:

* Calls method `initalize_ethernet_cards()` from ethernet/main.c that initalizes all ethernet cards that are in structure `ethernet_cards[]`
* Calls method `initalize_network_stack()` from packets/main.c that allocate memory needed for building packets
* Calls method `initalize_send_packet_with_response_interface()` from send_packets.c that prepare interface for sending packets that will generate some response from router
* Sets `internet.status` to INTERNET_STATUS_DISCONNECTED because now we are not connected to internet
* Calls method `check_change_in_internet_connection()` from same file that checks if there is any change in internet connection and if is, it tries to connect to internet
* Creates task that will every 250 milliseconds call method `check_change_in_internet_connection()` to check actual state of internet connection

### void check_change_in_internet_connection(void)

This method at first updates connection status of all ethernet cards and then if BleskOS is connected to internet checks, if this connection was not disconnected, and if BleskOS is disconnected from internet, it checks if some card is not connected, so it can initiate connection process by calling DHCP protocol.

### void network_irq(void)

Every network card IRQ is connected to this method. It is because it is possible that more cards share same IRQ. When it is called, it calls all IRQ routines of cards so it is guaranteed to process all interrupt conditions. TODO: rewrite global interrupt methods so any device can share IRQ with any other device

### internet.send_packet(byte_t *packet_memory, dword_t packet_size)

If internet.status equals INTERNET_STATUS_CONNECTED, by this method you can send packet to internet.

## network_transfers.c

This file connects request for transfers with all other code. Currently it supports only URL requests for HTTP/HTTPS transfers.

### void initalize_network_transfers(void)

This method initalized network transfers interface.

### byte_t download_file_from_url(byte_t *url)

This method starts transfer from given url. It returns either NETWORK_TRANSFER_ERROR_NO_FREE_ENTRY or number of transfer. Informations about transfers are saved in struct `network_transfer_info_t network_transfers[]` from file network_transfers.h. There are more possible types of transfers, and they manages themselves in other structures. This structure therefore contains only type of transfer (network_transfers->type_of_transfer like NETWORK_TRANSFER_TYPE_HTTP) and number of this transfer in its structure (network_transfers->number_of_transfer_entry).

For example, transfer_number can be 2, `network_transfers[transfer_number].type_of_transfer` is NETWORK_TRANSFER_TYPE_HTTP and `network_transfers[transfer_number].number_of_transfer_entry` is 4. In this case, this network transfer refers to entry `http_file_transfers[4]` from protocols/http.h.

### byte_t download_file_from_url_by_http_s_protocols(byte_t *url)

This method directly calls methods from folder *protocols* to start transfer of HTTP or HTTPS protocol, and returns number of transfer entry of HTTP structure `http_file_transfers[]` from protocols/http.h. It is because HTTPS URLs are downloaded by workaround through HTTP protocol. This method assumes that (byte_t *url) is pointing to valid URL address.

### byte_t get_number_of_network_transfer_from_type_of_transfer(byte_t type_of_transfer, byte_t number_of_transfer_entry)

This method return number of `network_transfers[]` entry based on type of transfer and transfer entry. For example if on `network_transfers[2]` is type_of_transfer NETWORK_TRANSFER_TYPE_HTTP and number_of_transfer_entry 4, then calling `get_number_of_network_transfer_from_type_of_transfer(NETWORK_TRANSFER_TYPE_HTTP, 4)` will return 2. If no network transfer with givn parameters exist, this method returns NETWORK_TRANSFER_NO_ENTRY_FOUNDED.

### byte_t get_status_of_network_transfer(byte_t transfer_number)

This method returns status of given transfer number. Currently it can return NETWORK_TRANSFER_INVALID_TRANSFER_NUMBER, NETWORK_TRANSFER_TRANSFERRING_DATA, NETWORK_TRANSFER_ERROR or NETWORK_TRANSFER_DONE.

### byte_t *get_file_memory_of_network_transfer(byte_t transfer_number)

This method returns pointer to downloaded file from URL. If entry do not exist, or is not fully transfered, it returns NETWORK_TRANSFER_INVALID_TRANSFER_NUMBER.

### dword_t get_file_size_of_network_transfer(byte_t transfer_number)

This method returns size of downloaded file from URL. If entry do not exist, or is not fully transfered, it returns NETWORK_TRANSFER_INVALID_TRANSFER_NUMBER.

### void close_network_transfer(byte_t transfer_number)

This method releases all memory connected with this network transfer.

### void kill_network_transfer(byte_t transfer_number)

This method immediately closes connection with server and releses all memory connected with this network transfer.

## send_packets.c

This file contains methods for sending packets to internet. It contains interface for sending packets which expects response. This interface has shortcut PWRM = Packet With Response Mechanism. Example of working with PWRM is in readme of *packets* folder.

### void initalize_send_packet_with_response_interface(void)

This method initalizes PWRM.

### void send_builded_packet_to_internet(void)

This method sends packet that was builded by methods from folder *packets* to internet.
 
### byte_t send_builded_packet_with_response_to_internet(word_t milliseconds_before_resending, byte_t max_attempts, byte_t response_packet_type, word_t response_packet_sender_port, word_t response_packet_reciever_port, byte_t process_response(byte_t number_of_packet_entry, byte_t *packet_memory, dword_t packet_length), byte_t process_error(byte_t number_of_packet_entry, byte_t error_type))

This method sends packet that was builded by methods from folder *packets* to internet and adds PWRM entry to monitor response. Method returns either PWRM_NO_FREE_ENTRY, in which case no packet is sended, or number of PWRM entry.

If response do not come after milliseconds_before_resending milliseconds, PWRM will automatically resend it again. It is done by method `send_packets_in_queue()` that is as task called every millisecond to check for packets waiting to be transferred. Maximal number of same packet sended is defined in max_attempts. If after max_attempts there still is no response, PWRM calls `process_error(PWRM_ERROR_TIMEOUT)` method, so host can respond to error, and deletes this PWRM entry.

Response is specified by response_packet_type that can be PWRM_PACKET_TYPE_ARP, PWRM_PACKET_TYPE_IPV4_TCP or PWRM_PACKET_TYPE_IPV4_UDP. In case of TCP or UDP transfer, response packet is further specified by response_packet_sender_port and response_packet_reciever_port. If packet matches, it is pushed to method `process_response()`. packet_memory and packet_length contains pointer inside of requested layer. So for example for PWRM_PACKET_TYPE_IPV4_UDP will `process_response(packet_memory)` point to start of UDP layer. PWRM_PACKET_TYPE_IPV4_TCP also saves pointer to TCP layer in global variable `struct network_packet_tcp_layer_t *full_packet_tcp_layer` from receive_packets.h. Method `process_response()` is called from method `process_packet()` from receive_packets.c. If `process_response()` returns PWRM_END_TRANSFER, this PWRM entry will be immediately deleted. If it returns PWRM_CONTINUE_TRANSFER, this entry will remain, because it was either updated to expect new packet, or expected packet contained some error, so we need to wait further, if packet witout error will arrive.

Parameter number_of_packet_entry sends number of PWRM entry to `process_response()` or `process_error()` methods.

### void remove_packet_entry(byte_t number_of_packet_entry)

This method deletes given PWRM entry.

### void update_packet_entry(byte_t number_of_packet_entry, word_t milliseconds_before_resending, byte_t max_attempts, byte_t response_packet_type, word_t response_packet_sender_port, word_t response_packet_reciever_port, byte_t process_response(byte_t number_of_packet_entry, byte_t *packet_memory, dword_t packet_length), byte_t process_error(byte_t number_of_packet_entry, byte_t error_type));

This method updates given PWRM entry. In its core it is same method as `send_builded_packet_with_response_to_internet()`, however you specify PWRM number. Also after update when is send_packets_in_queue() called, it immediately sends new packet from this entry. This method is used inside `process_response()` method, if new packet needs to be sended and new response is expected.

Example can be DHCP protocol. At first, host send DHCP Discover packet by `send_builded_packet_with_response_to_internet()` and expects DHCP Offer. When DHCP Offer arrives, method `process_response()` updates PWRM entry by this method to send DHCP Response and waits for DHCP Acknowledge and returns PWRM_CONTINUE_TRANSFER. When DHCP Acknowledge arrives, `process_response()` returns PWRM_END_TRANSFER to delete PWRM entry, because transfer was successfully finished.

It is very important to note, that unlike `send_builded_packet_with_response_to_internet()`, this method do not immediately sends updated packet to network, only after all packets that were received from internet in this interrupt are processed. It is because for example TCP protocol needs to send ACK packet only with number of last byte received, so we need to send it only after all arrived packets are processed. PWRM interface makes this easy to implement, because in `process_response()`, you need to update PWRM entry only to ACK packet with last number, and you are sure that only last updated packet will be really sended. This is done in particular driver of network card, which interrupt method calls `process_packet()` that calls `process_response()` for every received packet, and then after interrupt method, `network_irq()` from main.c calls `send_packets_in_queue()` that sends all packets from all updated entries.

### void send_packets_in_queue(void)

When this method is called, it sends on internet all PWRM entries that needs to be resended and also all PWRM entries that were updated. It also calls `process_error(PWRM_ERROR_TIMEOUT)` if some expected packet is not arriving.

## receive_packets.c

### void process_packet(byte_t *packet_memory, dword_t packet_size)

Every packet that is received from internet is given to this method that parse it, if it is ARP packet, it sends response and otherwise it checks if some entry from `packets_with_response_interface[]` waits for this packet, and if yes, it calls method `packets_with_response_interface[].process_response()`.

## Folder Ethernet

This folder contains all drivers of ethernet cards. Check readme of this folder for more informations.

## Folder Packets

This folder contains code for building packets in memory and parsing received packets. Check readme of this folder for more informations.

## Folder Protocols

This folder contains code for communicating in concrete internet protocols. Check readme of this folder for more informations.