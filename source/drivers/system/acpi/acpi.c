//BleskOS

/*
* MIT License
* Copyright (c) 2023-2025 BleskOS developers
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

void read_acpi_tables(void) {
 dword_t *mem, *mem_2;
 byte_t *mem_byte;
 dword_t memory_pointer = 0xE0000, number_of_entries;
 byte_t checksum;

 //set variables
 ps2_controller_present = DEVICE_PRESENT;
 acpi_pm1_control_register = 0;
 acpi_pm2_control_register = 0;
 
 //search for RSDP table
 rsdp_mem = 0;
 for(int i=0; i<0x20000; i++) {
  mem = (dword_t *) memory_pointer;
  if(*mem==0x20445352) { //signature "RSDP"
   //test RSDP checksum
   mem_byte = (byte_t *) memory_pointer;
   checksum = 0;
   for(int j=0; j<20; j++) {
    checksum += *mem_byte;
    mem_byte++;
   }
   
   //this is RSDP table
   if(checksum==0) {
    rsdp_mem = memory_pointer;
    break;
   }
  }
  memory_pointer++;
 }
 if(rsdp_mem==0) {
  logf("\n\nno RSDP table founded\n");
  return;
 }
 logf("\n\nRSDP table memory: %x\n", rsdp_mem);
 
 //RSDT table pointer
 rsdt_mem = mem[4];
 mem = (dword_t *) rsdt_mem;
 if(*mem!=0x54445352) { //signature "RSDT"
  rsdt_mem = 0;
  logf("no RSDT table founded\n");
  return;
 }
 logf("RSDT table memory: %x\n", rsdt_mem);
 number_of_entries = ((mem[1]-36)>>2);
 
 //search for HPET table
 hpet_table_mem = 0;
 mem = (dword_t *) (rsdt_mem+36);
 for(int i=0; i<number_of_entries; i++) {
  mem_2 = (dword_t *) (*mem);
  if(*mem_2==0x54455048) { //'HPET'
   hpet_table_mem = ((dword_t) mem_2);
   break;
  }
  mem++;
 }

 //search for FACP table
 facp_mem = 0;
 mem = (dword_t *) (rsdt_mem+36);
 for(int i=0; i<number_of_entries; i++) {
  mem_2 = (dword_t *) (*mem);
  if(*mem_2==0x50434146) { //'FACP'
   facp_mem = ((dword_t)mem_2);
   break;
  }
  mem++;
 }
 if(facp_mem==0) {
  logf("no FACP table founded\n");
  return;
 }
 logf("FACP table memory: %x\n", facp_mem);
 
 //read useful values from FACP
 acpi_version = mmio_inb(facp_mem + 8);
 dsdt_mem = mmio_ind(facp_mem + 40);
 acpi_command_register = mmio_inw(facp_mem + 48);
 acpi_turn_on_command = mmio_inb(facp_mem + 52);
 acpi_pm1_control_register = mmio_inw(facp_mem + 64);
 acpi_pm2_control_register = mmio_inw(facp_mem + 68);
 if(acpi_version>=3) {
  ps2_controller_present = ((mmio_inw(facp_mem + 109)>>1) & 0x1); //0 = not present 1 = present
 }

 logf("\nACPI SCI: %d", mmio_inw(facp_mem + 40 + 6));
 
 // //turn ACPI on
 // if(acpi_command_register!=0) {
 //  if((inw(acpi_command_register) & 0x1)==0x0) {
 //   outb(acpi_command_register, acpi_turn_on_command);
 //  }
 // }
 
 //read values for shutdown
 shutdown_value_pm1 = 0x2000;
 shutdown_value_pm2 = 0x2000;
 memory_pointer = dsdt_mem;
 number_of_entries = mem[1];
 for(int i=0; i<number_of_entries; i++) {
  mem = (dword_t *) (memory_pointer);
  if(*mem==0x5F35535F) { //signature "_S5_"
   shutdown_value_pm1 |= (word_t) ((mem[2] & 0xFF)<<10);
   shutdown_value_pm2 |= (word_t) (((mem[2]>>16) & 0xFF)<<10);
   break;
  }
  memory_pointer += 1;
 }
 
 //LOG
 logf("\nACPI pm1 control port: 0x%04x shutdown value for pm1: %0x4x", acpi_pm1_control_register, shutdown_value_pm1);
 logf("\nACPI pm2 control port: 0x%04x shutdown value for pm2: %0x4x", acpi_pm2_control_register, shutdown_value_pm2);
}

void shutdown(void) {
 if(acpi_pm1_control_register!=0) {
  outw(acpi_pm1_control_register, shutdown_value_pm1);
 }
 if(acpi_pm2_control_register!=0) {
  outw(acpi_pm2_control_register, shutdown_value_pm2);
 }
 for(dword_t i=0; i<1000; i++) {
  asm("hlt");
 }
}