//BleskOS

/*
* MIT License
* Copyright (c) 2023-2025 BleskOS developers
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

void wait_for_user_input(void) {
    // reset event indicators
    cli();
    mouse_prepare_for_next_event();
    keyboard_prepare_for_next_event();
    usb_device_change_event = STATUS_FALSE;
    internet_status_change = STATUS_FALSE;
    sti();
    
    // wait for new event to happen
    while(mouse_event == STATUS_FALSE
        && keyboard_event == STATUS_FALSE
        && internet_last_status == internet.status
        && usb_device_change_event == STATUS_FALSE) {
        //wait
        asm("hlt");

        //run tasks in scheduler
        scheduler_user_input();
    }

    // detect change in internet status
    if(internet_last_status != internet.status) {
        internet_last_status = internet.status;
        internet_status_change = STATUS_TRUE;
    }

    // make screenshot
    #ifndef NO_PROGRAMS
    if(keyboard_code_of_pressed_key==KEY_PRINT_SCREEN) {
        copy_memory((dword_t)monitors[0].double_buffer, screenshoot_image_info_data_mem, screenshoot_image_info_data_length);
        show_system_message("You made a screenshot");
        wait(500);
        remove_system_message();
        screenshot_was_made = STATUS_TRUE;
        screenshot_is_cropped = STATUS_FALSE;
    }
    #endif
}