//BleskOS

/*
* MIT License
* Copyright (c) 2023-2025 Vendelín Slezák
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

void initalize_network_connection(void) {
 //initalize all network cards
 initalize_ethernet_cards();

 //initalize network stack
 initalize_network_stack();
 initalize_send_packet_with_response_interface();
 initalize_http_protocol();
 initalize_network_transfers();

 //if there is some connection, start connecting to internet
 internet.status = INTERNET_STATUS_DISCONNECTED;
 internet_last_status = INTERNET_STATUS_DISCONNECTED; //TODO: create better way for wait_for_user_input()
 check_change_in_internet_connection();

 //set task for monitoring change in internet connection
 create_task(check_change_in_internet_connection, TASK_TYPE_PERIODIC_INTERRUPT, 250);
}

void check_change_in_internet_connection(void) {
 //check for changes of connection on ethernet cards
 ethernet_cards_update_cable_status();

 //if we are connected to internet, check if connection was not ended
 if(internet.status==INTERNET_STATUS_CONNECTED || internet.status==INTERNET_STATUS_CONNECTION_ERROR || internet.status==INTERNET_STATUS_CONNECTING) {
  if(internet.connection_type==INTERNET_CONNECTION_ETHERNET && ethernet_cards[internet.connection_device_number].cable_status==ETHERNET_CARD_CABLE_DISCONNECTED) {
   log("\n\nEthernet cable was removed");
   internet.status = INTERNET_STATUS_DISCONNECTED;
  }
 }

 //if we are disconnected from internet, check if there is not some new connection
 if(internet.status==INTERNET_STATUS_DISCONNECTED) {
  //check ethernet cards
  for(dword_t i=0; i<number_of_ethernet_cards; i++) {
   //this ethernet card is connected to something
   if(ethernet_cards[i].cable_status==ETHERNET_CARD_CABLE_CONNECTED) {
    log("\n\nConnecting to internet...");
    internet.status = INTERNET_STATUS_CONNECTING;
    select_ethernet_card_as_internet_connection_device(i);
    dhcp_connect_to_router();
    break;
   }
  }
 }
}

void network_irq(void) {
 //all network cards call this irq, so collision will not happen even if they share same irq
 //we do not know from where irq comes, so we will check all cards
 ethernet_cards_process_irq();

 //if there were some updates, send packets
 send_packets_in_queue();
}