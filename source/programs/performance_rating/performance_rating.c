//BleskOS

/*
* MIT License
* Copyright (c) 2023-2024 Vendelín Slezák, defdefred
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

void initalize_performance_rating(void) {

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
        message_window("Running task...");
        redraw_screen();
        mouse_movement_x = graphic_screen_x - mouse_cursor_x - (MOUSE_CURSOR_WIDTH>>1);
        mouse_movement_y = graphic_screen_y - mouse_cursor_y - (MOUSE_CURSOR_HEIGTH>>1);
        move_mouse_cursor(); // mouse need to be on expected position
        clear_screen(0x123456); // to make screen redraw visible
        performance_rating_run_task(PERFORMANCE_RATING_CURRENT_TASK);
        redraw_performance_rating();
        break;
      case KEY_A:
        for (int i=0; i<PERFORMANCE_RATING_NBTASK; i++) {
          PERFORMANCE_RATING_CURRENT_TASK=i;
          redraw_performance_rating();
          message_window("Running tasks...");
          redraw_screen();
          mouse_movement_x = graphic_screen_x - mouse_cursor_x - (MOUSE_CURSOR_WIDTH>>1);
          mouse_movement_y = graphic_screen_y - mouse_cursor_y - (MOUSE_CURSOR_HEIGTH>>1);
          move_mouse_cursor(); // mouse need to be on expectedi position
          clear_screen(0x123456); // to make screen redraw visible
          performance_rating_run_task(i);
        }
          redraw_performance_rating();
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

  for (int i=0; i<PERFORMANCE_RATING_NBTASK; i++) {
    if ( i == PERFORMANCE_RATING_CURRENT_TASK ) { color=RED; } else { color=BLACK; };
    print(performance_rating_tasks[i].name, 20, 10+i*10, color);
    print_var(performance_rating_tasks[i].result >> 7, graphic_screen_x_center, 10+i*10, color);
    print_var(performance_rating_tasks[i].result, graphic_screen_x_center+10*8, 10+i*10, color);
  }
  print("Results are time in microseconds for 1x run and 128x runs", 20, 10+PERFORMANCE_RATING_NBTASK*10+10, BLACK);
  print("[ESC] Quit | [ARROWS] Choose | [ENTER] Run | [A] Run all", 0, graphic_screen_y-8, BLACK);
  add_zone_to_click_board(0, graphic_screen_y-8, 11*8, 8, PERFORMANCE_RATING_CLICK_ZONE_BACK);
  redraw_screen();
}

void performance_rating_run_task(dword_t task_number) {  // get time of 128 runs
  reset_timer();
  for(dword_t i=0; i<PERFORMANCE_RATING_RUN_COUNT; i++) {
    performance_rating_tasks[task_number].run();
  }
  performance_rating_tasks[task_number].result = get_timer_value_in_microseconds();

  if(performance_rating_tasks[task_number].result < 1000000) {
    wait(1000);
  }
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


