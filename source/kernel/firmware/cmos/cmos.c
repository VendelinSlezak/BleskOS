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
#include <kernel/hardware/groups/logging/logging.h>
#include <kernel/hardware/devices/cpu/interrupt.h>
#include <kernel/hardware/devices/cpu/commands.h>
#include <kernel/libc/stdlib.h>

/* global variables */
datetime_t cmos_datetime;

/* local variables */
uint32_t cmos_time_format_24_hours;
uint32_t cmos_time_format_binary;

/* functions */
inline uint8_t cmos_reg_read(uint8_t reg) {
    outb(0x70, reg);
    return inb(0x71);
}

inline void cmos_reg_write(uint8_t reg, uint8_t value) {
    outb(0x70, reg);
    outb(0x71, value);
}

void read_cmos(void) {
    // read time format
    uint8_t status_register_b = cmos_reg_read(0x0B);
    cmos_time_format_24_hours = ((status_register_b >> 1) & 0x1);
    cmos_time_format_binary = ((status_register_b >> 2) & 0x1);

    // read actual time
    read_time();

    // log
    log("\n[CMOS] Time: %d/%d/%d %d:%d:%d", cmos_datetime.year,
                                            cmos_datetime.month,
                                            cmos_datetime.day,
                                            cmos_datetime.hour,
                                            cmos_datetime.minute,
                                            cmos_datetime.second);
    if(cmos_datetime.year < 2026) {
        log("\n[WARNING] Time is set to wrong value");
    }
}

uint8_t cmos_read_time_register(uint8_t reg) {
    uint8_t value = cmos_reg_read(reg);

    if(cmos_time_format_binary == 1) {
        return value;
    }
    else {
        return CONVERT_BCD_TO_DECIMAL(value);
    }
}

void read_time(void) {
    // read CMOS time registers
    cmos_datetime.second = cmos_read_time_register(0x00);
    cmos_datetime.minute = cmos_read_time_register(0x02);
    cmos_datetime.weekday = cmos_read_time_register(0x06);
    cmos_datetime.day = cmos_read_time_register(0x07);
    cmos_datetime.month = cmos_read_time_register(0x08);

    // convert year number to real year
    cmos_datetime.year = cmos_read_time_register(0x09);
    if(cmos_datetime.year < 80) {
        cmos_datetime.year += 2000;
    }
    else {
        cmos_datetime.year += 1900;
    }

    // read hour
    if(cmos_time_format_24_hours == 1) {
        cmos_datetime.hour = cmos_read_time_register(0x04);
    }
    else {
        uint8_t hour_reg_value = cmos_reg_read(0x04);
        uint8_t hour = (hour_reg_value & 0x7F);
        uint8_t pm_bit = (hour_reg_value & 0x80);

        // convert to binary value if it is in BCD format
        if(cmos_time_format_binary == 0) { 
            hour = CONVERT_BCD_TO_DECIMAL(hour);
        }

        // PM
        if(pm_bit == 0x80) {
            // convert to actual hour
            if(hour == 12) { // midnight
                hour = 0;
            }
            else { // PM
                hour += 12; 
            }
        }

        // save result
        cmos_datetime.hour = hour;
    }
}

void start_counting_time(void) {
    // clear interrupt flag
    cmos_reg_read(0x0C);

    // set Update Interrupt Enable bit
    uint8_t status_register_b = cmos_reg_read(0x0B);
    status_register_b |= (1 << 4);
    cmos_reg_write(0x0B, status_register_b);

    // register handler
    set_isa_interrupt_handler(8, rtc_update_handler);
}

void rtc_update_handler(interrupt_stack_t *stack_of_interrupt) {
    // read actual time
    read_time();

    // acknowledge interrupt
    cmos_reg_read(0x0C);
}