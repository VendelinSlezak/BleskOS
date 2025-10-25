/* 
* BleskOS
*
* MIT License
* Copyright (c) 2023-2025 BleskOS developers
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#ifndef KERNEL_H
#define KERNEL_H

#include <kernel/x86/libc/stdio.h>

typedef struct {
    /* scheduler */
    word_t running_command;
    dword_t pm_of_loaded_page_directory;
    dword_t time_of_new_tick;
    qword_t volatile kernel_time;

    /* CMOS */
    dword_t cmos_time_format_24_hours;
    dword_t cmos_time_format_binary;

    /* time */
    dword_t year;
    dword_t month;
    dword_t day;
    dword_t weekday;
    dword_t hour;
    dword_t minute;
    dword_t second;

    /* ACPI */
    dword_t p_acpi;
    dword_t pm_madt; dword_t size_of_madt;
    dword_t pm_hpet; dword_t size_of_hpet;
    dword_t pm_mcfg; dword_t size_of_mcfg;

    /* CPU */
    byte_t cpu_id[13];
    byte_t cpu_family;
    byte_t cpu_model;
    byte_t cpu_stepping;
    byte_t cpu_type;
    byte_t cpu_bits;
    dword_t cpu_number_of_logical_processors;

    dword_t cpu_running_in_emulator;

    dword_t cpu_p_mtrr;
    dword_t cpu_mtrr_number_of_entries;
    dword_t cpu_mtrr_default_memory_type;
    dword_t cpu_mtrr_write_combining_available;

    dword_t lapic_present;
    dword_t lapic_base;

    dword_t number_of_cores;
    dword_t core_id[32];
} e_kernel_attr_t;

extern e_kernel_attr_t *kernel_attr;
extern dword_t kernel_funcs[2];

typedef struct {
    byte_t *str;
    dword_t count;
} a_command_communication_area_t;

#endif