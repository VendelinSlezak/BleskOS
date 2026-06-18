/* 
* BleskOS
*
* MIT License
* Copyright (c) 2023-2026 BleskOS developers
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

/* includes */
#include <kernel/hardware/devices/cpu/commands.h>
#include <kernel/hardware/main.h>
#include <kernel/hardware/devices/memory/memory_allocators.h>
#include <kernel/hardware/devices/memory/virtual_memory.h>
#include <kernel/libc/string.h>
#include <kernel/hardware/groups/graphic_output/graphic_output.h>
#include <kernel/hardware/groups/logging/logging.h>

/* local variables */
static graphic_output_group_device_functions_t monitor_functions = {
    .get_output_width = get_monitor_width,
    .get_output_height = get_monitor_height,
    .get_output_bpp = get_monitor_bpp,
    .get_bytes_per_line = get_monitor_bytes_per_line,
    .get_linear_frame_buffer = get_monitor_linear_frame_buffer,
    .get_size_of_output_buffer = get_monitor_size_of_output_buffer,
    .redraw_full_screen = monitor_redraw_full_screen,
    .redraw_part_of_screen = monitor_redraw_part_of_screen
};

/* functions */
void initialize_monitor_device(hardware_t *monitor) {
    monitor_data_t *data = kalloc(sizeof(monitor_data_t));
    monitor->data = data;
    monitor_controller_data_t *data_from_controller = (monitor_controller_data_t *) monitor->data_from_controller;
    monitor_communication_functions_t *functions = (monitor_communication_functions_t *) monitor->communication_functions;
    functions->change_resolution(monitor, data_from_controller->best_mode);
    data->actual_mode = data_from_controller->best_mode;
    add_graphic_output_device(monitor, &monitor_functions);
    log("\n[MONITOR] Monitor %d has been added", monitor->id);
    monitor->is_initialized = true;
}

void monitor_mode_add_functions(monitor_mode_t *mode) {
    void (*redraw_full_screen)(hardware_t *monitor, void *double_buffer) = NULL;
    void (*redraw_part_of_screen)(hardware_t *monitor, uint32_t monitor_x, uint32_t monitor_y, void *double_buffer, uint32_t buffer_pixels_per_line, uint32_t buffer_x, uint32_t buffer_y, uint32_t width, uint32_t height) = NULL;
    uint32_t bpp = mode->bpp;
    uint32_t width = mode->active_width;
    uint32_t height = mode->active_height;
    uint32_t bytes_per_line = mode->bytes_per_line;
    if(bpp == 32) {
        if(bytes_per_line == width*4) {
            mode->redraw_cycles = width*height;
            redraw_full_screen = monitor_redraw_screen_32bpp_no_padding;

            // TODO: implement sse
            // redraw_full_screen = monitor_redraw_screen_32bpp_no_padding_sse2;
        }
        else {
            mode->redraw_cycles = height;
            redraw_full_screen = monitor_redraw_screen_32bpp_with_padding;
        }
        redraw_part_of_screen = monitor_redraw_part_of_screen_32bpp;
    }
    else if(bpp == 24) {
        if(bytes_per_line == width*3) {
            mode->redraw_cycles = width*height;
            redraw_full_screen = monitor_redraw_screen_24bpp_no_padding;
        }
        else {
            mode->redraw_cycles = height;
            redraw_full_screen = monitor_redraw_screen_24bpp_with_padding;
        }
        redraw_part_of_screen = monitor_redraw_part_of_screen_24bpp;
    }
    else if(bpp == 16) {
        if(bytes_per_line == width*2) {
            mode->redraw_cycles = width*height;
            redraw_full_screen = monitor_redraw_screen_16bpp_no_padding;
        }
        else {
            mode->redraw_cycles = height;
            redraw_full_screen = monitor_redraw_screen_16bpp_with_padding;
        }
        redraw_part_of_screen = monitor_redraw_part_of_screen_16bpp;
    }
    else if(bpp == 15) {
        if(bytes_per_line == width*2) {
            mode->redraw_cycles = width*height;
            redraw_full_screen = monitor_redraw_screen_15bpp_no_padding;
        }
        else {
            mode->redraw_cycles = height;
            redraw_full_screen = monitor_redraw_screen_15bpp_with_padding;
        }
        redraw_part_of_screen = monitor_redraw_part_of_screen_15bpp;
    }
    else if(bpp == 8) {
        if(bytes_per_line == width*1) {
            mode->redraw_cycles = width*height;
            redraw_full_screen = monitor_redraw_screen_8bpp_no_padding;
        }
        else {
            mode->redraw_cycles = height;
            redraw_full_screen = monitor_redraw_screen_8bpp_with_padding;
        }
        redraw_part_of_screen = monitor_redraw_part_of_screen_8bpp;
    }
    else {
        log("\n[DEFAULT GRAPHIC CONTROLLER] ERROR: Unsupported BPP mode selected");
    }
    mode->redraw_full_screen = redraw_full_screen;
    mode->redraw_part_of_screen = redraw_part_of_screen;
}

