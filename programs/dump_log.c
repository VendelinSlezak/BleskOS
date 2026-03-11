// this program assumes that BleskOS loaded PSF bitmap font at 0xE8000000 and that characters from logging are at 0x80000000

#include "syscalls.h"

syslib_t *syslib;
uint32_t *buffer_ptr = (uint32_t *) 0x80000000;
screen_buffer_t *window_buffer;
bitmap_font_t *font;

void key_up_handler(void) {
    if(buffer_ptr <= (uint32_t *) 0x80000000) {
        buffer_ptr = (uint32_t *) 0x80000000;
        syslib->redraw_gui();
        return;
    }
    buffer_ptr -= 2;
    if(buffer_ptr <= (uint32_t *) 0x80000000) {
        buffer_ptr = (uint32_t *) 0x80000000;
        syslib->redraw_gui();
        return;
    }
    if(*buffer_ptr == '\n') {
        buffer_ptr++;
        syslib->redraw_gui();
        return;
    }
    while(*buffer_ptr != '\n') {
        if(buffer_ptr == (uint32_t *) 0x80000000) {
            syslib->redraw_gui();
            return;
        }
        buffer_ptr--;
    }
    buffer_ptr++;
    syslib->redraw_gui();
}

void key_down_handler(void) {
    while(*buffer_ptr != NULL && *buffer_ptr != '\n') {
        buffer_ptr++;
    }
    if(*buffer_ptr == '\n') {
        buffer_ptr++;
    }
    syslib->redraw_gui();
}

void log_dump_canvas_redraw(uint32_t screen_x, uint32_t screen_y, uint32_t screen_width, uint32_t screen_height, uint32_t x_offset, uint32_t y_offset) {
    // we assume that offsets are always 0 and that area is at full window size
    
    uint32_t pixels = screen_width * screen_height;
    for(int i = 0; i < pixels; i++) {
        window_buffer->buffer[i] = 0x000000;
    }

    uint32_t *buffer = (uint32_t *) buffer_ptr;
    uint32_t x = screen_x; uint32_t y = screen_y;
    while(*buffer != 0) {
        if(y >= screen_height) {
            break;
        }
        if(*buffer == '\n') {
            x = 0;
            y += 16;
        }
        else {
            syslib->draw_bitmap_char(window_buffer, x, y, font, *buffer, 0xAAAAAA);
            x += 8;
        }
        buffer++;
    }
}

void main(syslib_t *syslib_ptr) {
    syslib = syslib_ptr;
    syslib->initialize(syslib);
    window_buffer = syslib->initialize_gui();
    font = syslib->load_bitmap_font((void *) 0xE8000000, 4096);
    syslib->add_canvas_component(BASE_SCREEN_AREA, log_dump_canvas_redraw);
    syslib->register_pressed_key_event_handler(KEY_UP_ARROW, key_up_handler);
    syslib->register_pressed_key_event_handler(KEY_DOWN_ARROW, key_down_handler);
    syslib->redraw_gui();

    while(true) {
        asm volatile("pause");
    }
}