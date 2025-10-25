/* 
* BleskOS
*
* MIT License
* Copyright (c) 2023-2025 BleskOS developers
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

/* includes */
#include <kernel/x86/kernel.h>
#include <hardware/groups/logging/logging.h>
#include <kernel/x86/memory/vm_allocator.h>
#include <kernel/x86/cpu/commands.h>
#include <kernel/x86/entities/entities.h>
#include <kernel/x86/libc/string.h>
#include <kernel/x86/hardware/id.h>
#include <hardware/groups/graphic_output/graphic_output.h>

/* local variables */
size_t *monitor_entity_functions[] = {
    (size_t *)initialize_monitor_device
};

graphic_output_device_functions_t monitor_group_functions[] = {
    { monitor_get_double_buffer, monitor_redraw_screen_32bpp_no_padding }
};

/* functions */
dword_t create_monitor_device_entity(dword_t controller_entity, size_t controller_functions[], dword_t number_of_modes, monitor_mode_t *modes, dword_t best_mode) {
    // create entity
    dword_t entity_number = create_entity("Monitor", E_TYPE_KERNEL_RING);

    // save controller info
    e_monitor_attr_t *attr = entity_get_attr_ptr(entity_number);
    attr->device_id = get_unique_device_id();
    attr->controller_entity = controller_entity;
    attr->change_monitor_resolution = (dword_t (*)(monitor_mode_t)) controller_functions[0];

    // save info about device from controller
    attr->best_mode = best_mode;
    attr->selected_mode = INVALID;
    attr->number_of_modes = number_of_modes;
    memcpy(attr->modes, modes, number_of_modes * sizeof(monitor_mode_t));

    // return new device entity number
    return entity_number;
}

void initialize_monitor_device(void) {
    command_info_t *command_info = (command_info_t *) P_MEM_COMMAND_INFO;
    e_monitor_attr_t *attr = command_info->entity_attributes;
    command_t *command = command_get_ptr(kernel_attr->running_command);
    dword_t entity_number = command->entity;

    // log available modes
    for(int i = 0; i < attr->number_of_modes; i++) {
        log("\n[MONITOR] Graphic mode %dx%dx%d", attr->modes[i].active_width, attr->modes[i].active_height, attr->modes[i].bpp);
        if(attr->best_mode == i) {
            log(" (best)");
        }
    }

    // set best monitor resolution
    attr->selected_mode = attr->best_mode;
    attr->change_monitor_resolution(attr->modes[attr->selected_mode]);
    dword_t width = attr->modes[attr->selected_mode].active_width;
    dword_t height = attr->modes[attr->selected_mode].active_height;
    dword_t bpp = attr->modes[attr->selected_mode].bpp;
    dword_t bytes_per_line = attr->modes[attr->selected_mode].bytes_per_line;
    void *linear_frame_buffer = attr->modes[attr->selected_mode].linear_frame_buffer;

    // allocate memory for double buffer
    attr->double_buffer = kcalloc(width*height*4);
    attr->redraw_pixels_per_line = width;
    if(bpp == 32) {
        if(bytes_per_line == width*4) {
            attr->redraw_cycles = width*height;
            monitor_group_functions->redraw_screen = monitor_redraw_screen_32bpp_no_padding;
        }
        else {
            attr->redraw_padding = bytes_per_line - width*4;
            attr->redraw_cycles = height;
            monitor_group_functions->redraw_screen = monitor_redraw_screen_32bpp_with_padding;
        }
    }
    else if(bpp == 24) {
        if(bytes_per_line == width*3) {
            attr->redraw_cycles = width*height;
            monitor_group_functions->redraw_screen = monitor_redraw_screen_24bpp_no_padding;
        }
        else {
            attr->redraw_padding = bytes_per_line - width*3;
            attr->redraw_cycles = height;
            monitor_group_functions->redraw_screen = monitor_redraw_screen_24bpp_with_padding;
        }
    }
    else if(bpp == 16) {
        if(bytes_per_line == width*2) {
            attr->redraw_cycles = width*height;
            monitor_group_functions->redraw_screen = monitor_redraw_screen_16bpp_no_padding;
        }
        else {
            attr->redraw_padding = bytes_per_line - width*2;
            attr->redraw_cycles = height;
            monitor_group_functions->redraw_screen = monitor_redraw_screen_16bpp_with_padding;
        }
    }
    else if(bpp == 15) {
        if(bytes_per_line == width*2) {
            attr->redraw_cycles = width*height;
            monitor_group_functions->redraw_screen = monitor_redraw_screen_15bpp_no_padding;
        }
        else {
            attr->redraw_padding = bytes_per_line - width*2;
            attr->redraw_cycles = height;
            monitor_group_functions->redraw_screen = monitor_redraw_screen_15bpp_with_padding;
        }
    }
    else if(bpp == 8) {
        if(bytes_per_line == width*1) {
            attr->redraw_cycles = width*height;
            monitor_group_functions->redraw_screen = monitor_redraw_screen_8bpp_no_padding;
        }
        else {
            attr->redraw_padding = bytes_per_line - width*1;
            attr->redraw_cycles = height;
            monitor_group_functions->redraw_screen = monitor_redraw_screen_8bpp_with_padding;
        }
    }
    else {
        log("\n[MONITOR] ERROR: unsupported bpp mode selected");
    }
    attr->vm_of_liner_frame_buffer = kpalloc((dword_t)linear_frame_buffer, bytes_per_line*height, VM_FLAG_WRITE_COMBINED);

    // connect to graphic group
    graphic_output_group_add_device(entity_number, attr->device_id, monitor_group_functions);
    log("\n[MONITOR] Device initialized");
}