uint32_t get_monitor_width(hardware_t *monitor) {
    monitor_data_t *data = monitor->data;
    return data->actual_mode->active_width;
}

uint32_t get_monitor_height(hardware_t *monitor) {
    monitor_data_t *data = monitor->data;
    return data->actual_mode->active_height;
}

uint32_t get_monitor_bpp(hardware_t *monitor) {
    monitor_data_t *data = monitor->data;
    return data->actual_mode->bpp;
}

uint32_t get_monitor_bytes_per_line(hardware_t *monitor) {
    monitor_data_t *data = monitor->data;
    return data->actual_mode->bytes_per_line;
}

void *get_monitor_linear_frame_buffer(hardware_t *monitor) {
    monitor_data_t *data = monitor->data;
    return data->actual_mode->linear_frame_buffer;
}

uint32_t get_monitor_size_of_output_buffer(hardware_t *monitor) {
    monitor_data_t *data = monitor->data;
    return data->actual_mode->active_width * data->actual_mode->active_height * 4;
}

void monitor_redraw_full_screen(hardware_t *monitor, void *double_buffer) {
    monitor_data_t *data = monitor->data;
    data->actual_mode->redraw_full_screen(monitor, double_buffer);
}

void monitor_redraw_part_of_screen(hardware_t *monitor, uint32_t monitor_x, uint32_t monitor_y, void *double_buffer, uint32_t buffer_pixels_per_line, uint32_t buffer_x, uint32_t buffer_y, uint32_t width, uint32_t height) {
    monitor_data_t *data = monitor->data;
    data->actual_mode->redraw_part_of_screen(monitor, monitor_x, monitor_y, double_buffer, buffer_pixels_per_line, buffer_x, buffer_y, width, height);
}

/* functions for redrawing full screen */
void monitor_redraw_screen_32bpp_no_padding(hardware_t *monitor, void *double_buffer) {
    monitor_data_t *data = monitor->data;
    void *dst = data->actual_mode->linear_frame_buffer;
    void *src = double_buffer;
    uint32_t pixels = (uint32_t) data->actual_mode->redraw_cycles;

    // copy all pixels
    asm volatile (
        "rep movsl"
        : "+D"(dst), "+S"(src), "+c"(pixels)
        :
        : "memory"
    );
}

