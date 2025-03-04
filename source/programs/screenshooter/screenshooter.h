//BleskOS

/*
* MIT License
* Copyright (c) 2023-2025 BleskOS developers
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#define SCREENSHOTER_CLICK_ZONE_BACK 1
#define SCREENSHOTER_CLICK_ZONE_SAVE 2
#define SCREENSHOTER_CLICK_ZONE_CROP 3
#define SCREENSHOTER_CLICK_ZONE_REMOVE_CROP 4

dword_t screenshoot_image_info_mem = 0, screenshoot_image_info_data_mem = 0, screenshoot_image_info_data_length = 0;
byte_t screenshot_was_made = STATUS_FALSE, screenshot_is_cropped = STATUS_FALSE;
dword_t cropped_image_info_mem = 0;
dword_t screenshot_crop_x = 0, screenshot_crop_y = 0, screenshot_crop_width = 0, screenshot_crop_height = 0;
dword_t screenshot_buffer_image_original_width = 0, screenshot_buffer_image_original_height = 0, screenshot_buffer_image_original_x = 0, screenshot_buffer_image_original_y = 0;

void initalize_screenshooter(void);
void screenshooter(void);
void redraw_screenshooter(void);