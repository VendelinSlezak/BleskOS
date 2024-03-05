//BleskOS

/*
* MIT License
* Copyright (c) 2023-2024 Vendelín Slezák
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

void initalize_screenshooter(void) {
 screenshoot_image_info_mem = create_image(screen_width, screen_height);
 cropped_image_info_mem = create_image(screen_width, screen_height);
 screenshoot_image_info_data_mem = get_image_data_memory(screenshoot_image_info_mem);
 screenshoot_image_info_data_length = (screen_width*screen_height*4);
 dword_t *screenshoot_image_info = (dword_t *) (screenshoot_image_info_mem);
 if(screen_width>=1024) {
  screenshot_buffer_image_original_width = (screen_width/15*13);
  screenshot_buffer_image_original_height = (screen_height/15*13);
 }
 else {
  screenshot_buffer_image_original_width = (screen_width/15*12);
  screenshot_buffer_image_original_height = (screen_height/15*12);
 }
 screenshot_buffer_image_original_x = (screen_x_center-(screenshot_buffer_image_original_width/2));
 screenshot_buffer_image_original_y = (screen_y_center-(screenshot_buffer_image_original_height/2));
 screenshoot_image_info[IMAGE_INFO_WIDTH] = screenshot_buffer_image_original_width;
 screenshoot_image_info[IMAGE_INFO_HEIGTH] = screenshot_buffer_image_original_height;
 screenshoot_image_info[IMAGE_INFO_SCREEN_X] = screenshot_buffer_image_original_x;
 screenshoot_image_info[IMAGE_INFO_SCREEN_Y] = screenshot_buffer_image_original_y;
 screenshot_was_made = STATUS_FALSE;
 screenshot_is_cropped = STATUS_FALSE;
}

void screenshooter(void) {
 redraw_screenshooter();

 while(1) {
  wait_for_user_input();
  move_mouse_cursor();

  dword_t click_value = get_mouse_cursor_click_board_value();

  //back
  if(keyboard_value==KEY_ESC || (mouse_click_button_state==MOUSE_CLICK && click_value==SCREENSHOTER_CLICK_ZONE_BACK)) {
   return;
  }

  //save
  if(keyboard_value==KEY_F2 || (mouse_click_button_state==MOUSE_CLICK && click_value==SCREENSHOTER_CLICK_ZONE_SAVE)) {
   file_dialog_save_set_extension("bmp");
   if(screenshot_is_cropped==STATUS_FALSE) {
    convert_image_data_to_bmp(screenshoot_image_info_mem);
   }
   else {
    convert_image_data_to_bmp(cropped_image_info_mem);
   }
   file_dialog_save(converted_file_memory, converted_file_size);
   free(converted_file_memory);
   redraw_screenshooter();
   continue;
  }

  //crop
  if(keyboard_value==KEY_SPACE || (mouse_click_button_state==MOUSE_CLICK && click_value==SCREENSHOTER_CLICK_ZONE_CROP)) {
   //draw screenshot buffer on whole screen
   dword_t *screenshoot_image_info = (dword_t *) (screenshoot_image_info_mem);
   screenshoot_image_info[IMAGE_INFO_SCREEN_X] = 0;
   screenshoot_image_info[IMAGE_INFO_SCREEN_Y] = 0;
   screenshoot_image_info[IMAGE_INFO_WIDTH] = screenshoot_image_info[IMAGE_INFO_REAL_WIDTH];
   screenshoot_image_info[IMAGE_INFO_HEIGTH] = screenshoot_image_info[IMAGE_INFO_REAL_HEIGTH];
   draw_image(screenshoot_image_info_mem);
   redraw_screen();
   mouse_click_button_state = NO_CLICK;

   //wait for click
   while(1) {
    wait_for_user_input();
    move_mouse_cursor();

    if(keyboard_value==KEY_ESC) {
     screenshoot_image_info[IMAGE_INFO_WIDTH] = screenshot_buffer_image_original_width;
     screenshoot_image_info[IMAGE_INFO_HEIGTH] = screenshot_buffer_image_original_height;
     screenshoot_image_info[IMAGE_INFO_SCREEN_X] = screenshot_buffer_image_original_x;
     screenshoot_image_info[IMAGE_INFO_SCREEN_Y] = screenshot_buffer_image_original_y;
     break;
    }

    if(mouse_click_button_state==MOUSE_CLICK) {
     screenshot_crop_x = mouse_cursor_x;
     screenshot_crop_y = mouse_cursor_y;
     screenshot_crop_width = 0;
     screenshot_crop_height = 0;
     mouse_click_button_state = MOUSE_DRAG;
     break;
    }
   }
   if(mouse_click_button_state!=MOUSE_DRAG) {
    redraw_screenshooter();
    continue; //ESC was pressed
   }

   //let user draw crop area
   while(1) {
    wait_for_user_input();
    move_mouse_cursor();

    //compute actual crop area
    if(mouse_cursor_x>screenshot_crop_x) {
     screenshot_crop_width = (mouse_cursor_x-screenshot_crop_x);
    }
    else {
     screenshot_crop_width = (mouse_cursor_x_click-mouse_cursor_x);
     screenshot_crop_x = mouse_cursor_x;
    }
    if(mouse_cursor_y>screenshot_crop_y) {
     screenshot_crop_height = (mouse_cursor_y-screenshot_crop_y);
    }
    else {
     screenshot_crop_height = (mouse_cursor_y_click-mouse_cursor_y);
     screenshot_crop_y = mouse_cursor_y;
    }

    //redraw
    draw_image(screenshoot_image_info_mem);
    draw_empty_square(screenshot_crop_x, screenshot_crop_y, screenshot_crop_width, screenshot_crop_height, BLACK);
    redraw_screen();

    //is crop area finished?
    if(mouse_click_button_state==NO_CLICK && screenshot_crop_width!=0 && screenshot_crop_height!=0) {
     break;
    }
   }

   //crop
   copy_raw_image_data(get_image_data_memory(screenshoot_image_info_mem), screen_width, screenshot_crop_x, screenshot_crop_y, screenshot_crop_width, screenshot_crop_height, get_image_data_memory(cropped_image_info_mem), screenshot_crop_width, 0, 0);

   //set variables for result image
   dword_t *cropped_image_info = (dword_t *) (cropped_image_info_mem);
   cropped_image_info[IMAGE_INFO_REAL_WIDTH] = screenshot_crop_width;
   cropped_image_info[IMAGE_INFO_DRAW_WIDTH] = screenshot_crop_width;
   if(screenshot_crop_width>screenshot_buffer_image_original_width) {
    cropped_image_info[IMAGE_INFO_WIDTH] = screenshot_buffer_image_original_width;
    cropped_image_info[IMAGE_INFO_SCREEN_X] = screenshot_buffer_image_original_x;
   }
   else {
    cropped_image_info[IMAGE_INFO_WIDTH] = screenshot_crop_width;
    cropped_image_info[IMAGE_INFO_SCREEN_X] = (screen_x_center-(screenshot_crop_width/2));
   }
   cropped_image_info[IMAGE_INFO_REAL_HEIGTH] = screenshot_crop_height;
   cropped_image_info[IMAGE_INFO_DRAW_HEIGTH] = screenshot_crop_height;
   if(screenshot_crop_height>screenshot_buffer_image_original_height) {
    cropped_image_info[IMAGE_INFO_HEIGTH] = screenshot_buffer_image_original_height;
    cropped_image_info[IMAGE_INFO_SCREEN_Y] = screenshot_buffer_image_original_y;
   }
   else {
    cropped_image_info[IMAGE_INFO_HEIGTH] = screenshot_crop_height;
    cropped_image_info[IMAGE_INFO_SCREEN_Y] = (screen_y_center-(screenshot_crop_height/2));
   }

   //redraw screen
   screenshoot_image_info[IMAGE_INFO_WIDTH] = (screen_width/15*13);
   screenshoot_image_info[IMAGE_INFO_HEIGTH] = screenshot_buffer_image_original_height;
   screenshoot_image_info[IMAGE_INFO_SCREEN_X] = screenshot_buffer_image_original_x;
   screenshoot_image_info[IMAGE_INFO_SCREEN_Y] = screenshot_buffer_image_original_y;
   screenshot_is_cropped = STATUS_TRUE;
   redraw_screenshooter();
   continue;
  }

  //remove crop
  if(keyboard_value==KEY_R || (mouse_click_button_state==MOUSE_CLICK && click_value==SCREENSHOTER_CLICK_ZONE_REMOVE_CROP)) {
   screenshot_is_cropped=STATUS_FALSE;
   redraw_screenshooter();
   continue;
  }

  //redraw
  if(screenshot_was_made==STATUS_TRUE) {
   redraw_screenshooter();
  }
 }
}

void redraw_screenshooter(void) {
 clear_click_board();
 bleskos_main_window_draw_background();
 print("Key PrtScr copy screen to screenshot buffer", 20, 20, BLACK);
 print_to_message_window("Actual screenshot buffer", 35);
 dword_t *image_info;
 if(screenshot_is_cropped==STATUS_FALSE) {
  image_info = (dword_t *)(screenshoot_image_info_mem);
 }
 else {
  image_info = (dword_t *)(cropped_image_info_mem);
 }
 draw_resized_image((dword_t)image_info);
 draw_empty_square(image_info[IMAGE_INFO_SCREEN_X]-1, image_info[IMAGE_INFO_SCREEN_Y]-1, image_info[IMAGE_INFO_WIDTH]+2, image_info[IMAGE_INFO_HEIGTH]+2, BLACK);
 if(screen_width>=900) {
  draw_button_with_click_zone("[esc] Back", 20, screen_height-40, 200, 20, SCREENSHOTER_CLICK_ZONE_BACK);
  draw_button_with_click_zone("[F2] Save screenshot", 20+200+20, screen_height-40, 200, 20, SCREENSHOTER_CLICK_ZONE_SAVE);
  draw_button_with_click_zone("[space] Crop", 20+200+20+200+20, screen_height-40, 200, 20, SCREENSHOTER_CLICK_ZONE_CROP);
  if(screenshot_is_cropped==STATUS_TRUE) {
   draw_button_with_click_zone("[r] Remove crop", 20+200+20+200+20+200+20, screen_height-40, 200, 20, SCREENSHOTER_CLICK_ZONE_REMOVE_CROP);
  }
 }
 else {
  draw_button_with_click_zone("[esc] Back", 20, screen_height-40, 100, 20, SCREENSHOTER_CLICK_ZONE_BACK);
  draw_button_with_click_zone("[F2] Save screenshot", 20+100+10, screen_height-40, 180, 20, SCREENSHOTER_CLICK_ZONE_SAVE);
  draw_button_with_click_zone("[space] Crop", 20+100+10+180+10, screen_height-40, 110, 20, SCREENSHOTER_CLICK_ZONE_CROP);
  if(screenshot_is_cropped==STATUS_TRUE) {
   draw_button_with_click_zone("[r] Remove crop", 20+100+10+180+10+110+10, screen_height-40, 150, 20, SCREENSHOTER_CLICK_ZONE_REMOVE_CROP);
  }
 }
 redraw_screen();
 screenshot_was_made = STATUS_FALSE;
}