void *monitor_get_double_buffer(dword_t entity_number) {
    e_monitor_attr_t *attr = entity_get_attr_ptr(entity_number);
    return attr->double_buffer;
}

void monitor_redraw_screen_32bpp_no_padding(dword_t entity_number) {
    e_monitor_attr_t *attr = entity_get_attr_ptr(entity_number);

    void *dst = attr->vm_of_liner_frame_buffer;
    void *src = attr->double_buffer;
    dword_t dwords = attr->redraw_cycles;

    // copy all pixels
    asm volatile (
        "rep movsl"
        : "+D"(dst), "+S"(src), "+c"(dwords)
        :
        : "memory"
    );
}

void monitor_redraw_screen_32bpp_no_padding_sse(dword_t entity_number) {
    e_monitor_attr_t *attr = entity_get_attr_ptr(entity_number);

    void *dst = attr->vm_of_liner_frame_buffer;
    void *src = attr->double_buffer;
    dword_t size = attr->redraw_cycles*4;
    byte_t *d = dst;
    byte_t *s = src;
    size_t blocks = size / 128;
    size_t rem = size % 128;

    if (blocks > 0) {
        asm volatile (
            "1:\n\t"
            "prefetchnta 256(%[src])\n\t"
            "movdqa  (%[src]), %%xmm0\n\t"
            "movdqa  16(%[src]), %%xmm1\n\t"
            "movdqa  32(%[src]), %%xmm2\n\t"
            "movdqa  48(%[src]), %%xmm3\n\t"
            "movdqa  64(%[src]), %%xmm4\n\t"
            "movdqa  80(%[src]), %%xmm5\n\t"
            "movdqa  96(%[src]), %%xmm6\n\t"
            "movdqa 112(%[src]), %%xmm7\n\t"
            "movntdq %%xmm0, (%[dst])\n\t"
            "movntdq %%xmm1, 16(%[dst])\n\t"
            "movntdq %%xmm2, 32(%[dst])\n\t"
            "movntdq %%xmm3, 48(%[dst])\n\t"
            "movntdq %%xmm4, 64(%[dst])\n\t"
            "movntdq %%xmm5, 80(%[dst])\n\t"
            "movntdq %%xmm6, 96(%[dst])\n\t"
            "movntdq %%xmm7, 112(%[dst])\n\t"
            "add $128, %[src]\n\t"
            "add $128, %[dst]\n\t"
            "dec %[blocks]\n\t"
            "jnz 1b\n\t"
            :
            [dst] "+r" (d), [src] "+r" (s), [blocks] "+r" (blocks)
            :
            : "memory"
        );
    }

    // copy rest to alignment
    for(size_t i = 0; i < rem; i++) {
        *d++ = *s++;
    }

    // flush
    asm volatile("sfence" ::: "memory");
}

