;BleskOS

;;;;;
;; MIT License
;; Copyright (c) 2023-2025 Vendelín Slezák
;; Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
;; The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
;; THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
;;;;;

extern binary_program_mode

global stack_before_calling_binary_program
stack_before_calling_binary_program dd 0

; void call_binary_program(dword_t system_call_method_memory);
global call_binary_program
call_binary_program:
 pusha

 mov dword [stack_before_calling_binary_program], esp ; save stack position to be able to kill program

 mov dword [binary_program_mode], 0 ; no mode

 mov eax, [esp+32+4]
 push eax ; this is position of system_call in memory
 mov eax, 0x10000 ; program is loaded here
 call eax
 pop eax

 mov dword [stack_before_calling_binary_program], 0 ; this mean that program is not running

 popa
 ret

; void kill_binary_program(void);
global kill_binary_program
kill_binary_program:
 mov esp, dword [stack_before_calling_binary_program] ; move stack to where it was before calling binary program = equals to returning from all methods that are opened
 mov dword [stack_before_calling_binary_program], 0 ; this mean that program is not running
 popa
 ret ; this will return from where was binary program called