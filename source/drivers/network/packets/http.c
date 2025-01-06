//BleskOS

/*
* MIT License
* Copyright (c) 2023-2025 Vendelín Slezák
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

void network_packet_add_string_to_http_layer(byte_t *string) {
 while(*string!=0) {
  *network_packet_http_layer = *string;
  network_packet_http_layer++;
  string++;
  network_packet_size_of_http_layer++;
 }
}

void network_packet_add_bytes_to_http_layer(byte_t *mem, dword_t size) {
 while(size>0) {
  *network_packet_http_layer = *mem;
  network_packet_http_layer++;
  mem++;
  size--;
  network_packet_size_of_http_layer++;
 }
}

void network_packet_add_http_layer(byte_t *url) {
 network_packet_http_layer = (byte_t *) ((dword_t)memory_for_building_network_packet+size_of_network_packet);
 network_packet_size_of_http_layer = 0;

 //parse URL address
 if(is_memory_equal_with_memory(url, "http://", 7)==STATUS_TRUE) {
  url += 7;
 }
 else if(is_memory_equal_with_memory(url, "https://", 8)==STATUS_TRUE) {
  url += 8;
 }
 byte_t *domain = url;
 dword_t domain_length = 0;
 byte_t *path = 0;
 while(*url!=0 && *url!='/') {
  domain_length++;
  url++;
 }
 if(*url=='/') {
  path = url;
 }

 //GET method
 network_packet_add_string_to_http_layer("GET ");
 if(path == 0) {
  network_packet_add_string_to_http_layer("/");
 }
 else {
  network_packet_add_string_to_http_layer(path);
 }
 network_packet_add_string_to_http_layer(" HTTP/1.1\r\n");

 //Host
 network_packet_add_string_to_http_layer("Host: ");
 network_packet_add_bytes_to_http_layer(domain, domain_length);
 network_packet_add_string_to_http_layer("\r\n");

 //User-agent
 network_packet_add_string_to_http_layer("User-Agent: BleskOSv2025u1\r\n");

 //Connection
 network_packet_add_string_to_http_layer("Connection: close\r\n");

 //end of HTTP
 network_packet_add_string_to_http_layer("\r\n");

 //set variables
 size_of_network_packet += network_packet_size_of_http_layer;
 network_packet_save_layer(NETWORK_PACKET_HTTP_LAYER, network_packet_size_of_http_layer, 0);
}

byte_t parse_http_layer(byte_t *http_layer_pointer, dword_t http_layer_size) {
 //save pointers
 byte_t *start = http_layer_pointer;
 byte_t *end = (byte_t *) ((dword_t)start + http_layer_size);

 //clear info
 clear_memory((dword_t)&http_info, sizeof(struct http_info_t));

 //PARSE STATUS LINE
 //check signature
 if(are_equal_b_string_b_string(http_layer_pointer, "HTTP/1.1 ")!=STATUS_TRUE && is_number(http_layer_pointer[9])!=STATUS_TRUE) {
  return STATUS_ERROR;
 }

 //read status code
 http_info.status_code = convert_byte_string_to_number((dword_t)&http_layer_pointer[9]);
 if(http_info.status_code<100 || http_info.status_code>=600) {
  return STATUS_ERROR;
 }
 else if(http_info.status_code<200) {
  http_info.status_type = HTTP_STATUS_TYPE_INFORMATIONAL;
 }
 else if(http_info.status_code<300) {
  http_info.status_type = HTTP_STATUS_TYPE_SUCCESS;
 }
 else if(http_info.status_code<400) {
  http_info.status_type = HTTP_STATUS_TYPE_REDIRECTION;
 }
 else if(http_info.status_code<500) {
  http_info.status_type = HTTP_STATUS_TYPE_CLIENT_ERROR;
 }
 else {
  http_info.status_type = HTTP_STATUS_TYPE_SERVER_ERROR;
 }

 //skip status line
 while(http_layer_pointer < end) {
  //check if we are at end of line
  if(http_layer_pointer[0]==0x0D && http_layer_pointer[1]==0x0A) {
   http_layer_pointer += 2; //skip end of line
   break;
  }

  //skip char
  http_layer_pointer++;
 }
 if(http_layer_pointer >= end) {
  return STATUS_ERROR; //packet ends after status line
 }

 //PARSE HTTP HEADERS
 while(http_layer_pointer < end) {
  //check if we are at end of headers
  if(http_layer_pointer[0]==0x0D && http_layer_pointer[1]==0x0A) {
   http_layer_pointer += 2; //skip end of headers
   break;
  }

  //parse basic info about header
  byte_t *header_start = http_layer_pointer;
  byte_t *header_data_start = 0;
  dword_t header_data_length = 0;
  while(http_layer_pointer < end) {
   //check if we are at end of header
   if(http_layer_pointer[0]==0x0D && http_layer_pointer[1]==0x0A) {
    http_layer_pointer += 2; //skip end of header
    break;
   }

   //check if we are not at start of header data
   if(http_layer_pointer[0]==':' && http_layer_pointer[1]==' ') {
    http_layer_pointer += 2; //skip end of header
    header_data_start = http_layer_pointer;
   }

   //move variables
   if(header_data_start!=0) {
    header_data_length++;
   }
   http_layer_pointer++;
  }
  if(http_layer_pointer >= end) {
   return STATUS_ERROR; //malformed packet that ends in headers
  }

  //check if this is header we will parse
  if(are_equal_b_string_b_string(header_start, "Content-Length: ")==STATUS_TRUE || are_equal_b_string_b_string(header_start, "content-length: ")==STATUS_TRUE) {
   http_info.content_length = convert_byte_string_to_number((dword_t)header_data_start);
  }
  else if(are_equal_b_string_b_string(header_start, "Transfer-Encoding: ")==STATUS_TRUE || are_equal_b_string_b_string(header_start, "transfer-encoding: ")==STATUS_TRUE) {
   if(are_equal_b_string_b_string(header_data_start, "chunked")==STATUS_TRUE) {
    http_info.transfer_encoding_type = TRANSFER_ENCODING_CHUNKED;
   }
   else {
    http_info.transfer_encoding_type = UNSUPPORTED_TRANSFER_ENCODING;
   }
  }
  else if(are_equal_b_string_b_string(header_start, "Location: ")==STATUS_TRUE || are_equal_b_string_b_string(header_start, "location: ")==STATUS_TRUE) {
   http_info.location_data_pointer = header_data_start;
   http_info.location_data_size = header_data_length;
  }
 }

 //save pointer to body
 if(http_layer_pointer < end) {
  http_info.body_data_pointer = http_layer_pointer;
 }

 return STATUS_GOOD;
}