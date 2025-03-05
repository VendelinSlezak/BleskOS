//BleskOS

/*
* MIT License
* Copyright (c) 2023-2025 BleskOS developers
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

void initalize_mtrr(void) {
 logf("\n");

 //start with assumption that MTRR do not exist
 mtrr_available = STATUS_FALSE;

 //read CPU flags
 read_cpuid(0x01);

 //check if MTRR is available
 if((cpuid.edx & (1 << 12)) != (1 << 12)) {
  logf("\nMemory-Type Range Registers are not available");
  return;
 }

 //read capabilities of MTRR
 qword_t capability = read_msr(MTRR_REG_CAPABILITY);
 mtrr_write_combining_available = ((capability >> 10) & 0x1);
 mtrr_number_of_entries = (capability & 0xFF);
 mtrr_default_memory_type = (read_msr(MTRR_REG_DEFAULT_TYPE) & 0xFF);

 //check if MTRR has any entries
 if(mtrr_number_of_entries == 0) {
  logf("\nMemory-Type Range Registers without entries");
  return;
 }

 //enable MTRR
 if((read_msr(MTRR_REG_DEFAULT_TYPE) & (1 << 11)) == 0) {
  write_msr(MTRR_REG_DEFAULT_TYPE, read_msr(MTRR_REG_DEFAULT_TYPE) | (1 << 11));
  
  //check if MTRR was enabled
  if((read_msr(MTRR_REG_DEFAULT_TYPE) & (1 << 11)) == (1 << 11)) {
   logf("\nMemory-Type Range Registers successfully enabled");
  }
  else {
   logf("\nMemory-Type Range Registers were not successfully enabled");
   return;
  }
 }

 //we can use MTRR
 mtrr_available = STATUS_TRUE;

 //log all entries
 logf("\nMemory-Type Range Registers");
 logf("\n Number of entries: %d", mtrr_number_of_entries);
 if(mtrr_write_combining_available == STATUS_TRUE) {
  logf("\n Write-combining available");
 }
 logf("\n Default memory type:");
 log_mtrr_memory_type(mtrr_default_memory_type);
 logf("\n");
 for(dword_t i=0; i<mtrr_number_of_entries; i++) {
  logf("\nMTRR entry %d: ", i);
  if((read_msr(MTRR_REG_MASK + i*2) & (1 << 11)) == (1 << 11)) {
   log_mtrr_memory_type(read_msr(MTRR_REG_BASE + i*2) & 0xFF);
   logf(" ");
   if((read_msr(MTRR_REG_BASE + i*2) >> 32) != 0) {
    logf("0x%x ", (dword_t)(read_msr(MTRR_REG_BASE + i*2) >> 32));
   }
   logf("0x%x %d Kb", (dword_t)(read_msr(MTRR_REG_BASE + i*2) & 0xFFFFF000), (dword_t)((~(read_msr(MTRR_REG_MASK + i*2) & 0xFFFFF000))+1)/1024);
  }
  else {
   logf("Free");
  }
 }
}

void log_mtrr_memory_type(byte_t type) {
 switch(type) {
  case (MTRR_MEMORY_TYPE_UNCACHEABLE):
   logf("Uncacheable");
   break;

  case (MTRR_MEMORY_TYPE_WRITE_COMBINING):
   logf("Write Combining");
   break;

  case (MTRR_MEMORY_TYPE_WRITE_THROUGH):
   logf("Write Through");
   break;

  case (MTRR_MEMORY_TYPE_WRITE_PROTECT):
   logf("Write Protect");
   break;

  case (MTRR_MEMORY_TYPE_WRITE_BACK):
   logf("Write Back");
   break;
  
  default:
   logf("%d", type);
 }
}

void mtrr_set_free_entry(dword_t memory, dword_t mask, byte_t type) {
 //find free entry
 for(dword_t entry=0; entry<mtrr_number_of_entries; entry++) {
  //check if this is free entry
  if((read_msr(MTRR_REG_MASK + entry*2) & (1 << 11)) == 0) {
   write_msr(MTRR_REG_BASE + entry*2, memory | type);
   write_msr(MTRR_REG_MASK + entry*2, mask | (1 << 11));
   return;
  }
 }

 //log
 logf("\nERROR: no free MTRR entry");

 //we do not need to report error to higher method, because memory will always work in some mode
}

void mtrr_set_memory_type(void *memory, dword_t size, byte_t type) {
 if(mtrr_available == STATUS_FALSE || (type == mtrr_default_memory_type) || (type == MTRR_MEMORY_TYPE_WRITE_COMBINING && mtrr_write_combining_available == STATUS_FALSE)) {
  return;
 }

 //disable interrupts
 cli();

 //enter no-fill cache mode
 write_cr0((read_cr0() | (1 << 30)) & ~(1 << 29));

 //invalidate cache
 asm("wbinvd");

 //TODO: after writing driver for virtual memory disable paging here

 //TODO: after writing driver for virtual memory flush TLB here

 //disable MTRR
 write_msr(MTRR_REG_DEFAULT_TYPE, read_msr(MTRR_REG_DEFAULT_TYPE) & ~(1 << 11));
 
 //set all entries that are needed to describe this block of memory
 //every entry can hold information that is aligned to bits
 //for example if we need to set 3 MB = 0x300000, at first set block 0x200000 and then 0x100000
 for(dword_t i=31, mask=0; i>12; i--, mask = ((mask >> 1) | 0x80000000)) {
  if((size & (1 << i)) == (1 << i)) {
   //check if memory is aligned
   if(((dword_t)memory & (~mask)) != 0) {
    break; //memory is not aligned, so we can not set it
   }

   //set entry
   mtrr_set_free_entry((dword_t)memory, mask, type);

   //move pointer to memory
   memory += (1 << i);

   //update to new unallocated size
   size &= ~(1 << i); //clear bit
  }
 }

 //enable MTRR
 write_msr(MTRR_REG_DEFAULT_TYPE, read_msr(MTRR_REG_DEFAULT_TYPE) | (1 << 11));

 //invalidate cache
 asm("wbinvd");

 //TODO: after writing driver for virtual memory flush TLB here

 //enable caching
 write_cr0(read_cr0() & ~((1 << 29) | (1 << 30)));

 //TODO: after writing driver for virtual memory enable paging here

 //enable interrupts
 sti();
}