//BleskOS

#define PATA_MASTER 0xE0
#define PATA_SLAVE 0xF0

byte_t ide_drive_info[512];

dword_t ide_controllers_info_mem = 0, ide_controllers_pointer = 0;
byte_t ide_0x1F0_controller_present = STATUS_FALSE, ide_0x170_controller_present = STATUS_FALSE;
word_t ide_drive_type = 0;
dword_t ide_drive_size = 0, ide_hdd_size = 0;
word_t ide_hdd_base = 0, ide_hdd_alt_base = 0, ide_hdd_drive = 0, ide_cdrom_base = 0, ide_cdrom_alt_base = 0, ide_cdrom_drive = 0;

byte_t ide_wait_drive_not_busy(word_t base_port, dword_t wait_ticks);
byte_t ide_wait_drive_not_busy_with_error_status(word_t base_port, dword_t wait_ticks);
byte_t ide_wait_for_data(word_t base_port, dword_t wait_ticks);
void ide_clear_device_output(word_t base_port);
byte_t ide_reset_controller(word_t base_port, word_t alt_base_port);
void ide_select_drive(word_t base_port, byte_t drive);
byte_t ide_send_identify_drive(word_t base_port);
byte_t ide_send_identify_packet_device(word_t base_port);
void initalize_ide_controllers(void);