void monitor_redraw_screen_32bpp_no_padding_sse2(hardware_t *monitor, void *double_buffer) {
    monitor_data_t *data = monitor->data;
    void *dst = data->actual_mode->linear_frame_buffer;
    void *src = double_buffer;
    
    uint32_t total_pixels = (uint32_t) data->actual_mode->redraw_cycles;
    uint32_t unrolled_loops = total_pixels / 16;
    uint32_t remaining_pixels = total_pixels % 16;

    asm volatile (
        // save registers
        "sub $64, %%esp\n\t"
        "movups %%xmm0, 0(%%esp)\n\t"
        "movups %%xmm1, 16(%%esp)\n\t"
        "movups %%xmm2, 32(%%esp)\n\t"
        "movups %%xmm3, 48(%%esp)\n\t"

        // copy pixels in unrolled loops
        "test %%ecx, %%ecx\n\t"
        "jz 2f\n\t"
        "1:\n\t"
            "movaps 0(%%esi),  %%xmm0\n\t"
            "movaps 16(%%esi), %%xmm1\n\t"
            "movaps 32(%%esi), %%xmm2\n\t"
            "movaps 48(%%esi), %%xmm3\n\t"
            
            "movntps %%xmm0, 0(%%edi)\n\t"
            "movntps %%xmm1, 16(%%edi)\n\t"
            "movntps %%xmm2, 32(%%edi)\n\t"
            "movntps %%xmm3, 48(%%edi)\n\t"

            "add $64, %%esi\n\t"
            "add $64, %%edi\n\t"
        "loop 1b\n\t"

        // copy remaining pixels
        "2:\n\t"
        "mov %3, %%ecx\n\t"
        "rep movsl\n\t"

        // restore registers
        "movups 0(%%esp),  %%xmm0\n\t"
        "movups 16(%%esp), %%xmm1\n\t"
        "movups 32(%%esp), %%xmm2\n\t"
        "movups 48(%%esp), %%xmm3\n\t"
        "add $64, %%esp\n\t"

        : "+D"(dst), "+S"(src), "+c"(unrolled_loops)
        : "r"(remaining_pixels)
        : "memory"
    );
}

void monitor_redraw_screen_32bpp_with_padding(hardware_t *monitor, void *double_buffer) {
    monitor_data_t *data = monitor->data;
    uint32_t *dst = data->actual_mode->linear_frame_buffer;
    uint32_t *src = double_buffer;

    uint32_t rows = data->actual_mode->redraw_cycles;
    uint32_t pixels_per_line = data->actual_mode->active_width;
    uint32_t padding_bytes = data->actual_mode->bytes_per_line - (pixels_per_line * 4);

    asm volatile (
        "cld\n\t"
        "1:\n\t"
            "mov %[pixels_per_line], %%ecx\n\t"
            "rep movsl\n\t"
            "add %[padding_bytes], %%edi\n\t"
            "dec %%ebx\n\t"
        "jnz 1b\n\t"
        : "+S"(src), "+D"(dst), "+b"(rows)
        : [pixels_per_line] "g"(pixels_per_line), [padding_bytes] "g"(padding_bytes)
        : "ecx", "memory"
    );
}

void monitor_redraw_screen_24bpp_no_padding(hardware_t *monitor, void *double_buffer) {
    monitor_data_t *data = monitor->data;
    uint32_t *dst = data->actual_mode->linear_frame_buffer;
    uint32_t *src = double_buffer;
    uint32_t pixels = (uint32_t) data->actual_mode->redraw_cycles;

    // copy whole screen except for last pixel
    for(int i = 0; i < (pixels - 1); i++) {
        *dst = *src;
        dst = (uint32_t *)((uint32_t)dst + 3);
        src++;
    }

    // copy last pixel
    dst = (uint32_t *)((uint32_t)dst - 1);
    *dst = ((*src & 0xFFFFFF) << 8) | (*dst & 0xFF);
}