void monitor_redraw_screen_32bpp_with_padding(dword_t entity_number) {
    e_monitor_attr_t *attr = entity_get_attr_ptr(entity_number);

    dword_t *src = attr->double_buffer;
    dword_t *dst = attr->vm_of_liner_frame_buffer;

    dword_t rows = attr->redraw_cycles;
    dword_t pixels_per_line = attr->redraw_pixels_per_line;
    dword_t padding_bytes = attr->redraw_padding;

    // copy all lines and skip padding at the end of every line
    asm volatile (
        "1:\n\t"
            "mov %[pixels_per_line], %%ecx\n\t"
            "rep movsl\n\t"
            "add %[padding_bytes], %%edi\n\t"
            "dec %%ebx\n\t"
        "jnz 1b\n\t"
        : "+S"(src), "+D"(dst), "+b"(rows)
        : [pixels_per_line] "r"(pixels_per_line), [padding_bytes] "r"(padding_bytes)
        : "ecx", "memory"
    );
}

void monitor_redraw_screen_24bpp_no_padding(dword_t entity_number) {
    e_monitor_attr_t *attr = entity_get_attr_ptr(entity_number);

    dword_t *dst = attr->vm_of_liner_frame_buffer;
    dword_t *src = attr->double_buffer;
    dword_t pixels = attr->redraw_cycles;

    // copy whole screen except for last pixel
    for(int i = 0; i < (pixels - 1); i++) {
        *dst = *src;
        dst = (dword_t *)((dword_t)dst + 3);
        src++;
    }

    // copy last pixel
    dst = (dword_t *)((dword_t)dst - 1);
    *dst = ((*src & 0xFFFFFF) << 8) | (*dst & 0xFF);
}

void monitor_redraw_screen_24bpp_with_padding(dword_t entity_number) {
    e_monitor_attr_t *attr = entity_get_attr_ptr(entity_number);

    dword_t *dst = attr->vm_of_liner_frame_buffer;
    dword_t *src = attr->double_buffer;
    dword_t pixels = attr->redraw_cycles;

    dword_t rows = attr->redraw_cycles;
    dword_t pixels_per_line = attr->redraw_pixels_per_line;
    dword_t padding_bytes = attr->redraw_padding;

    // copy whole screen
    for(int i = 0; i < rows; i++) {
        // copy whole line except for last pixel
        for(int i = 0; i < (pixels_per_line - 1); i++) {
            *dst = *src;
            dst = (dword_t *)((dword_t)dst + 3);
            src++;
        }

        // copy last pixel
        dst = (dword_t *)((dword_t)dst - 1);
        *dst = ((*src & 0xFFFFFF) << 8) | (*dst & 0xFF);
        
        // move to next line
        dst = (dword_t *) ((dword_t)dst + 4 + padding_bytes);
    }
}

void monitor_redraw_screen_16bpp_no_padding(dword_t entity_number) {
    e_monitor_attr_t *attr = entity_get_attr_ptr(entity_number);

    word_t *dst = attr->vm_of_liner_frame_buffer;
    dword_t *src = attr->double_buffer;
    dword_t pixels = attr->redraw_cycles;

    // copy all pixels
    for(int i = 0; i < pixels; i++) {
        *dst = (((*src >> 19) & 0x1F) << 11 | ((*src >> 10) & 0x3F) << 5 | ((*src >> 3) & 0x1F) << 0);
        dst++;
        src++;
    }
}

