//BleskOS

/*
* MIT License
* Copyright (c) 2023-2025 BleskOS developers
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

void initalize_binary_programs_interface(void) {
 //set variables for text mode
 text_mode_screen_columns = (monitors[0].width/8);
 text_mode_screen_lines = (monitors[0].height/10);
 text_mode_screen_pixels = (text_mode_screen_lines*text_mode_screen_columns);

 text_mode_x_on_screen = 0;
 text_mode_y_on_screen = 0;

 text_mode_cursor_is_visible = STATUS_TRUE;
 text_mode_cursor_line = 0;
 text_mode_cursor_column = 0;

 text_mode_show_changes_on_screen = STATUS_TRUE;
 
 //allocate memory for text_mode
 text_mode_screen = (struct text_mode_screen_char_t *) (calloc(text_mode_screen_lines*text_mode_screen_columns*sizeof(struct text_mode_screen_char_t)));
 cursor_line_input_buffer = (word_t *) (calloc(text_mode_screen_columns*2+2));

 //set black background with white characters
 for(dword_t i=0; i<text_mode_screen_pixels; i++) {
  text_mode_screen[i].background_color = BLACK;
  text_mode_screen[i].character_color = WHITE;
 }

 //set default variables numbers
 pressed_key_value = 0;
 pressed_key_unicode_value = 0;
 binary_program_mode = PROGRAM_NO_MODE;
 extern dword_t stack_before_calling_binary_program; stack_before_calling_binary_program = 0; //no program is running
 system_calls_is_execution_in_repeating_method = STATUS_FALSE;
}

byte_t does_program_have_signature(byte_t *program_memory, dword_t program_size) {
 for(dword_t i=0; i<(program_size-sizeof(bleskos_program_signature)-1); i++) {
  if(is_memory_equal_with_memory(&program_memory[i], bleskos_program_signature, sizeof(bleskos_program_signature)-1)==STATUS_TRUE) {
   return STATUS_TRUE;
  }
 }

 return STATUS_FALSE;
}

void should_be_program_killed(void) {
 extern dword_t stack_before_calling_binary_program;
 
 if(stack_before_calling_binary_program!=0 && keyboard_keys_state.ctrl == 1 && keyboard_keys_state.alt == 1 && keyboard_code_of_pressed_key==KEY_K) {
  destroy_task(should_be_program_killed);

  extern dword_t *stack_of_interrupt;
  extern void kill_binary_program(void);
  *stack_of_interrupt = (dword_t)(&kill_binary_program); //after end of interrupt processor will jump to this method
 }
}