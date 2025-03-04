//BleskOS

/*
* MIT License
* Copyright (c) 2023-2025 BleskOS developers
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

void network_packet_add_dns_layer(word_t id, word_t qtype, byte_t *domain) {
 network_packet_dns_layer = (struct network_packet_dns_layer_t *) ((dword_t)memory_for_building_network_packet+size_of_network_packet);
 dword_t size_of_dns_layer = sizeof(struct network_packet_dns_layer_t);

 //set DNS layer header
 network_packet_dns_layer->id = id;
 network_packet_dns_layer->flags = 0x0001; //standard query with no recursion
 network_packet_dns_layer->qd_count = BIG_ENDIAN_WORD(1); //one question
 network_packet_dns_layer->an_count = 0;
 network_packet_dns_layer->ns_count = 0;
 network_packet_dns_layer->ar_count = 0;

 //set DNS question
 // www.example.com needs to be translated to:
 // byte 3, 'w', 'w', 'w', 7, 'e', 'x', 'a', 'm', 'p', 'l', 'e', 3, 'c', 'o', 'm', 0
 // word type_of_question
 // word class_of_question
 byte_t *dns_question = (byte_t *) ((dword_t)network_packet_dns_layer + sizeof(struct network_packet_dns_layer_t));
 byte_t *dns_length_of_domain_part = dns_question; //pointer to number of chars in first part of domain
 dns_question++; //pointer first char of first part of domain
 size_of_dns_layer++; //increase size of DNS layer
 while(*domain!=0x00) {
  //end of domain part
  if(*domain=='.') {
   dns_length_of_domain_part = dns_question;
  }
  else { //char of domain
   (*dns_length_of_domain_part)++; //increase number of chars in this domain part
   *dns_question = *domain; //copy char
  }

  //move to next char
  dns_question++;
  domain++;
  size_of_dns_layer++;
 }
 *dns_question = 0; //end of domain name
 dns_question++;
 *((word_t *)dns_question) = qtype; //type of question
 dns_question += 2;
 *((word_t *)dns_question) = BIG_ENDIAN_WORD(0x0001); //class of question 0x0001 = internet
 size_of_dns_layer += 5;

 //set variables
 size_of_network_packet += size_of_dns_layer;
 network_packet_save_layer(NETWORK_PACKET_DNS_LAYER, size_of_dns_layer, 0);
}

byte_t parse_dns_reply(byte_t *dns_layer_pointer, dword_t dns_layer_size) {
 //clear output
 clear_memory((dword_t)&dns_info, sizeof(struct dns_info_t));

 //too small
 if(dns_layer_size < sizeof(struct network_packet_dns_layer_t)) {
  log("\nDNS parse error: small layer "); log_var(dns_layer_size);
  return STATUS_ERROR;
 }

 //set pointer to dns layer
 struct network_packet_dns_layer_t *dns_layer = (struct network_packet_dns_layer_t *) (dns_layer_pointer);

 //read ID of packet
 dns_info.packet_id = dns_layer->id;

 //read reply code
 dns_info.reply = ((dns_layer->flags >> 8) & 0xF);
 if(dns_info.reply!=DNS_REPLY_GOOD) {
  return STATUS_GOOD; //this means that we successfully recieved packet, but from reply it will be seen that it is without response
 }

 //check number of questions and answers
 if(dns_layer->qd_count!=BIG_ENDIAN_WORD(1)) {
  log("\nDNS parse error: more questions at once");
  return STATUS_ERROR;
 }

 //move pointer after header
 byte_t *end = (byte_t *) ((dword_t)dns_layer_pointer + dns_layer_size);
 dns_layer_pointer += sizeof(struct network_packet_dns_layer_t);

 //skip all questions
 for(dword_t i = 0; i < BIG_ENDIAN_WORD(dns_layer->qd_count); i++) {
  //skip domain name
  while(*dns_layer_pointer != 0) {
   dns_layer_pointer += ((*dns_layer_pointer) + 1); //skip one part of domain name

   if(dns_layer_pointer >= end) {
    log("\nDNS parse error: small layer questions");
    return STATUS_ERROR;
   }
  }

  dns_layer_pointer += 5; //skip zero at end of domain name, type of question and class of question
 }

 //process all answers
 for(dword_t i = 0; i < BIG_ENDIAN_WORD(dns_layer->an_count); i++) {
  if(dns_layer_pointer >= end) {
   log("\nDNS parse error: small layer answers");
   return STATUS_ERROR;
  }

  //skip name
  if((*dns_layer_pointer & 0xC0) == 0xC0) {
   dns_layer_pointer += 2; //name is comprimated
  }
  else {
   //skip domain name
   while(*dns_layer_pointer != 0) {
    dns_layer_pointer += ((*dns_layer_pointer) + 1); //skip one part of domain name

    if(dns_layer_pointer >= end) {
     log("\nDNS parse error: small layer answers 2");
     return STATUS_ERROR;
    }
   }
   dns_layer_pointer++; //skip zero at end of domain name
  }

  //move pointers
  struct network_packet_dns_layer_answer_t *network_packet_dns_layer_answer = (struct network_packet_dns_layer_answer_t *) dns_layer_pointer;
  dns_layer_pointer += (sizeof(struct network_packet_dns_layer_answer_t) + BIG_ENDIAN_WORD(network_packet_dns_layer_answer->data_length));
  if(dns_layer_pointer > end) {
   log("\nDNS parse error: answer above packet");
   return STATUS_ERROR; //answer is above packet
  }

  //read IP address
  if(network_packet_dns_layer_answer->type==NETWORK_PACKET_DNS_QUESTION_TYPE_IPV4
     && network_packet_dns_layer_answer->class==NETWORK_PACKET_DNS_QUESTION_CLASS_INTERNET
     && BIG_ENDIAN_WORD(network_packet_dns_layer_answer->data_length)==4) {
   for(dword_t i=0; i<4; i++) {
    dns_info.ip[i] = network_packet_dns_layer_answer->address[i];
   }

   return STATUS_GOOD;
  }
 }

 log("\nDNS parse error: no IP");

 return STATUS_ERROR; //IP address was not readed
}