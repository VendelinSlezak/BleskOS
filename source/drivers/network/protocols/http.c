//BleskOS

/*
* MIT License
* Copyright (c) 2023-2025 Vendelín Slezák
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

void initalize_http_protocol(void) {
 free_tcp_port_for_http_transfer = 40000;
 clear_memory((dword_t)&http_file_transfers, sizeof(http_file_transfers));
}

byte_t get_number_of_hft_entry_from_pwrm_entry(byte_t pwrm_entry) {
 //find which HTTP file transfer entry is connected with this PWRM entry
 for(dword_t i=0; i<MAX_NUMBER_OF_SIMULTANOUS_HTTP_TRANSFERS; i++) {
  if(http_file_transfers[i].pwrm_entry == pwrm_entry) {
   return i;
  }
 }

 //this PWRM entry is not connected with any HTTP file transfer entry
 return HFT_ENTRY_NOT_FOUND;
}

byte_t http_download_file_from_url(byte_t *url) {
 //find free entry
 for(dword_t i=0; i<MAX_NUMBER_OF_SIMULTANOUS_HTTP_TRANSFERS; i++) {
  if(http_file_transfers[i].status == HFT_STATUS_FREE_ENTRY) {
   //clear entry
   clear_memory((dword_t)&http_file_transfers[i], sizeof(struct http_file_transfer_t));

   //save url
   dword_t pointer = 0;
   while(pointer<(MAX_LENGTH_OF_URL-1) && url[pointer]!=0) {
    //copy char
    http_file_transfers[i].url[pointer] = url[pointer];

    //move to next char
    pointer++;
   }

   //copy only domain name from url, this method is always called without "http://" prefix
   byte_t domain_name[MAX_LENGTH_OF_URL];
   dword_t length_of_domain_name = 0;
   while(url[length_of_domain_name]!=0 && length_of_domain_name<(MAX_LENGTH_OF_URL-1)) {
    if(url[length_of_domain_name]=='/') {
     break;
    }
    else {
     length_of_domain_name++;
    }
   }
   copy_memory((dword_t)url, (dword_t)&domain_name, length_of_domain_name);
   domain_name[length_of_domain_name] = 0;

   //create DNS request packet
   start_building_network_packet();
   network_packet_add_ethernet_layer(internet.router_mac);
   network_packet_add_ipv4_layer(internet.our_ip, internet.dns_server_ip);
   network_packet_add_udp_layer(40000+i, 53);
   network_packet_add_dns_layer(0x1234, NETWORK_PACKET_DNS_QUESTION_TYPE_IPV4, domain_name);
   finalize_builded_network_packet();

   //send DNS request packet and dns_reply_received() or dns_request_error() will be called based on response
   http_file_transfers[i].pwrm_entry = send_builded_packet_with_response_to_internet(2000, 4, PWRM_PACKET_TYPE_IPV4_UDP, 53, 40000+i, dns_reply_received, dns_request_error);
   if(http_file_transfers[i].pwrm_entry == PWRM_NO_FREE_ENTRY) {
    return HTTP_ERROR_NO_FREE_ENTRIES;
   }

   //update entry
   http_file_transfers[i].starting_time = time_of_system_running;
   http_file_transfers[i].status = HFT_STATUS_DNS_REQUEST;
   http_file_transfers[i].data = create_byte_stream(BYTE_STREAM_1_MB_BLOCK);
   http_file_transfers[i].http_header_received = STATUS_FALSE;
   //other variables are zeroes

   //return number of file transfer in list
   return i;
  }
 }

 //this means there are no free entries
 return HTTP_ERROR_NO_FREE_ENTRIES;
}

byte_t dns_reply_received(byte_t number_of_packet_entry, byte_t *packet_memory, dword_t packet_length) {
 //get number of HTTP file transfer entry associated with this packet
 byte_t hft_entry = get_number_of_hft_entry_from_pwrm_entry(number_of_packet_entry);
 if(hft_entry==HFT_ENTRY_NOT_FOUND) {
  return PWRM_END_TRANSFER;
 }

 //parse DNS reply
 if(parse_dns_reply(packet_memory, packet_length)==STATUS_ERROR) {
  log("\nDNS parse error");
  return PWRM_CONTINUE_TRANSFER;
 }

 //check if this is valid ID of this transfer
 if(dns_info.packet_id != 0x1234) {
  log("\nDNS ID error");
  return PWRM_CONTINUE_TRANSFER;
 }

 //check if this DNS reply has IP address
 if(dns_info.reply != DNS_REPLY_GOOD) {
  if(dns_info.reply == DNS_REPLY_DOMAIN_DO_NOT_EXIST) {
   log("\nDomain name do not exist");
  }
  else {
   log("\nDNS packet reply error "); log_var(dns_info.reply);
  }
  http_file_transfers[hft_entry].status = HFT_STATUS_ERROR;
  return PWRM_END_TRANSFER;
 }

 //save IP address of domain
 for(dword_t i=0; i<4; i++) {
  http_file_transfers[hft_entry].domain_ip[i] = dns_info.ip[i];
 }

 //log
 log("\nIP of domain: "); log_var(dns_info.ip[0]); log("."); log_var(dns_info.ip[1]); log("."); log_var(dns_info.ip[2]); log("."); log_var(dns_info.ip[3]);

 //create TCP handshake packet
 start_building_network_packet();
 network_packet_add_ethernet_layer(internet.router_mac);
 network_packet_add_ipv4_layer(internet.our_ip, http_file_transfers[hft_entry].domain_ip);
 network_packet_add_tcp_layer(free_tcp_port_for_http_transfer, 80, 0, 0, TCP_LAYER_SYN_FLAG);
 finalize_builded_network_packet();

 //send TCP handshake packet and tcp_server_handshake_received() or tcp_handshake_error() will be called based on response
 update_packet_entry(number_of_packet_entry, 2000, 4, PWRM_PACKET_TYPE_IPV4_TCP, 80, free_tcp_port_for_http_transfer, tcp_server_handshake_received, tcp_handshake_error);
 
 //save variables
 http_file_transfers[hft_entry].status = HFT_STATUS_TCP_TRANSFER;
 http_file_transfers[hft_entry].our_tcp_port = free_tcp_port_for_http_transfer;
 free_tcp_port_for_http_transfer++;
 if(free_tcp_port_for_http_transfer >= 50000) {
  free_tcp_port_for_http_transfer = 40000;
 }

 return PWRM_CONTINUE_TRANSFER;
}

byte_t tcp_server_handshake_received(byte_t number_of_packet_entry, byte_t *packet_memory, dword_t packet_length) {
 //get number of HTTP file transfer entry associated with this packet
 byte_t hft_entry = get_number_of_hft_entry_from_pwrm_entry(number_of_packet_entry);
 if(hft_entry==HFT_ENTRY_NOT_FOUND) {
  return PWRM_END_TRANSFER;
 }

 //check if this is not kill of connection by RST
 if((full_packet_tcp_layer->control & TCP_LAYER_RST_FLAG) == TCP_LAYER_RST_FLAG) {
  log("\nConnection on HFTE "); log_var_with_space(hft_entry); log("was killed");

  hft_close_entry(hft_entry);
  http_file_transfers[hft_entry].status = HFT_STATUS_ERROR;

  return PWRM_END_TRANSFER;
 }

 //check if this is really TCP handshake
 if(full_packet_tcp_layer->control != (TCP_LAYER_SYN_FLAG | TCP_LAYER_ACK_FLAG)) {
  log("\nnot TCP server handshake "); log_hex(full_packet_tcp_layer->control);
  return PWRM_CONTINUE_TRANSFER;
 }

 //save variables
 http_file_transfers[hft_entry].sequence_number = BIG_ENDIAN_DWORD(full_packet_tcp_layer->acknowledgment_number);
 http_file_transfers[hft_entry].acknowledgment_number = BIG_ENDIAN_DWORD(full_packet_tcp_layer->sequence_number)+1;

 //create TCP handshake ACK packet
 start_building_network_packet();
 network_packet_add_ethernet_layer(internet.router_mac);
 network_packet_add_ipv4_layer(internet.our_ip, http_file_transfers[hft_entry].domain_ip);
 network_packet_add_tcp_layer(http_file_transfers[hft_entry].our_tcp_port, 80, http_file_transfers[hft_entry].sequence_number, http_file_transfers[hft_entry].acknowledgment_number, TCP_LAYER_ACK_FLAG);
 finalize_builded_network_packet();

 //send TCP handshake ACK packet
 send_builded_packet_to_internet();

 //create HTTP request packet
 start_building_network_packet();
 network_packet_add_ethernet_layer(internet.router_mac);
 network_packet_add_ipv4_layer(internet.our_ip, http_file_transfers[hft_entry].domain_ip);
 network_packet_add_tcp_layer(http_file_transfers[hft_entry].our_tcp_port, 80, http_file_transfers[hft_entry].sequence_number, http_file_transfers[hft_entry].acknowledgment_number, TCP_LAYER_PUSH_FLAG | TCP_LAYER_ACK_FLAG);
 network_packet_add_http_layer(http_file_transfers[hft_entry].url);
 finalize_builded_network_packet();

 //send HTTP request packet and tcp_http_data_received() or tcp_http_error() will be called based on response
 update_packet_entry(number_of_packet_entry, 4000, 3, PWRM_PACKET_TYPE_IPV4_TCP, 80, http_file_transfers[hft_entry].our_tcp_port, tcp_http_data_received, tcp_http_error);

 return PWRM_CONTINUE_TRANSFER;
}

byte_t tcp_http_data_received(byte_t number_of_packet_entry, byte_t *packet_memory, dword_t packet_length) {
 //get number of HTTP file transfer entry associated with this packet
 byte_t hft_entry = get_number_of_hft_entry_from_pwrm_entry(number_of_packet_entry);
 if(hft_entry==HFT_ENTRY_NOT_FOUND) {
  return PWRM_END_TRANSFER;
 }

 //check if this is not kill of connection by RST
 if((full_packet_tcp_layer->control & TCP_LAYER_RST_FLAG) == TCP_LAYER_RST_FLAG) {
  log("\nConnection on HFTE "); log_var_with_space(hft_entry); log("was killed");

  hft_close_entry(hft_entry);
  http_file_transfers[hft_entry].status = HFT_STATUS_ERROR;

  return PWRM_END_TRANSFER;
 }

 //check if this is really TCP data
 if((full_packet_tcp_layer->control & TCP_LAYER_ACK_FLAG) != TCP_LAYER_ACK_FLAG || (full_packet_tcp_layer->control==TCP_LAYER_ACK_FLAG && packet_length==0)) {
  return PWRM_CONTINUE_TRANSFER;
 }

 //check if this has correct sequence number
 if(BIG_ENDIAN_DWORD(full_packet_tcp_layer->sequence_number)!=http_file_transfers[hft_entry].acknowledgment_number) {
  return PWRM_CONTINUE_TRANSFER;
 }

 //copy data in this packet
 add_bytes_to_byte_stream(http_file_transfers[hft_entry].data, packet_memory, packet_length);

 //check if we did not received full HTTP header, and if yes, parse it
 if(http_file_transfers[hft_entry].http_header_received == STATUS_FALSE && parse_http_layer((byte_t *)http_file_transfers[hft_entry].data->start_of_allocated_memory, http_file_transfers[hft_entry].data->size_of_stream) == STATUS_GOOD) {
  //log
  log("\nHFT entry "); log_var_with_space(hft_entry); log("HTTP header received");
  
  //update status
  http_file_transfers[hft_entry].http_header_received = STATUS_TRUE;

  //save useful info
  http_file_transfers[hft_entry].expected_file_size = http_info.content_length;
 }

 //update sequence and acknowledge number
 http_file_transfers[hft_entry].sequence_number = BIG_ENDIAN_DWORD(full_packet_tcp_layer->acknowledgment_number);
 http_file_transfers[hft_entry].acknowledgment_number = BIG_ENDIAN_DWORD(full_packet_tcp_layer->sequence_number)+packet_length;

 //create TCP acknowledge/finalize packet
 start_building_network_packet();
 network_packet_add_ethernet_layer(internet.router_mac);
 network_packet_add_ipv4_layer(internet.our_ip, http_file_transfers[hft_entry].domain_ip);
 if((full_packet_tcp_layer->control & TCP_LAYER_FIN_FLAG) == TCP_LAYER_FIN_FLAG) {
  http_file_transfers[hft_entry].acknowledgment_number++;
  network_packet_add_tcp_layer(http_file_transfers[hft_entry].our_tcp_port, 80, http_file_transfers[hft_entry].sequence_number, http_file_transfers[hft_entry].acknowledgment_number, TCP_LAYER_ACK_FLAG | TCP_LAYER_FIN_FLAG);
 }
 else {
  network_packet_add_tcp_layer(http_file_transfers[hft_entry].our_tcp_port, 80, http_file_transfers[hft_entry].sequence_number, http_file_transfers[hft_entry].acknowledgment_number, TCP_LAYER_ACK_FLAG);
 }
 finalize_builded_network_packet();

 //send TCP packet
 if((full_packet_tcp_layer->control & TCP_LAYER_FIN_FLAG) == TCP_LAYER_FIN_FLAG) {
  update_packet_entry(number_of_packet_entry, 2000, 4, PWRM_PACKET_TYPE_IPV4_TCP, 80, http_file_transfers[hft_entry].our_tcp_port, tcp_fin_received, tcp_fin_error);
 }
 else {
  update_packet_entry(number_of_packet_entry, 2000, 4, PWRM_PACKET_TYPE_IPV4_TCP, 80, http_file_transfers[hft_entry].our_tcp_port, tcp_http_data_received, tcp_http_error);
 }

 return PWRM_CONTINUE_TRANSFER;
}

byte_t tcp_fin_received(byte_t number_of_packet_entry, byte_t *packet_memory, dword_t packet_length) {
 //get number of HTTP file transfer entry associated with this packet
 byte_t hft_entry = get_number_of_hft_entry_from_pwrm_entry(number_of_packet_entry);
 if(hft_entry==HFT_ENTRY_NOT_FOUND) {
  return PWRM_END_TRANSFER;
 }

 //log
 log("\nHFT entry ");
 log_var_with_space(number_of_packet_entry);
 log("was transferred in ");
 log_var(time_of_system_running-http_file_transfers[hft_entry].starting_time);
 log("ms with size ");
 log_var(http_file_transfers[hft_entry].data->size_of_stream/1024);
 log("kb");

 //close byte stream
 http_file_transfers[hft_entry].file_size = http_file_transfers[hft_entry].data->size_of_stream;
 http_file_transfers[hft_entry].file_memory = close_byte_stream(http_file_transfers[hft_entry].data);
 http_file_transfers[hft_entry].data = 0;

 //parse HTTP header
 if(parse_http_layer(http_file_transfers[hft_entry].file_memory, http_file_transfers[hft_entry].file_size)==STATUS_ERROR) {
  log("\nMalformed HTTP response on "); log_var_with_space(hft_entry);
  goto error;
 }

 //process HTTP data
 if(http_info.status_type==HTTP_STATUS_TYPE_SUCCESS) {
  //FILE WAS SUCCESSFULLY TRANSFERRED
  //if there is not zero data, parse transferred file
  if(http_info.body_data_pointer!=0) {
   //check transfer encoding
   if(http_info.transfer_encoding_type == UNSUPPORTED_TRANSFER_ENCODING) {
    log("\nHTTP unknown transfer encoding");
    goto error;
   }

   //copy data of file
   if(http_info.transfer_encoding_type == NO_TRANSFER_ENCODING) {
    //copy data
    http_file_transfers[hft_entry].file_size = (http_file_transfers[hft_entry].file_size - (http_info.body_data_pointer-http_file_transfers[hft_entry].file_memory)); //size of file without HTTP header
    copy_memory((dword_t)http_info.body_data_pointer, (dword_t)http_file_transfers[hft_entry].file_memory, http_file_transfers[hft_entry].file_size);
   }
   else { //http_info.transfer_encoding_type is TRANSFER_ENCODING_CHUNKED
    //copy data from chunks
    byte_t *start = (byte_t *) (http_info.body_data_pointer);
    byte_t *end = (byte_t *) ((dword_t)http_file_transfers[hft_entry].file_memory + http_file_transfers[hft_entry].file_size);
    byte_t *memory_pointer = (byte_t *) (http_file_transfers[hft_entry].file_memory);
    http_file_transfers[hft_entry].file_size = 0;
    while(start < end) {
     //check if chunk starts with number
     if(is_hex_number(*start)==STATUS_FALSE) {
      log("\nHTTP malformed chunk without size");
      goto error;
     }

     //read size of chunk
     dword_t size_of_chunk = convert_hex_string_to_number(start);

     //check if this is not ending chunk
     if(size_of_chunk == 0) {
      break;
     }

     //skip string of size of chunk
     while(start < end) {
      //check if this is not number
      if(is_number(*start)==STATUS_FALSE) {
       break;
      }

      //move to next char
      start++;
     }
     if(start >= end) {
      log("\nHTTP malformed chunk in header");
      goto error;
     }

     //skip chunk trailer
     while(start < end) {
      //check if this is not end of chunk
      if(start[0]==0x0D && start[1]==0x0A) {
       start += 2;
       break;
      }

      //move to next char
      start++;
     }
     if(start >= end) {
      log("\nHTTP malformed chunk without start");
      goto error;
     }

     //check if this is not too big chunk
     if(((dword_t)start + size_of_chunk) >= (dword_t)end) {
      log("\nHTTP malformed too big chunk");
      goto error;
     }

     //copy chunk data
     copy_memory((dword_t)start, (dword_t)memory_pointer, size_of_chunk);
     start += size_of_chunk;
     memory_pointer += size_of_chunk;
     http_file_transfers[hft_entry].file_size += size_of_chunk;

     //skip ending of chunk
     if(((dword_t)start + 2) >= (dword_t)end || is_memory_equal_with_memory(start, "\r\n", 2)==STATUS_FALSE) {
      log("\nHTTP malformed chunk without ending");
      goto error;
     }
     start += 2;
    }
    if(start >= end) {
     log("\nHTTP malformed chunked response");
     goto error;
    }
   }

   //reallocate used memory and add zero ending
   http_file_transfers[hft_entry].file_memory = (byte_t *) realloc((dword_t)http_file_transfers[hft_entry].file_memory, http_file_transfers[hft_entry].file_size+1);
   http_file_transfers[hft_entry].file_memory[http_file_transfers[hft_entry].file_size] = 0;
  }
  else {
   //free allocated data
   free((dword_t)http_file_transfers[hft_entry].file_memory);
   http_file_transfers[hft_entry].file_memory = 0;
   http_file_transfers[hft_entry].file_size = 0;
  }
 }
 else if(http_info.status_type==HTTP_STATUS_TYPE_REDIRECTION) {
  //FILE IS IN OTHER URL LOCATION
  //check if HTTP contained new location
  if(http_info.location_data_size == 0) {
   log("\nHTTP redirection without address");
   goto error;
  }

  //copy new location
  byte_t url_of_new_location[MAX_LENGTH_OF_URL];
  if(http_info.location_data_size >= (MAX_LENGTH_OF_URL-1)) {
   log("\nRedirection to too long URL");
   goto error;
  }
  copy_memory((dword_t)http_info.location_data_pointer, (dword_t)&url_of_new_location, http_info.location_data_size);
  url_of_new_location[http_info.location_data_size] = 0;

  //log
  log("\nRedirection to location: "); log(url_of_new_location);

  //free used entries
  remove_packet_entry(number_of_packet_entry);
  hft_close_entry(hft_entry);

  //find which network entry uses this transfer
  byte_t network_transfer_number = get_number_of_network_transfer_from_type_of_transfer(NETWORK_TRANSFER_TYPE_HTTP, hft_entry);
  if(network_transfer_number == NETWORK_TRANSFER_NO_ENTRY_FOUNDED) {
   log("\nSERIOUS ERROR: transfer was lost");
   return PWRM_END_TRANSFER;
  }

  //check number of redirections
  if(network_transfers[network_transfer_number].max_number_of_redirections == 0) {
   log("\nToo many redirections");
   goto error;
  }

  //update network transfer url
  copy_memory((dword_t)&url_of_new_location, (dword_t)&network_transfers[network_transfer_number].url, MAX_LENGTH_OF_URL);

  //use this network entry to start new transfer of new location
  //there can never be error of not free entry, because we just freed entry used for this transaction, so at least it is available
  network_transfers[network_transfer_number].number_of_transfer_entry = download_file_from_url_by_http_s_protocols(url_of_new_location);
  network_transfers[network_transfer_number].max_number_of_redirections--;

  //if download_file_from_url_by_http_s_protocols() uses this entry, this will not touch it, if it do not, we already cleared this entry, do we do not to do anything with it
  return PWRM_CONTINUE_TRANSFER;
 }
 else {
  //THERE WAS ERROR DURING HTTP TRANSFER
  //log
  log("\nHTTP transfer not successful ");
  log_var(http_info.status_code);
  goto error;
 }

 //update status
 http_file_transfers[hft_entry].status = HFT_STATUS_DONE;

 return PWRM_END_TRANSFER;

 error:
 //close entry
 hft_close_entry(hft_entry);
 http_file_transfers[hft_entry].status = HFT_STATUS_ERROR;

 return PWRM_END_TRANSFER;
}

/* processing of transfer errors */

