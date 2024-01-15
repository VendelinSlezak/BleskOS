//BleskOS

/*
* MIT License
* Copyright (c) 2023-2024 Vendelín Slezák
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

		switch (keyboard_value ) {
			case KEY_UP:
				if (PERFORMANCE_RATING_CURRENT_TASK > 0) { PERFORMANCE_RATING_CURRENT_TASK--; };
				redraw_performance_rating();
				break;
			case KEY_DOWN:
				if (PERFORMANCE_RATING_CURRENT_TASK < PERFORMANCE_RATING_NBTASK-1) { PERFORMANCE_RATING_CURRENT_TASK++; };
				redraw_performance_rating();
				break;
			case KEY_ENTER:
				performance_rating_tasks[PERFORMANCE_RATING_CURRENT_TASK].result=performance_rating_tasks[PERFORMANCE_RATING_CURRENT_TASK].run();
				redraw_performance_rating();
				break;
			case KEY_A:
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

	for (int i=0;i<PERFORMANCE_RATING_NBTASK;i++) {
		if ( i == PERFORMANCE_RATING_CURRENT_TASK ) { color=RED; } else { color=BLACK; };
		print(performance_rating_tasks[i].name, 20, 10+i*10, color);
		print_var(performance_rating_tasks[i].result, graphic_screen_x_center, 10+i*10, color);
	}
	print("ESC to quit | ARROW to choose | ENTER to run | All task is run 100x", 0, graphic_screen_y-8, BLACK);
	redraw_screen();
}

dword_t performance_rating_task0() {
	dword_t start,stop;

	start=ticks;
	for (int i=0;i<PERFORMANCE_RATING_RUN_COUNT;i++) {
		clear_screen(0x123456);
	}
	stop=ticks;

	if ( stop > start  ) {
		return stop - start;
	} else {
		return 1<<32 - start + stop;
	}
}
dword_t performance_rating_task1() {
	dword_t start,stop;

	mouse_movement_x = 0 - mouse_cursor_x ;
	mouse_movement_y = graphic_screen_y_center - mouse_cursor_y ;

	start=ticks;
	for (int i=0;i<PERFORMANCE_RATING_RUN_COUNT;i++) {
		move_mouse_cursor();
		mouse_movement_x = 6;
	}
	stop=ticks;

	if ( stop > start  ) {
		return stop - start;
	} else {
		return 1<<32 - start + stop;
	}
}
dword_t performance_rating_task2() {
	return 0;
}
dword_t performance_rating_task3() {
	return 0;
}
dword_t performance_rating_task4() {
	dword_t start,stop;

	start=ticks;
	for (int i=0;i<PERFORMANCE_RATING_RUN_COUNT;i++) {
		redraw_screen();
	}
	stop=ticks;

	if ( stop > start  ) {
		return stop - start;
	} else {
		return 1<<32 - start + stop;
	}
	return 0;
}
dword_t performance_rating_task5() {
	return 0;
}
dword_t performance_rating_task6() {
	return 0;
}
dword_t performance_rating_task7() {
	return 0;
}
