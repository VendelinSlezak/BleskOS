//BleskOS

/*
* MIT License
* Copyright (c) 2023-2024 Vendelín Slezák, defdefred
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

void initalize_performance_rating(void) {
  PERFORMANCE_RATING_NBTASK_PER_PAGE=(graphic_screen_y-26)/10;
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
        PERFORMANCE_RATING_CURRENT_TASK=0;
        redraw_performance_rating();
        for (; PERFORMANCE_RATING_CURRENT_TASK<PERFORMANCE_RATING_NBTASK; PERFORMANCE_RATING_CURRENT_TASK++) {
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
        if (PERFORMANCE_RATING_CURRENT_TASK < PERFORMANCE_RATING_NBTASK-5) { PERFORMANCE_RATING_CURRENT_TASK+=5; } else { PERFORMANCE_RATING_CURRENT_TASK=PERFORMANCE_RATING_NBTASK-1; }
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
  print_var( graphic_screen_x, 580, line, BLACK);
  print("x", 630, line, BLACK);
  print_var( graphic_screen_y, 640, line, BLACK);
  print("x", 690, line, BLACK);
  print_var( graphic_screen_bpp, 700, line, BLACK);
  print("x1", graphic_screen_x-10*8*2, line, BLACK);
  print("x128", graphic_screen_x-10*8, line, BLACK);
  draw_line(0,12,graphic_screen_x,12,BLACK);
  line+=14;

  page=PERFORMANCE_RATING_CURRENT_TASK / PERFORMANCE_RATING_NBTASK_PER_PAGE; 

  for (task=page*PERFORMANCE_RATING_NBTASK_PER_PAGE; task<PERFORMANCE_RATING_NBTASK && task<(page+1)*PERFORMANCE_RATING_NBTASK_PER_PAGE; task++) {
    if ( task == PERFORMANCE_RATING_CURRENT_TASK ) { color=WHITE; } else { color=BLACK; };
    print(performance_rating_tasks[task].name, 10, line, color);
    print_var(performance_rating_tasks[task].result >> 7, graphic_screen_x-10*8*2, line, color);
    print_var(performance_rating_tasks[task].result, graphic_screen_x-10*8, line, color);
    line+=10;
  }

  print("[ESC] Quit | [ARROWS] Choose | [ENTER] Run | [I] Inspect | [A] Run all", 10, graphic_screen_y-12, BLACK);
  add_zone_to_click_board(10, graphic_screen_y-12, 11*8, 8, PERFORMANCE_RATING_CLICK_ZONE_BACK);
  redraw_screen();
}

void performance_rating_run_task(dword_t task_number) {  // get time of 128 runs
  message_window("Running tasks...");
  redraw_screen();
  mouse_movement_x = graphic_screen_x - mouse_cursor_x - (MOUSE_CURSOR_WIDTH>>1);
  mouse_movement_y = graphic_screen_y - mouse_cursor_y - (MOUSE_CURSOR_HEIGTH>>1);
  move_mouse_cursor(); // mouse need to be on expected position
  reset_timer();

  for(PERFORMANCE_RATING_CURRENT_RUN=0; PERFORMANCE_RATING_CURRENT_RUN<PERFORMANCE_RATING_RUN_COUNT; PERFORMANCE_RATING_CURRENT_RUN++) {
    performance_rating_tasks[task_number].run();
  }
  performance_rating_tasks[task_number].result = get_timer_value_in_microseconds();
}

void performance_rating_inspect_task(dword_t task_number) {  // get time of 128 runs
  message_window("Running tasks, [ESC] to quit...");
  redraw_screen();
  mouse_movement_x = graphic_screen_x - mouse_cursor_x - (MOUSE_CURSOR_WIDTH>>1);
  mouse_movement_y = graphic_screen_y - mouse_cursor_y - (MOUSE_CURSOR_HEIGTH>>1);
  move_mouse_cursor(); // mouse need to be on expected position

  for(PERFORMANCE_RATING_CURRENT_RUN=0; PERFORMANCE_RATING_CURRENT_RUN<PERFORMANCE_RATING_RUN_COUNT; PERFORMANCE_RATING_CURRENT_RUN++) {
    performance_rating_tasks[task_number].run();
    draw_full_square(0, 0, 32, 10, BLACK);
    print_var(PERFORMANCE_RATING_CURRENT_RUN,2,2,WHITE);
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
  scalable_font_print("AaBbCcDdEeFfGgHhIiJjKkLlMmNnOoPpQqRrSsTtUuVvWwXxYyZz0123456789", graphic_screen_x_center-(PERFORMANCE_RATING_CURRENT_RUN<<2), graphic_screen_y-(PERFORMANCE_RATING_CURRENT_RUN<<1), BLACK);  
}
void performance_rating_task9() { // bitmap font print
  print("AaBbCcDdEeFfGgHhIiJjKkLlMmNnOoPpQqRrSsTtUuVvWwXxYyZz0123456789", graphic_screen_x_center-(PERFORMANCE_RATING_CURRENT_RUN<<2), graphic_screen_y-(PERFORMANCE_RATING_CURRENT_RUN<<1), BLACK);  
}
void performance_rating_task10() { // print numeric variable 
  print_var(3473898+(PERFORMANCE_RATING_CURRENT_RUN<<2), graphic_screen_x_center-(PERFORMANCE_RATING_CURRENT_RUN<<2), graphic_screen_y-(PERFORMANCE_RATING_CURRENT_RUN<<1), BLACK);  
}
void performance_rating_task11() { // print unicode string ÀÁÂÃÄÅÈÉÊËÌÍÎÏÑÒÓÔÕÖÙÚÛÜÝßàáâãäåèéêëìíîïñòóôõöùúûüýÿčďěľňŕřšťž
  print_unicode((word_t *)"\u00c0\u00c1\u00c2\u00c3\u00c4\u00c5\u00c8\u00c9\u00ca\u00cb\u00cc\u00cd\u00ce\u00cf\u00d1\u00d2\u00d3\u00d4\u00d5\u00d6\u00d9\u00da\u00db\u00dc\u00dd\u00df\u00e0\u00e1\u00e2\u00e3\u00e4\u00e5\u00e8\u00e9\u00ea\u00eb\u00ec\u00ed\u00ee\u00ef\u00f1\u00f2\u00f3\u00f4\u00f5\u00f6\u00f9\u00fa\u00fb\u00fc\u00fd\u00ff\u010d\u010f\u011b\u013e\u0148\u0155\u0159\u0161\u0165\u017e",
    graphic_screen_x_center-(PERFORMANCE_RATING_CURRENT_RUN<<2), graphic_screen_y-(PERFORMANCE_RATING_CURRENT_RUN<<1), BLACK);  
}
void performance_rating_task12() { // print hexa number
  print_hex(3467423+(PERFORMANCE_RATING_CURRENT_RUN<<2), graphic_screen_x_center-(PERFORMANCE_RATING_CURRENT_RUN<<2), graphic_screen_y-(PERFORMANCE_RATING_CURRENT_RUN<<1), BLACK);  
}
void performance_rating_task13() { // print ascii
  print_ascii("AaBbCcDdEeFfGgHhIiJjKkLlMmNnOoPpQqRrSsTtUuVvWwXxYyZz0123456789", graphic_screen_x_center-(PERFORMANCE_RATING_CURRENT_RUN<<2), graphic_screen_y-(PERFORMANCE_RATING_CURRENT_RUN<<1), BLACK);  
}
void performance_rating_task14() { // draw pixel
  draw_pixel(graphic_screen_x_center-(PERFORMANCE_RATING_CURRENT_RUN<<2), graphic_screen_y-(PERFORMANCE_RATING_CURRENT_RUN<<1));
}
void performance_rating_task15() { // draw straigth line
  draw_straigth_line(graphic_screen_x_center>>1, graphic_screen_y-(PERFORMANCE_RATING_CURRENT_RUN<<1), graphic_screen_x_center );
}
void performance_rating_task16() { // draw straigth column
  draw_straigth_column(graphic_screen_x-(PERFORMANCE_RATING_CURRENT_RUN<<1), graphic_screen_y_center>>1, graphic_screen_y_center);
}
void performance_rating_task17() { // draw line
  draw_line(0,(PERFORMANCE_RATING_CURRENT_RUN<<2), graphic_screen_x, graphic_screen_y-(PERFORMANCE_RATING_CURRENT_RUN<<2), BLACK);
}
void performance_rating_task18() { // draw quadratic bezier
//  draw_quadratic_bezier(int x0, int y0, int x1, int y1, int x2, int y2, dword_t color);
}
void performance_rating_task19() { // draw empty square
//  draw_empty_square(dword_t x, dword_t y, dword_t width, dword_t height, dword_t color);
}
void performance_rating_task20() { // draw full square
//  draw_full_square(dword_t x, dword_t y, dword_t width, dword_t height, dword_t color);
}
void performance_rating_task21() { // draw empty circle point
//  draw_empty_circle_point(dword_t x, dword_t y);
}
void performance_rating_task22() { // draw empty circle
//  draw_empty_circle(dword_t x, dword_t y, dword_t radius, dword_t color);
}
void performance_rating_task23() { // draw full circle line
//  draw_full_circle_line(dword_t x, dword_t y);
}
void performance_rating_task24() { // draw full circle
//  draw_full_circle(dword_t x, dword_t y, dword_t radius, dword_t color);
}
void performance_rating_task25() { // draw empty ellipse
//  draw_empty_ellipse(int x0, int y0, int x1, int y1, dword_t color);
}
void performance_rating_task26() { // draw part empty ellipse
//  draw_parts_of_empty_ellipse(byte_t parts, int x0, int y0, int x1, int y1, dword_t color);
}
void performance_rating_task27() { // draw full ellipse
//  draw_full_ellipse(int x0, int y0, int x1, int y1, dword_t color);
}
void performance_rating_task28() { //
}
void performance_rating_task29() { //
}
void performance_rating_task30() { //
}
void performance_rating_task31() { //
}
