/* 
* BleskOS
*
* MIT License
* Copyright (c) 2023-2026 BleskOS developers
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include <kernel/cpu/interrupt.h>
#include <libraries/screen.h>
#include <libraries/bitmap_font.h>

enum {
    DEMAND_TYPE_CREATE_THREAD = 1, // ebx = entry point, ecx = stack pointer, edx = end of thread signal memory
    DEMAND_TYPE_RESPAWN_PROCESS,
    DEMAND_TYPE_SEND_FILE_TO_APPLICATION,
    DEMAND_TYPE_SWITCH_THREADS, // nothing
    DEMAND_TYPE_SLEEP_FOR_THREAD, // ebx = milliseconds to sleep
    DEMAND_TYPE_CLOSE_THREAD, // nothing
    DEMAND_TYPE_MAP_PHYSICAL_PAGES_TO_USERSPACE, // ebx = address, ecx = size
    DEMAND_TYPE_UNMAP_PHYSICAL_PAGES_FROM_USERSPACE, // ebx = address, ecx = size
    DEMAND_TYPE_GET_LIST_OF_DEVICES, // ebx = pointer where to store list, ecx = size
    DEMAND_TYPE_SEND_COMMAND_TO_DEVICE // ebx = device id, ecx = pointer to command
};

typedef struct syslib {
    uint32_t version;
    uint32_t num_of_logical_processors;
    uint32_t userspace_start;
    uint32_t userspace_size;

    uint32_t window_width;
    uint32_t window_height;

    void (*initialize)(struct syslib *syslib_ptr);

    void (*log)(char *string);
    void (*logf)(char *string, ...);
    
    uint64_t (*get_time_in_microseconds)(void);

    void *(*malloc)(uint32_t size);
    void *(*calloc)(uint32_t number, uint32_t size);
    void *(*realloc)(void *ptr, uint32_t size);
    void (*free)(void *ptr);

    screen_buffer_t *(*create_window)(void);
    void (*redraw_window)(void);

    screen_buffer_t *(*create_screen_buffer)(uint32_t width, uint32_t height);
    void (*destroy_screen_buffer)(screen_buffer_t *buffer);

    bitmap_font_t *(*load_bitmap_font)(void *file, uint32_t size);
    void (*draw_bitmap_char)(screen_buffer_t *buffer, uint32_t x, uint32_t y, bitmap_font_t *font, uint32_t character, uint32_t color);
    void (*draw_bitmap_string)(screen_buffer_t *buffer, uint32_t x, uint32_t y, bitmap_font_t *font, char *string, uint32_t color);
    void (*destroy_bitmap_font)(bitmap_font_t *font);

    screen_buffer_t *(*initialize_gui)(void);
    void (*add_canvas_component)(uint32_t area_number, void (*redraw)(uint32_t screen_x, uint32_t screen_y, uint32_t screen_width, uint32_t screen_height, uint32_t x_offset, uint32_t y_offset));
    void (*register_pressed_key_event_handler)(uint32_t key, void (*handler)(void));
    void (*redraw_gui)(void);

    /*

    void (*copy_screen_buffer)(screen_buffer_t *dst, uint32_t dst_x, uint32_t dst_y, screen_buffer_t *src, uint32_t src_x, uint32_t src_y, uint32_t width, uint32_t height);

    void (*redraw_part_of_window)(uint32_t window_x, uint32_t window_y, screen_buffer_t *buffer, uint32_t buffer_x, uint32_t buffer_y, uint32_t width, uint32_t height);

    */
} syslib_t;

#define syscall_create_thread(entry_point, stack_pointer, end_of_thread_signal) asm volatile ("int %0" : : "i" (0xD0), "a" (DEMAND_TYPE_CREATE_THREAD), "b" (entry_point), "c" (stack_pointer), "d" (end_of_thread_signal))


#define syscall_switch() asm volatile ("int %0" : : "i" (0xD0), "a" (DEMAND_TYPE_SWITCH_THREADS))
#define syscall_sleep(milliseconds) asm volatile ("int %0" : : "i" (0xD0), "a" (DEMAND_TYPE_SLEEP_FOR_THREAD), "b" (milliseconds))
#define syscall_close() asm volatile ("int %0" : : "i" (0xD0), "a" (DEMAND_TYPE_CLOSE_THREAD))
#define syscall_print(string) asm volatile ("int %0" : : "i" (0xD0), "a" (DEMAND_TYPE_PRINT_STRING), "b" (string))
#define syscall_map_pages(address, size) asm volatile ("int %0" : : "i" (0xD0), "a" (DEMAND_TYPE_MAP_PHYSICAL_PAGES_TO_USERSPACE), "b" (address), "c" (size))
#define syscall_unmap_pages(address, size) asm volatile ("int %0" : : "i" (0xD0), "a" (DEMAND_TYPE_UNMAP_PHYSICAL_PAGES_FROM_USERSPACE), "b" (address), "c" (size))
#define syscall_get_list_of_devices(pointer, size) asm volatile ("int %0" : : "i" (0xD0), "a" (DEMAND_TYPE_GET_LIST_OF_DEVICES), "b" (pointer), "c" (size))
#define syscall_send_command_to_device(id, pointer) asm volatile ("int %0" : : "i" (0xD0), "a" (DEMAND_TYPE_SEND_COMMAND_TO_DEVICE), "b" (id), "c" (pointer))