//BleskOS

/*
* MIT License
* Copyright (c) 2023 Vendelín Slezák
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

void system_board_print_hardware_item(byte_t *string) {
 print(string, 20, system_board_draw_line, BLACK);
 system_board_draw_line += 30;
}

void system_board_redraw(void) {
 clear_screen(0xA04000);

 print("Hardware", 10, 10, BLACK);
 draw_empty_square(graphic_screen_x-22*8, graphic_screen_y-10-5-8-5, 21*8, 18, BLACK);
 print("[L] Open system log", graphic_screen_x-21*8, graphic_screen_y-10-5-8, BLACK);
 draw_empty_square(10, graphic_screen_y-10-5-8-5, 27*8, 18, BLACK);
 print("[Esc] Back to Main window", 18, graphic_screen_y-10-5-8, BLACK);

 //draw background of selected device
 draw_full_square(10, 30+system_board_selected_item*30, 200, 20, WHITE);

 //read available items
 for(dword_t i=0; i<100; i++) {
  system_board_items_list[i]=0;
 }
 system_board_items_list[0]=SYSTEM_BOARD_MEMORY;
 system_board_items_list[1]=SYSTEM_BOARD_PCI;
 system_board_items_list[2]=SYSTEM_BOARD_PC_SPEAKER;
 system_board_items_list[3]=SYSTEM_BOARD_GRAPHIC;
 system_board_num_of_items = 4;
 if(rsdt_mem!=0) {
  system_board_items_list[system_board_num_of_items]=SYSTEM_BOARD_ACPI;
  system_board_num_of_items++;
 }
 if(hpet_base!=0) {
  system_board_items_list[system_board_num_of_items]=SYSTEM_BOARD_HPET_TIMER;
  system_board_num_of_items++;
 }
 if(ps2_controller_present==DEVICE_PRESENT) {
  system_board_items_list[system_board_num_of_items]=SYSTEM_BOARD_PS2_CONTROLLER;
  system_board_num_of_items++;
 }
 if(ide_hdd_base!=0) {
  system_board_items_list[system_board_num_of_items]=SYSTEM_BOARD_IDE_HDD;
  system_board_num_of_items++;
 }
 if(ide_cdrom_base!=0) {
  system_board_items_list[system_board_num_of_items]=SYSTEM_BOARD_IDE_CDROM;
  system_board_num_of_items++;
 }
 if(ahci_hdd_base!=0) {
  system_board_items_list[system_board_num_of_items]=SYSTEM_BOARD_AHCI_HDD;
  system_board_num_of_items++;
 }
 if(ahci_cdrom_base!=0) {
  system_board_items_list[system_board_num_of_items]=SYSTEM_BOARD_AHCI_CDROM;
  system_board_num_of_items++;
 }

 //draw items
 system_board_draw_line = 36;
 for(dword_t i=0; i<100; i++) {
  if(system_board_items_list[i]==0) {
   break;
  }
  else if(system_board_items_list[i]==SYSTEM_BOARD_MEMORY) {
   system_board_print_hardware_item("Memory");
  }
  else if(system_board_items_list[i]==SYSTEM_BOARD_PCI) {
   system_board_print_hardware_item("PCI");
  }
  else if(system_board_items_list[i]==SYSTEM_BOARD_PC_SPEAKER) {
   system_board_print_hardware_item("PC speaker");
  }
  else if(system_board_items_list[i]==SYSTEM_BOARD_GRAPHIC) {
   system_board_print_hardware_item("Graphic");
  }
  else if(system_board_items_list[i]==SYSTEM_BOARD_ACPI) {
   system_board_print_hardware_item("ACPI tables");
  }
  else if(system_board_items_list[i]==SYSTEM_BOARD_HPET_TIMER) {
   system_board_print_hardware_item("HPET timer");
  }
  else if(system_board_items_list[i]==SYSTEM_BOARD_PS2_CONTROLLER) {
   system_board_print_hardware_item("PS/2 controller");
  }
  else if(system_board_items_list[i]==SYSTEM_BOARD_IDE_HDD) {
   system_board_print_hardware_item("IDE hard disk");
  }
  else if(system_board_items_list[i]==SYSTEM_BOARD_IDE_CDROM) {
   system_board_print_hardware_item("IDE optical disk");
  }
  else if(system_board_items_list[i]==SYSTEM_BOARD_AHCI_HDD) {
   system_board_print_hardware_item("AHCI hard disk");
  }
  else if(system_board_items_list[i]==SYSTEM_BOARD_AHCI_CDROM) {
   system_board_print_hardware_item("AHCI optical disk drive");
  }
 }

 //draw item specific things
 if(system_board_items_list[system_board_selected_item]==SYSTEM_BOARD_MEMORY) {
  print("All memory in MB:", graphic_screen_x_center, 30, BLACK);
  print_var(all_memory_in_bytes/1024/1024, graphic_screen_x_center+18*8, 30, BLACK);

  print("All free memory in MB:", graphic_screen_x_center, 50, BLACK);
  print_var(all_free_memory_in_bytes/1024/1024, graphic_screen_x_center+23*8, 50, BLACK);

  print("All actual free memory in MB:", graphic_screen_x_center, 70, BLACK);
  print_var(all_actual_free_memory_in_bytes/1024/1024, graphic_screen_x_center+30*8, 70, BLACK);

  dword_t *memory_entries = (dword_t *) (mem_memory_entries+system_item_variabile*16);
  dword_t num_of_printed_items = 0, num_of_max_printable_items = ((graphic_screen_y-100)/10);

  for(dword_t i=0; i<(0xFFFF-system_item_variabile) && num_of_printed_items<num_of_max_printable_items; i++) {
   if(memory_entries[MEM_ENTRY_TYPE]==MEM_FREE_MEMORY) {
    print("Free memory", graphic_screen_x_center, 90+num_of_printed_items*10, BLACK);
    print_hex(memory_entries[MEM_ENTRY_START], graphic_screen_x_center+12*8, 90+num_of_printed_items*10, BLACK); //start of memory
    print_var(memory_entries[MEM_ENTRY_LENGTH], graphic_screen_x_center+12*8+11*8, 90+num_of_printed_items*10, BLACK); //length in B
    print_var(memory_entries[MEM_ENTRY_LENGTH]/1024/1024, graphic_screen_x_center+12*8+11*8+10*8, 90+num_of_printed_items*10, BLACK); //length in MB
    num_of_printed_items++;
   }
   else if(memory_entries[MEM_ENTRY_TYPE]==MEM_ALLOCATED_MEMORY) {
    print("Used memory", graphic_screen_x_center, 90+num_of_printed_items*10, BLACK);
    print_hex(memory_entries[MEM_ENTRY_START], graphic_screen_x_center+12*8, 90+num_of_printed_items*10, BLACK); //start of memory
    print_var(memory_entries[MEM_ENTRY_LENGTH], graphic_screen_x_center+12*8+11*8, 90+num_of_printed_items*10, BLACK); //length in B
    print_var(memory_entries[MEM_ENTRY_LENGTH]/1024/1024, graphic_screen_x_center+12*8+11*8+10*8, 90+num_of_printed_items*10, BLACK); //length in MB
    num_of_printed_items++;
   }

   memory_entries+=4;
  }
 }
 else if(system_board_items_list[system_board_selected_item]==SYSTEM_BOARD_PCI) {
  dword_t *pci_devices_array = (dword_t *) (pci_devices_array_mem+system_item_variabile*12);
  dword_t num_of_printed_items = 0, num_of_max_printable_items = ((graphic_screen_y-20)/10);

  for(dword_t i=0; i<(pci_num_of_devices-system_item_variabile) && num_of_printed_items<num_of_max_printable_items; i++) {
   if(pci_devices_array[1]==0x020000) {
    print(("Ethernet card"), graphic_screen_x_center, 10+num_of_printed_items*10, BLACK); //device type
   }
   else if(pci_devices_array[1]==0x040100) {
    print(("Sound card"), graphic_screen_x_center, 10+num_of_printed_items*10, BLACK); //device type
   }
   else if(pci_devices_array[1]==0x040300) {
    print(("HDA sound card"), graphic_screen_x_center, 10+num_of_printed_items*10, BLACK); //device type
   }
   else if(pci_devices_array[1]==0x0C0300) {
    print(("UHCI"), graphic_screen_x_center, 10+num_of_printed_items*10, BLACK); //device type
   }
   else if(pci_devices_array[1]==0x0C0310) {
    print(("OHCI"), graphic_screen_x_center, 10+num_of_printed_items*10, BLACK); //device type
   }
   else if(pci_devices_array[1]==0x0C0320) {
    print(("EHCI"), graphic_screen_x_center, 10+num_of_printed_items*10, BLACK); //device type
   }
   else if(pci_devices_array[1]==0x0C0330) {
    print(("xHCI"), graphic_screen_x_center, 10+num_of_printed_items*10, BLACK); //device type
   }
   else {
    print_hex(pci_devices_array[1], graphic_screen_x_center, 10+num_of_printed_items*10, BLACK); //device type
   }

   //vendor ID
   byte_t *vendor_id_string = get_pci_vendor_string((pci_devices_array[2] & 0xFFFF));
   if(vendor_id_string[0]==0) {
    print_hex((pci_devices_array[2] & 0xFFFF), graphic_screen_x_center+15*8, 10+num_of_printed_items*10, BLACK);
   }
   else {
    print(vendor_id_string, graphic_screen_x_center+15*8, 10+num_of_printed_items*10, BLACK);
   }

   print_hex((pci_devices_array[2] >> 16), graphic_screen_x_center+30*8, 10+num_of_printed_items*10, BLACK); //device ID
   num_of_printed_items++;
   pci_devices_array+=3;
  }
 }
 else if(system_board_items_list[system_board_selected_item]==SYSTEM_BOARD_PC_SPEAKER) {
  print("PC speaker port:", graphic_screen_x_center, 10, BLACK);
  print_hex(inb(0x61), graphic_screen_x_center+17*8, 10, BLACK);
  print("[b] Beep [s] Stop beep", graphic_screen_x_center, 30, BLACK);
 }
 else if(system_board_items_list[system_board_selected_item]==SYSTEM_BOARD_GRAPHIC) {
  print("Graphic mode:", graphic_screen_x_center, 10, BLACK);
  dword_t column = graphic_screen_x_center+14*8;
  print_var(graphic_screen_x, column, 10, BLACK);
  if(graphic_screen_x<1000) {
   column += 3*8;
  }
  else {
   column += 4*8;
  }
  print("x", column, 10, BLACK);
  column += 1*8;
  print_var(graphic_screen_y, column, 10, BLACK);
  if(graphic_screen_y<1000) {
   column += 3*8;
  }
  else {
   column += 4*8;
  }
  print("x", column, 10, BLACK);
  column += 1*8;
  print_var(graphic_screen_bpp, column, 10, BLACK);

  print("Best mode:", graphic_screen_x_center, 30, BLACK);
  if(is_edid_present==STATUS_FALSE) {
   print("EDID not founded", graphic_screen_x_center+11*8, 30, BLACK);
  }
  else {
   dword_t column = graphic_screen_x_center+11*8;
   print_var(mode_horizontal_active, column, 30, BLACK);
   if(mode_horizontal_active<1000) {
    column += 3*8;
   }
   else {
    column += 4*8;
   }
   print("x", column, 30, BLACK);
   column += 1*8;
   print_var(mode_vertical_active, column, 30, BLACK);
   if(mode_vertical_active<1000) {
    column += 3*8;
   }
   else {
    column += 4*8;
   }
  }

  print("VBE version:  .", graphic_screen_x_center, 50, BLACK);
  byte_t *vesa_version = (byte_t *) (0x3004);
  print_var(vesa_version[1], graphic_screen_x_center+13*8, 50, BLACK);
  print_var(vesa_version[0], graphic_screen_x_center+15*8, 50, BLACK);

  print("VBE OEM:", graphic_screen_x_center, 70, BLACK);
  word_t *vesa_info_block = (word_t *) (0x3006);
  byte_t *vesa_oem_string = (byte_t *) (vesa_info_block[1]*0x10+vesa_info_block[0]);
  dword_t num_of_chars_in_oem_string = ((graphic_screen_x_center-10)/8-9);
  dword_t vesa_oem_string_mem = malloc(num_of_chars_in_oem_string);
  byte_t *vesa_copied_oem_string = (byte_t *) (vesa_oem_string_mem);
  for(dword_t i=0; i<num_of_chars_in_oem_string; i++) {
   vesa_copied_oem_string[i]=vesa_oem_string[i];
  }
  vesa_copied_oem_string[num_of_chars_in_oem_string]=0;
  print_ascii(vesa_copied_oem_string, graphic_screen_x_center+9*8, 70, BLACK);
  free(vesa_oem_string_mem);
 }
 else if(system_board_items_list[system_board_selected_item]==SYSTEM_BOARD_ACPI) {
  dword_t *rsdt = (dword_t *) (rsdt_mem);
  dword_t num_of_acpi_tables = ((rsdt[1]-36)>>2);
  dword_t table_string[2];

  if(num_of_acpi_tables==0) {
   print("No ACPI tables founded", graphic_screen_x_center, 10, BLACK);
  }
  else {
   rsdt = (dword_t *) (rsdt_mem+36);
   dword_t line = 10;

   for(dword_t i=0; i<num_of_acpi_tables; i++) {
    dword_t *table = (dword_t *) (*rsdt);
    table_string[0] = *table;
    table_string[1] = 0;
    print((byte_t *)(&table_string), graphic_screen_x_center, line, BLACK);
    line += 10;
    rsdt++;
   }
  }
 }
 else if(system_board_items_list[system_board_selected_item]==SYSTEM_BOARD_HPET_TIMER) {
  print("Capabilites:", graphic_screen_x_center, 30, BLACK);
  print_hex(mmio_ind(hpet_base+0x00), graphic_screen_x_center+13*8, 30, BLACK);
  print("Nanoseconds of one tick:", graphic_screen_x_center, 50, BLACK);
  print_var(hpet_one_tick_how_many_nanoseconds, graphic_screen_x_center+25*8, 50, BLACK);
  print("Actual timer:", graphic_screen_x_center, 70, BLACK);
  print_var(mmio_ind(hpet_base+0xF0), graphic_screen_x_center+14*8, 70, BLACK);
  print("[space] Show timer [R] Reset timer", graphic_screen_x_center, 90, BLACK);
 }
 else if(system_board_items_list[system_board_selected_item]==SYSTEM_BOARD_PS2_CONTROLLER) {
  print("First channel:", graphic_screen_x_center, 10, BLACK);
  if(ps2_first_channel_present==DEVICE_NOT_PRESENT) {
   print("Not present", graphic_screen_x_center+15*8, 10, BLACK);
  }
  else {
   if(ps2_first_channel_device==PS2_CHANNEL_NO_DEVICE_CONNECTED) {
    print("No device connected", graphic_screen_x_center+15*8, 10, BLACK);
   }
   else if(ps2_first_channel_device==PS2_CHANNEL_KEYBOARD_CONNECTED) {
    print("Keyboard connected", graphic_screen_x_center+15*8, 10, BLACK);
   }
   else if(ps2_first_channel_device==PS2_CHANNEL_KEYBOARD_INITALIZED) {
    print("Keyboard", graphic_screen_x_center+15*8, 10, BLACK);
   }
   else if(ps2_first_channel_device==PS2_CHANNEL_MOUSE_CONNECTED) {
    print("Mouse connected", graphic_screen_x_center+15*8, 10, BLACK);
   }
   else if(ps2_first_channel_device==PS2_CHANNEL_MOUSE_INITALIZED) {
    if(ps2_first_channel_mouse_data_bytes==4) {
     print("Mouse with wheel", graphic_screen_x_center+15*8, 30, BLACK);
    }
    else {
     print("Mouse", graphic_screen_x_center+15*8, 30, BLACK);
    }
   }
   else {
    print_hex(ps2_first_channel_device, graphic_screen_x_center+15*8, 10, BLACK);
   }
  }

  print("Second channel:", graphic_screen_x_center, 30, BLACK);
  if(ps2_second_channel_present==DEVICE_NOT_PRESENT) {
   print("Not present", graphic_screen_x_center+16*8, 30, BLACK);
  }
  else {
   if(ps2_second_channel_device==PS2_CHANNEL_NO_DEVICE_CONNECTED) {
    print("No device connected", graphic_screen_x_center+16*8, 30, BLACK);
   }
   else if(ps2_second_channel_device==PS2_CHANNEL_KEYBOARD_CONNECTED) {
    print("Keyboard connected", graphic_screen_x_center+16*8, 30, BLACK);
   }
   else if(ps2_second_channel_device==PS2_CHANNEL_KEYBOARD_INITALIZED) {
    print("Keyboard", graphic_screen_x_center+16*8, 30, BLACK);
   }
   else if(ps2_second_channel_device==PS2_CHANNEL_MOUSE_CONNECTED) {
    print("Mouse connected", graphic_screen_x_center+16*8, 30, BLACK);
   }
   else if(ps2_second_channel_device==PS2_CHANNEL_MOUSE_INITALIZED) {
    if(ps2_second_channel_mouse_data_bytes==4) {
     print("Mouse with wheel", graphic_screen_x_center+16*8, 30, BLACK);
    }
    else {
     print("Mouse", graphic_screen_x_center+16*8, 30, BLACK);
    }
   }
   else {
    print_hex(ps2_second_channel_device, graphic_screen_x_center+16*8, 30, BLACK);
   }
  }
 }
 else if(system_board_items_list[system_board_selected_item]==SYSTEM_BOARD_IDE_HDD) {
  print("Status register:", graphic_screen_x_center, 30, BLACK);
  print_hex(inb(ide_hdd_base+7), graphic_screen_x_center+17*8, 30, BLACK);
  print("Error register:", graphic_screen_x_center, 50, BLACK);
  print_hex(inb(ide_hdd_base+1), graphic_screen_x_center+16*8, 50, BLACK);
  print("Size in sectors:", graphic_screen_x_center, 70, BLACK);
  print_var(hard_disk_size, graphic_screen_x_center+17*8, 70, BLACK);
  print("Size in GB:", graphic_screen_x_center, 90, BLACK);
  print_var(hard_disk_size/2048/1024, graphic_screen_x_center+12*8, 90, BLACK);
 }
 else if(system_board_items_list[system_board_selected_item]==SYSTEM_BOARD_IDE_CDROM) {
  print("Status register:", graphic_screen_x_center, 30, BLACK);
  print_hex(inb(ide_cdrom_base+7), graphic_screen_x_center+17*8, 30, BLACK);
  print("Error register:", graphic_screen_x_center, 50, BLACK);
  print_hex(inb(ide_cdrom_base+1), graphic_screen_x_center+16*8, 50, BLACK);
  print("[d] Detect disk [e] Eject drive", graphic_screen_x_center, 90, BLACK);
  if(detect_optical_disk()==STATUS_TRUE) {
   print("Optical disk inserted", graphic_screen_x_center, 70, BLACK);
  }
  else {
   print("Optical disk not inserted", graphic_screen_x_center, 70, BLACK);
  }
 }
 else if(system_board_items_list[system_board_selected_item]==SYSTEM_BOARD_AHCI_HDD) {
  /*print("Status register:", graphic_screen_x_center, 30, BLACK);
  print_hex(inb(ide_hdd_base+7), graphic_screen_x_center+17*8, 30, BLACK);
  print("Error register:", graphic_screen_x_center, 50, BLACK);
  print_hex(inb(ide_hdd_base+1), graphic_screen_x_center+16*8, 50, BLACK);*/
  print("Size in sectors:", graphic_screen_x_center, 70, BLACK);
  print_var(hard_disk_size, graphic_screen_x_center+17*8, 70, BLACK);
  print("Size in GB:", graphic_screen_x_center, 90, BLACK);
  print_var(hard_disk_size/2048/1024, graphic_screen_x_center+12*8, 90, BLACK);
 }
 else if(system_board_items_list[system_board_selected_item]==SYSTEM_BOARD_AHCI_CDROM) {
  /*print("Status register:", graphic_screen_x_center, 30, BLACK);
  print_hex(inb(ide_cdrom_base+7), graphic_screen_x_center+17*8, 30, BLACK);
  print("Error register:", graphic_screen_x_center, 50, BLACK);
  print_hex(inb(ide_cdrom_base+1), graphic_screen_x_center+16*8, 50, BLACK);*/
  print("[d] Detect disk [e] Eject drive", graphic_screen_x_center, 90, BLACK);
  if(detect_optical_disk()==STATUS_TRUE) {
   print("Optical disk inserted", graphic_screen_x_center, 70, BLACK);
  }
  else {
   print("Optical disk not inserted", graphic_screen_x_center, 70, BLACK);
  }
 }
}

