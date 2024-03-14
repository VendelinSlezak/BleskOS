//BleskOS

/*
* MIT License
* Copyright (c) 2023-2024 Vendelín Slezák
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#define TASK_TYPE_PERIODIC_INTERRUPT 1
#define TASK_TYPE_USER_INPUT 2

#define SCHEDULER_MAX_NUMBER_OF_TASKS 20
struct scheduler_task_info {
 void (*task)(void);
 byte_t *name;
 byte_t type;
 dword_t counter_of_ms;
 dword_t number_of_ms_to_be_executed;
}__attribute__((packed));
struct scheduler_task_info scheduler_tasks[SCHEDULER_MAX_NUMBER_OF_TASKS];

byte_t actual_number_of_tasks, is_task_table_changing;

void initalize_scheduler(void);
void create_task(byte_t *name, void (*task)(), byte_t type, dword_t number_of_ms_to_be_executed);
void destroy_task(void (*task)());
void scheduler_periodic_interrupt(void);
void scheduler_user_input(void);