// TODO: this is probably not correct
void monitor_redraw_screen_24bpp_with_padding(hardware_t *monitor, void *double_buffer) {
    monitor_data_t *data = monitor->data;
    uint32_t *dst = data->actual_mode->linear_frame_buffer;
    uint32_t *src = double_buffer;
    uint32_t pixels = (uint32_t) data->actual_mode->redraw_cycles;

    uint32_t rows = data->actual_mode->redraw_cycles;
    uint32_t pixels_per_line = data->actual_mode->active_width;
    uint32_t padding_bytes = data->actual_mode->bytes_per_line - (pixels_per_line * data->actual_mode->bpp);

    // copy whole screen
    for(int i = 0; i < rows; i++) {
        // copy whole line except for last pixel
        for(int i = 0; i < (pixels_per_line - 1); i++) {
            *dst = *src;
            dst = (uint32_t *)((uint32_t)dst + 3);
            src++;
        }

        // copy last pixel
        dst = (uint32_t *)((uint32_t)dst - 1);
        *dst = ((*src & 0xFFFFFF) << 8) | (*dst & 0xFF);
        
        // move to next line
        dst = (uint32_t *) ((uint32_t)dst + 4 + padding_bytes);
    }
}

void monitor_redraw_screen_16bpp_no_padding(hardware_t *monitor, void *double_buffer) {
    monitor_data_t *data = monitor->data;
    uint16_t *dst = data->actual_mode->linear_frame_buffer;
    uint32_t *src = double_buffer;
    uint32_t pixels = (uint32_t) data->actual_mode->redraw_cycles;

    // copy all pixels
    for(int i = 0; i < pixels; i++) {
        *dst = (((*src >> 19) & 0x1F) << 11 | ((*src >> 10) & 0x3F) << 5 | ((*src >> 3) & 0x1F) << 0);
        dst++;
        src++;
    }
}

void monitor_redraw_screen_16bpp_with_padding(hardware_t *monitor, void *double_buffer) {
    monitor_data_t *data = monitor->data;
    uint16_t *dst = data->actual_mode->linear_frame_buffer;
    uint32_t *src = double_buffer;
    uint32_t pixels = (uint32_t) data->actual_mode->redraw_cycles;

    uint32_t rows = data->actual_mode->redraw_cycles;
    uint32_t pixels_per_line = data->actual_mode->active_width;
    uint32_t padding_bytes = data->actual_mode->bytes_per_line - (pixels_per_line * data->actual_mode->bpp);

    // copy whole screen
    for(int i = 0; i < rows; i++) {
        // copy whole line
        for(int i = 0; i < pixels_per_line; i++) {
            *dst = (((*src >> 19) & 0x1F) << 11 | ((*src >> 10) & 0x3F) << 5 | ((*src >> 3) & 0x1F) << 0);
            dst++;
            src++;
        }
        
        // move to next line
        dst = (uint16_t *) ((uint32_t)dst + padding_bytes);
    }
}

void monitor_redraw_screen_15bpp_no_padding(hardware_t *monitor, void *double_buffer) {
    monitor_data_t *data = monitor->data;
    uint16_t *dst = data->actual_mode->linear_frame_buffer;
    uint32_t *src = double_buffer;
    uint32_t pixels = (uint32_t) data->actual_mode->redraw_cycles;

    // copy all pixels
    for(int i = 0; i < pixels; i++) {
        *dst = (((*src >> 19) & 0x1F) << 10 | ((*src >> 11) & 0x1F) << 5 | ((*src >> 3) & 0x1F) << 0);
        dst++;
        src++;
    }
}

void monitor_redraw_screen_15bpp_with_padding(hardware_t *monitor, void *double_buffer) {
    monitor_data_t *data = monitor->data;
    uint16_t *dst = data->actual_mode->linear_frame_buffer;
    uint32_t *src = double_buffer;
    uint32_t pixels = (uint32_t) data->actual_mode->redraw_cycles;

    uint32_t rows = data->actual_mode->redraw_cycles;
    uint32_t pixels_per_line = data->actual_mode->active_width;
    uint32_t padding_bytes = data->actual_mode->bytes_per_line - (pixels_per_line * data->actual_mode->bpp);

    // copy whole screen
    for(int i = 0; i < rows; i++) {
        // copy whole line
        for(int i = 0; i < pixels_per_line; i++) {
            *dst = (((*src >> 19) & 0x1F) << 10 | ((*src >> 11) & 0x1F) << 5 | ((*src >> 3) & 0x1F) << 0);
            dst++;
            src++;
        }
        
        // move to next line
        dst = (uint16_t *) ((uint32_t)dst + padding_bytes);
    }
}

