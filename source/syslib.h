/* 
* BleskOS
*
* MIT License
* Copyright (c) 2023-2026 BleskOS developers
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

// enum {
//     DEMAND_TYPE_CREATE_THREAD = 1, // ebx = entry point, ecx = stack pointer, edx = delete signal handler
//     DEMAND_TYPE_SPAWN_THREAD, // ebx = entry point, ecx = delete signal handler
//     DEMAND_TYPE_SWITCH_THREADS, // nothing
//     DEMAND_TYPE_SLEEP_FOR_THREAD, // ebx = microseconds to sleep
//     DEMAND_TYPE_CLOSE_THREAD, // nothing
//     DEMAND_TYPE_MAP_PHYSICAL_PAGES_TO_USERSPACE, // ebx = address, ecx = size
//     DEMAND_TYPE_UNMAP_PHYSICAL_PAGES_FROM_USERSPACE, // ebx = address, ecx = size
//     DEMAND_TYPE_SEND_DOORBELL_TO_VIRTUAL_DEVICE // ebx = device id, ecx = demand
// };

#define syscall_create_thread(entry_point, stack_pointer, delete_signal_handler) ({ \
    uint32_t ret = 1; \
    asm volatile ( \
        "int $0xD0" \
        : "+a" (ret) \
        : "b" (entry_point), \
          "c" (stack_pointer), \
          "d" (delete_signal_handler) \
        : "memory" \
    ); \
    ret; \
})
#define syscall_spawn_thread(entry_point, delete_signal_handler) ({ \
    uint32_t ret = 2; \
    asm volatile ( \
        "int $0xD0" \
        : "+a" (ret) \
        : "b" (entry_point), \
          "c" (delete_signal_handler) \
        : "memory" \
    ); \
    ret; \
})
#define syscall_switch_thread() ({ \
    asm volatile ( \
        "int $0xD0" \
        : \
        : "a" (3) \
        : "memory" \
    ); \
})
#define syscall_thread_sleep(microseconds) ({ \
    asm volatile ( \
        "int $0xD0" \
        : \
        : "a" (4), \
          "b" (microseconds) \
        : "memory" \
    ); \
})
#define syscall_close_thread() ({ \
    asm volatile ( \
        "int $0xD0" \
        : \
        : "a" (5) \
        : "memory" \
    ); \
})
#define syscall_map_pages(address, size) ({ \
    uint32_t ret = 6; \
    asm volatile ( \
        "int $0xD0" \
        : "+a" (ret) \
        : "b" (address), \
          "c" (size) \
        : "memory" \
    ); \
    ret; \
})
#define syscall_unmap_pages(address, size) ({ \
    uint32_t ret = 7; \
    asm volatile ( \
        "int $0xD0" \
        : "+a" (ret) \
        : "b" (address), \
          "c" (size) \
        : "memory" \
    ); \
    ret; \
})
#define syscall_virtual_hardware(id, demand) ({ \
    uint32_t ret = 8; \
    asm volatile ( \
        "int $0xD0" \
        : "+a" (ret) \
        : "b" (id), \
          "c" (demand) \
        : "memory" \
    ); \
    ret; \
})

#define VIRTUAL_HARDWARE_SCREEN_ID 1
    #define VH_SCREEN_DEMAND_WINDOW_CHANGE 1
    #define VH_SCREEN_DEMAND_REDRAW 2
#define VIRTUAL_HARDWARE_LOG_ID 2
    #define VH_LOG_DEMAND_PRINTLNF 1
#define VIRTUAL_HARDWARE_TIMER_ID 3
    #define VH_TIMER_DEMAND_MICROSECONDS 1
    #define VH_TIMER_DEMAND_GET_TIME 2
#define VIRTUAL_HARDWARE_HUMAN_INPUT_ID 4
    #define VH_HUMAN_INPUT_DEMAND_ENABLE_STREAMING 1
    #define VH_HUMAN_INPUT_DEMAND_DISABLE_STREAMING 2

#define VH_HUMAN_INPUT_SIZE_OF_RING 32
enum {
    VH_HUMAN_INPUT_EVENT_KEY_PRESSED = 1,
    VH_HUMAN_INPUT_EVENT_KEY_RELEASED = 2,
    VH_HUMAN_INPUT_EVENT_BUTTON_PRESSED = 3,
    VH_HUMAN_INPUT_EVENT_BUTTON_RELEASED = 4,
    VH_HUMAN_INPUT_EVENT_BUTTON_DRAGGED = 5,
    VH_HUMAN_INPUT_EVENT_MOUSE_MOVEMENT = 6,
};
typedef struct {
    uint32_t type;
    int argument1;
    int argument2;
    int argument3;
} vh_human_input_event_t;

typedef struct {
    uint8_t log_string[256];
    uint32_t log_string_parameters[16];

    uint64_t microseconds;
    uint32_t year;
    uint32_t month;
    uint32_t day;
    uint32_t hour;
    uint32_t minute;
    uint32_t second;

    volatile uint32_t producer;
    uint32_t consumer;
    vh_human_input_event_t events[VH_HUMAN_INPUT_SIZE_OF_RING];

    uint8_t flag_window_is_different_from_buffer;
    uint8_t flag_buffer_is_on_screen;
    uint32_t window_width;
    uint32_t window_height;
    uint32_t buffer_width;
    uint32_t buffer_height;
    uint32_t size_of_buffer;
    uint32_t buffer[];
} virtual_hardware_t;

typedef struct {
    void (*initialize)(virtual_hardware_t *virtual_hardware_ptr);
    void (*log)(char *string, ...);
} syslib_t;