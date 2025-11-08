#ifndef BUILD_HARDWARE_GROUPS_GRAPHIC_OUTPUT_GRAPHIC_OUTPUT_H
#define BUILD_HARDWARE_GROUPS_GRAPHIC_OUTPUT_GRAPHIC_OUTPUT_H

/* 
* BleskOS
*
* MIT License
* Copyright (c) 2023-2025 BleskOS developers
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#define GRAPHIC_OUTPUT_FUNCTION_INIT(x) \
    dword_t d = graphic_output_group_get_arr_num(device_id); \
    if(d == INVALID) { return x; }

typedef struct {
    void *(*get_double_buffer)(dword_t entity_number);
    void (*redraw_screen)(dword_t entity_number);
} graphic_output_device_functions_t;

#define GRAPHIC_OUTPUT_MAX_NUMBER_OF_DEVICES 16
typedef struct {
    dword_t number_of_devices;
    dword_t devices[GRAPHIC_OUTPUT_MAX_NUMBER_OF_DEVICES];
    dword_t ids[GRAPHIC_OUTPUT_MAX_NUMBER_OF_DEVICES];
    graphic_output_device_functions_t *functions[GRAPHIC_OUTPUT_MAX_NUMBER_OF_DEVICES];
} e_graphic_output_attr_t;
void initialize_graphic_output_group(void);
void graphic_output_group_add_device(dword_t device_entity, dword_t device_id, graphic_output_device_functions_t *functions);
dword_t graphic_output_group_get_arr_num(dword_t device_id);
dword_t go_get_num_of_devices(void);
dword_t go_get_device_id(dword_t arr_num);
dword_t go_is_device_connected(dword_t device_id);
void *go_get_double_buffer(dword_t device_id);
void go_redraw_screen(dword_t device_id);
dword_t go_get_number_of_available_modes(dword_t device_id);
dword_t go_get_actual_mode(dword_t device_id);
dword_t go_get_width_of_mode(dword_t device_id, dword_t mode_number);
dword_t go_get_width_of_actual_mode(dword_t device_id);
dword_t go_get_height_of_mode(dword_t device_id, dword_t mode_number);
dword_t go_get_height_of_actual_mode(dword_t device_id);
dword_t go_get_bpp_of_mode(dword_t device_id, dword_t mode_number);
dword_t go_change_resolution(dword_t device_id, dword_t mode_number);

#endif /* BUILD_HARDWARE_GROUPS_GRAPHIC_OUTPUT_GRAPHIC_OUTPUT_H */
