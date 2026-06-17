/* 
* BleskOS
*
* MIT License
* Copyright (c) 2023-2026 BleskOS developers
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

enum {
    VIRTUAL_HARDWARE_TIMER = 1,
    VIRTUAL_HARDWARE_LOGGER,
    VIRTUAL_HARDWARE_WINDOW,
    VIRTUAL_HARDWARE_HUMAN_INPUT_DEVICE
};

typedef struct {
    uint32_t type;
} virtual_hardware_list_entry_t;

#define MAX_NUMBER_OF_ENTRIES_IN_HARDWARE_LIST 256
typedef struct {
    uint32_t number_of_entries;
    virtual_hardware_list_entry_t entries[];
} virtual_hardware_list_t;

extern virtual_hardware_list_t *hardware_list;
extern uint32_t number_of_unintialized_devices;

typedef struct hardware {
    uint32_t id;                                // unique ID of the hardware
    uint8_t *name;                              // name of the hardware
    void *data;                                 // data of the hardware that is acquired by hardware driver

    struct hardware *next;                      // pointer to the next hardware in the list connected to same controller
    struct hardware *prev;                      // pointer to the previous hardware in the list connected to same controller

    struct hardware *controller;                // pointer to the controller to which this hardware is connected
    void *communication_functions;              // pointer to structure of functions that are used to communicate with the controller
    void *data_from_controller;                 // data of the hardware that is acquired by controller

    struct hardware *first_device;              // if this device is a controller, pointer to the first device connected to this controller

    uint32_t is_initialized;                    // whether the hardware is initialized
    void (*init)(struct hardware *self);        // pointer to the function that is used to initialize the hardware
    void (*remove)(struct hardware *self);      // pointer to the function that is used to clear state after removing the hardware
} hardware_t;
extern hardware_t *motherboard;