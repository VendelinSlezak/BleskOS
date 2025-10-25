/* 
* BleskOS
*
* MIT License
* Copyright (c) 2023-2025 BleskOS developers
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include <kernel/x86/scheduler/lock.h>

#define MAX_NUMBER_OF_ENTITES 256
typedef struct {
    mutex_t mutex;
    word_t lifo_stack_ptr;
    word_t stack[MAX_NUMBER_OF_ENTITES];
}__attribute__((packed)) entities_metadata_t;

#define MAX_NUMBER_OF_COMMANDS 256
typedef struct {
    mutex_t mutex;
    word_t lifo_stack_ptr;
    word_t stack[MAX_NUMBER_OF_COMMANDS];
}__attribute__((packed)) commands_metadata_t;

#define E_TYPE_KERNEL_RING 0
#define E_TYPE_USER_RING 1
typedef struct {
    byte_t name[80];
    dword_t type;

    dword_t pm_page_of_vm_bitmap;
    dword_t pm_page_of_page_directory;

    byte_t attributes[];
}__attribute__((packed)) entity_t;

typedef struct {
    dword_t commanding_command;
    dword_t entity;

    dword_t pm_page_of_page_directory;
    dword_t esp;
    dword_t finalizing_method;

    qword_t time_of_creation;
    qword_t time_of_running;
    qword_t time_of_actual_run;
    qword_t time_for_end_of_sleep;

    dword_t signaling_command;

    dword_t vm_of_communication_area;
    dword_t vm_of_command_list;
    dword_t vm_of_vma_bitmap;
    dword_t vm_of_vma_metadata;
    dword_t vm_of_user_stack;
    dword_t number_of_pages_in_user_stack;
    dword_t vm_of_kernel_stack;

    mutex_t entity_attr_mutex;
}__attribute__((packed)) command_t;

typedef struct {
    dword_t number_of_command;
    dword_t vm_of_communication_area;
}__attribute__((packed)) command_list_entry_t;
typedef struct {
    dword_t number_of_commands;
    command_list_entry_t commands[MAX_NUMBER_OF_COMMANDS];
}__attribute__((packed)) command_list_t;

typedef struct {
    dword_t vm_of_communication_area;
    dword_t vm_of_command_list;
    dword_t vm_of_vma_bitmap;
    dword_t vm_of_vma_metadata;
    dword_t vm_of_user_stack;
    dword_t number_of_pages_in_user_stack;
    dword_t vm_of_kernel_stack;

    dword_t entity_number;
    void *entity_attributes;
    dword_t command_number;
}__attribute__((packed)) command_info_t;

extern command_info_t *command_info;