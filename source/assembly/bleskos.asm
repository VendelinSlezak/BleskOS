;BleskOS

;;;;;
;; MIT License
;; Copyright (c) 2023-2024 Vendelín Slezák
;; Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
;; The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
;; THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
;;;;;

bits 32

global start
global call
global call_bleskos_binary_program
extern bleskos

start:
 call bleskos

 halt:
  hlt
 jmp halt

call:
 push eax
 mov eax, dword [esp+8]
 call eax
 pop eax
 ret

%include "source/assembly/idt.asm"
%include "source/assembly/binary_font.asm"
%include "source/assembly/scalable_font.asm"
%include "source/assembly/strings.asm"
%include "source/assembly/binary_programs.asm"