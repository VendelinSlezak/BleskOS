//BleskOS

/*
* MIT License
* Copyright (c) 2023-2024 Vendelín Slezák
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

dword_t http_transfer_file(byte_t *url) { //TODO: another location
 http_response_type = 0;
 already_sended_seq = 0;
 file_transfer_error_type = FILE_TRANSFER_NO_ERROR;
 if(connected_to_network==STATUS_FALSE) {
  file_transfer_error_type = FILE_TRANSFER_ERROR_NO_NETWORK_CONNECTION;
  return STATUS_ERROR;
 }

 //copy requested url
 if(are_equal_b_string_b_string(url, "gate.aspero.pro/?site=")==STATUS_TRUE) {
  for(int i=22; i<2048; i++) {
   file_full_url[i] = url[i];

   if(url[i]==0) {
    break;
   }
  }
 }
 else {
  for(int i=0; i<2048; i++) {
   file_full_url[i] = url[i];

   if(url[i]==0) {
    break;
   }
  }
 }
 
 //get IP address of server
 dword_t webpage_ip = get_ip_address_of_url(url);
 if(webpage_ip==0xFFFFFFFF) {
  file_transfer_error_type = FILE_TRANSFER_ERROR_DNS_DO_NOT_EXIST;
  return STATUS_ERROR;
 }
 
 //select free port
 file_tcp_port++;
 if(file_tcp_port<40000 || file_tcp_port>50000) {
  file_tcp_port = 40000;
 }
 
 //estabilish connection
 tcp_connection_status = TCP_NO_CONNECTION;
 send_tcp_handshake(webpage_ip, file_tcp_port);
 ticks=0;
 while(tcp_connection_status!=TCP_HANDSHAKE_COMPLETE) {
  asm("hlt");
  if(connected_to_network==STATUS_FALSE) {
   file_transfer_error_type = FILE_TRANSFER_ERROR_NO_NETWORK_CONNECTION;
   return STATUS_ERROR;
  }

  if(ticks>250) {
   //connection not estabilished after 500 milliseconds, try again
   send_tcp_handshake(webpage_ip, file_tcp_port);
   ticks=0;
   while(tcp_connection_status!=TCP_HANDSHAKE_COMPLETE) {
    asm("hlt");
    if(connected_to_network==STATUS_FALSE) {
     file_transfer_error_type = FILE_TRANSFER_ERROR_NO_NETWORK_CONNECTION;
     return STATUS_ERROR;
    }

    if(ticks>250) {
     //connection not estabilished after 1 second, reset port and try again
     send_tcp_reset(webpage_ip, file_tcp_port, 0, 0);
     send_tcp_handshake(webpage_ip, file_tcp_port);
     ticks=0;
     while(tcp_connection_status!=TCP_HANDSHAKE_COMPLETE) {
      asm("hlt");
      if(connected_to_network==STATUS_FALSE) {
       file_transfer_error_type = FILE_TRANSFER_ERROR_NO_NETWORK_CONNECTION;
       return STATUS_ERROR;
      }

      if(ticks>500) {
       file_transfer_error_type = FILE_TRANSFER_ERROR_CONNECTION_NOT_ESTABILISHED;
       return STATUS_ERROR; //connection not estabilished after 2 seconds
      }
     }
    }
   }
  }
 }
 
 //request for file
 tcp_connection_status = TCP_HTTP_REQUEST;
 send_http_get(url, webpage_ip, file_tcp_port);
 
 //wait for file to be transferred
 tcp_new_packet = 0;
 ticks=0;
 while(tcp_connection_status!=TCP_CONNECTION_CLOSED) {
  asm("hlt");
  if(connected_to_network==STATUS_FALSE) {
   file_transfer_error_type = FILE_TRANSFER_ERROR_NO_NETWORK_CONNECTION;
   return STATUS_ERROR;
  }

  if(tcp_new_packet==1) { //new packet received, reset timer
   ticks=0;
   tcp_new_packet = 0;
  }

  if(ticks>2000) {
   //no packet after 4 seconds, kill connection
   send_tcp_reset(webpage_ip, file_tcp_port, 0, 0);
   file_transfer_error_type = FILE_TRANSFER_ERROR_SERVER_NOT_RESPONDING;
   return STATUS_ERROR;
  }
 }
 
 //if this is redirect response, load new url
 if(http_response_type==HTTP_RESPONSE_301_MOVED_PERMANENTLY || http_response_type==HTTP_RESPONSE_302_MOVED_TEMPORARILY) {
  if(file_transfer_num_of_redirect>5) { //max five redirections
   #ifndef NO_PROGRAMS
   internet_browser_print_message("Too many redirections");
   #endif
   file_transfer_error_type = FILE_TRANSFER_ERROR_TOO_MANY_REDIRECTIONS;
   return STATUS_ERROR;
  }
  
  #ifndef NO_PROGRAMS
  internet_browser_print_message("Redirected");
  #endif

  file_transfer_num_of_redirect++;
  free(file_memory); //we do not need keep data about redirecting
  return network_transfer_file_redirect(http_url_new_location);
 }

 //copy resulting requested url
 if(are_equal_b_string_b_string(url, "gate.aspero.pro/?site=https://")==STATUS_TRUE) {
  url += 22;
 }
 for(int i=0; i<2048; i++) {
  file_full_url[i] = url[i];

  if(url[i]==0) {
   break;
  }
 }
 
 return file_memory;
}

dword_t https_transfer_file(byte_t *url) {
 //BleskOS do not have code for Secure HTTP connection, but we use workaround for acessing https pages
 //this workaround is through page gate.aspero.pro, which resends code of HTTPS pages through HTTP connection
 byte_t *workaround_url = "gate.aspero.pro/?site=https://";
 dword_t url_length = 0;
 for(int i=0; i<2048; i++) {
  if(url[i]==0) {
   break;
  }
  url_length++;
 }
 url_length += 31;
 dword_t https_url = malloc(url_length);
 byte_t *https_url_pointer = (byte_t *) https_url;
 
 for(int i=0; i<30; i++) {
  https_url_pointer[i]=workaround_url[i];
 }
 for(int i=0, j=30; i<(url_length); i++, j++) {
  https_url_pointer[j]=url[i];
 }
 https_url_pointer[url_length]=0;

 dword_t file_mem = http_transfer_file(https_url_pointer);
 
 free(https_url);
 
 return file_mem;
}

dword_t network_transfer_file_redirect(byte_t *url) {
 if(url[0]=='h' && url[1]=='t' && url[2]=='t' && url[3]=='p' && url[4]==':' && url[5]=='/' && url[6]=='/') {
  url += 7;
  return http_transfer_file(url);
 }
 else if(url[0]=='h' && url[1]=='t' && url[2]=='t' && url[3]=='p' && url[4]=='s' && url[5]==':' && url[6]=='/' && url[7]=='/') {
  url += 8;
  return https_transfer_file(url);
 }
 else {
  return http_transfer_file(url);
 }
}

dword_t network_transfer_file(byte_t *url) {
 file_transfer_num_of_redirect = 0;
 
 if(url[0]=='h' && url[1]=='t' && url[2]=='t' && url[3]=='p' && url[4]==':' && url[5]=='/' && url[6]=='/') {
  url += 7;
  return http_transfer_file(url);
 }
 else if(url[0]=='h' && url[1]=='t' && url[2]=='t' && url[3]=='p' && url[4]=='s' && url[5]==':' && url[6]=='/' && url[7]=='/') {
  url += 8;
  return https_transfer_file(url);
 }
 else {
  return http_transfer_file(url);
 }
}