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
#define PERFORMANCE_RATING_NBTASK 32

dword_t PERFORMANCE_RATING_NBTASK_PER_PAGE=0;
dword_t PERFORMANCE_RATING_CURRENT_TASK=0;
dword_t PERFORMANCE_RATING_CURRENT_PAGE=0;
dword_t PERFORMANCE_RATING_CURRENT_RUN=0;

void performance_rating_task0() ; // clear screen
void performance_rating_task1() ; // on screen center
void performance_rating_task2() ; // on screen border rigth
void performance_rating_task3() ; //on screen border bottom
void performance_rating_task4() ; // full screen
void performance_rating_task5() ; // 25% screen center
void performance_rating_task6() ; // 25% screen half rigth
void performance_rating_task7() ; // 25% screen half bottom
void performance_rating_task8() ; //  Scalable font print
void performance_rating_task9() ; // bitmap font print
void performance_rating_task10() ; // print numeric variable 
void performance_rating_task11() ; // print unicode string ÀÁÂÃÄÅÈÉÊËÌÍÎÏÑÒÓÔÕÖÙÚÛÜÝßàáâãäåèéêëìíîïñòóôõöùúûüýÿčďěľňŕřšťž
void performance_rating_task12() ; // print hexa number
void performance_rating_task13() ; // print ascii
void performance_rating_task14() ; // draw pixel
void performance_rating_task15() ; // draw straigth line
void performance_rating_task16() ; // draw straigth line
void performance_rating_task17() ; // draw line
void performance_rating_task18() ; // draw quadratic bezier
void performance_rating_task19() ; // draw empty square
void performance_rating_task20() ; // draw full square
void performance_rating_task21() ; // draw empty circle point
void performance_rating_task22() ; // draw empty circle
void performance_rating_task23() ; // draw full circle line
void performance_rating_task24() ; // draw full circle
void performance_rating_task25() ; // draw empty ellipse
void performance_rating_task26() ; // draw part empty ellipse
void performance_rating_task27() ; // draw full ellipse
void performance_rating_task28() ; //
void performance_rating_task29() ; //
void performance_rating_task30() ; //
void performance_rating_task31() ; //

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
	{ 0, &performance_rating_task10, "Numeric variable print" },
	{ 0, &performance_rating_task11, "Unicode string print" },
	{ 0, &performance_rating_task12, "Hexa number print" },
   { 0, &performance_rating_task13, "Ascii string print" },
   { 0, &performance_rating_task14, "draw pixel" },
   { 0, &performance_rating_task15, "draw straigth line" },
   { 0, &performance_rating_task16, "draw straigth line" },
   { 0, &performance_rating_task17, "draw line" },
   { 0, &performance_rating_task18, "draw quadratic bezier" },
   { 0, &performance_rating_task19, "draw empty square" },
   { 0, &performance_rating_task20, "draw full square" },
   { 0, &performance_rating_task21, "draw empty circle point" },
   { 0, &performance_rating_task22, "draw empty circle" },
   { 0, &performance_rating_task23, "draw full circle line" },
   { 0, &performance_rating_task24, "draw full circle" },
   { 0, &performance_rating_task25, "draw empty ellipse" },
   { 0, &performance_rating_task26, "draw part empty ellipse" },
   { 0, &performance_rating_task27, "draw full ellipse" },
   { 0, &performance_rating_task28, "todo" },
   { 0, &performance_rating_task29, "todo" },
   { 0, &performance_rating_task30, "todo" },
   { 0, &performance_rating_task31, "todo" }
};

void initalize_performance_rating(void);
void performance_rating(void);
void redraw_performance_rating(void);
void performance_rating_run_task(dword_t task_number);
void performance_rating_inspect_task(dword_t task_number);

