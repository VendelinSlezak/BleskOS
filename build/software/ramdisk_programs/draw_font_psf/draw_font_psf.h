#ifndef BUILD_SOFTWARE_RAMDISK_PROGRAMS_DRAW_FONT_PSF_DRAW_FONT_PSF_H
#define BUILD_SOFTWARE_RAMDISK_PROGRAMS_DRAW_FONT_PSF_DRAW_FONT_PSF_H


void draw_char_psf(dword_t monitor_id, dword_t ch, dword_t x, dword_t y, dword_t color);
void print_psf(dword_t monitor_id, char *string, dword_t x, dword_t y, dword_t color);

#endif /* BUILD_SOFTWARE_RAMDISK_PROGRAMS_DRAW_FONT_PSF_DRAW_FONT_PSF_H */
