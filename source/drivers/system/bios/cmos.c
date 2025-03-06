//BleskOS

/*
* MIT License
* Copyright (c) 2023-2025 BleskOS developers
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

byte_t cmos_read(byte_t reg) {
    outb(0x70, reg);
    return inb(0x71);
}

byte_t cmos_write(byte_t reg, byte_t value) {
    outb(0x70, reg);
    outb(0x71, value);
}

void read_cmos_data(void) {
    // read time format
    byte_t status_register_b = cmos_read(0x0B);
    components->cmos.time_format_24_hours = ((status_register_b >> 1) & 0x1);
    components->cmos.time_format_binary = ((status_register_b >> 2) & 0x1);

    // read actual time
    read_time();
}

byte_t cmos_read_time_register(byte_t reg) {
    byte_t value = cmos_read(reg);

    // binary format
    if(components->cmos.time_format_binary == STATUS_TRUE) {
        return value;
    }
    // BCD format
    else {
        return ((value & 0xF) + ((value >> 4)*10));
    }
}

void read_time(void) {
    // read CMOS time registers
    time.second = cmos_read_time_register(0x00);
    time.minute = cmos_read_time_register(0x02);
    time.weekday = cmos_read_time_register(0x06);
    time.day = cmos_read_time_register(0x07);
    time.month = cmos_read_time_register(0x08);
    time.year = cmos_read_time_register(0x09);

    // convert year number to real year
    if(time.year < 80) {
        time.year += 2000;
    }
    else {
        time.year += 1900;
    }

    // read hour
    if(components->cmos.time_format_24_hours==STATUS_TRUE) {
        time.hour = cmos_read_time_register(0x04);
    }
    else {
        time.hour = cmos_read(0x04);

        // AM
        if((time.hour & 0x80) == 0x00) {
            if(components->cmos.time_format_binary==STATUS_FALSE) { // convert if in BCD format
                time.hour = ((time.hour & 0xF) + ((time.hour >> 4)*10));
            }
        }
        // PM
        else {
            time.hour &= ~0x80; //clear PM bit

            if(components->cmos.time_format_binary==STATUS_FALSE) { // convert if in BCD format
                time.hour = ((time.hour & 0xF) + ((time.hour >> 4)*10));
            }

            // convert to actual hour
            if(time.hour==12) {
                time.hour = 0; // midnight
            }
            else {
                time.hour += 12;
            }
        }
    }
}
