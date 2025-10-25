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
#include <kernel/x86/cpu/commands.h>
#include <kernel/x86/libc/stdlib.h>

/* functions */
inline byte_t cmos_reg_read(byte_t reg) {
    outb(0x70, reg);
    return inb(0x71);
}

inline void cmos_reg_write(byte_t reg, byte_t value) {
    outb(0x70, reg);
    outb(0x71, value);
}

void read_cmos(void) {
    // read time format
    byte_t status_register_b = cmos_reg_read(0x0B);
    kernel_attr->cmos_time_format_24_hours = ((status_register_b >> 1) & 0x1);
    kernel_attr->cmos_time_format_binary = ((status_register_b >> 2) & 0x1);

    // read actual time
    read_time();

    // log
    log("\n[CMOS] Time: %d/%d/%d %d:%d:%d", kernel_attr->year,
                                            kernel_attr->month,
                                            kernel_attr->day,
                                            kernel_attr->hour,
                                            kernel_attr->minute,
                                            kernel_attr->second);
    if(kernel_attr->year < 2025) {
        log("\n[WARNING] Time is set to wrong value");
    }
}

byte_t cmos_read_time_register(byte_t reg) {
    byte_t value = cmos_reg_read(reg);

    // binary format
    if(kernel_attr->cmos_time_format_binary == 1) {
        return value;
    }
    // BCD format
    else {
        return CONVERT_BCD_TO_DECIMAL(value);
    }
}

void read_time(void) {
    // read CMOS time registers
    kernel_attr->second = cmos_read_time_register(0x00);
    kernel_attr->minute = cmos_read_time_register(0x02);
    kernel_attr->weekday = cmos_read_time_register(0x06);
    kernel_attr->day = cmos_read_time_register(0x07);
    kernel_attr->month = cmos_read_time_register(0x08);

    // convert year number to real year
    kernel_attr->year = cmos_read_time_register(0x09);
    if(kernel_attr->year < 80) {
        kernel_attr->year += 2000;
    }
    else {
        kernel_attr->year += 1900;
    }

    // read hour
    if(kernel_attr->cmos_time_format_24_hours == 1) {
        kernel_attr->hour = cmos_read_time_register(0x04);
    }
    else {
        byte_t hour_reg_value = cmos_reg_read(0x04);
        byte_t hour = (hour_reg_value & 0x7F);
        byte_t pm_bit = (hour_reg_value & 0x80);

        // convert to binary value if it is in BCD format
        if(kernel_attr->cmos_time_format_binary == 0) { 
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
        kernel_attr->hour = hour;
    }
}