void http_any_transfer_error(byte_t number_of_packet_entry, byte_t *error_description) {
 //get number of HTTP file transfer entry associated with this packet
 byte_t hft_entry = get_number_of_hft_entry_from_pwrm_entry(number_of_packet_entry);
 if(hft_entry==HFT_ENTRY_NOT_FOUND) {
  return;
 }

 //log
 log("\nERROR: ");
 log(error_description);

 //update status
 hft_close_entry(hft_entry);
 http_file_transfers[hft_entry].status = HFT_STATUS_ERROR;
}

void dns_request_error(byte_t number_of_packet_entry, byte_t error) {
 http_any_transfer_error(number_of_packet_entry, "DNS reply was not received");
}

void tcp_handshake_error(byte_t number_of_packet_entry, byte_t error) {
 http_any_transfer_error(number_of_packet_entry, "TCP handshake was not received");
}

void tcp_http_error(byte_t number_of_packet_entry, byte_t error) {
 http_any_transfer_error(number_of_packet_entry, "HTTP data were not received");
}

void tcp_fin_error(byte_t number_of_packet_entry, byte_t error) {
 http_any_transfer_error(number_of_packet_entry, "TCP FIN acknowledge was not received");
}

/* methods for interacting with HFT entry */

byte_t get_hft_status(dword_t hft_entry) {
 return http_file_transfers[hft_entry].status;
}