void system_board(void) {
 system_board_selected_item = 0;
 system_item_variabile = 0;

 redraw:
 system_board_redraw();
 mouse_cursor_save_background(mouse_cursor_x, mouse_cursor_y);
 draw_mouse_cursor(mouse_cursor_x, mouse_cursor_y);
 redraw_screen();

 while(1) {
  wait_for_usb_mouse();
  move_mouse_cursor();

  if(keyboard_value==KEY_ESC || (mouse_drag_and_drop==MOUSE_CLICK && is_mouse_in_zone(graphic_screen_y-10-5-8-5, graphic_screen_y-10, 10, 10+27*8)==STATUS_TRUE)) {
   return;
  }

  if(keyboard_value==KEY_L) {
   developer_program_log();
   goto redraw;
  }

  if(keyboard_value==KEY_UP) {
   if(system_board_selected_item>0) {
    system_board_selected_item--;
    system_item_variabile = 0;
    goto redraw;
   }
  }

  if(keyboard_value==KEY_DOWN) {
   if(system_board_selected_item<(system_board_num_of_items-1)) {
    system_board_selected_item++;
    system_item_variabile = 0;
    goto redraw;
   }
  }

  if(mouse_drag_and_drop==MOUSE_CLICK) {
   if(is_mouse_in_zone(graphic_screen_y-10-5-8-5, graphic_screen_y-10, graphic_screen_x-22*8, graphic_screen_x-8)==STATUS_TRUE) {
    developer_program_log();
    goto redraw;
   }
   else if(is_mouse_in_zone(30, graphic_screen_y-30, 10, 210)==STATUS_TRUE) {
    if(((mouse_cursor_y-30)/30)<system_board_num_of_items) {
     system_board_selected_item = ((mouse_cursor_y-30)/30);
     system_item_variabile = 0;
     goto redraw;
    }
   }
  }

  //hardware specific things
  if(system_board_items_list[system_board_selected_item]==SYSTEM_BOARD_MEMORY) {
   if(keyboard_value==KEY_LEFT && system_item_variabile>0) {
    system_item_variabile--;
    goto redraw;
   }
   else if(keyboard_value==KEY_RIGHT && system_item_variabile<0xFFFF) {
    system_item_variabile++;
    goto redraw;
   }
  }
  else if(system_board_items_list[system_board_selected_item]==SYSTEM_BOARD_PCI) {
   if(keyboard_value==KEY_SPACE) {
    byte_t *pci_device = (byte_t *) (pci_devices_array_mem+system_item_variabile*12);

    //draw info about device
    draw_full_square(graphic_screen_x_center, 10, graphic_screen_x_center-10, graphic_screen_y-20, 0xA04000);
    print("Vendor ID:", graphic_screen_x_center, 10, BLACK);
    byte_t *vendor_id_string = get_pci_vendor_string((pci_read(pci_device[0], pci_device[1], pci_device[2], 0x00) & 0xFFFF));
    if(vendor_id_string[0]==0) {
     print_hex((pci_read(pci_device[0], pci_device[1], pci_device[2], 0x00) & 0xFFFF), graphic_screen_x_center+11*8, 10, BLACK);
    }
    else {
     print(vendor_id_string, graphic_screen_x_center+11*8, 10, BLACK);
    }
    print("Device ID:", graphic_screen_x_center, 30, BLACK);
    print_hex((pci_read(pci_device[0], pci_device[1], pci_device[2], 0x00) >> 16), graphic_screen_x_center+11*8, 30, BLACK);
    print("Command:", graphic_screen_x_center, 50, BLACK);
    print_hex((pci_read(pci_device[0], pci_device[1], pci_device[2], 0x04) & 0xFFFF), graphic_screen_x_center+9*8, 50, BLACK);
    print("Status:", graphic_screen_x_center, 70, BLACK);
    print_hex((pci_read(pci_device[0], pci_device[1], pci_device[2], 0x04) >> 16), graphic_screen_x_center+8*8, 70, BLACK);
    print("Device type:", graphic_screen_x_center, 90, BLACK);
    print_hex(pci_read(pci_device[0], pci_device[1], pci_device[2], 0x08), graphic_screen_x_center+14*8, 90, BLACK);
    print("Class:", graphic_screen_x_center, 110, BLACK);
    if((pci_read(pci_device[0], pci_device[1], pci_device[2], 0x08)>>24)==0) {
     print("Unclassified", graphic_screen_x_center+7*8, 110, BLACK);
    }
    else if((pci_read(pci_device[0], pci_device[1], pci_device[2], 0x08)>>24)==1) {
     print("Mass Storage Controller", graphic_screen_x_center+7*8, 110, BLACK);
    }
    else if((pci_read(pci_device[0], pci_device[1], pci_device[2], 0x08)>>24)==2) {
     print("Network Controller", graphic_screen_x_center+7*8, 110, BLACK);
    }
    else if((pci_read(pci_device[0], pci_device[1], pci_device[2], 0x08)>>24)==3) {
     print("Display Controller", graphic_screen_x_center+7*8, 110, BLACK);
    }
    else if((pci_read(pci_device[0], pci_device[1], pci_device[2], 0x08)>>24)==4) {
     print("Multimedia Controller", graphic_screen_x_center+7*8, 110, BLACK);
    }
    else if((pci_read(pci_device[0], pci_device[1], pci_device[2], 0x08)>>24)==5) {
     print("Memory Controller", graphic_screen_x_center+7*8, 110, BLACK);
    }
    else if((pci_read(pci_device[0], pci_device[1], pci_device[2], 0x08)>>24)==6) {
     print("Bridge", graphic_screen_x_center+7*8, 110, BLACK);
    }
    else if((pci_read(pci_device[0], pci_device[1], pci_device[2], 0x08)>>24)==7) {
     print("Simple Communication Controller", graphic_screen_x_center+7*8, 110, BLACK);
    }
    else if((pci_read(pci_device[0], pci_device[1], pci_device[2], 0x08)>>24)==8) {
     print("Base System Pheriphal", graphic_screen_x_center+7*8, 110, BLACK);
    }
    else if((pci_read(pci_device[0], pci_device[1], pci_device[2], 0x08)>>24)==9) {
     print("Input Device Controller", graphic_screen_x_center+7*8, 110, BLACK);
    }
    else if((pci_read(pci_device[0], pci_device[1], pci_device[2], 0x08)>>24)==10) {
     print("Docking Station", graphic_screen_x_center+7*8, 110, BLACK);
    }
    else if((pci_read(pci_device[0], pci_device[1], pci_device[2], 0x08)>>24)==11) {
     print("Processor", graphic_screen_x_center+7*8, 110, BLACK);
    }
    else if((pci_read(pci_device[0], pci_device[1], pci_device[2], 0x08)>>24)==12) {
     print("Serial Bus Controller", graphic_screen_x_center+7*8, 110, BLACK);
    }
    else if((pci_read(pci_device[0], pci_device[1], pci_device[2], 0x08)>>24)==13) {
     print("SWireless Controller", graphic_screen_x_center+7*8, 110, BLACK);
    }
    else if((pci_read(pci_device[0], pci_device[1], pci_device[2], 0x08)>>24)==14) {
     print("Intelligent Controller", graphic_screen_x_center+7*8, 110, BLACK);
    }
    else if((pci_read(pci_device[0], pci_device[1], pci_device[2], 0x08)>>24)==15) {
     print("Satellite Communication Controller", graphic_screen_x_center+7*8, 110, BLACK);
    }
    else if((pci_read(pci_device[0], pci_device[1], pci_device[2], 0x08)>>24)==16) {
     print("Encryption Controller", graphic_screen_x_center+7*8, 110, BLACK);
    }
    else {
     print("Other", graphic_screen_x_center+7*8, 110, BLACK);
    }
    print("BAR0:", graphic_screen_x_center, 130, BLACK);
    print_hex(pci_read(pci_device[0], pci_device[1], pci_device[2], 0x10), graphic_screen_x_center+6*8, 130, BLACK);
    print("BAR1:", graphic_screen_x_center, 150, BLACK);
    print_hex(pci_read(pci_device[0], pci_device[1], pci_device[2], 0x14), graphic_screen_x_center+6*8, 150, BLACK);
    print("BAR2:", graphic_screen_x_center, 170, BLACK);
    print_hex(pci_read(pci_device[0], pci_device[1], pci_device[2], 0x18), graphic_screen_x_center+6*8, 170, BLACK);
    print("BAR3:", graphic_screen_x_center, 190, BLACK);
    print_hex(pci_read(pci_device[0], pci_device[1], pci_device[2], 0x1C), graphic_screen_x_center+6*8, 190, BLACK);
    print("BAR4:", graphic_screen_x_center, 210, BLACK);
    print_hex(pci_read(pci_device[0], pci_device[1], pci_device[2], 0x20), graphic_screen_x_center+6*8, 210, BLACK);
    print("BAR5:", graphic_screen_x_center, 230, BLACK);
    print_hex(pci_read(pci_device[0], pci_device[1], pci_device[2], 0x24), graphic_screen_x_center+6*8, 230, BLACK);
    print("IRQ:", graphic_screen_x_center, 250, BLACK);
    print_var((pci_read(pci_device[0], pci_device[1], pci_device[2], 0x3C) & 0xFF), graphic_screen_x_center+5*8, 250, BLACK);

    redraw_screen();
    
    while(1) {
     wait_for_usb_keyboard();

     if(keyboard_value==KEY_SPACE || keyboard_value==KEY_LEFT || keyboard_value==KEY_RIGHT) {
      goto redraw;
     }
    }
   }

   if(keyboard_value==KEY_LEFT && system_item_variabile>0) {
    system_item_variabile--;
    goto redraw;
   }
   else if(keyboard_value==KEY_RIGHT && system_item_variabile<(pci_num_of_devices-1)) {
    system_item_variabile++;
    goto redraw;
   }
  }
  else if(system_board_items_list[system_board_selected_item]==SYSTEM_BOARD_PC_SPEAKER) {
   if(keyboard_value==KEY_B) {
    pc_speaker_beep(500);
    goto redraw;
   }
   else if(keyboard_value==KEY_S) {
    pc_speaker_mute();
    goto redraw;
   }
  }
  else if(system_board_items_list[system_board_selected_item]==SYSTEM_BOARD_HPET_TIMER) {
   if(keyboard_value==KEY_SPACE) {
    goto redraw;
   }
   else if(keyboard_value==KEY_R) {
    hpet_reset_counter();
    goto redraw;
   }
  }
  else if(system_board_items_list[system_board_selected_item]==SYSTEM_BOARD_IDE_CDROM) {
   if(keyboard_value==KEY_D) {
    goto redraw;
   }
   else if(keyboard_value==KEY_E) {
    eject_optical_disk();
    goto redraw;
   }
  }

 }
}