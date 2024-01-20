//BleskOS

/*
* MIT License
* Copyright (c) 2023-2024 Vendelín Slezák, defdefred
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

void initalize_performance_rating(void) {
  PERFORMANCE_RATING_NBTASK_PER_PAGE=(graphic_screen_y-20)/10;
}

void performance_rating(void) {
  redraw_performance_rating();

  while(1) {
    wait_for_user_input();
    move_mouse_cursor();

    dword_t click_value = get_mouse_cursor_click_board_value();
  if(mouse_click_button_state==MOUSE_CLICK && click_value==PERFORMANCE_RATING_CLICK_ZONE_BACK) {
  return;
  }

    switch (keyboard_value) {
      case KEY_UP:
        if (PERFORMANCE_RATING_CURRENT_TASK > 0) { PERFORMANCE_RATING_CURRENT_TASK--; };
        redraw_performance_rating();
        break;
      case KEY_DOWN:
        if (PERFORMANCE_RATING_CURRENT_TASK < PERFORMANCE_RATING_NBTASK-1) { PERFORMANCE_RATING_CURRENT_TASK++; };
        redraw_performance_rating();
        break;
      case KEY_ENTER:
        performance_rating_run_task(PERFORMANCE_RATING_CURRENT_TASK);
        redraw_performance_rating();
        break;
      case KEY_A:
          PERFORMANCE_RATING_CURRENT_TASK=0;
          redraw_performance_rating();
        for (; PERFORMANCE_RATING_CURRENT_TASK<PERFORMANCE_RATING_NBTASK; PERFORMANCE_RATING_CURRENT_TASK++) {
          performance_rating_run_task(PERFORMANCE_RATING_CURRENT_TASK);
          redraw_performance_rating();
        }
        break;
      case KEY_PAGE_UP:
        break;
      case KEY_PAGE_DOWN:
        break;
      case KEY_ESC:
        return;
    }
  }
}

void redraw_performance_rating(void) {
  dword_t color;
  clear_screen(0xbb00dd);
  clear_click_board();

  print("Results are time in microseconds for 1x run and 128x runs", 10, 10, BLACK);
  print("x1", graphic_screen_x-10*8*2, 10, BLACK);
  print("x128", graphic_screen_x-10*8, 10, BLACK);

  if ( PERFORMANCE_RATING_CURRENT_TASK > PERFORMANCE_RATING_NBTASK_PER_PAGE ) { color=RED; } else { color=BLACK; }
  for (int i=0; i<PERFORMANCE_RATING_NBTASK && i<PERFORMANCE_RATING_NBTASK_PER_PAGE ; i++) {
    if ( i == PERFORMANCE_RATING_CURRENT_TASK ) { color=RED; } else { color=BLACK; };
    print(performance_rating_tasks[i].name, 10, 20+i*10, color);
    print_var(performance_rating_tasks[i].result >> 7, graphic_screen_x-10*8*2, 20+i*10, color);
    print_var(performance_rating_tasks[i].result, graphic_screen_x-10*8, 20+i*10, color);
  }

  print("[ESC] Quit | [ARROWS] Choose | [ENTER] Run | [A] Run all", 2, graphic_screen_y-10, BLACK);
  add_zone_to_click_board(2, graphic_screen_y-10, 11*8, 8, PERFORMANCE_RATING_CLICK_ZONE_BACK);
  redraw_screen();
}

void performance_rating_run_task(dword_t task_number) {  // get time of 128 runs
  message_window("Running tasks...");
  redraw_screen();
  mouse_movement_x = graphic_screen_x - mouse_cursor_x - (MOUSE_CURSOR_WIDTH>>1);
  mouse_movement_y = graphic_screen_y - mouse_cursor_y - (MOUSE_CURSOR_HEIGTH>>1);
  move_mouse_cursor(); // mouse need to be on expected position
  clear_screen(0x123456); // to make screen redraw visible
  reset_timer();

  for(PERFORMANCE_RATING_CURRENT_RUN=0; PERFORMANCE_RATING_CURRENT_RUN<PERFORMANCE_RATING_RUN_COUNT; PERFORMANCE_RATING_CURRENT_RUN++) {
    performance_rating_tasks[task_number].run();
  }
  performance_rating_tasks[task_number].result = get_timer_value_in_microseconds();

  if(performance_rating_tasks[task_number].result < 1000000) {
    wait(1000);
  }
    wait(1000);
}

void performance_rating_task0() {
  clear_screen(0x123456);
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

void performance_rating_task5() { // 25% screen center
  redraw_part_of_framebuffer( graphic_screen_x_center>>1, graphic_screen_y_center>>1, graphic_screen_x>>1, graphic_screen_y>>1);
}

void performance_rating_task6() { // 25% screen half rigth
  redraw_part_of_framebuffer( graphic_screen_x_center+(graphic_screen_x_center>>1), graphic_screen_y_center>>1, graphic_screen_x>>1, graphic_screen_y>>1);
}

void performance_rating_task7() { // 25% screen half bottom
  redraw_part_of_framebuffer( graphic_screen_x_center>>1, graphic_screen_y_center+(graphic_screen_y_center>>1), graphic_screen_x>>1, graphic_screen_y>>1);
}

void performance_rating_task8() { //  Scalable font print
  scalable_font_print("AaBbCcDdEeFfGgHhIiJjKkLlMmNnOoPpQqRrSsTtUuVvWwXxYyZz0123456789", 10+PERFORMANCE_RATING_CURRENT_RUN, 10+PERFORMANCE_RATING_CURRENT_RUN, BLACK);  
}
void performance_rating_task9() { // todo 
}
void performance_rating_task10() { // todo 
}
void performance_rating_task11() { // todo 
}
void performance_rating_task12() { // todo 
}
void performance_rating_task13() { // todo 
}
void performance_rating_task14() { // todo 
}
void performance_rating_task15() { // todo 
}

