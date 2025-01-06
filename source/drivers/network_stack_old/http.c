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
 network_packet_add_string_to_http_layer("User-Agent: BleskOSv2024u38\r\n");

 //Connection
 network_packet_add_string_to_http_layer("Connection: close\r\n");

 //end of HTTP
 network_packet_add_string_to_http_layer("\r\n");

 //set variables
 size_of_network_packet += network_packet_size_of_http_layer;
 network_packet_save_layer(NETWORK_PACKET_HTTP_LAYER, network_packet_size_of_http_layer, 0);
}

void parse_http_layer(byte_t *http_layer_pointer, dword_t http_layer_size) {
 //TODO:
}