//BleskOS

/*
* MIT License
* Copyright (c) 2023-2025 Vendelín Slezák
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

void initalize_send_packet_with_response_interface(void) {
 //clear interface
 for(dword_t i=0; i<MAX_NUMBER_OF_PACKETS_WITH_RESPONSE; i++) {
  packets_with_response_interface[i].milliseconds_count = 0;
 }

 //set task to check if there is not need to resend some packet
 create_task(send_packets_in_queue, TASK_TYPE_PERIODIC_INTERRUPT, 1);
}

void send_builded_packet_to_internet(void) {
 internet.send_packet(memory_for_building_network_packet, size_of_network_packet);
}

byte_t send_builded_packet_with_response_to_internet(word_t milliseconds_before_resending, byte_t max_attempts, byte_t response_packet_type, word_t response_packet_sender_port, word_t response_packet_reciever_port, byte_t process_response(byte_t number_of_packet_entry, byte_t *packet_memory, dword_t packet_length), byte_t process_error(byte_t number_of_packet_entry, byte_t error_type)) {
 for(dword_t i=0; i<MAX_NUMBER_OF_PACKETS_WITH_RESPONSE; i++) {
  //we find free entry
  if(packets_with_response_interface[i].milliseconds_count==0) {
   //fill entry
   copy_memory((dword_t)memory_for_building_network_packet, (dword_t)(&packets_with_response_interface[i].packet), size_of_network_packet);
   packets_with_response_interface[i].packet_length = size_of_network_packet;
   
   packets_with_response_interface[i].milliseconds_before_resending = milliseconds_before_resending;
   packets_with_response_interface[i].max_attempts = max_attempts;

   packets_with_response_interface[i].response_packet_type = response_packet_type;
   packets_with_response_interface[i].response_packet_sender_port = response_packet_sender_port;
   packets_with_response_interface[i].response_packet_reciever_port = response_packet_reciever_port;

   packets_with_response_interface[i].process_response = process_response;
   packets_with_response_interface[i].process_error = process_error;

   packets_with_response_interface[i].milliseconds_count = (time_of_system_running+milliseconds_before_resending); //this enables entry

   //send packet for first time
   send_builded_packet_to_internet();
   packets_with_response_interface[i].max_attempts--;

   //return number of entry
   return i;
  }
 }

 return PWRM_NO_FREE_ENTRY; 
}

void remove_packet_entry(byte_t number_of_packet_entry) {
 packets_with_response_interface[number_of_packet_entry].milliseconds_count = 0;
}

void update_packet_entry(byte_t number_of_packet_entry, word_t milliseconds_before_resending, byte_t max_attempts, byte_t response_packet_type, word_t response_packet_sender_port, word_t response_packet_reciever_port, byte_t process_response(byte_t number_of_packet_entry, byte_t *packet_memory, dword_t packet_length), byte_t process_error(byte_t number_of_packet_entry, byte_t error_type)) {
 copy_memory((dword_t)memory_for_building_network_packet, (dword_t)(&packets_with_response_interface[number_of_packet_entry].packet), size_of_network_packet);
 packets_with_response_interface[number_of_packet_entry].milliseconds_before_resending = milliseconds_before_resending;
 packets_with_response_interface[number_of_packet_entry].packet_length = size_of_network_packet;
 packets_with_response_interface[number_of_packet_entry].max_attempts = max_attempts;
 packets_with_response_interface[number_of_packet_entry].response_packet_type = response_packet_type;
 packets_with_response_interface[number_of_packet_entry].response_packet_sender_port = response_packet_sender_port;
 packets_with_response_interface[number_of_packet_entry].response_packet_reciever_port = response_packet_reciever_port;
 packets_with_response_interface[number_of_packet_entry].process_response = process_response;
 packets_with_response_interface[number_of_packet_entry].process_error = process_error;
 packets_with_response_interface[number_of_packet_entry].milliseconds_count = time_of_system_running; //send_packets_in_queue() will immediately send packet
}

void send_packets_in_queue(void) {
 for(dword_t i=0; i<MAX_NUMBER_OF_PACKETS_WITH_RESPONSE; i++) {
  //this packet needs to be sended again
  if(packets_with_response_interface[i].milliseconds_count!=0 && time_of_system_running>=packets_with_response_interface[i].milliseconds_count) {
   //time out - error with sending packet
   if(packets_with_response_interface[i].max_attempts==0) {
    //call error method
    if(packets_with_response_interface[i].process_error!=0) {
     packets_with_response_interface[i].process_error(i, PWRM_ERROR_TIMEOUT);
    }

    //remove PWRM entry
    remove_packet_entry(i);

    continue;
   }

   //send packet
   internet.send_packet((byte_t *)&packets_with_response_interface[i].packet, packets_with_response_interface[i].packet_length);

   //update variables
   packets_with_response_interface[i].max_attempts--;
   packets_with_response_interface[i].milliseconds_count = (time_of_system_running+packets_with_response_interface[i].milliseconds_before_resending);
  }
 }
}