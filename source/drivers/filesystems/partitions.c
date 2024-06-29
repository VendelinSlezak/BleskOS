//BleskOS

/*
* MIT License
* Copyright (c) 2023-2024 Vendelín Slezák
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

void add_partition(byte_t type, dword_t first_sector, dword_t number_of_sectors) {
 if(number_of_partitions_in_table>=MAX_NUMBER_OF_PARTITIONS_IN_PARTITION_INFO_TABLE) {
  return;
 }

 partitions[number_of_partitions_in_table].type = type;
 partitions[number_of_partitions_in_table].first_sector = first_sector;
 partitions[number_of_partitions_in_table].num_of_sectors = number_of_sectors;
 number_of_partitions_in_table++;
}

void read_partitions_of_medium(void) {
 //clear info about partitions
 clear_memory((dword_t)(&partitions), sizeof(partitions));
 
 /* OPTICAL DISK */
 //we currently support only one partition on optical disk so it is handled in special way
 if(storage_medium==MEDIUM_OPTICAL_DRIVE) {
  //test if disk is present
  if(detect_optical_disk()==STATUS_FALSE) {
   return;
  }

  //find filesystem of partition
  partitions[0].first_sector = 0;
  partitions[0].num_of_sectors = optical_disk_size;
  if(is_partition_iso9660(0)==STATUS_TRUE) { //ISO9660
   partitions[0].type = PARTITION_FILESYSTEM_ISO9660;
  }
  else if(is_optical_disk_cdda()==STATUS_TRUE) { //CDDA
   partitions[0].type = PARTITION_FILESYSTEM_CDDA;
  }
  else if(read_optical_disk_toc()==STATUS_TRUE && optical_disk_table_of_content.first_track==0) { //free disk
   partitions[0].type = PARTITION_FREE_SPACE;
  }
  else { //unknown filesystem
   partitions[0].type = PARTITION_UNKNOWN_FILESYSTEM;
  }
  
  return;
 }
 
 /* HARD DISK or USB MASS STORAGE DEVICE */
 struct mbr_sector_t mbr_sector;
 struct ebr_sector_t ebr_sector;

 //read sector of Master Boot Record
 if(read_storage_medium(0, 1, (dword_t)(&mbr_sector))==STATUS_ERROR) {
  return;
 }

 //parse informations from Master Boot Record
 number_of_partitions_in_table = 0;
 if(   mbr_sector.partition_entry[0].number_of_sectors!=0
    || mbr_sector.partition_entry[1].number_of_sectors!=0
    || mbr_sector.partition_entry[2].number_of_sectors!=0
    || mbr_sector.partition_entry[3].number_of_sectors!=0) { //there is some info about partitions
  //read partitions of Master Boot Record
  for(int i=0; i<4; i++) {
   if(mbr_sector.partition_entry[i].type==0 && mbr_sector.partition_entry[i].number_of_sectors>0) { //partition of free space
    add_partition(PARTITION_FREE_SPACE, mbr_sector.partition_entry[i].first_sector, mbr_sector.partition_entry[i].number_of_sectors);
   }
   else if(mbr_sector.partition_entry[i].type==0x05 || mbr_sector.partition_entry[i].type==0x0F) { //this partition is Extended Boot Record
    //read sector of Extended Boot Record
    if(read_storage_medium(mbr_sector.partition_entry[i].first_sector, 1, (dword_t)(&ebr_sector))==STATUS_ERROR) {
     continue;
    }

    //read partitions of Extended Boot Record
    for(dword_t j=0; j<2; j++) {
     if(ebr_sector.partition_entry[j].type==0 && mbr_sector.partition_entry[i].number_of_sectors>0) { //partition of free space
      add_partition(PARTITION_FREE_SPACE, (mbr_sector.partition_entry[i].first_sector+ebr_sector.partition_entry[j].first_sector), ebr_sector.partition_entry[j].number_of_sectors);
     }
     else { //some filesystem
      add_partition(PARTITION_UNKNOWN_FILESYSTEM, (mbr_sector.partition_entry[i].first_sector+ebr_sector.partition_entry[j].first_sector), ebr_sector.partition_entry[j].number_of_sectors);
     }
    }
   }
   else if(mbr_sector.partition_entry[i].type==0xEE) { //Global Partition Table
    dword_t gpt = malloc(512*32); //allocate memory for whole Global Partition Table
    if(read_storage_medium(mbr_sector.partition_entry[i].first_sector, 32, gpt)==STATUS_ERROR) { //read Global Partition Table
     free(gpt);
     continue;
    }

    //check valid info
    struct gpt_partition_header_t *gpt_partition_header = (struct gpt_partition_header_t *) (gpt);
    if((gpt_partition_header->first_lba_of_guid_partition_entry_array>>32)!=0 || gpt_partition_header->first_lba_of_guid_partition_entry_array==0 || gpt_partition_header->first_lba_of_guid_partition_entry_array>2 || gpt_partition_header->size_of_partition_entry<128 || (gpt_partition_header->size_of_partition_entry%128)!=0) { //partition array is saved above 2 TB / invalid partition entry size
     log("\nInvalid GPT table ");
     log_hex_with_space((gpt_partition_header->first_lba_of_guid_partition_entry_array>>32));
     log_hex_with_space((gpt_partition_header->first_lba_of_guid_partition_entry_array & 0xFFFFFFFF));
     log_var(gpt_partition_header->size_of_partition_entry);
     continue;
    }

    //read partition entry array
    struct gpt_partition_entry_t *gpt_partition_entry = (struct gpt_partition_entry_t *) (gpt+0x5C); //array may be immediately after header
    if(gpt_partition_header->first_lba_of_guid_partition_entry_array==2) { //othwewise it will start at second sector
     gpt_partition_entry = (struct gpt_partition_entry_t *) (gpt+512);
    }
    for(dword_t j=0; j<gpt_partition_header->number_of_partition_entries; j++, gpt_partition_entry = (struct gpt_partition_entry_t *) (((dword_t)gpt_partition_entry)+gpt_partition_header->size_of_partition_entry)) {
     //no partition info
     if(gpt_partition_entry->first_sector==0 && gpt_partition_entry->last_sector==0) {
      break;
     }

     //parse partition info
     if((gpt_partition_entry->first_sector>>32)==0 && (gpt_partition_entry->last_sector>>32)==0) { //we do not support partitions bigger than 2 TB
      add_partition(PARTITION_UNKNOWN_FILESYSTEM, gpt_partition_entry->first_sector, (gpt_partition_entry->last_sector-gpt_partition_entry->first_sector));
     }
    }

    //release allocated memory
    free(gpt);
   }
   else if(mbr_sector.partition_entry[i].number_of_sectors>0) { //normal partition
    add_partition(PARTITION_UNKNOWN_FILESYSTEM, mbr_sector.partition_entry[i].first_sector, mbr_sector.partition_entry[i].number_of_sectors);
   }
  }
 }
 else { //no Master Boot Record info
  add_partition(PARTITION_UNKNOWN_FILESYSTEM, 0, 0); //add whole medium as partition
 }
 
 //find type of filesystem in founded partitions
 for(int i=0; i<MAX_NUMBER_OF_PARTITIONS_IN_PARTITION_INFO_TABLE; i++) {
  if(partitions[i].type==PARTITION_UNKNOWN_FILESYSTEM) {
   if(is_partition_fat(partitions[i].first_sector)==STATUS_TRUE) {
    partitions[i].type = PARTITION_FILESYSTEM_FAT;
   }
   else if(is_partition_ext(partitions[i].first_sector)==STATUS_TRUE) {
    partitions[i].type = PARTITION_FILESYSTEM_EXT;
   }
   else if(is_partition_bleskos_bootable_partition(partitions[i].first_sector)==STATUS_TRUE) {
    partitions[i].type = PARTITION_FILESYSTEM_BLESKOS_BOOTABLE_PARTITION;
   }
  }
 }
}