void monitor_redraw_screen_8bpp_no_padding(hardware_t *monitor, void *double_buffer) {
    monitor_data_t *data = monitor->data;
    uint8_t *dst = data->actual_mode->linear_frame_buffer;
    uint32_t *src = double_buffer;
    uint32_t pixels = (uint32_t) data->actual_mode->redraw_cycles;

    // copy all pixels
    for(int i = 0; i < pixels; i++) {
        *dst = (((*src >> 21) & 0x7) << 5 | ((*src >> 12) & 0x7) << 2 | ((*src >> 6) & 0x3) << 0);
        dst++;
        src++;
    }
}

void monitor_redraw_screen_8bpp_with_padding(hardware_t *monitor, void *double_buffer) {
    monitor_data_t *data = monitor->data;
    uint8_t *dst = data->actual_mode->linear_frame_buffer;
    uint32_t *src = double_buffer;
    uint32_t pixels = (uint32_t) data->actual_mode->redraw_cycles;

    uint32_t rows = data->actual_mode->redraw_cycles;
    uint32_t pixels_per_line = data->actual_mode->active_width;
    uint32_t padding_bytes = data->actual_mode->bytes_per_line - (pixels_per_line * data->actual_mode->bpp);

    // copy whole screen
    for(int i = 0; i < rows; i++) {
        // copy whole line
        for(int i = 0; i < pixels_per_line; i++) {
            *dst = (((*src >> 21) & 0x7) << 5 | ((*src >> 12) & 0x7) << 2 | ((*src >> 6) & 0x3) << 0);
            dst++;
            src++;
        }
        
        // move to next line
        dst += padding_bytes;
    }
}

/* functions for redrawing part of screen */
void monitor_redraw_part_of_screen_32bpp(hardware_t *monitor, uint32_t monitor_x, uint32_t monitor_y, void *double_buffer, uint32_t buffer_pixels_per_line, uint32_t buffer_x, uint32_t buffer_y, uint32_t width, uint32_t height) {
    monitor_data_t *data = monitor->data;
    uint32_t *dst = (uint32_t *) ((uint32_t)data->actual_mode->linear_frame_buffer + (monitor_y * data->actual_mode->bytes_per_line) + (monitor_x * 4));
    uint32_t *src = (uint32_t *) ((uint32_t)double_buffer + (buffer_y * buffer_pixels_per_line * 4) + (buffer_x * 4));
    for(uint32_t i = 0; i < height; i++) {
        for(uint32_t j = 0; j < width; j++) {
            dst[j] = src[j];
        }
        dst = (uint32_t *) ((uint32_t)dst + data->actual_mode->bytes_per_line);
        src += buffer_pixels_per_line;
    }
}

void monitor_redraw_part_of_screen_24bpp(hardware_t *monitor, uint32_t monitor_x, uint32_t monitor_y, void *double_buffer, uint32_t buffer_pixels_per_line, uint32_t buffer_x, uint32_t buffer_y, uint32_t width, uint32_t height) {
    monitor_data_t *data = monitor->data;
    uint8_t *dst = (uint8_t *) ((uint32_t)data->actual_mode->linear_frame_buffer + (monitor_y * data->actual_mode->bytes_per_line) + (monitor_x * 3));
    uint32_t *src = (uint32_t *) ((uint32_t)double_buffer + (buffer_y * buffer_pixels_per_line * 4) + (buffer_x * 4));
    for(uint32_t i = 0; i < height; i++) {
        for(uint32_t j = 0; j < width; j++) {
            dst[j * 3] = (src[j] >> 16) & 0xFF;
            dst[j * 3 + 1] = (src[j] >> 8) & 0xFF;
            dst[j * 3 + 2] = (src[j] >> 0) & 0xFF;
        }
        dst += data->actual_mode->bytes_per_line;
        src += buffer_pixels_per_line;
    }
}