void monitor_redraw_screen_16bpp_with_padding(dword_t entity_number) {
    e_monitor_attr_t *attr = entity_get_attr_ptr(entity_number);

    word_t *dst = attr->vm_of_liner_frame_buffer;
    dword_t *src = attr->double_buffer;
    dword_t pixels = attr->redraw_cycles;

    dword_t rows = attr->redraw_cycles;
    dword_t pixels_per_line = attr->redraw_pixels_per_line;
    dword_t padding_bytes = attr->redraw_padding;

    // copy whole screen
    for(int i = 0; i < rows; i++) {
        // copy whole line
        for(int i = 0; i < pixels_per_line; i++) {
            *dst = (((*src >> 19) & 0x1F) << 11 | ((*src >> 10) & 0x3F) << 5 | ((*src >> 3) & 0x1F) << 0);
            dst++;
            src++;
        }
        
        // move to next line
        dst = (word_t *) ((dword_t)dst + padding_bytes);
    }
}

void monitor_redraw_screen_15bpp_no_padding(dword_t entity_number) {
    e_monitor_attr_t *attr = entity_get_attr_ptr(entity_number);

    word_t *dst = attr->vm_of_liner_frame_buffer;
    dword_t *src = attr->double_buffer;
    dword_t pixels = attr->redraw_cycles;

    // copy all pixels
    for(int i = 0; i < pixels; i++) {
        *dst = (((*src >> 19) & 0x1F) << 10 | ((*src >> 11) & 0x1F) << 5 | ((*src >> 3) & 0x1F) << 0);
        dst++;
        src++;
    }
}

void monitor_redraw_screen_15bpp_with_padding(dword_t entity_number) {
    e_monitor_attr_t *attr = entity_get_attr_ptr(entity_number);

    word_t *dst = attr->vm_of_liner_frame_buffer;
    dword_t *src = attr->double_buffer;
    dword_t pixels = attr->redraw_cycles;

    dword_t rows = attr->redraw_cycles;
    dword_t pixels_per_line = attr->redraw_pixels_per_line;
    dword_t padding_bytes = attr->redraw_padding;

    // copy whole screen
    for(int i = 0; i < rows; i++) {
        // copy whole line
        for(int i = 0; i < pixels_per_line; i++) {
            *dst = (((*src >> 19) & 0x1F) << 10 | ((*src >> 11) & 0x1F) << 5 | ((*src >> 3) & 0x1F) << 0);
            dst++;
            src++;
        }
        
        // move to next line
        dst = (word_t *) ((dword_t)dst + padding_bytes);
    }
}

void monitor_redraw_screen_8bpp_no_padding(dword_t entity_number) {
    e_monitor_attr_t *attr = entity_get_attr_ptr(entity_number);

    byte_t *dst = attr->vm_of_liner_frame_buffer;
    dword_t *src = attr->double_buffer;
    dword_t pixels = attr->redraw_cycles;

    // copy all pixels
    for(int i = 0; i < pixels; i++) {
        *dst = (((*src >> 21) & 0x7) << 5 | ((*src >> 12) & 0x7) << 2 | ((*src >> 6) & 0x3) << 0);
        dst++;
        src++;
    }
}

void monitor_redraw_screen_8bpp_with_padding(dword_t entity_number) {
    e_monitor_attr_t *attr = entity_get_attr_ptr(entity_number);

    byte_t *dst = attr->vm_of_liner_frame_buffer;
    dword_t *src = attr->double_buffer;
    dword_t pixels = attr->redraw_cycles;

    dword_t rows = attr->redraw_cycles;
    dword_t pixels_per_line = attr->redraw_pixels_per_line;
    dword_t padding_bytes = attr->redraw_padding;

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