void connect_partitions_of_medium(byte_t medium_type, byte_t medium_number) {
 //select medium
 select_storage_medium(medium_type, medium_number);

 //read partitions
 read_partitions_of_medium();

 //connect partitions with recognized filesystem
 for(dword_t i=0; i<number_of_partitions_in_table; i++) {
  if(partitions[i].type!=PARTITION_FREE_SPACE && partitions[i].type!=PARTITION_UNKNOWN_FILESYSTEM && partitions[i].type!=PARTITION_FILESYSTEM_BLESKOS_BOOTABLE_PARTITION) {
   add_new_partition_to_list(medium_type, medium_number, partitions[i].type, partitions[i].first_sector);
  }
 }

 //log partitions
 log("\n\nPartitions on ");
 if(medium_type==MEDIUM_HARD_DISK) {
  log("hard disk");
 }
 else if(medium_type==MEDIUM_OPTICAL_DRIVE) {
  log("optical disk");
 }
 else if(medium_type==MEDIUM_USB_MSD) {
  log("USB mass storage device");
 }
 log(":");
 for(int i=0; i<number_of_partitions_in_table; i++) {
  log("\n");
  log_hex_with_space(partitions[i].first_sector);
  log_var_with_space(partitions[i].num_of_sectors);
  if(partitions[i].type==PARTITION_FILESYSTEM_FAT) {
   log("FAT");
  }
  else if(partitions[i].type==PARTITION_FILESYSTEM_EXT) {
   log("Ext");
  }
  else if(partitions[i].type==PARTITION_FILESYSTEM_ISO9660) {
   log("ISO9660");
  }
  else if(partitions[i].type==PARTITION_FILESYSTEM_CDDA) {
   log("CDDA");
  }
  else if(partitions[i].type==PARTITION_FILESYSTEM_BLESKOS_BOOTABLE_PARTITION) {
   log("BleskOS bootable partition");
  }
  else {
   log("unknown filesystem");
  }
 }
}