void monitor_redraw_part_of_screen_16bpp(hardware_t *monitor, uint32_t monitor_x, uint32_t monitor_y, void *double_buffer, uint32_t buffer_pixels_per_line, uint32_t buffer_x, uint32_t buffer_y, uint32_t width, uint32_t height) {
    monitor_data_t *data = monitor->data;
    uint16_t *dst = (uint16_t *) ((uint32_t)data->actual_mode->linear_frame_buffer + (monitor_y * data->actual_mode->bytes_per_line) + (monitor_x * 2));
    uint32_t *src = (uint32_t *) ((uint32_t)double_buffer + (buffer_y * buffer_pixels_per_line * 4) + (buffer_x * 4));
    for(uint32_t i = 0; i < height; i++) {
        for(uint32_t j = 0; j < width; j++) {
            dst[j] = (((src[j] >> 19) & 0x1F) << 11 | ((src[j] >> 10) & 0x3F) << 5 | ((src[j] >> 3) & 0x1F) << 0);
        }
        dst = (uint16_t *) ((uint32_t)dst + data->actual_mode->bytes_per_line);
        src += buffer_pixels_per_line;
    }
}

void monitor_redraw_part_of_screen_15bpp(hardware_t *monitor, uint32_t monitor_x, uint32_t monitor_y, void *double_buffer, uint32_t buffer_pixels_per_line, uint32_t buffer_x, uint32_t buffer_y, uint32_t width, uint32_t height) {
    monitor_data_t *data = monitor->data;
    uint16_t *dst = (uint16_t *) ((uint32_t)data->actual_mode->linear_frame_buffer + (monitor_y * data->actual_mode->bytes_per_line) + (monitor_x * 2));
    uint32_t *src = (uint32_t *) ((uint32_t)double_buffer + (buffer_y * buffer_pixels_per_line * 4) + (buffer_x * 4));
    for(uint32_t i = 0; i < height; i++) {
        for(uint32_t j = 0; j < width; j++) {
            dst[j] = (((src[j] >> 19) & 0x1F) << 10 | ((src[j] >> 11) & 0x1F) << 5 | ((src[j] >> 3) & 0x1F) << 0);
        }
        dst = (uint16_t *) ((uint32_t)dst + data->actual_mode->bytes_per_line);
        src += buffer_pixels_per_line;
    }
}

void monitor_redraw_part_of_screen_8bpp(hardware_t *monitor, uint32_t monitor_x, uint32_t monitor_y, void *double_buffer, uint32_t buffer_pixels_per_line, uint32_t buffer_x, uint32_t buffer_y, uint32_t width, uint32_t height) {
    monitor_data_t *data = monitor->data;
    uint8_t *dst = (uint8_t *) ((uint32_t)data->actual_mode->linear_frame_buffer + (monitor_y * data->actual_mode->bytes_per_line) + (monitor_x * 1));
    uint32_t *src = (uint32_t *) ((uint32_t)double_buffer + (buffer_y * buffer_pixels_per_line * 4) + (buffer_x * 4));
    for(uint32_t i = 0; i < height; i++) {
        for(uint32_t j = 0; j < width; j++) {
            dst[j] = (((src[j] >> 21) & 0x7) << 5 | ((src[j] >> 12) & 0x7) << 2 | ((src[j] >> 6) & 0x3) << 0);
        }
        dst = (uint8_t *) ((uint32_t)dst + data->actual_mode->bytes_per_line);
        src += buffer_pixels_per_line;
    }
}