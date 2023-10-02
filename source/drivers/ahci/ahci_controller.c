//BleskOS

/*
* MIT License
* Copyright (c) 2023 Vendelín Slezák
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

void initalize_ahci_controllers(void) {
 ahci_hdd_base = 0;
 ahci_cdrom_base = 0;
 ahci_ports_pointer = 0;

 if(ahci_controllers_pointer==0) {
  log("\nno AHCI controller founded\n");
  return; //no AHCI controller
 }
 
 log("\n");
 log_var_with_space(ahci_controllers_pointer);
 log("AHCI controllers\n");

 //initalize controllers
 for(int i=0; i<ahci_controllers_pointer; i++) {
  ahci_base = ahci_controllers[i];

  //disable interrupts
  mmio_outd(ahci_base + 0x04, 0x80000000);

  //disable BIOS ownership
  if(mmio_ind(ahci_base + 0x10)>=0x00010200) { //in lower version this is not supported
   if((mmio_ind(ahci_base + 0x24) & 0x1)==0x1) { //BIOS ownership is supported
    mmio_outd(ahci_base + 0x28, 0x2); //we want to take ownership
    
    ticks=0;
    while(ticks<500) {
     if((mmio_ind(ahci_base + 0x28) & 0x3)==0x2) {
      break; //we have ownership
     }
    }

    continue;
   }
  }

  //scan for ports
  for(int port_num=0, mask=1; port_num<32; port_num++, mask<<=1) {
   if(ahci_ports_pointer>=32) {
    return;
   }
   
   //test if port exist
   if((mmio_ind(ahci_base + 0x0C) & mask)==mask) {
    ahci_ports[ahci_ports_pointer][0]=(ahci_base + 0x100 + (port_num*0x80)); //save port base address
    ahci_ports_pointer++;
   }
  }
 }
 
 log_var_with_space(ahci_ports_pointer);
 log("AHCI ports\n");

 if(ahci_ports_pointer==0) {
  return; //no AHCI ports
 }

 //allocate memory for AHCI ports
 ahci_hdd_cmd_mem = aligned_calloc(32*32, 0x3FF);
 ahci_cdrom_cmd_mem = aligned_calloc(32*32, 0x3FF);
 ahci_hdd_receive_fis_mem = aligned_calloc(32*32, 0xFF);
 ahci_cdrom_receive_fis_mem = aligned_calloc(32*32, 0xFF);
 ahci_hdd_fis_mem = calloc(0x90);
 ahci_cdrom_fis_mem = calloc(0x90);
 ahci_drive_info = malloc(512);

 //read port signatures
 for(dword_t i=0; i<ahci_ports_pointer; i++) {
  //TODO: reset port
  
  //read signature
  ahci_ports[i][1] = mmio_ind(ahci_ports[i][0] + 0x24);
 }

 //find hard disk
 for(int i=0; i<ahci_ports_pointer; i++) {
  if(ahci_ports[i][1]==0x00000101) {
   ahci_hdd_base = ahci_ports[i][0];
   initalize_ahci_port(ahci_hdd_base, ahci_hdd_cmd_mem, ahci_hdd_receive_fis_mem);
   sata_read_drive_info(ahci_hdd_base, ahci_hdd_cmd_mem, ahci_hdd_fis_mem);
   ahci_hdd_size = ahci_drive_size;
   hard_disk_size = ahci_hdd_size;
   break;
  }
 }
 
 //find cdrom
 for(int i=0; i<ahci_ports_pointer; i++) {
  if(ahci_ports[i][1]==0xEB140101) {
   ahci_cdrom_base = ahci_ports[i][0];
   initalize_ahci_port(ahci_cdrom_base, ahci_cdrom_cmd_mem, ahci_cdrom_receive_fis_mem);
   break;
  }
 }
 
 //LOG
 for(int i=0; i<ahci_ports_pointer; i++) {
  log("AHCI port signature: ");
  log_hex(ahci_ports[i][1]);
  log("\n");
 }
 if(ahci_hdd_base!=0) {
  log("AHCI HDD size: ");
  log_var(ahci_hdd_size);
  log("\n");
 }
}

void initalize_ahci_port(dword_t base_port, dword_t commands_memory, dword_t receive_fis_memory) {
 //stop port
 mmio_outd(base_port + 0x18, 0x6);
 wait(100);
 
 //disable interrupts
 mmio_outd(base_port + 0x14, 0x0);
 mmio_outd(base_port + 0x10, 0xFFFFFFFF);
 
 //set memory
 mmio_outd(base_port + 0x00, commands_memory);
 mmio_outd(base_port + 0x04, 0);
 mmio_outd(base_port + 0x08, receive_fis_memory);
 mmio_outd(base_port + 0x0C, 0);
 
 //disable port multiplier
 mmio_outd(base_port + 0x40, 0x0);
 
 //start port
 mmio_outd(base_port + 0x18, 0x16);
 wait(5);
 mmio_outd(base_port + 0x18, 0x17);
 wait(5);
}