byte_t *get_hft_file_memory(dword_t hft_entry) {
 if(http_file_transfers[hft_entry].status==HFT_STATUS_DONE) {
  return http_file_transfers[hft_entry].file_memory;
 }
 else {
  return HFT_ERROR_INVALID_ENTRY;
 }
}

dword_t get_hft_file_size(dword_t hft_entry) {
 if(http_file_transfers[hft_entry].status==HFT_STATUS_DONE) {
  return http_file_transfers[hft_entry].file_size;
 }
 else if(http_file_transfers[hft_entry].status==HFT_STATUS_TCP_TRANSFER) {
  return http_file_transfers[hft_entry].expected_file_size;
 }
 else {
  return HFT_ERROR_INVALID_ENTRY;
 }
}

dword_t get_hft_transferred_file_size(dword_t hft_entry) {
 if(http_file_transfers[hft_entry].status==HFT_STATUS_TCP_TRANSFER) {
  return http_file_transfers[hft_entry].data->size_of_stream;
 }
 else if(http_file_transfers[hft_entry].status==HFT_STATUS_DONE) {
  return http_file_transfers[hft_entry].file_size;
 }
 else {
  return HFT_ERROR_INVALID_ENTRY;
 }
}

void hft_close_entry(dword_t hft_entry) {
 if(http_file_transfers[hft_entry].status==HFT_STATUS_FREE_ENTRY) {
  return;
 }
 
 //release memory of byte stream
 if(http_file_transfers[hft_entry].data!=0) {
  free((dword_t)close_byte_stream(http_file_transfers[hft_entry].data));
 }

 //release memory of file in memory
 if(http_file_transfers[hft_entry].file_memory!=0) {
  free((dword_t)http_file_transfers[hft_entry].file_memory);
 }

 //clear whole entry, it will set status to HFT_STATUS_FREE_ENTRY
 clear_memory((dword_t)&http_file_transfers[hft_entry], sizeof(struct http_file_transfer_t));
}

void hft_kill_entry_transfer(dword_t hft_entry) {
 if(http_file_transfers[hft_entry].status==HFT_STATUS_FREE_ENTRY) {
  return;
 }
 
 //disable PWRM entry
 asm("cli");
 remove_packet_entry(http_file_transfers[hft_entry].pwrm_entry);
 asm("sti");

 //send RST TCP packet to kill connection
 if(http_file_transfers[hft_entry].status==HFT_STATUS_TCP_TRANSFER) {
  start_building_network_packet();
  network_packet_add_ethernet_layer(internet.router_mac);
  network_packet_add_ipv4_layer(internet.our_ip, http_file_transfers[hft_entry].domain_ip);
  network_packet_add_tcp_layer(http_file_transfers[hft_entry].our_tcp_port, 80, http_file_transfers[hft_entry].sequence_number, 0, TCP_LAYER_RST_FLAG);
  finalize_builded_network_packet();
  send_builded_packet_to_internet();
 }

 //close HTTP file transfer entry
 hft_close_entry(hft_entry);
}