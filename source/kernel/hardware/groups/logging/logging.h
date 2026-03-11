/* 
* BleskOS
*
* MIT License
* Copyright (c) 2023-2026 BleskOS developers
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

extern int logging_enabled;

typedef struct {
    uint32_t id;
    void (*send_character)(uint32_t character);
} logging_device_t;

#define MAX_NUMBER_OF_LOGGING_GROUP_DEVICES 8
typedef struct {
    uint32_t number_of_devices;
    logging_device_t devices[MAX_NUMBER_OF_LOGGING_GROUP_DEVICES];
} logging_group_t;

enum {
    LOGGING_GROUP_COMMAND_SEND_CHARACTER = 1,
    LOGGING_GROUP_COMMAND_SEND_STRING = 2,
    LOGGING_GROUP_COMMAND_SEND_FORMATTED_STRING = 3
};

typedef struct {
    uint32_t type;
    uint32_t argument[];
} logging_device_command_t;