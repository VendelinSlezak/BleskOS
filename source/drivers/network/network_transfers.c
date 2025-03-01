//BleskOS

/*
* MIT License
* Copyright (c) 2023-2025 Vendelín Slezák
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

void initalize_network_transfers(void) {
 for(dword_t i=0; i<MAX_NUMBER_OF_NETWORK_TRANSFERS; i++) {
  network_transfers[i].status = NETWORK_TRANSFER_FREE_ENTRY;
 }
}

byte_t download_file_from_url(byte_t *url) {
 //check if URL is not too long
 dword_t url_length = get_number_of_chars_in_ascii_string(url);
 if(url_length>=(MAX_LENGTH_OF_URL-1)) {
  logf("\nInvalid URL requested: Too long %d", url_length);
  return NETWORK_TRANSFER_ERROR_INVALID_URL;
 }

 //check if URL is without unallowed characters
 for(dword_t i=0; i<url_length; i++) {
  //check if char is in allowed characters A-Z a-z 0-9
  if(is_char(url[i]) == STATUS_FALSE) {
   //check other allowed characters
   byte_t *url_allowed_characters = "/?&=-_.~:#\x25+@"; //\x25 is %
   byte_t valid = STATUS_FALSE;
   for(dword_t j=0; j<13; j++) {
    if(url[i] == url_allowed_characters[j]) {
     valid = STATUS_TRUE;
     break;
    }
   }

   //if this is unallowed character, return error immediately
   if(valid == STATUS_FALSE) {
    logf("\nInvalid URL requested: Unallowed character %02x", url[i]);
    return NETWORK_TRANSFER_ERROR_INVALID_URL;
   }
  }
 }

 //check if URL has domain
 dword_t domain_detected = STATUS_FALSE;
 dword_t pointer = 0;
 if(is_memory_equal_with_memory(url, "https://", 8)==STATUS_TRUE) {
  pointer += 8;
 }
 else if(is_memory_equal_with_memory(url, "http://", 7)==STATUS_TRUE) {
  pointer += 7;
 }
 while(url[pointer]!=0) {
  if(url[pointer]=='/') { //this ends domain name
   break;
  }
  else if(url[pointer]=='.') { //domain is characterized by at least one dot
   domain_detected = STATUS_TRUE;
  }

  //move to next char
  pointer++;
 }
 if(domain_detected == STATUS_FALSE) {
  log("\nInvalid URL requested: No domain name");
  return NETWORK_TRANSFER_ERROR_INVALID_URL;
 }

 //check if there is free entry
 for(dword_t i=0; i<MAX_NUMBER_OF_NETWORK_TRANSFERS; i++) {
  if(network_transfers[i].status == NETWORK_TRANSFER_FREE_ENTRY) {
   //copy URL
   dword_t pointer = 0;
   while(url[pointer] != 0) {
    network_transfers[i].url[pointer] = url[pointer];
    pointer++;
   }
   network_transfers[i].url[pointer] = 0; //add zero ending

   //we assume that URL is HTTP(S) request
   network_transfers[i].max_number_of_redirections = 10;
   network_transfers[i].number_of_transfer_entry = download_file_from_url_by_http_s_protocols(url);
   if(network_transfers[i].number_of_transfer_entry == HTTP_ERROR_NO_FREE_ENTRIES) {
    return NETWORK_TRANSFER_ERROR_NO_FREE_ENTRY;
   }
   network_transfers[i].type_of_transfer = NETWORK_TRANSFER_TYPE_HTTP;
   network_transfers[i].status = NETWORK_TRANSFER_USED_ENTRY;

   //log
   logf("\nFile transfer from URL: %d %s", network_transfers[i].number_of_transfer_entry, url);

   //return number of this entry
   return i;
  }
 }

 //no free entry founded
 return NETWORK_TRANSFER_ERROR_NO_FREE_ENTRY;
}

byte_t download_file_from_url_by_http_s_protocols(byte_t *url) {
 if(is_memory_equal_with_memory(url, "https://", 8)==STATUS_TRUE) {
  url += 8;
  return https_download_file_from_url(url);
 }
 else if(is_memory_equal_with_memory(url, "http://", 7)==STATUS_TRUE) {
  url += 7;
 }
 
 return http_download_file_from_url(url);
}

byte_t get_number_of_network_transfer_from_type_of_transfer(byte_t type_of_transfer, byte_t number_of_transfer_entry) {
 for(dword_t i=0; i<MAX_NUMBER_OF_SIMULTANOUS_HTTP_TRANSFERS; i++) {
  if(network_transfers[i].type_of_transfer == type_of_transfer && network_transfers[i].number_of_transfer_entry == number_of_transfer_entry) {
   return i;
  }
 }

 return NETWORK_TRANSFER_NO_ENTRY_FOUNDED;
}

byte_t get_status_of_network_transfer(byte_t transfer_number) {
 if(transfer_number>=MAX_NUMBER_OF_SIMULTANOUS_HTTP_TRANSFERS) {
  return NETWORK_TRANSFER_INVALID_TRANSFER_NUMBER;
 }

 //HTTP transfers
 if(network_transfers[transfer_number].type_of_transfer==NETWORK_TRANSFER_TYPE_HTTP) {
  byte_t hft_entry_status = get_hft_status(network_transfers[transfer_number].number_of_transfer_entry);
  if(hft_entry_status==HFT_STATUS_DNS_REQUEST || hft_entry_status==HFT_STATUS_TCP_TRANSFER) {
   return NETWORK_TRANSFER_TRANSFERRING_DATA;
  }
  else if(hft_entry_status==HFT_STATUS_DONE) {
   return NETWORK_TRANSFER_DONE;
  }
  else {
   return NETWORK_TRANSFER_ERROR;
  }
 }
 
 return NETWORK_TRANSFER_INVALID_TRANSFER_NUMBER;
}

byte_t *get_file_memory_of_network_transfer(byte_t transfer_number) {
 if(transfer_number>=MAX_NUMBER_OF_SIMULTANOUS_HTTP_TRANSFERS) {
  return STATUS_ERROR;
 }

 //HTTP transfer
 if(network_transfers[transfer_number].type_of_transfer==NETWORK_TRANSFER_TYPE_HTTP) {
  byte_t *status = get_hft_file_memory(network_transfers[transfer_number].number_of_transfer_entry);
  if(status != HFT_ERROR_INVALID_ENTRY) {
   return status;
  }
 }
 
 return STATUS_ERROR;
}

dword_t get_file_size_of_network_transfer(byte_t transfer_number) {
 if(transfer_number>=MAX_NUMBER_OF_SIMULTANOUS_HTTP_TRANSFERS) {
  return STATUS_ERROR;
 }

 //HTTP transfer
 if(network_transfers[transfer_number].type_of_transfer==NETWORK_TRANSFER_TYPE_HTTP) {
  dword_t status = get_hft_file_size(network_transfers[transfer_number].number_of_transfer_entry);
  if(status != HFT_ERROR_INVALID_ENTRY) {
   return status;
  }
 }
 
 return STATUS_ERROR;
}

dword_t get_transferred_file_size_of_network_transfer(byte_t transfer_number) {
 if(transfer_number>=MAX_NUMBER_OF_SIMULTANOUS_HTTP_TRANSFERS) {
  return STATUS_ERROR;
 }

 //HTTP transfer
 if(network_transfers[transfer_number].type_of_transfer==NETWORK_TRANSFER_TYPE_HTTP) {
  dword_t status = get_hft_transferred_file_size(network_transfers[transfer_number].number_of_transfer_entry);
  if(status != HFT_ERROR_INVALID_ENTRY) {
   return status;
  }
 }
 
 return STATUS_ERROR;
}

void close_network_transfer(byte_t transfer_number) {
 if(transfer_number>=MAX_NUMBER_OF_SIMULTANOUS_HTTP_TRANSFERS) {
  return;
 }

 //HTTP transfer
 if(network_transfers[transfer_number].type_of_transfer==NETWORK_TRANSFER_TYPE_HTTP) {
  hft_close_entry(network_transfers[transfer_number].number_of_transfer_entry);
  clear_memory((dword_t)&network_transfers[transfer_number], sizeof(struct network_transfer_info_t));
 }
}

void kill_network_transfer(byte_t transfer_number) {
 if(transfer_number>=MAX_NUMBER_OF_SIMULTANOUS_HTTP_TRANSFERS) {
  return;
 }

 //HTTP transfer
 if(network_transfers[transfer_number].type_of_transfer==NETWORK_TRANSFER_TYPE_HTTP) {
  hft_kill_entry_transfer(network_transfers[transfer_number].number_of_transfer_entry);
  clear_memory((dword_t)&network_transfers[transfer_number], sizeof(struct network_transfer_info_t));
 }
}