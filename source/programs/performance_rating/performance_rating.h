//BleskOS

/*
* MIT License
* Copyright (c) 2023-2024 Vendelín Slezák
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#define PERFORMANCE_RATING_CLICK_ZONE_BACK 1

#define PERFORMANCE_RATING_RUN_COUNT 128
#define PERFORMANCE_RATING_NBTASK 16

dword_t PERFORMANCE_RATING_CURRENT_TASK=0;
dword_t PERFORMANCE_RATING_CURRENT_PAGE=0;
dword_t PERFORMANCE_RATING_CURRENT_RUN=0;

void performance_rating_task0();
void performance_rating_task1();
void performance_rating_task2();
void performance_rating_task3();
void performance_rating_task4();
void performance_rating_task5();
void performance_rating_task6();
void performance_rating_task7();
void performance_rating_task8();
void performance_rating_task9();
void performance_rating_task10();
void performance_rating_task11();
void performance_rating_task12();
void performance_rating_task13();
void performance_rating_task14();
void performance_rating_task15();

struct performance_rating_task { dword_t result; void (*run)(void); byte_t name[100]; } performance_rating_tasks[PERFORMANCE_RATING_NBTASK] = {
	{ 0, &performance_rating_task0, "Clear double buffer with solid color 0x123456" },
	{ 0, &performance_rating_task1, "Mouse move 100% visible" },
	{ 0, &performance_rating_task2, "Mouse move 50% visible right" },
	{ 0, &performance_rating_task3, "Mouse move 50% visible bottom" },
	{ 0, &performance_rating_task4, "Redraw screen from double buffer to framebuffer" },
	{ 0, &performance_rating_task5, "Redraw 25% of screen 100% visible" },
	{ 0, &performance_rating_task6, "Redraw 25% of screen 50% right visible" },
	{ 0, &performance_rating_task7, "Redraw 25% of screen 50% bottom visible" },
	{ 0, &performance_rating_task8, "Scalable font print" },
	{ 0, &performance_rating_task9, "Bitmap font print" },
	{ 0, &performance_rating_task10, "todo" },
	{ 0, &performance_rating_task11, "todo" },
	{ 0, &performance_rating_task12, "todo" },
	{ 0, &performance_rating_task13, "todo" },
	{ 0, &performance_rating_task14, "todo" },
	{ 0, &performance_rating_task15, "todo" }
};

void initalize_performance_rating(void);
void performance_rating(void);
void redraw_performance_rating(void);
void performance_rating_run_task(dword_t task_number);
