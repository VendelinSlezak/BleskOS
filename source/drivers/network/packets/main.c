//BleskOS

/*
* MIT License
* Copyright (c) 2023-2025 Vendelín Slezák
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

void initalize_network_stack(void) {
 memory_for_building_network_packet = (byte_t *) malloc(MAX_SIZE_OF_ONE_PACKET);
}

void start_building_network_packet(void) {
 size_of_network_packet = 0;
 number_of_layers_in_network_packet = 0;
 clear_memory((dword_t)memory_for_building_network_packet, MAX_SIZE_OF_ONE_PACKET);
}

void network_packet_save_layer(word_t type_of_layer, dword_t size_of_layer, void (*finalize_layer)(dword_t type_of_next_layer, dword_t size_of_layer)) {
 if(number_of_layers_in_network_packet>=10) {
  return;
 }

 //save layer to array
 layout_of_network_packet[number_of_layers_in_network_packet].type_of_layer = type_of_layer;
 layout_of_network_packet[number_of_layers_in_network_packet].size_of_layer = size_of_layer;
 layout_of_network_packet[number_of_layers_in_network_packet].finalize_layer = finalize_layer;
 number_of_layers_in_network_packet++;
}

void finalize_builded_network_packet(void) {
 //finalize every layer (set type of next layer, size of layer and checksum of layer)
 for(dword_t i=number_of_layers_in_network_packet, type_of_next_layer=0, size_of_layer=0; i>0; i--) {
  //calculate size of layer
  size_of_layer += layout_of_network_packet[(i-1)].size_of_layer;

  //get type of next layer
  if(i<number_of_layers_in_network_packet) {
   type_of_next_layer = layout_of_network_packet[i].type_of_layer;
  }

  //finalize layer
  if(layout_of_network_packet[(i-1)].finalize_layer!=0) {
   layout_of_network_packet[(i-1)].finalize_layer(type_of_next_layer, size_of_layer);
  }
 }
}

//TODO:
void log_network_packet(void) {
 log("\nPACKET "); log_var(size_of_network_packet);
 
 log("\n");
 dword_t pointer;
 for(dword_t i=0; i<size_of_network_packet; i++) {
  log_hex_specific_size_with_space(memory_for_building_network_packet[pointer], 2);
  pointer++;
  if((pointer % 10)==0) {
   log("\n");
  }
  if(pointer == size_of_network_packet) {
   break;
  }
 }
}