//BleskOS

/*
* MIT License
* Copyright (c) 2023-2024 Vendelín Slezák, defdefred
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

void initalize_performance_rating(void) {
//  dword_t *image;
  PERFORMANCE_RATING_NBTASK_PER_PAGE=(screen_height-26)/10;
//  PERFORMANCE_RATING_IMAGE=(dword_t *)create_image(640, 480);

//  for(int i=0, image=PERFORMANCE_RATING_IMAGE;i<640*480;i++) {
//    *image++=((i<<2) & 0xffffff00);
//  }
}

void performance_rating(void) {
  redraw_performance_rating();
  set_pen_width(1,WHITE);

  while(1) {
    wait_for_user_input();
    move_mouse_cursor();

    dword_t click_value = get_mouse_cursor_click_board_value();
    if(mouse_click_button_state==MOUSE_CLICK && click_value==PERFORMANCE_RATING_CLICK_ZONE_BACK) {
      return;
    }

    switch (keyboard_value) {
      case KEY_UP:
        if (PERFORMANCE_RATING_CURRENT_TASK > 0) { PERFORMANCE_RATING_CURRENT_TASK--; }
        redraw_performance_rating();
        break;
      case KEY_DOWN:
        if (PERFORMANCE_RATING_CURRENT_TASK < PERFORMANCE_RATING_NBTASK-1) { PERFORMANCE_RATING_CURRENT_TASK++; }
        redraw_performance_rating();
        break;
      case KEY_ENTER:
        performance_rating_run_task(PERFORMANCE_RATING_CURRENT_TASK);
        redraw_performance_rating();
        break;
      case KEY_I:
        performance_rating_inspect_task(PERFORMANCE_RATING_CURRENT_TASK);
        redraw_performance_rating();
        break;
      case KEY_A:
        for (PERFORMANCE_RATING_CURRENT_TASK=0; PERFORMANCE_RATING_CURRENT_TASK<PERFORMANCE_RATING_NBTASK; PERFORMANCE_RATING_CURRENT_TASK++) {
          performance_rating_tasks[PERFORMANCE_RATING_CURRENT_TASK].result = 0;
          redraw_performance_rating();
          performance_rating_run_task(PERFORMANCE_RATING_CURRENT_TASK);
          redraw_performance_rating();
			 if( keyboard_value == KEY_ESC ) {break;}
        }
        break;
      case KEY_PAGE_UP:
        if (PERFORMANCE_RATING_CURRENT_TASK > 5) { PERFORMANCE_RATING_CURRENT_TASK-=5; } else { PERFORMANCE_RATING_CURRENT_TASK=0; }
        redraw_performance_rating();
        break;
      case KEY_PAGE_DOWN:
        if (PERFORMANCE_RATING_CURRENT_TASK < PERFORMANCE_RATING_NBTASK-5) { PERFORMANCE_RATING_CURRENT_TASK+=5; }
          else { PERFORMANCE_RATING_CURRENT_TASK=PERFORMANCE_RATING_NBTASK-1; }
        redraw_performance_rating();
        break;
      case KEY_ESC:
        return;
    }
  }
}

void redraw_performance_rating(void) {
  dword_t color,line=2, page, task;
  clear_screen(0xbb00dd);
  clear_click_board();

  print("Results are time in microseconds for 1x run and 128x runs", 10, line, BLACK);
  print_var( screen_width, 580, line, BLACK);
  print("x", 630, line, BLACK);
  print_var( screen_height, 640, line, BLACK);
  print("x", 690, line, BLACK);
  print_var( screen_bpp, 700, line, BLACK);
  print("x1", screen_width-10*8*2, line, BLACK);
  print("x128", screen_width-10*8, line, BLACK);
  draw_line(0,12,screen_width,12,BLACK);
  line+=14;

  page=PERFORMANCE_RATING_CURRENT_TASK / PERFORMANCE_RATING_NBTASK_PER_PAGE; 

  for (task=page*PERFORMANCE_RATING_NBTASK_PER_PAGE; task<PERFORMANCE_RATING_NBTASK && task<(page+1)*PERFORMANCE_RATING_NBTASK_PER_PAGE; task++) {
    if ( task == PERFORMANCE_RATING_CURRENT_TASK ) { color=WHITE; } else { color=BLACK; };
    print(performance_rating_tasks[task].name, 10, line, color);
    print_var(performance_rating_tasks[task].result >> 7, screen_width-10*8*2, line, color);
    print_var(performance_rating_tasks[task].result, screen_width-10*8, line, color);
    line+=10;
  }

  print("[ESC] Quit | [ARROWS] Choose | [ENTER] Run | [I] Inspect | [A] Run all", 10, screen_height-12, BLACK);
  add_zone_to_click_board(10, screen_height-12, 11*8, 8, PERFORMANCE_RATING_CLICK_ZONE_BACK);
  redraw_screen();
}

void performance_rating_run_task(dword_t task_number) {  // get time of 128 runs
  message_window("Running tasks...");
  redraw_screen();
  mouse_movement_x = screen_width - mouse_cursor_x - (MOUSE_CURSOR_WIDTH>>1);
  mouse_movement_y = screen_height - mouse_cursor_y - (MOUSE_CURSOR_HEIGTH>>1);
  move_mouse_cursor(); // mouse need to be on expected position
  reset_timer();

  for(PERFORMANCE_RATING_CURRENT_RUN=0; PERFORMANCE_RATING_CURRENT_RUN<PERFORMANCE_RATING_RUN_COUNT; PERFORMANCE_RATING_CURRENT_RUN++) {
    performance_rating_tasks[task_number].run();
  }
  performance_rating_tasks[task_number].result = get_timer_value_in_microseconds();
}

void performance_rating_inspect_task(dword_t task_number) {  // to be sure what's going on
  mouse_movement_x = screen_width - mouse_cursor_x - (MOUSE_CURSOR_WIDTH>>1);
  mouse_movement_y = screen_height - mouse_cursor_y - (MOUSE_CURSOR_HEIGTH>>1);
  move_mouse_cursor(); // mouse need to be on expected position

  for(PERFORMANCE_RATING_CURRENT_RUN=0; PERFORMANCE_RATING_CURRENT_RUN<PERFORMANCE_RATING_RUN_COUNT; PERFORMANCE_RATING_CURRENT_RUN++) {
    clear_screen(PERFORMANCE_RATING_CURRENT_RUN<<1);
    PERFORMANCE_RATING_RESULT_STRING[0]=0;
    PERFORMANCE_RATING_RESULT_NUMBER=0;
    PERFORMANCE_RATING_RESULT_FLOAT=0.0;
    performance_rating_tasks[task_number].run();
    draw_full_square(0, 0, 64, 60, BLACK);
    print("[ESC]",2,2,WHITE);
    print_var(PERFORMANCE_RATING_CURRENT_RUN,42,2,WHITE);
    print_ascii(&PERFORMANCE_RATING_RESULT_STRING[0],2,12,WHITE);
    print_unicode((word_t *)&PERFORMANCE_RATING_RESULT_STRING[0],2,24,WHITE);
    print_var(PERFORMANCE_RATING_RESULT_NUMBER,2,36,WHITE);
    // Todo print results
    redraw_screen();
    if (keyboard_value == KEY_ESC) { break;}
  }
}

void performance_rating_task0() { // clear screen
  clear_screen(PERFORMANCE_RATING_CURRENT_RUN<<1);
}

void performance_rating_task1() { // on screen center
  mouse_movement_x = -3;
  mouse_movement_y = -3; 
  move_mouse_cursor();
}

void performance_rating_task2() { // on screen border rigth
  mouse_movement_y = -3;
  move_mouse_cursor();
}

void performance_rating_task3() { //on screen border bottom
  mouse_movement_x = -3;
  move_mouse_cursor();
}
void performance_rating_task4() { // full screen
  redraw_screen();
}
void performance_rating_task5() { // 25% screen
  redraw_part_of_screen( screen_x_center>>1, screen_y_center>>1, screen_width>>1, screen_height>>1);
}
void performance_rating_task6() { // 25% screen half rigth
  redraw_part_of_screen( screen_x_center+(screen_x_center>>1), screen_y_center>>1, screen_width>>1, screen_height>>1);
}
void performance_rating_task7() { // 25% screen half bottom
  redraw_part_of_screen( screen_x_center>>1, screen_y_center+(screen_y_center>>1), screen_width>>1, screen_height>>1);
}
void performance_rating_task8() { //  Scalable font print
  scalable_font_print("AaBbCcDdEeFfGgHhIiJjKkLlMmNnOoPpQqRrSsTtUuVvWwXxYyZz0123456789", screen_x_center-(PERFORMANCE_RATING_CURRENT_RUN<<2), screen_y_center-(PERFORMANCE_RATING_CURRENT_RUN<<1), WHITE);  
}
void performance_rating_task9() { // bitmap font print
  print("AaBbCcDdEeFfGgHhIiJjKkLlMmNnOoPpQqRrSsTtUuVvWwXxYyZz0123456789", screen_x_center-(PERFORMANCE_RATING_CURRENT_RUN<<2), screen_y_center-(PERFORMANCE_RATING_CURRENT_RUN<<1), WHITE);  
}
void performance_rating_task10() { // print numeric variable 
  print_var(3473898+(PERFORMANCE_RATING_CURRENT_RUN<<2), screen_x_center-(PERFORMANCE_RATING_CURRENT_RUN<<2), screen_y_center-(PERFORMANCE_RATING_CURRENT_RUN<<1), WHITE);  
}
void performance_rating_task11() { // print unicode string ÀÁÂÃÄÅÈÉÊËÌÍÎÏÑÒÓÔÕÖÙÚÛÜÝßàáâãäåèéêëìíîïñòóôõöùúûüýÿčďěľňŕřšťž
  print_unicode((word_t *)"\xc0\x00\xc1\x00\xc2\x00\xc3\x00\xc4\x00\xc5\x00\xc8\x00\xc9\x00\xca\x00\xcb\x00\xcc\x00\xcd\x00\xce\x00\xcf\x00\xd1\x00\xd2\x00\xd3\x00\xd4\x00\xd5\x00\xd6\x00\xd9\x00\xda\x00\xdb\x00\xdc\x00\xdd\x00\xdf\x00\xe0\x00\xe1\x00\xe2\x00\xe3\x00\xe4\x00\xe5\x00\xe8\x00\xe9\x00\xea\x00\xeb\x00\xec\x00\xed\x00\xee\x00\xef\x00\xf1\x00\xf2\x00\xf3\x00\xf4\x00\xf5\x00\xf6\x00\xf9\x00\xfa\x00\xfb\x00\xfc\x00\xfd\x00\xff\x00\x0d\x01\x0f\x01\x1b\x01\x3e\x01\x48\x01\x55\x01\x59\x01\x61\x01\x65\x01\x7e\x01\x00\x00",
    screen_x_center-(PERFORMANCE_RATING_CURRENT_RUN<<2), screen_y_center-(PERFORMANCE_RATING_CURRENT_RUN<<1), WHITE);  
}
void performance_rating_task12() { // print hexa number
  print_hex(3467423+(PERFORMANCE_RATING_CURRENT_RUN<<2), screen_x_center-(PERFORMANCE_RATING_CURRENT_RUN<<2), screen_y_center-(PERFORMANCE_RATING_CURRENT_RUN<<1), WHITE);  
}
void performance_rating_task13() { // print ascii
  print_ascii("AaBbCcDdEeFfGgHhIiJjKkLlMmNnOoPpQqRrSsTtUuVvWwXxYyZz0123456789", screen_x_center-(PERFORMANCE_RATING_CURRENT_RUN<<2), screen_y_center-(PERFORMANCE_RATING_CURRENT_RUN<<1), WHITE);  
}
void performance_rating_task14() { // draw pixel
  draw_pixel(screen_x_center-(PERFORMANCE_RATING_CURRENT_RUN<<2), screen_y_center-(PERFORMANCE_RATING_CURRENT_RUN<<1), WHITE);
}
void performance_rating_task15() { // draw straigth line
  draw_straigth_line(screen_x_center>>1, screen_y_center-(PERFORMANCE_RATING_CURRENT_RUN<<1), screen_x_center, WHITE);
}
void performance_rating_task16() { // draw straigth column
  draw_straigth_column(screen_x_center-(PERFORMANCE_RATING_CURRENT_RUN<<1), screen_y_center>>1, screen_y_center, WHITE);
}
void performance_rating_task17() { // draw line
  draw_line(0,(PERFORMANCE_RATING_CURRENT_RUN<<2), screen_width, screen_height-(PERFORMANCE_RATING_CURRENT_RUN<<2), WHITE);
}
void performance_rating_task18() { // draw quadratic bezier
  draw_quadratic_bezier(PERFORMANCE_RATING_CURRENT_RUN<<3,0,0,PERFORMANCE_RATING_CURRENT_RUN<<2, screen_width, screen_height-(PERFORMANCE_RATING_CURRENT_RUN<<2), WHITE);
}
void performance_rating_task19() { // draw empty square
  draw_empty_square(PERFORMANCE_RATING_CURRENT_RUN,PERFORMANCE_RATING_CURRENT_RUN, screen_width-(PERFORMANCE_RATING_CURRENT_RUN<<1), screen_height-(PERFORMANCE_RATING_CURRENT_RUN<<1), WHITE);
}
void performance_rating_task20() { // draw full square
  draw_full_square(PERFORMANCE_RATING_CURRENT_RUN,PERFORMANCE_RATING_CURRENT_RUN, screen_width-(PERFORMANCE_RATING_CURRENT_RUN<<1), screen_height-(PERFORMANCE_RATING_CURRENT_RUN<<1), WHITE);
}
void performance_rating_task21() { // draw empty circle point
  draw_empty_circle_point((PERFORMANCE_RATING_CURRENT_RUN<<2),(PERFORMANCE_RATING_CURRENT_RUN<<1), WHITE);
}
void performance_rating_task22() { // draw empty circle
  draw_empty_circle((PERFORMANCE_RATING_CURRENT_RUN<<2),(PERFORMANCE_RATING_CURRENT_RUN<<1),(PERFORMANCE_RATING_CURRENT_RUN<<1), WHITE);
}
void performance_rating_task23() { // draw full circle line
  draw_full_circle_line((PERFORMANCE_RATING_CURRENT_RUN<<1),(PERFORMANCE_RATING_CURRENT_RUN<<2), WHITE);
}
void performance_rating_task24() { // draw full circle
  draw_full_circle((PERFORMANCE_RATING_CURRENT_RUN<<2),(PERFORMANCE_RATING_CURRENT_RUN<<1),(PERFORMANCE_RATING_CURRENT_RUN<<1), WHITE);
}
void performance_rating_task25() { // draw empty ellipse
  draw_empty_ellipse((PERFORMANCE_RATING_CURRENT_RUN<<2),(PERFORMANCE_RATING_CURRENT_RUN<<1),screen_width-(PERFORMANCE_RATING_CURRENT_RUN<<2),screen_height-(PERFORMANCE_RATING_CURRENT_RUN<<1), WHITE);
}
void performance_rating_task26() { // draw part empty ellipse
  draw_parts_of_empty_ellipse(PERFORMANCE_RATING_CURRENT_RUN,(PERFORMANCE_RATING_CURRENT_RUN<<2),(PERFORMANCE_RATING_CURRENT_RUN<<1),screen_width-(PERFORMANCE_RATING_CURRENT_RUN<<2),screen_height-(PERFORMANCE_RATING_CURRENT_RUN<<1), WHITE);
}
void performance_rating_task27() { // draw full ellipse
  draw_full_ellipse((PERFORMANCE_RATING_CURRENT_RUN<<2),(PERFORMANCE_RATING_CURRENT_RUN<<1),screen_width-(PERFORMANCE_RATING_CURRENT_RUN<<2),screen_height-(PERFORMANCE_RATING_CURRENT_RUN<<1), WHITE);
}
void performance_rating_task28() { // full framebuffer
  redraw_framebuffer();
}
void performance_rating_task29() { // 25% framebuffer
  redraw_part_of_framebuffer( screen_x_center>>1, screen_y_center>>1, screen_width>>1, screen_height>>1);
}
void performance_rating_task30() { // math: abs
  PERFORMANCE_RATING_RESULT_NUMBER=abs((int)-PERFORMANCE_RATING_CURRENT_RUN);
}
void performance_rating_task31() { // math: fabs
  PERFORMANCE_RATING_RESULT_FLOAT=fabs((float)-PERFORMANCE_RATING_CURRENT_RUN);
}
void performance_rating_task32() { // math: floor
  PERFORMANCE_RATING_RESULT_FLOAT=floor((float)-PERFORMANCE_RATING_CURRENT_RUN);
}
void performance_rating_task33() { // math: power
  PERFORMANCE_RATING_RESULT_NUMBER=power(PERFORMANCE_RATING_CURRENT_RUN, PERFORMANCE_RATING_CURRENT_RUN);
}
void performance_rating_task34() { // number of digit
  PERFORMANCE_RATING_RESULT_NUMBER=get_number_of_digits_in_number(PERFORMANCE_RATING_CURRENT_RUN<<16);
}
void performance_rating_task35() { // convert byte string to dword
  PERFORMANCE_RATING_RESULT_NUMBER=convert_byte_string_to_number((dword_t)(&PERFORMANCE_RATING_SOURCE_STRING));
}
void performance_rating_task36() { // convert word string  to dword
  PERFORMANCE_RATING_RESULT_NUMBER=convert_word_string_to_number((dword_t)(&PERFORMANCE_RATING_SOURCE_NUMBER));
}
void performance_rating_task37() { // convert word string  to float 
  PERFORMANCE_RATING_RESULT_FLOAT=convert_word_string_to_float_number((dword_t)(&PERFORMANCE_RATING_SOURCE_FLOAT));
}
void performance_rating_task38() { // convert hex word string to dword 
  PERFORMANCE_RATING_RESULT_NUMBER=convert_hex_word_string_to_number((dword_t)(&PERFORMANCE_RATING_SOURCE_HEXA));
}
void performance_rating_task39() { // convert dword to byte string 
  convert_number_to_byte_string(PERFORMANCE_RATING_CURRENT_RUN<<16, (dword_t)(&PERFORMANCE_RATING_RESULT_STRING));
}
void performance_rating_task40() { // convert dword to word string 
  convert_number_to_word_string(PERFORMANCE_RATING_CURRENT_RUN<<16, (dword_t)(&PERFORMANCE_RATING_RESULT_STRING));
}
void performance_rating_task41() { // convert unix time
  convert_unix_time(PERFORMANCE_RATING_CURRENT_RUN<<16);
}
void performance_rating_task42() { // draw image
//  draw_image((dword_t)PERFORMANCE_RATING_IMAGE);
 return;
}
void performance_rating_task43() { // qoi compress
 return;
}
void performance_rating_task44() { // qoi decompress
 return;
}

