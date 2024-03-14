//BleskOS

/*
* MIT License
* Copyright (c) 2023-2024 Vendelín Slezák
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

void initalize_scheduler(void) {
 actual_number_of_tasks = 0;
 is_task_table_changing = STATUS_FALSE;
}

void create_task(byte_t *name, void (*task)(), byte_t type, dword_t number_of_ms_to_be_executed) {
 if(actual_number_of_tasks>=SCHEDULER_MAX_NUMBER_OF_TASKS) {
  return;
 }

 is_task_table_changing = STATUS_TRUE;

 scheduler_tasks[actual_number_of_tasks].task = task;
 scheduler_tasks[actual_number_of_tasks].name = name;
 scheduler_tasks[actual_number_of_tasks].type = type;
 scheduler_tasks[actual_number_of_tasks].counter_of_ms = 0;
 scheduler_tasks[actual_number_of_tasks].number_of_ms_to_be_executed = number_of_ms_to_be_executed;

 actual_number_of_tasks++;

 is_task_table_changing = STATUS_FALSE;
}

void destroy_task(void (*task)()) {
 for(dword_t i=0; i<actual_number_of_tasks; i++) {
  if(scheduler_tasks[i].task==task) {
   is_task_table_changing = STATUS_TRUE;
   remove_space_from_memory_area((dword_t)(&scheduler_tasks), sizeof(struct scheduler_task_info)*SCHEDULER_MAX_NUMBER_OF_TASKS,( (dword_t)(&scheduler_tasks))+(sizeof(struct scheduler_task_info)*i), sizeof(struct scheduler_task_info));
   actual_number_of_tasks--;
   is_task_table_changing = STATUS_FALSE;
   return;
  }
 }
}

void scheduler_periodic_interrupt(void) {
 //this method is called from inside of IRQ handler, so in these methods you can not use wait() because it will lock computer
 //now it is connected with IRQ0 and is called every 2 ms

 if(is_task_table_changing==STATUS_TRUE) {
  return;
 }

 for(dword_t i=0; i<actual_number_of_tasks; i++) {
  scheduler_tasks[i].counter_of_ms+=2;

  if(scheduler_tasks[i].type==TASK_TYPE_PERIODIC_INTERRUPT && scheduler_tasks[i].counter_of_ms>=scheduler_tasks[i].number_of_ms_to_be_executed) {
   scheduler_tasks[i].counter_of_ms = 0;
   scheduler_tasks[i].task();
  }
 }
}

void scheduler_user_input(void) {
 //this method is called from inside of wait_for_user_input() after asm("hlt") wait

 if(is_task_table_changing==STATUS_TRUE) {
  return;
 }

 for(dword_t i=0; i<actual_number_of_tasks; i++) {
  if(scheduler_tasks[i].type==TASK_TYPE_USER_INPUT && scheduler_tasks[i].counter_of_ms>=scheduler_tasks[i].number_of_ms_to_be_executed) {
   scheduler_tasks[i].counter_of_ms = 0;
   scheduler_tasks[i].task();
  }
 }
}