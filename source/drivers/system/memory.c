//BleskOS

/*
* MIT License
* Copyright (c) 2023 Vendelín Slezák
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

void initalize_memory(void) {
 dword_t *memory_map = (dword_t *) 0x1000; //bootloader loaded memory map here
 dword_t *memory_entries;

 //convert 64 bit entries
 for(dword_t i=0, offset=0; i<50; i++, offset+=6) {
  if(memory_map[offset+4]==0) {
   break; //we reach end of list
  }
  
  //entry that starts above 32 bits or is longer than 32 bits
  if(memory_map[offset+1]>0x00000000) {
   memory_map[offset+0]=0xFFFFFFFF;
   memory_map[offset+1]=0x00000000;
   memory_map[offset+2]=0;
   memory_map[offset+3]=0;
   memory_map[offset+4]=MEMORY_TYPE_USED; //used memory
  }
  else if(memory_map[offset+3]>0 || ((0xFFFFFFFF-memory_map[offset+0])<memory_map[offset+2])>0x00000000) { //entry that ends above 32 bits
   memory_map[offset+2]=(0xFFFFFFFF-memory_map[offset+0]); //change length that entry will end on 0xFFFFFFFF
  }
 }
 
 //calculate all amount of memory
 all_memory_in_bytes=0;
 all_free_memory_in_bytes=0;
 for(dword_t i=0, offset=0; i<50; i++, offset+=6) {
  if(memory_map[offset+4]==0) {
   break; //we reach end of list
  }
  
  all_memory_in_bytes+=memory_map[offset+2];

  if(memory_map[offset+4]==MEMORY_TYPE_FREE && memory_map[offset+0]!=0xFFFFFFFF) {
   all_free_memory_in_bytes+=memory_map[offset+2];
  }
 }
 all_actual_free_memory_in_bytes = all_free_memory_in_bytes;
 
 //find memory for memory entries
 for(dword_t i=0, offset=0; i<50; i++, offset+=6) {
  if(memory_map[offset+4]==0) {
   memory_error_debug(0xFF0000); //ERROR: not enough free memory for memory entries founded
  }
  
  //find 1 MB for memory entries
  if(memory_map[offset+4]==MEMORY_TYPE_FREE && memory_map[offset+2]>=0x100000) {
   mem_memory_entries = memory_map[offset+0]; //save pointer to free memory for memory entries

   //change memory entry because we already allocated 1 MB
   memory_map[offset+0]+=0x100000;
   memory_map[offset+2]-=0x100000;
   break;
  }
 }
 
 //clear memory of memory entries
 clear_memory(mem_memory_entries, 0x100000);
 
 //insert free memory entries
 memory_entries = (dword_t *) mem_memory_entries;
 for(dword_t i=0, offset=0; i<50; i++, offset+=6) {
  if(memory_map[offset+4]==0) {
   break; //we reach end of list
  }
  
  if(memory_map[offset+0]!=0xFFFFFFFF && memory_map[offset+0]>=0x100000 && memory_map[offset+2]>0 && memory_map[offset+4]==MEMORY_TYPE_FREE) { //we add only high memory
   memory_entries[MEM_ENTRY_TYPE]=MEM_FREE_MEMORY;
   memory_entries[MEM_ENTRY_LENGTH]=memory_map[offset+2]; //length of memory
   memory_entries[MEM_ENTRY_START]=memory_map[offset+0]; //start of memory
   memory_entries[MEM_ENTRY_END]=(memory_map[offset+0]+memory_map[offset+2]); //end of memory
   memory_entries+=4;
  }
 }
}

void log_starting_memory(void) {
 dword_t *memory_map = (dword_t *) 0x1000; //bootloader loaded memory map here
 
 log("Memory in B: ");
 log_var_with_space(all_memory_in_bytes);
 log("\nMemory in MB: ");
 log_var_with_space(all_memory_in_bytes/1024/1024);
 log("\nStarting memory entries:");
 
 //calculate all amount of memory
 for(dword_t i=0, offset=0; i<50; i++, offset+=6) {
  log("\n");
  
  if(memory_map[offset+4]==0) {
   break; //we reach end of list
  }
  
  if(memory_map[offset+4]==MEMORY_TYPE_FREE) {
   log("Free memory ");
   log_hex_with_space(memory_map[offset+1]);
   log_hex_with_space(memory_map[offset+0]);
   log("length ");
   log_var_with_space(memory_map[offset+3]);
   log_var_with_space(memory_map[offset+2]);
   log("B ");
   log_var_with_space(memory_map[offset+2]/1024/1024);
   log("MB");
  }
  else if(memory_map[offset+4]==MEMORY_TYPE_USED) {
   log("Used memory ");
   log_hex_with_space(memory_map[offset+1]);
   log_hex_with_space(memory_map[offset+0]);
   log("length ");
   log_var_with_space(memory_map[offset+3]);
   log_var_with_space(memory_map[offset+2]);
   log("B ");
   log_var_with_space(memory_map[offset+2]/1024/1024);
   log("MB");
  }
  else {
   log("Other entry");
   log_hex_with_space(memory_map[offset+1]);
   log_hex_with_space(memory_map[offset+0]);
   log("length ");
   log_var_with_space(memory_map[offset+3]);
   log_var_with_space(memory_map[offset+2]);
   log("B ");
   log_var_with_space(memory_map[offset+2]/1024/1024);
   log("MB");
  }
 }
 
 log("\n");
}

dword_t malloc(dword_t mem_length) {
 dword_t *memory_entries = (dword_t *) mem_memory_entries;
 dword_t mem_entry_start = 0;
 dword_t mem_entry_end = 0;
  
 //scan memory entries
 for(dword_t i=0; i<0xFFFF; i++, memory_entries+=4) {
  if(memory_entries[MEM_ENTRY_TYPE]==MEM_FREE_MEMORY) {
   if(memory_entries[MEM_ENTRY_LENGTH]==mem_length) {
    all_actual_free_memory_in_bytes -= mem_length;
    memory_entries[MEM_ENTRY_TYPE]=MEM_ALLOCATED_MEMORY;
    return memory_entries[MEM_ENTRY_START];
   }
   else if(memory_entries[MEM_ENTRY_LENGTH]>mem_length) {
    mem_entry_start = memory_entries[MEM_ENTRY_START];
    mem_entry_end = (mem_entry_start+mem_length);
    
    //decrease size of free memory entry
    memory_entries[MEM_ENTRY_LENGTH]-=mem_length;
    memory_entries[MEM_ENTRY_START]+=mem_length;
    
    //create entry for allocated memory
    memory_entries = (dword_t *) mem_memory_entries;
    for(dword_t j=0; j<0xFFFF; j++, memory_entries+=4) {
     if(memory_entries[MEM_ENTRY_TYPE]==MEM_FREE_ENTRY) {
      all_actual_free_memory_in_bytes -= mem_length;
      memory_entries[MEM_ENTRY_TYPE]=MEM_ALLOCATED_MEMORY;
      memory_entries[MEM_ENTRY_LENGTH]=mem_length; //length of memory
      memory_entries[MEM_ENTRY_START]=mem_entry_start; //start of memory
      memory_entries[MEM_ENTRY_END]=mem_entry_end; //end of memory
      
      return mem_entry_start;
     }
    }
    
    //ERROR: no free entry
    return 0;
   }
  }
 }
 
 //ERROR: no free memory
 memory_error_debug(0xFF0000);
 return 0;
}

dword_t aligned_malloc(dword_t mem_length, dword_t mem_alignment) {
 dword_t *memory_entries = (dword_t *) mem_memory_entries;
 dword_t *pointed_memory_entry = (dword_t *) 0;
 dword_t mem_entry_start = 0;
 dword_t mem_entry_length = 0;
 dword_t mem_entry_end = 0;
 dword_t mem_gap_entry_start = 0;
 dword_t alignment_mask = ~mem_alignment;
 dword_t alignment_increment = mem_alignment+1;
 dword_t entry_alignment_start, entry_alignment_length;
 
 //scan memory entries
 for(dword_t i=0; i<0xFFFF; i++, memory_entries+=4) {
  if(memory_entries[MEM_ENTRY_TYPE]==MEM_FREE_MEMORY) {  
   //test if this entry have enough free memory
   if(memory_entries[MEM_ENTRY_START]<mem_length) {
    continue;
   }
   //test if start of memory is enough for alignment
   if(memory_entries[MEM_ENTRY_START]<alignment_increment) {
    continue;
   }
   //test if this entry is not already aligned enough
   if((memory_entries[MEM_ENTRY_START] & mem_alignment)==0) {
    //if yes, allocate memory
    if(memory_entries[MEM_ENTRY_LENGTH]==mem_length) {
     all_actual_free_memory_in_bytes -= mem_length;
     memory_entries[MEM_ENTRY_TYPE]=MEM_ALLOCATED_MEMORY;
     return memory_entries[MEM_ENTRY_START];
    }
    else if(memory_entries[MEM_ENTRY_LENGTH]>mem_length) {
     mem_entry_start = memory_entries[MEM_ENTRY_START];
     mem_entry_end = (mem_entry_start+mem_length);
    
     //decrease size of free memory entry
     memory_entries[MEM_ENTRY_LENGTH]-=mem_length;
     memory_entries[MEM_ENTRY_START]+=mem_length;
    
     //create entry for allocated memory
     memory_entries = (dword_t *) mem_memory_entries;
     for(dword_t j=0; j<0xFFFF; j++, memory_entries+=4) {
      if(memory_entries[MEM_ENTRY_TYPE]==MEM_FREE_ENTRY) {
       all_actual_free_memory_in_bytes -= mem_length;
       memory_entries[MEM_ENTRY_TYPE]=MEM_ALLOCATED_MEMORY;
       memory_entries[MEM_ENTRY_LENGTH]=mem_length; //length of memory
       memory_entries[MEM_ENTRY_START]=mem_entry_start; //start of memory
       memory_entries[MEM_ENTRY_END]=mem_entry_end; //end of memory
      
       return mem_entry_start;
      }
     }
    
     //ERROR: no free entry
     return 0;
    }
    
   }
   entry_alignment_start = ((memory_entries[MEM_ENTRY_START] & alignment_mask)+alignment_increment);
   if(entry_alignment_start>memory_entries[MEM_ENTRY_END]) {
    continue; //alignment is above this free memory
   }
   entry_alignment_length = memory_entries[MEM_ENTRY_END]-entry_alignment_start;   
   if(entry_alignment_length<mem_length) {
    continue; //not enough free memory in this entry
   }
   
   //initalize values
   pointed_memory_entry = (dword_t *) memory_entries;
   mem_entry_start = memory_entries[MEM_ENTRY_START];
   mem_entry_length = memory_entries[MEM_ENTRY_LENGTH];
   mem_entry_end = memory_entries[MEM_ENTRY_END];
  
   //remove gap between start of free memory and aligned free memory
   if(entry_alignment_start>memory_entries[MEM_ENTRY_START]) {    
    //change free memory entry to aligned values
    memory_entries[MEM_ENTRY_START] = entry_alignment_start;
    memory_entries[MEM_ENTRY_LENGTH] = entry_alignment_length;
    
    //create memory entry from gap
    memory_entries = (dword_t *) mem_memory_entries;
    for(dword_t j=0; j<0xFFFF; j++, memory_entries+=4) {
     if(memory_entries[MEM_ENTRY_TYPE]==MEM_FREE_ENTRY) {
      memory_entries[MEM_ENTRY_TYPE]=MEM_ALLOCATED_MEMORY;
      memory_entries[MEM_ENTRY_LENGTH]=entry_alignment_start-mem_entry_start; //length of gap memory
      memory_entries[MEM_ENTRY_START]=mem_entry_start; //start of gap memory
      memory_entries[MEM_ENTRY_END]=mem_entry_start+memory_entries[MEM_ENTRY_LENGTH]; //end of gap memory
      mem_gap_entry_start = mem_entry_start;      
      goto gap_memory_entry_created;
     }
    }
    return 0; //ERROR: no free entry

    gap_memory_entry_created:
    memory_entries = (dword_t *) pointed_memory_entry;
   }
  
   //allocate memory
   if(memory_entries[MEM_ENTRY_LENGTH]==mem_length) {
    all_actual_free_memory_in_bytes -= mem_length;
    memory_entries[MEM_ENTRY_TYPE]=MEM_ALLOCATED_MEMORY;
    return memory_entries[MEM_ENTRY_START];
   }
   else if(memory_entries[MEM_ENTRY_LENGTH]>mem_length) {
    mem_entry_start = memory_entries[MEM_ENTRY_START];
    mem_entry_end = (mem_entry_start+mem_length);
    
    //decrease size of free memory entry
    memory_entries[MEM_ENTRY_LENGTH]-=mem_length;
    memory_entries[MEM_ENTRY_START]+=mem_length;
    
    //create entry for allocated memory
    memory_entries = (dword_t *) mem_memory_entries;
    for(dword_t j=0; j<0xFFFF; j++, memory_entries+=4) {
     if(memory_entries[MEM_ENTRY_TYPE]==MEM_FREE_ENTRY) {
      all_actual_free_memory_in_bytes -= mem_length;
      memory_entries[MEM_ENTRY_TYPE]=MEM_ALLOCATED_MEMORY;
      memory_entries[MEM_ENTRY_LENGTH]=mem_length; //length of memory
      memory_entries[MEM_ENTRY_START]=mem_entry_start; //start of memory
      memory_entries[MEM_ENTRY_END]=mem_entry_end; //end of memory
      
      //free memory from gap
      if(mem_gap_entry_start!=0) {
       free(mem_gap_entry_start);
      }
      
      return mem_entry_start;
     }
    }
    
    //ERROR: no free entry
    return 0;
   }
  }
 }
 
 //ERROR: no free memory
 memory_error_debug(0xFF0000);
 return 0;
}

dword_t calloc(dword_t mem_length) {
 //allocate memory
 dword_t mem_pointer = malloc(mem_length);
 if(mem_pointer==0) {
  return 0; //some error
 }
 
 //clear memory
 clear_memory(mem_pointer, mem_length);
 
 return mem_pointer;
}

dword_t aligned_calloc(dword_t mem_length, dword_t mem_alignment) {
 //allocate memory
 dword_t mem_pointer = aligned_malloc(mem_length, mem_alignment);
 if(mem_pointer==0) {
  return 0; //some error
 }
 
 //clear memory
 clear_memory(mem_pointer, mem_length);
 
 return mem_pointer;
}

dword_t realloc(dword_t mem_pointer, dword_t mem_length) {
 dword_t *memory_entries = (dword_t *) mem_memory_entries;
 dword_t *pointed_memory_entry = (dword_t *) 0;
 byte_t *old_mem_ptr;
 byte_t *new_mem_ptr;
 dword_t pointed_memory_entry_start = 0;
 dword_t pointed_memory_entry_length = 0;

 //find memory entry
 for(dword_t i=0; i<0xFFFF; i++, memory_entries+=4) {
  if(memory_entries[MEM_ENTRY_TYPE]==MEM_ALLOCATED_MEMORY && memory_entries[MEM_ENTRY_START]==mem_pointer) {
   pointed_memory_entry = (dword_t *) memory_entries;
   pointed_memory_entry_start = pointed_memory_entry[2];
   pointed_memory_entry_length = pointed_memory_entry[1];
   break;
  }
 }
 if(pointed_memory_entry==0) {
  return 0; //ERROR: memory entry not founded
 }
 
 //reallocation to equal size
 if(pointed_memory_entry[1]==mem_length) {
  return pointed_memory_entry[2]; //no reallocation needed
 }

 //reallocation to other size
 free(mem_pointer);
 mem_pointer = malloc(mem_length);
 if(mem_pointer==0) {
  return 0; //ERROR: some error during allocation 
 }
 if(mem_pointer==pointed_memory_entry_start) { //reallocated to same memory  
  //clear if reallocated to bigger size
  if(mem_length>pointed_memory_entry_length) {
   clear_memory(mem_pointer+pointed_memory_entry_length, (mem_length-pointed_memory_entry_length));
  }
  return mem_pointer;
 }
 else if(mem_length<=pointed_memory_entry_length) { //otherwise copy data from old memory to new memory
  old_mem_ptr = (byte_t *) pointed_memory_entry_start;
  new_mem_ptr = (byte_t *) mem_pointer;
  for(dword_t i=0; i<mem_length; i++) {
   *new_mem_ptr = *old_mem_ptr;
   new_mem_ptr++;
   old_mem_ptr++;
  }
 }
 else { //copy data and clear bigger size
  old_mem_ptr = (byte_t *) pointed_memory_entry_start;
  new_mem_ptr = (byte_t *) mem_pointer;
  for(dword_t i=0; i<pointed_memory_entry_length; i++) {
   *new_mem_ptr = *old_mem_ptr;
   new_mem_ptr++;
   old_mem_ptr++;
  }
  
  clear_memory(mem_pointer+pointed_memory_entry_length, (mem_length-pointed_memory_entry_length));
 }

 return mem_pointer;
}

void free(dword_t mem_pointer) {
 dword_t *memory_entries = (dword_t *) mem_memory_entries;
 dword_t mem_entry_below = 0;
 dword_t mem_entry = 0;
 dword_t mem_entry_above = 0;
 dword_t mem_entry_start = 0;
 dword_t mem_entry_length = 0;
 dword_t mem_entry_end = 0;
 dword_t mem_pointer_end = 0;
 
 //find entry
 for(dword_t i=0; i<0xFFFF; i++, memory_entries+=4) {
  if(memory_entries[MEM_ENTRY_START]==mem_pointer && memory_entries[MEM_ENTRY_TYPE]==MEM_ALLOCATED_MEMORY) {
   mem_entry = (dword_t) memory_entries;
   mem_entry_length = memory_entries[MEM_ENTRY_LENGTH];
   mem_entry_start = memory_entries[MEM_ENTRY_START];
   mem_entry_end = memory_entries[MEM_ENTRY_END];
   mem_pointer_end = mem_entry_end;
   all_actual_free_memory_in_bytes += mem_entry_length;
   break;
  }
 }
 
 if(mem_entry==0) {
  return;
 }

 //find if there is some free memory above or below
 memory_entries = (dword_t *) mem_memory_entries;
 for(dword_t i=0; i<0xFFFF; i++, memory_entries+=4) {
  if(memory_entries[MEM_ENTRY_END]==mem_pointer && memory_entries[MEM_ENTRY_TYPE]==MEM_FREE_MEMORY) {
   mem_entry_below = (dword_t) memory_entries;
   mem_entry_start = memory_entries[MEM_ENTRY_START];
   mem_entry_length += memory_entries[MEM_ENTRY_LENGTH];
  }
  if(memory_entries[MEM_ENTRY_START]==mem_pointer_end && memory_entries[MEM_ENTRY_TYPE]==MEM_FREE_MEMORY) {
   mem_entry_above = (dword_t) memory_entries;
   mem_entry_end = memory_entries[MEM_ENTRY_END];
   mem_entry_length += memory_entries[MEM_ENTRY_LENGTH];
  }
  if(mem_entry_below!=0 && mem_entry_above!=0) {
   break;
  }
 }
 
 //change entry
 memory_entries = (dword_t *) mem_entry;
 memory_entries[MEM_ENTRY_TYPE]=MEM_FREE_MEMORY;
 memory_entries[MEM_ENTRY_LENGTH]=mem_entry_length;
 memory_entries[MEM_ENTRY_START]=mem_entry_start;
 memory_entries[MEM_ENTRY_END]=mem_entry_end;
 
 //delete other entries about same memory
 if(mem_entry_below!=0) {
  memory_entries = (dword_t *) mem_entry_below;
  memory_entries[MEM_ENTRY_TYPE]=MEM_FREE_ENTRY;
  memory_entries[MEM_ENTRY_LENGTH]=0;
  memory_entries[MEM_ENTRY_START]=0;
  memory_entries[MEM_ENTRY_END]=0;
 }
 if(mem_entry_above!=0) {
  memory_entries = (dword_t *) mem_entry_above;
  memory_entries[MEM_ENTRY_TYPE]=MEM_FREE_ENTRY;
  memory_entries[MEM_ENTRY_LENGTH]=0;
  memory_entries[MEM_ENTRY_START]=0;
  memory_entries[MEM_ENTRY_END]=0;
 }
}

void memory_error_debug(dword_t color) {
 dword_t *framebuffer = (dword_t *) 0x70028;
 byte_t *screen = (byte_t *) (*framebuffer);

 if(graphic_screen_bpp==32) {
  screen_mem = graphic_screen_lfb;
  clear_screen(color);
 }
 else if(graphic_screen_bpp==24) {
  for(dword_t i=0; i<(graphic_screen_x*graphic_screen_y); i++) {
   *screen = (color & 0xFF);
   screen++;
   *screen = ((color >> 8) & 0xFF);
   screen++;
   *screen = ((color >> 16) & 0xFF);
   screen++;
  }
 }
 
 while(1) {
  asm volatile ("hlt");
 }
}

void clear_memory(dword_t memory, dword_t length) {
 byte_t *mem = (byte_t *) memory;
 
 for(dword_t i=0; i<length; i++) {
  *mem = 0;
  mem++;
 }
}

void copy_memory(dword_t source_memory, dword_t destination_memory, dword_t size) {
 byte_t *source = (byte_t *) (source_memory);
 byte_t *destination = (byte_t *) (destination_memory);
 
 for(dword_t i=0; i<size; i++) {
  *destination=*source;
  destination++;
  source++;
 }
}

void copy_memory_back(dword_t source_memory, dword_t destination_memory, dword_t size) {
 byte_t *source = (byte_t *) (source_memory);
 byte_t *destination = (byte_t *) (destination_memory);

 for(dword_t i=0; i<size; i++) {
  *destination=*source;
  destination--;
  source--;
 }
}
