#ifndef BUILD_SOFTWARE_RAMDISK_PROGRAMS_DRAW_DRAW_H
#define BUILD_SOFTWARE_RAMDISK_PROGRAMS_DRAW_DRAW_H


dword_t draw_get_default_monitor_id(void);
void draw_update_default_monitor_screen(void);
void draw_clear_screen(dword_t monitor_id, dword_t color);

#endif /* BUILD_SOFTWARE_RAMDISK_PROGRAMS_DRAW_DRAW_H */
